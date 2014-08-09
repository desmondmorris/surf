<?php

use desmondmorris\surf\Site;

define('PROJECT_ROOT', dirname(dirname(__FILE__)));

require_once PROJECT_ROOT . '/includes/utils.inc';

class pmTest extends PHPUnit_Framework_TestCase
{

    protected static $config, $configJSON, $tmp_dir;

    public static function setUpBeforeClass() {

      parent::setUpBeforeClass();

      // Load Drush - @todo throw this in a base test case somewhere
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/environment.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/command.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/drush.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/engines.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/backend.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/batch.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/context.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/sitealias.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/exec.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/drupal.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/output.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/cache.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/filesystem.inc';
      require_once PROJECT_ROOT . '/vendor/drush/drush/includes/dbtng.inc';

      require_once PROJECT_ROOT . '/src/desmondmorris/surf/commands/pm.inc';

      self::$config = array(
        'name' => 'test-site',
        'title' => 'Test Site',
        'description' => 'A test site',
        'version' => '0.0.1',
        'core' => '7.x'
      );
      self::$configJSON = jsonpp(self::$config);
      self::$tmp_dir = sys_get_temp_dir() . '/surf-tests-' . uniqid(time());

      mkdir(self::$tmp_dir);
      file_put_contents(self::$tmp_dir . '/surf.json', self::$configJSON);
    }

    public function testProjectAdd()
    {
      chdir(self::$tmp_dir);
      $site = new Site();

      $projects = array(
        array('name' => 'views', 'version' => '3.8'),
        array('name' => 'ctools'),
        array('name' => 'admin_menu', 'version' => '3.0-rc4')
      );

      $names = array();

      foreach($projects as $project) {
        $name = $project['name'];
        $name .= (isset($project['version']) ? '@' . $project['version'] : '');
        $names[] = $name;
      }

      _surf_pm_add($names, TRUE);

      $config = $site->loadConfig();

      $this->assertTrue(isset($config['projects']['module']));

      foreach($projects as $project) {
        $this->assertTrue(isset($config['projects']['module'][$project['name']]));

        if (isset($project['version'])) {
          $this->assertEquals($config['projects']['module'][$project['name']], $project['version']);
        }
      }
    }
}
