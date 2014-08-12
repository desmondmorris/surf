<?php

class SurfTest extends PHPUnit_Framework_TestCase
{

  public $tmp_dir, $test_config, $test_projects,  $test_projects_prepared;

  function setup() {

    require_once PROJECT_ROOT . '/surf.drush.inc';

    $project_name = 'surf-tests-' . uniqid(time());

    $this->test_config = array(
      'name' => $project_name,
      'title' => 'Test Project',
      'description' => 'A little test about nothing',
      'type' => 'website',
      'version' => '1.0.0',
      'core' => '7.x'
    );

    $this->test_config_string = '';

    foreach($this->test_config as $config => $value) {
      $this->test_config_string .= '--' . $config . '="' . $value . '" ';
    }

    $this->tmp_dir = sys_get_temp_dir() . '/' . $project_name;
    mkdir($this->tmp_dir);
    chdir($this->tmp_dir);
    shell_exec('drush surf-init ' . $this->test_config_string . ' -y');

    $this->test_projects = array(
      array('name' => 'views', 'version' => '3.8'),
      array('name' => 'ctools'),
      array('name' => 'admin_menu', 'version' => '3.0-rc4')
    );

    $this->test_projects_prepared = array();

    foreach($this->test_projects as $project) {
      $name = $project['name'];
      $name .= (isset($project['version']) ? '@' . $project['version'] : '');
      $this->test_projects_prepared[] = $name;
    }

  }

  function testProjectInit() {
    $this->assertTrue(file_exists($this->tmp_dir . '/surf.json'));
  }

  function testProjectConfig() {
    $config = site_get_config($this->tmp_dir);
    $this->assertTrue(is_array($config));
    foreach ($this->test_config as $key => $pair) {
      $this->assertTrue(isset($config[$key]));
      $this->assertEquals($config[$key], $pair);
    }
  }

  function testProjectSaveConfig() {
    $config = site_get_config($this->tmp_dir);

    $config['test_setting'] = '1';

    site_save_config($config, $this->tmp_dir);

    $config = site_get_config($this->tmp_dir);

    $this->assertTrue(isset($config['test_setting']));
    $this->assertEquals($config['test_setting'], 1);
  }

  public function testProjectAdd() {
    _surf_pm_add($this->test_projects_prepared, $this->tmp_dir, TRUE);

    $config = site_get_config($this->tmp_dir);

    $this->assertTrue(isset($config['projects']['module']));

    foreach($this->test_projects as $project) {
      $this->assertTrue(isset($config['projects']['module'][$project['name']]));

      if (isset($project['version'])) {
        $this->assertEquals($config['projects']['module'][$project['name']], $project['version']);
      }
    }
  }

  /**
   * @expectedException Exception
   */
  function testLinkProjectsPreBuild() {
    _surf_link(FALSE, $this->tmp_dir);
  }

  function testLinkProjects() {

    $config = site_get_config($this->tmp_dir);

    $docroot = $this->tmp_dir . '/' . $config['docroot'];

    mkdir($docroot);
    mkdir($docroot . '/sites');
    mkdir($docroot . '/sites/all');
    mkdir($docroot . '/sites/all/modules');

    _surf_link(FALSE, $this->tmp_dir);

    foreach ($config['linked'] as $dir => $path) {
      $this->assertTrue(is_link($docroot . '/' . $path));
    }

    _surf_link(TRUE, $this->tmp_dir);

    foreach ($config['linked'] as $dir => $path) {
      $this->assertTrue(is_dir($docroot . '/' . $path));
    }

  }

  function testGetVersion() {
    $version = site_get_version($this->tmp_dir);
    $this->assertEquals($this->test_config['version'], $version);
  }

  function testBumpVersion() {
    $inc = array(
      'patch' => '1.0.1',
      'minor' => '1.1.0',
      'major' => '2.0.0'
    );

    foreach ($inc as $key => $pair) {
      site_bump_version($key, $this->tmp_dir);
      $config = site_get_config($this->tmp_dir);
      $this->assertEquals($config['version'], $pair);
    }
  }


}
