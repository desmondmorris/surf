<?php

namespace desmondmorris\surf;

use Symfony\Component\Yaml\Yaml;
use vierbergenlars\SemVer\version;

class Site {

  const MANIFEST_FILE = 'site.json';

  private $config;

  public function __construct($config = null) {
    if (!$config) {
      $this->setConfig(self::loadConfig());
    }
    else {
      $this->setConfig($config);
    }
  }

  public static function loadConfig($manifest = self::MANIFEST_FILE) {
    $json = file_get_contents($manifest);
    return json_decode($json);
  }

  public static function saveConfig($config, $manifest = self::MANIFEST_FILE) {
    $json = json_encode($config, JSON_PRETTY_PRINT);
    file_put_contents($manifest, $json);

    return $config;
  }

  public function setConfig($config) {
    $this->config = $config;
  }

  public function getConfig() {
    return $this->config;
  }

  public function bumpVersion($type = 'patch') {

    $config = $this->getConfig();

    $whitelist = array(
      'patch',
      'minor',
      'major',
      'build'
    );

    if (!in_array($type, $whitelist)){
      throw new Exception('Invalid version bump type');
    }

    $version = new version($config->version);
    $config->version = (string) $version->inc($type);

    $saved = self::saveConfig($config);

    $this->setConfig($saved);

  }

  public function getVersion() {
    if (!isset($this->config->version)) {
      throw new Exception('No version found in config');
    }
    return $this->config->version;
  }
}
