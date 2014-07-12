<?php

namespace desmondmorris\surf;

class Make {

  private $site;
  private $raw;

  public function __construct(Site $site, $options) {
    $this->setSite($site);
    $make = $this->_generate($options);
    $this->setMake($make);
  }

  public function setSite($site) {
    $this->site = $site;
  }

  public function getSite() {
    return $this->site;
  }

  public function setMake($make) {
    $this->raw = $make;
  }

  public function getMake() {
    return $this->raw;
  }

  public function build() {

    $config = $this->site->getConfig();

    $cwd = getcwd();

    $tmp_dir = $cwd . "/.build";

    exec("rm -rf $tmp_dir");
    mkdir($tmp_dir);

    //Create the drupal-org make file and build it
    $drupal_org_make = $tmp_dir . '/drupal-org.make';

    file_put_contents($drupal_org_make, $this->getMake());
    shell_exec(implode(
      " ",
      array(
        "drush",
        "make",
        "--no-core",
        "--contrib-destination=\".\"",
        $drupal_org_make,
        $tmp_dir . "/projects"
      )
    ));

    $drupal_org_core_make = $tmp_dir . '/drupal-org-core.make';
    # Build a drupal-org-core.make file if it doesn't exist.

    $drupal_org_core_make_raw = "api = 2
    core = " . $config->core .  "
    ";


    if (isset($config->projects->core)) {
      if (count($config->projects->core) == 1) {
        foreach($config->projects->core as $core_project_name => $core_project_version) {

          if (!is_array($core_project_version)) {
            $drupal_org_core_make_raw .= "projects[" . $core_project_name . "] = " . $core_project_version;
          }
          else {

            // _drush_make_generate_makefile_body(array($core_project_name => $core_project_version));
            // //$str = $this->_arrayToInfo(array($core_project_name => $core_project_version));
            //
            // //print_r($str);
            // exit;
          }
        }
      }
      else {


      }
    }

    file_put_contents($drupal_org_core_make, $drupal_org_core_make_raw);

    shell_exec(implode(
      " ",
      array(
        "drush",
        "make",
        $drupal_org_core_make,
        $tmp_dir . "/core"
      )
    ));

    if (isset($config->type) && $config->type == 'profile') {
      $profile_dir = $tmp_dir . "/core/profiles/" . $config->name;
      mkdir($profile_dir);
      shell_exec("cp -rf $tmp_dir/projects/* $profile_dir");
      shell_exec("find $cwd -mindepth 1 -maxdepth 1 -name '.build' -or -exec cp -r {} $profile_dir \;");
    }
    else {
      # Copy the projects into the sites dir
      shell_exec("cp -rf $tmp_dir/projects/* $tmp_dir/core/sites/all");
    }

    $docroot = $config->docroot;
    
    if (file_exists($docroot)) {
      exec("rm -rf $docroot");
    }
    shell_exec("cp -rf " . $tmp_dir . "/core " . $docroot);

    exec("rm -rf $tmp_dir");
  }

  private function _generate($options) {

    $config = $this->site->getConfig();

    $contrib_dir =
      isset($config->contrib_dir) && !empty($config->contrib_dir)
      ? $config->contrib_dir
      : "contrib";

    $make = "api = 2\n";
    $make .= "core = {$config->core}\n";
    $make .= "projects[] = \"drupal\"\n\n";

    $types = array('projects');

    if(isset($options['dev'])) {
      $types[] = 'devProjects';
    }

    foreach($types as $project_type) {

      if (isset($config->$project_type) && !empty($config->$project_type)) {

        foreach ($config->$project_type as $type => $projects) {

          if ($type == 'core') {
            continue;
          }

          if ($type == 'distribution') {
            $type = 'profile';
          }

          $make .= "; " . strtoupper($type . 's') . "\n\n";

          foreach($projects as $name => $project) {
            $make .= "; $name\n";
            $make .= "projects[$name][type] = $type\n";
            $make .= "projects[$name][version] = $project\n";

            if ($type === 'module') {
              $make .= "projects[$name][subdir] = $contrib_dir\n";
            }

          }
        }

      }

    }

    return $make;
  }

  public function make() {

    $config = $this->site->getConfig();
    $docroot = $config->docroot;
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
