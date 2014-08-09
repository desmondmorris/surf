<?php

use desmondmorris\surf\Site;

require_once dirname(dirname(__FILE__)) . '/includes/utils.inc';

class siteTest extends PHPUnit_Framework_TestCase
{
    protected static $config, $configJSON, $tmp_dir;

    public static function setUpBeforeClass() {

      parent::setUpBeforeClass();

      self::$config = array(
        'name' => 'test-site',
        'title' => 'Test Site',
        'description' => 'A test site',
        'version' => '0.0.1'
      );
      self::$configJSON = jsonpp(self::$config);
      self::$tmp_dir = sys_get_temp_dir() . '/surf-tests-' . uniqid(time());

      mkdir(self::$tmp_dir);
      file_put_contents(self::$tmp_dir . '/surf.json', self::$configJSON);
    }

    /**
     * @expectedException Exception
     */
    public function testMissingConfig()
    {
      $site = new Site();
    }

    public function testWithConfig() {
      chdir(self::$tmp_dir);
      $site = new Site();
      $this->assertTrue(is_array($site->getConfig()));
    }

    public function testGetConfig() {
      $site = new Site(self::$config);

      $config = $site->getConfig();
      foreach ($config as $key => $pair) {
        $this->assertTrue(isset(self::$config[$key]));
        $this->assertEquals(self::$config[$key], $pair);
      }
    }

    public function testSaveConfig() {
      chdir(self::$tmp_dir);
      $site = new Site();
      $config = $site->getConfig();

      $config['item'] = 'value';

      Site::saveConfig($config);

      $updated = $site->loadConfig();

      $this->assertTrue(isset($updated['item']));
      $this->assertEquals($updated['item'], 'value');
    }

    public function testGetVersion() {
      $site = new Site(self::$config);
      $version = $site->getVersion();
      $this->assertEquals($version, self::$config['version']);
    }

    public function testBumpVersion() {

      chdir(self::$tmp_dir);
      $site = new Site();

      $site->bumpVersion();
      $this->assertEquals('0.0.2', $site->getVersion());

      $site->bumpVersion('minor');
      $this->assertEquals('0.1.0', $site->getVersion());

      $site->bumpVersion('major');
      $this->assertEquals('1.0.0', $site->getVersion());

    }

    /**
     * @expectedException Exception
     */
    public function testInvalidBumpType()
    {
      chdir(self::$tmp_dir);
      $site = new Site();

      $site->bumpVersion('invalid');
    }

}
