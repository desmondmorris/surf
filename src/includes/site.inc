<?php

use vierbergenlars\SemVer\version;

function site_get_config($project_root = PROJECT_ROOT) {
  if (!file_exists($project_root . '/' . MANIFEST_FILE)) {
    throw new Exception('Missing project manifest');
  }
  $json = file_get_contents($project_root . '/' . MANIFEST_FILE);
  return json_decode($json, TRUE);
}

function site_save_config($config, $project_root = PROJECT_ROOT) {
  $json = jsonpp($config);
  file_put_contents($project_root . '/' . MANIFEST_FILE, $json);
}

function site_get_version($project_root = PROJECT_ROOT) {
  $config = site_get_config($project_root);
  if (!isset($config['version'])) {
    throw new Exception('No version found in config');
  }
  return $config['version'];
}

function site_bump_version($type = 'patch', $project_root = PROJECT_ROOT) {
  $config = site_get_config($project_root);

  $whitelist = array(
    'patch',
    'minor',
    'major'
  );

  if (!in_array($type, $whitelist)){
    throw new Exception('Invalid version bump type');
  }

  $version = new version($config['version']);
  $config['version'] = (string) $version->inc($type);

  $saved = site_save_config($config, $project_root);
}

function site_generate_make($core_only = FALSE, $config = NULL) {
  $config = $config ? $config : site_get_config();

  $projects = array();
  if (isset($config['projects'])) {
    foreach($config['projects'] as $type => $sub_projects) {

      if (($core_only === TRUE) && ($type != 'core')) {
        continue;
      }

      if (is_array($sub_projects)) {
        foreach($sub_projects as $sub_project_name => $sub_project) {

          if (!is_array($sub_project)){
            $projects[$sub_project_name] = array(
              'type' => $type,
              '_type' => $type,
              'version' => $sub_project
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
