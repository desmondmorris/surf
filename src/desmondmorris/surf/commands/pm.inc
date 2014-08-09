<?php

/**
 * @file
 * Surf drupal project manager
 */

use desmondmorris\surf\Site;

function _surf_pm_add($names, $test = FALSE) {

  $config = Site::loadConfig();


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
  Site::saveConfig($config);
}

function _surf_pm_list() {

  $config = Site::loadConfig();

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
