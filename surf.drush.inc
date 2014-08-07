<?php

/**
 * @file
 * Surf drupal project manager
 */

require 'vendor/autoload.php';

use desmondmorris\surf\Site;
use desmondmorris\surf\Make;

define('SURF_PATH', dirname(__FILE__));
define('SURF_SRC_PATH', SURF_PATH . '/src/desmondmorris/surf');
define('VENDOR_PATH', SURF_PATH . '/vendor');

/**
 * Implements hook_drush_command().
 */
function surf_drush_command() {
  $items = array();

  $items['surf'] = array(
    'description' => 'Surf README',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'aliases' => array('s'),
    'callback' => 'drush_print_file',
    'callback arguments' => array(dirname(__FILE__) . '/README.md'),
  );

  $items['surf-init'] = array(
    'description' => 'Create a new surf project',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-init' => 'Creates new surf project.',
    )
  );


  $items['surf-link'] = array(
    'description' => 'Link custom projects and directories',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'command hook' => '_drush_surf_copy',
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-link' => 'Link custom projects and directories',
    )
  );

  $items['surf-commands'] = array(
    'description' => 'Installs drush commands.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-commands' => 'Installs drush commands.',
    )
  );

  $items['surf-pm-list'] = array(
    'description' => 'List projects.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-pm-list' => 'Lists all projects.',
      'drush surf-pm-list --no-dev' => 'Exclude dev projects.',
    ),
    'aliases' => array('spml'),
    'options' => array(
      'no-dev' => array(
        'descriptions' => 'Lists all projects except dev.'
      )
    ),
  );

  $items['surf-build'] = array(
    'description' => 'Build site from project config.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-desmond' => 'Build site from project config.',
    ),
    'options' => array(
      'dev' => 'Include dev projects'
    ),
  );

  $items['surf-pm-add'] = array(
    'description' => 'Add project dependencies.',
    'arguments' => array(
      'name' => 'Project name',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-pm-add views' => 'Add views.',
      'drush surf-pm-add views@1.0' => 'Add views 1.0.',
    ),
    'aliases' => array('spma'),
    'options' => array(
      'dev' => 'Development only projects'
    ),
  );

  $items['surf-project-version'] = array(
    'description' => 'Get current site version or bump current version.',
    'command hook' => '_drush_surf_get_version',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-project-version' => 'Get current site version.',
      'drush surf-project-version --bump=minor' => 'Bumps the minor version of the project',
    ),
    'aliases' => array('surf-version'),
    'options' => array(
      'bump' => array(
        'description' => 'Optional: This will bump the version number',
        'example-value' => 'patch|minor|major|build',
      ),
    ),
  );

  $items['surf-sniff'] = array(
    'description' => 'Run code sniffer against our custom code.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-sniff' => 'Run code sniffer against our custom code.',
    )
  );

  return $items;
}

/**
 *
 */
function drush_surf_pm_list() {
  require_once(SURF_SRC_PATH . '/commands/pm.inc');
  _surf_pm_list();
}


/**
 *
 */
function drush_surf_pm_add() {
  require_once(SURF_SRC_PATH . '/commands/pm.inc');
  $names = func_get_args();
  _surf_pm_add($names);
}

function drush_surf_build() {

  if (drush_get_option('dev')) {
    $options['dev'] = TRUE;
  }

  $make = new Make(new Site(), $options);
  $make->build();

}

/**
 *
 */
