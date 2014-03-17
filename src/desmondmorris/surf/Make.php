<?php

namespace desmondmorris\surf;

class Make {

  private $site;
  private $make;

  public function __construct(Site $site) {
    $this->setSite($site);
    $make = $this->_generate();
    $this->setMake($make);
  }

  public function setSite($site) {
    $this->site = $site;
  }

  public function getSite() {
    return $this->site;
  }

  public function setMake($make) {
    $this->make = $make;
  }

  public function getMake() {
    return $this->make;
  }

  private function _generate() {

    $config = $this->site->getConfig();

    $make = "api = 2\n";
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

    return $make;
  }

  public function make() {

    $config = $this->site->getConfig();
    $docroot = $config['docroot'];
    $manifest = sys_get_temp_dir() . "/site.make";
    $make = $this->getMake();

    file_put_contents($manifest, $make);

    if (file_exists($docroot)) {
      exec("sudo rm -rf $docroot");
    }

    drush_invoke('make', array($manifest, $docroot));
    unlink($manifest);
  }

  public function __toString() {
    return $this->getMake();
  }

}