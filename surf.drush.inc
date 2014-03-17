<?php

/**
 * @file
 * Surf drupal project manager
 */

require 'vendor/autoload.php';

use desmondmorris\surf\Site;
use desmondmorris\surf\Make;
use desmondmorris\surf\Project;

/**
 * Implements hook_drush_command().
 */
function surf_drush_command() {
  $items = array();

  $items['surf-create'] = array(
    'description' => 'Create new project manifest.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-create' => 'Creates new project manifest.',
    ),
    'options' => array(
      'scaffold' => array(
        'description' => 'Adds a basic project directory structure and files.'
      ),
    ),
  );

  $items['surf-build'] = array(
    'description' => 'Build site from project config.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-build' => 'Build site from project config.',
    ),
  );

  $items['surf-add'] = array(
    'description' => 'Add project dependencies.',
    'arguments' => array(
      'name' => 'Project name',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-add' => 'Add project dependencies.',
    ),
  );

  $items['surf-version'] = array(
    'description' => 'Get current site version or bump current version.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-version' => 'Get current site version.',
      'drush surf-version --bump=minor' => 'Bumps the minor version of the project'
    ),
    'options' => array(
      'bump' => array(
        'description' => 'Optional: This will bump the version number',
        'example-value' => 'patch|minor|major|build',
      ),
    ),
  );

  return $items;
}


/**
 *
 */
function drush_surf_create() {

  $config = array();

  $config['name'] = drush_prompt(
    'Name',
    basename(getcwd())
  );

  $config['description'] = drush_prompt(
    'Description',
    'My cool new Drupal project'
  );

  $config['version'] = drush_prompt(
    'Project version',
    '0.0.1'
  );

  $config['core'] = drush_prompt(
    'Drupal Core version',
    '7.x'
  );

  $scaffold = drush_get_option('scaffold');

  if ($scaffold) {

    $directories = array('config', 'lib/modules', 'lib/themes', 'lib/libraries');
    $files = array(
      '.gitignore' => 'docroot/',
      'config/README.md' => 'Keep your configuration files here.',
      'config/settings.php' => '<?php\n',
      'lib/README.md' => 'This is where your custom code lives.',
      'lib/modules/README.md' => 'A home for your custom modules',
      'lib/themes/README.md' => 'A home for your custom themes',
      'lib/libraries/README.md' => 'A home for your custom libraries',
    );

    exec('mkdir -p ' . implode(' ', $directories));

    foreach($files as $filename => $contents) {
      exec("echo \"$contents\" > $filename");
    }

    $config['linked'] = array(
      'lib/modules' => 'sites/all/modules/custom',
      'lib/themes' => 'sites/all/themes',
      'lib/libraries' => 'sites/all/libraries',
      'config/settings.php' => 'sites/default/settings.php'
    );

  }

  $config['docroot'] = 'docroot';

  drush_print(Site::configToYaml($config));

  $confirm = drush_confirm("Is this correct?");

  if ($confirm) {
    Site::saveConfig($config);

    drush_print('Success!  Happy Drupalling');
  }



}

/**
 *
 */
function drush_surf_version() {

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
function drush_surf_build() {

  $site = new Site();
  $config = $site->getConfig();
  $make = new Make($site);
  $make->make();

  if (isset($config['linked'])) {
    foreach($config['linked'] as $src => $dest) {

      $src = realpath($src);
      $dest = realpath($config['docroot']) . '/' . $dest;

      if(file_exists($dest)) {
        exec("sudo rm -rf $dest");
      }

      exec("ln -sfn $src $dest");
    }
  }

}

/**
 *
 */
function drush_surf_add() {

  $names = func_get_args();

  if (empty($names)) {
    return drush_set_error(
      'DRUSH_SURF_PROJECT_ERROR',
      'You must pass atleast one project.'
    );
  }

  $config = Site::loadConfig();

  if(!isset($config['projects'])){
    $config['projects'] = array();
  }

  foreach($names as $name) {

    try {
      $info = Project::getInfo($name, $config['core']);
    }
    catch (Exception $e) {
      drush_set_error(
        'DRUSH_SURF_PROJECT_ERROR',
        $e->getMessage()
      );
      continue;
    }

    $recommended = $info->releases->release[0];
    $type = str_replace('project_', '', strval($info->type) );

    if (!isset($config['projects'][$type])) {
      $config['projects'][$type] = array();
    }

    $project_version = $recommended->version_major . '.' . $recommended->version_patch;

    if (isset($recommended->version_extra)) {
      $project_version .= "-" . $recommended->version_extra;
    }

    $config['projects'][$type][$name] = $project_version;

    ksort($config['projects'][$type]);
  }
  ksort($config['projects']);
  Site::saveConfig($config);

}