function drush_surf_init() {

  require_once(SURF_SRC_PATH . '/commands/project.inc');

  $config = array();

  $config['name'] = drush_prompt(
    'Machine-name',
    basename(getcwd())
  );

  $config['title'] = drush_prompt(
    'Title',
    'My Drupal Project'
  );

  $config['description'] = drush_prompt(
    'Description',
    'My cool new Drupal project'
  );

  $config['type'] = drush_prompt(
    'Project type (website, profile)',
    'website'
  );

  $config['version'] = drush_prompt(
    'Project version',
    '0.0.1'
  );

  $config['core'] = drush_prompt(
    'Drupal Core version',
    '7.x'
  );

  $config['docroot'] = 'drupal';

  drush_print(json_encode($config, JSON_PRETTY_PRINT));

  $confirm = drush_confirm("Is this correct?");

  if (!$confirm) {
    return;
  }

  _surf_init($config);

  if (!file_exists('.git')) {
    $git = drush_confirm("Initialize git?");

    if ($git) {
      exec('git init');

      // $githooks = drush_confirm("Add githooks?");
      //
      // if ($githooks) {
      //   exec("cp " . SURF_SRC_PATH . "/scripts/pre-commit .git/hooks");
      // }

    }
  }

  // Install vendor drush commands
  drush_surf_commands();

  drush_print('Success!  Happy Drupalling');

}

/**
 *
 */
function drush_surf_project_version() {

  $site = new Site();

  $bump = drush_get_option('bump');

  if ($bump) {
    if ($bump === TRUE) {
      $bump = 'patch';
    }
    $site->bumpVersion($bump);
  }
  drush_print($site->getVersion());

}

/**
 *
 */
function drush_surf_sniff() {
  $site = new Site();
  $config = $site->getConfig();

  if (empty($config['directories'])) {
    drush_print('You have not set up any directories in your config file');
    return;
  }

  $directories = implode(" ", (array) $config['directories']);

  $command = array(
    VENDOR_PATH . '/bin/phpcs',
    '--standard=' . VENDOR_PATH . '/drupal/coder/coder_sniffer/Drupal',
    $directories,
    '--ignore=*.js,*.css'
  );

  $command = implode(' ', $command);
  exec($command, $output);

  if (!empty($output)) {
    $results = implode("\n", $output);
    drush_print($results);
    return $results;
  }

}

function drush_surf_commands() {
  $site = new Site();
  $config = $site->getConfig();
  $cwd = getcwd();

  if (!isset($config['commands'])) {
    return;
  }

  foreach ($config['commands'] as $command => $url) {

    $vendor_path = "vendor/drush-commands/" . $command;
    rrmdir($vendor_path);
    exec("git clone " . $url . " " . $vendor_path);

    if (file_exists("./" . $vendor_path . "/composer.json")) {
      exec("cd " . $vendor_path . " && composer update --no-dev && cd " . $cwd);
    }

  }

}

function drush_surf_link($copy = FALSE) {
  $site = new Site();
  $config = $site->getConfig();
  $cwd = getcwd();

  $docroot = isset($config['docroot']) ? $config['docroot'] : 'docroot';

  if (!file_exists($cwd . "/" . $docroot)) {
    return drush_set_error(
      'DRUSH_SURF_PROJECT_ERROR',
      'Docroot not found.'
    );
  }

  $directories = isset($config['directories']) ? $config['directories'] : array();
  $linked = isset($config['linked']) ? $config['linked'] : array();



  foreach($linked as $src => $dest) {

    if (isset($directories[$src])) {
      $src_path = $cwd . "/" . $directories[$src];
      $dest_path = $cwd . "/" . $docroot . "/" . $dest;

      if (file_exists($dest_path)) {
        if (is_dir($dest_path) && !is_link($dest_path)) {
          rrmdir($dest_path);
        }
        else {
          unlink($dest_path);
        }
      }

      if ($copy === TRUE) {
        drush_print("Copying " . $src);
        copy($src_path, $dest_path);
      }
      else {
        drush_print("Linking " . $src);
        symlink($src_path, $dest_path);
      }
    }

  }

}

function rrmdir($dir) {
   if (is_dir($dir)) {
     $objects = scandir($dir);
     foreach ($objects as $object) {
       if ($object != "." && $object != "..") {
         if (filetype($dir."/".$object) == "dir") rrmdir($dir."/".$object); else unlink($dir."/".$object);
       }
     }
     reset($objects);
     rmdir($dir);
   }
 }
