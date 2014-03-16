<?php

/**
 * @file
 * Surf drupal project manager
 */

require 'vendor/autoload.php';

use desmondmorris\surf\Site;

// Project
define('PROJECT_INFO_URL', 'http://updates.drupal.org/release-history');

// Site - override in config?
define('BUILD_MANIFEST', sys_get_temp_dir() . "/site.make");

// Site - override in config?
define('DOCROOT_NAME', 'docroot');

/**
 * Implements hook_drush_command().
 */
function surf_drush_command() {
  $items = array();

  $items['surf-init'] = array(
    'description' => 'Create new project manifest.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-init' => 'Creates new project manifest.',
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
function drush_surf_init() {

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

  $yml = Site::configToYaml($config);

  drush_print($yml);

  $confirm = drush_confirm("Is this correct?");

  if ($confirm) {
    Site::saveConfig($config);
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

  $config = Site::loadConfig();

  $make = "";

  $make .= "api = 2\n";
  $make .= "core = {$config['core']}\n";
  $make .= "projects[] = \"drupal\"\n\n";


  if (isset($config['projects']) && is_array($config['projects'])) {

    foreach ($config['projects'] as $type => $projects) {
      if ($type == 'distribution') {
        $type = 'profile';
      }

      $make .= "; " . strtoupper($type . 's') . "\n\n";

      foreach($projects as $name => $project) {
        $make .= "; $name\n";
        $make .= "projects[$name][type] = $type\n";
        $make .= "projects[$name][version] = $project\n\n";
      }
    }

  }

  file_put_contents(BUILD_MANIFEST, $make);

  if (file_exists(DOCROOT_NAME)) {
    exec("sudo rm -rf " . DOCROOT_NAME);
  }

  drush_invoke('make', array(BUILD_MANIFEST, DOCROOT_NAME));
  unlink(BUILD_MANIFEST);

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
    $info = _surf_get_project_info($name, $config['core']);

    if ($info === FALSE) {
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

function _surf_get_project_info($name, $version) {

  $url = PROJECT_INFO_URL . "/{$name}/{$version}";

  $xml = file_get_contents($url);

  $object = simplexml_load_string($xml);

  switch($object[0]) {
    case "No release history was found for the requested project ($name).":
      drush_set_error(
        'DRUSH_SURF_PROJECT_ERROR',
        'Project not found.'
      );
      return FALSE;
      break;
    case "No release history available for $name $version.":
      drush_set_error(
        'DRUSH_SURF_PROJECT_ERROR',
        'Compatible version not found.'
      );

      return FALSE;
      break;
    case "You must specify an API compatibility version as the final argument to the path.":
      drush_set_error(
        'DRUSH_SURF_PROJECT_ERROR',
        'Please provide a core version in your project.json file.'
      );

      return FALSE;
      break;
  }

  return $object[0];

}
