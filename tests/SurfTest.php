<?php
/**
 * @file
 * Surf tests.
 */

class SurfTest extends PHPUnit_Framework_TestCase {

  public $tmpDir = "";
  public $testConfig = array();
  public $testProjects = array();
  public $testProjectsPrepared = "";

  /**
   * Setup common variables for all tests.
   */
  public function setup() {

    require_once PROJECT_ROOT . '/surf.drush.inc';

    $project_name = 'surf-tests-' . uniqid(time());

    $this->testConfig = array(
      'name' => $project_name,
      'title' => 'Test Project',
      'description' => 'A little test about nothing',
      'type' => 'website',
      'version' => '1.0.0',
      'core' => '7.x',
    );

    $this->testConfig_string = '';

    foreach ($this->testConfig as $config => $value) {
      $this->testConfig_string .= '--' . $config . '="' . $value . '" ';
    }

    $this->tmpDir = sys_get_temp_dir() . '/' . $project_name;
    mkdir($this->tmpDir);
    chdir($this->tmpDir);
    shell_exec('drush surf-init ' . $this->testConfig_string . ' -y');

    $this->testProjects = array(
      array('name' => 'views', 'version' => '3.8'),
      array('name' => 'ctools'),
      array('name' => 'admin_menu', 'version' => '3.0-rc4'),
    );

    $this->testProjectsPrepared = array();

    foreach ($this->testProjects as $project) {
      $name = $project['name'];
      $name .= (isset($project['version']) ? '@' . $project['version'] : '');
      $this->testProjectsPrepared[] = $name;
    }

  }

  /**
   * Tests new project initialization.
   */
  public function testProjectInit() {
    $this->assertTrue(file_exists($this->tmpDir . '/surf.json'));
  }

  /**
   * Tests retrieving project config from manifest.
   */
  public function testProjectConfig() {
    $config = site_get_config($this->tmpDir);
    $this->assertTrue(is_array($config));
    foreach ($this->testConfig as $key => $pair) {
      $this->assertTrue(isset($config[$key]));
      $this->assertEquals($config[$key], $pair);
    }
  }

  /**
   * Tests saving config to manifest.
   */
  public function testProjectSaveConfig() {
    $config = site_get_config($this->tmpDir);

    $config['test_setting'] = '1';

    site_save_config($config, $this->tmpDir);

    $config = site_get_config($this->tmpDir);

    $this->assertTrue(isset($config['test_setting']));
    $this->assertEquals($config['test_setting'], 1);
  }

  /**
   * Tests adding projects to manifest.
   */
  public function testProjectAdd() {
    _surf_pm_add($this->testProjectsPrepared, $this->tmpDir, TRUE);

    $config = site_get_config($this->tmpDir);

    $this->assertTrue(isset($config['projects']['module']));

    foreach ($this->testProjects as $project) {
      $this->assertTrue(isset($config['projects']['module'][$project['name']]));

      if (isset($project['version'])) {
        $this->assertEquals($config['projects']['module'][$project['name']], $project['version']);
      }
    }
  }

  /**
   * Tests link behavior if a project manifest is not present.
   *
   * @expectedException Exception.
   */
  public function testLinkProjectsPreBuild() {
    _surf_link(FALSE, $this->tmpDir);
  }

  /**
   * Tests linking and copying project directories.
   */
  public function testLinkProjects() {

    $config = site_get_config($this->tmpDir);

    $docroot = $this->tmpDir . '/' . $config['docroot'];

    mkdir($docroot);
    mkdir($docroot . '/sites');
    mkdir($docroot . '/sites/all');
    mkdir($docroot . '/sites/all/modules');

    _surf_link(FALSE, $this->tmpDir);

    foreach ($config['linked'] as $dir => $path) {
      $this->assertTrue(is_link($docroot . '/' . $path));
    }

    _surf_link(TRUE, $this->tmpDir);

    foreach ($config['linked'] as $dir => $path) {
      $this->assertTrue(is_dir($docroot . '/' . $path));
    }

  }

  /**
   * Tests getting version from manifest.
   */
  public function testGetVersion() {
    $version = site_get_version($this->tmpDir);
    $this->assertEquals($this->testConfig['version'], $version);
  }

  /**
   * Tests incrementing version in manifest.
   */
  public function testBumpVersion() {
    $inc = array(
      'patch' => '1.0.1',
      'minor' => '1.1.0',
      'major' => '2.0.0',
    );

    foreach ($inc as $key => $pair) {
      site_bump_version($key, $this->tmpDir);
      $config = site_get_config($this->tmpDir);
      $this->assertEquals($config['version'], $pair);
    }
  }


}
