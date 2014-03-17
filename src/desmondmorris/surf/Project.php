<?php

namespace desmondmorris\surf;

class Project {

  const PROJECT_INFO_URL = 'http://updates.drupal.org/release-history';

  public static function getInfo($name, $version) {

    $url = self::PROJECT_INFO_URL . "/{$name}/{$version}";

    $xml = file_get_contents($url);

    $object = simplexml_load_string($xml);

    switch($object[0]) {
      case "No release history was found for the requested project ($name).":
        throw new Exception('Project not found.', 300);
        break;
      case "No release history available for $name $version.":
        throw new Exception('Compatible version not found.', 301);
        break;
      case "You must specify an API compatibility version as the final argument to the path.":
        throw new Exception('Missing core version.', 302);
        break;
    }

    return $object[0];

  }

}
