<?php

/**
 * @file
 * Surf drupal project manager
 */

require_once 'vendor/autoload.php';

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
    ),
    'options' => array(
      'name' => array(
        'description' => 'Project name'
      ),
      'title' => array(
        'description' => 'Project title'
      ),
      'description' => array(
        'description' => 'Project description'
      ),
      'type' => array(
        'description' => 'Project type'
      ),
      'version' => array(
        'description' => 'Project version'
      ),
      'core' => array(
        'description' => 'Project core version'
      ),
    ),
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

  $items['surf-install-commands'] = array(
    'description' => 'Installs drush commands.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'aliases' => array('sic'),
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
  _surf_pm_list();
}

function _surf_pm_list($project_root = PROJECT_ROOT) {
  $config = site_get_config($project_root);

  $no_dev = drush_get_option('no-dev');

  $project_types = array('projects', 'devProjects');

  foreach($project_types as $project_type) {

    if (($project_type == 'devProjects') && ($no_dev)) {
      break;
    }

    if (isset($config[$project_type])) {
      foreach($config[$project_type] as $type => $projects) {

        $group = $type . 's';

        if ($project_type == 'devProjects') {
          $group = 'Development ' . $group;
        }

        drush_print(ucwords($group) . ':');
        foreach($projects as $name => $version) {
          drush_print($name . '-' . $version);
        }

      }
    }

  }
}


/**
 *
 */
function drush_surf_pm_add() {
  $names = func_get_args();
  _surf_pm_add($names);
}

function _surf_pm_add($names, $project_root = PROJECT_ROOT, $test = FALSE) {

  $config = site_get_config($project_root);

  $project_type = 'projects';
  if(!isset($config['projects'])){
    $config['projects'] = array();
  }

  if ($test === FALSE) {
    drush_include_engine('release_info', 'updatexml');

    if (drush_get_option('dev')) {
      $project_type = 'devProjects';
      if(!isset($config['devProjects'])){
        $config['devProjects'] = array();
      }
    }
  }

  if (empty($names)) {
    throw new Exception('You must add atleast one project.');
  }

  foreach($names as $name) {

    preg_match("/([a-z_-]*)(@(.*))?$/", $name, $matches);

    $project_version = isset($matches[3]) ? $matches[3] : null;
    $name = $matches[1];

    $project = array(
      'name' => $name,
      'drupal_version' => $config['core']
    );

    if ($test === FALSE) {
      if (!$xml = updatexml_get_release_history_xml($project)) {
        continue;
      }
      $info = updatexml_get_releases_from_xml($xml, $name);
      $type = updatexml_determine_project_type($xml);
      $project_version = $project_version ? $project_version : $info['recommended'];
      drush_print('Adding '  . $info['title'] . ' (' . $name . '-' . $project_version . ')');
    }
    else {
      $info['title'] = ucwords($name);
      $type = 'module';
      $project_version = $project_version ? $project_version : '1.0';
    }


    preg_match("/^([0-9]\.x\-)?([0-9a-zA-Z\-\.]*)/", $project_version, $matches);

    $config[$project_type][$type][$name] = $matches[2];

    ksort($config[$project_type][$type]);
  }
  ksort($config[$project_type]);

  site_save_config($config, $project_root);
}

function drush_surf_build() {

  $options = array();

  if (drush_get_option('dev')) {
    $options['dev'] = TRUE;
  }

  _surf_build($options);

}

function _surf_build($options = array(), $project_root = PROJECT_ROOT) {
  $config = site_get_config($project_root);

  if (isset($config['docroot'])  && file_exists($project_root . '/' . $config['docroot'])) {
    if( !drush_confirm($config['docroot'] . " exists.  Overwrite?") ) {
      return;
    }else {
      rrmdir($config['docroot']);
    }
  }

  $make = site_generate_make(FALSE, $config);
  $file = sys_get_temp_dir() . '/' . $config['name'] . '-' . uniqid(time()) . '.make';
  file_put_contents($file, $make);

  drush_invoke(
    'make',
    array(
      $file,
      $config['docroot']
    )
  );
}

/**
 *
 */
