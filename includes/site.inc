<?php
/**
 * @file
 * Project specific functionality.
 */

use vierbergenlars\SemVer\version;

/**
 * Returns project configuration manifest as an array.
 *
 * @param string $project_root
 *   Optional project directory path
 *
 * @return array
 *   Configuration file as an associative array
 */
function site_get_config($project_root = PROJECT_ROOT) {
  if (!file_exists($project_root . '/' . MANIFEST_FILE)) {
    throw new Exception('Missing project manifest');
  }
  $json = file_get_contents($project_root . '/' . MANIFEST_FILE);
  return json_decode($json, TRUE);
}

/**
 * Saves project manifest from configration array.
 *
 * @param array $config
 *   Configuration array to be saved as manifest
 * @param string $project_root
 *   Optional project directory path
 */
function site_save_config($config, $project_root = PROJECT_ROOT) {
  $json = jsonpp($config);
  file_put_contents($project_root . '/' . MANIFEST_FILE, $json);
}

/**
 * Returns current project version from manifest.
 *
 * @param string $project_root
 *   Optional project directory path
 */
function site_get_version($project_root = PROJECT_ROOT) {
  $config = site_get_config($project_root);
  if (!isset($config['version'])) {
    throw new Exception('No version found in config');
  }
  return $config['version'];
}

/**
 * Increment version number in manifest.
 *
 * @param string $type
 *   Version increment type.  Valid types inclue patch, minor and major
 * @param string $project_root
 *   Optional project directory path.
 */
function site_bump_version($type = 'patch', $project_root = PROJECT_ROOT) {
  $config = site_get_config($project_root);

  $whitelist = array(
    'patch',
    'minor',
    'major',
  );

  if (!in_array($type, $whitelist)) {
    throw new Exception('Invalid version bump type');
  }

  $version = new version($config['version']);
  $config['version'] = (string) $version->inc($type);

  site_save_config($config, $project_root);
}

/**
 * Returns a make file from project configuration manifest.
 *
 * @param bool $core_only
 *   Generate a make file with only core project set
 * @param array $config
 *   Optional configuration array
 *
 * @return string
 *   Valid drush makefile with project dependencies from the manifest
 */
function site_generate_make($core_only = FALSE, $config = NULL) {
  $config = $config ? $config : site_get_config();

  $projects = array();
  if (isset($config['projects'])) {
    foreach ($config['projects'] as $type => $sub_projects) {

      if (($core_only === TRUE) && ($type != 'core')) {
        continue;
      }

      if (is_array($sub_projects)) {
        foreach ($sub_projects as $sub_project_name => $sub_project) {

          if (!is_array($sub_project)) {
            $projects[$sub_project_name] = array(
              'type' => $type,
              '_type' => $type,
              'version' => $sub_project,
            );
          }
          else {
            $sub_project['type'] = $type;
            $sub_project['_type'] = $type;

            $projects[$sub_project_name] = $sub_project;
          }
        }
      }
    }
  }

  $core = isset($config['core']) ? $config['core'] : DEFAULT_CORE_VERSION;

  $make = make_generate_makefile_contents($projects, $core);
  return $make;

}
