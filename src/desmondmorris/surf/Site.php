<?php

namespace desmondmorris\surf;

use Exception;

use vierbergenlars\SemVer\version;

class Site {

  const MANIFEST_FILE = 'surf.json';

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
    if (!file_exists($manifest)) {
      throw new Exception('Missing project manifest');
    }

    $json = file_get_contents($manifest);
    return json_decode($json, TRUE);
  }

  public static function saveConfig($config, $manifest = self::MANIFEST_FILE) {
    $json = json_encode($config, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
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
      'major'
    );

    if (!in_array($type, $whitelist)){
      throw new Exception('Invalid version bump type');
    }

    $version = new version($config['version']);
    $config['version'] = (string) $version->inc($type);

    $saved = self::saveConfig($config);

    $this->setConfig($saved);
  }

  public function getVersion() {
    if (!isset($this->config['version'])) {
      throw new Exception('No version found in config');
    }
    return $this->config['version'];
  }
}