function drush_surf_init() {

  $config = array();


  $config['name'] = drush_get_option('name') ? drush_get_option('name') : drush_prompt(
    'Machine-name',
    basename(getcwd())
  );

  $config['title'] = drush_get_option('title') ? drush_get_option('title') : drush_prompt(
    'Title',
    'My Drupal Project'
  );

  $config['description'] = drush_get_option('description') ? drush_get_option('description') : drush_prompt(
    'Description',
    'My cool new Drupal project'
  );

  $config['type'] = drush_get_option('type') ? drush_get_option('type') : drush_prompt(
    'Project type (website, profile)',
    'website'
  );

  $config['version'] = drush_get_option('version') ? drush_get_option('version') : drush_prompt(
    'Project version',
    '0.0.1'
  );

  $config['core'] = drush_get_option('core') ? drush_get_option('core') : drush_prompt(
    'Drupal Core version',
    DEFAULT_CORE_VERSION
  );

  $config['docroot'] = DEFAULT_DOCROOT;

  drush_print(jsonpp($config));

  $confirm = drush_confirm("Is this correct?");

  if (!$confirm) {
    return;
  }

  $templates_path = '/src/templates';
  $templates_dir = dirname(__FILE__) . $templates_path;

  $objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($templates_dir),
  RecursiveIteratorIterator::SELF_FIRST);

  foreach($objects as $name => $object){

      $fileName = $object->getFileName();
      $pathName = $object->getPathName();
      $relative_path = str_replace($templates_dir, '', $pathName);

      if(in_array($fileName, array('.', '..'))) {
        continue;
      }

      if (is_dir($pathName)) {
        exec('mkdir -p .' . $relative_path);
      }
      else {

        preg_match_all('/PROJECT([A-Z]*)/', $fileName, $matches, PREG_SET_ORDER);

        if (!empty($matches)) {

          foreach ($matches as $match){
            if (isset($config[strtolower($match[1])])) {
              $fileName = str_replace($matches[0], $vars[strtolower($match[1])], $fileName);
              $relative_path = str_replace($matches[0], $vars[strtolower($match[1])], $relative_path);
            }
          }
        }

        if (substr($fileName, -8) == '.tpl.php') {
           extract($config);
           ob_start();
           include($pathName);
           $data = ob_get_contents();
           ob_end_clean();

           $filename = str_replace('.tpl.php', '', '.' . $relative_path);

           if (file_exists($filename)) {
             if(drush_confirm("Overwrite " . $filename . "?")) {
               file_put_contents($filename, $data);
             }
           }
           else {
             file_put_contents($filename, $data);
           }

        }
        else {
          if (file_exists('./' . $relative_path)) {
            if(drush_confirm("Overwrite ." . $relative_path . "?")) {
              exec('cp ' . $pathName . ' .' . $relative_path);
            }
          }
          else {
            exec('cp ' . $pathName . ' .' . $relative_path);
          }

        }
      }
  }

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
  drush_surf_install_commands();

  drush_print('Success!  Happy Drupalling');

}

/**
 *
 */
function drush_surf_project_version() {

  $bump = drush_get_option('bump');

  if ($bump) {
    if ($bump === TRUE) {
      $bump = 'patch';
    }
    site_bump_version($bump);
  }
  drush_print(site_get_version());

}

/**
 *
 */
function drush_surf_sniff() {
  $config = site_get_config();

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

function drush_surf_install_commands() {
  $config = site_get_config();
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

function drush_surf_link() {

  try {
    _surf_link();
  } catch (Exception $e) {

    if ($e->getMessage() == 'Docroot not found.') {
      return drush_set_error(
        'DRUSH_SURF_PROJECT_ERROR',
        'Docroot not found.'
      );
    }

  }
}

function _surf_link($copy = FALSE, $project_root = PROJECT_ROOT) {

  $config = site_get_config($project_root);
  $cwd = getcwd();

  $docroot = isset($config['docroot']) ? $config['docroot'] : DEFAULT_DOCROOT;

  if (!file_exists($cwd . "/" . $docroot)) {
    throw new Exception('Docroot not found.');
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
        recurse_copy($src_path, $dest_path);
      }
      else {
        symlink($src_path, $dest_path);
      }
    }
  }
}
