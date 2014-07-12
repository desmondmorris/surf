<?php

/**
 * @file
 * Surf drupal project manager
 */

require 'vendor/autoload.php';

use desmondmorris\surf\Site;
use desmondmorris\surf\Make;
use desmondmorris\surf\Project;

define('SURF_PATH', dirname(__FILE__));
define('SURF_SRC_PATH', SURF_PATH . '/src/desmondmorris/surf');
define('VENDOR_PATH', SURF_PATH . '/vendor');

/**
 * Implements hook_drush_command().
 */
function surf_drush_command() {
  $items = array();

  $items['surf-create'] = array(
    'description' => 'Create new project manifest.',
    'callback' => '_drush_surf_create',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-create' => 'Creates new project manifest.',
    ),
    'options' => array(
      'scaffold' => array(
        'description' => 'Adds a basic project directory structure and files.',
      ),
    ),
  );

  $items['surf-projects'] = array(
    'description' => 'List projects.',
    'callback' => '_drush_surf_projects',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-projects' => 'Lists all projects.',
      'drush surf-projects --no-dev' => 'Exclude dev projects.',
    ),
    'options' => array(
      'no-dev' => array(
        'descriptions' => 'Lists all projects except dev.'
      )
    ),
  );

  $items['surf-build'] = array(
    'description' => 'Build site from project config.',
    'callback' => '_drush_surf_build',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-desmond' => 'Build site from project config.',
    ),
    'options' => array(
      'dev' => 'Include dev projects'
    ),
  );

  $items['surf-add'] = array(
    'description' => 'Add project dependencies.',
    'callback' => '_drush_surf_add',
    'arguments' => array(
      'name' => 'Project name',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-add views' => 'Add views.',
      'drush surf-add views@1.0' => 'Add views 1.0.',
    ),
    'options' => array(
      'dev' => 'Development only projects'
    ),
  );

  $items['surf-version'] = array(
    'description' => 'Get current site version or bump current version.',
    'callback' => '_drush_surf_version',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-version' => 'Get current site version.',
      'drush surf-version --bump=minor' => 'Bumps the minor version of the project',
    ),
    'options' => array(
      'bump' => array(
        'description' => 'Optional: This will bump the version number',
        'example-value' => 'patch|minor|major|build',
      ),
    ),
  );

  $items['surf-sniff'] = array(
    'description' => 'Run code sniffer against our custom code.',
    'callback' => '_drush_surf_sniff',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'topics' => array('surf'),
    'examples' => array(
      'drush surf-sniff' => 'Run code sniffer against our custom code.',
    )
  );

  return $items;
}

/**
 *
 */
function _drush_surf_projects() {
  $config = Site::loadConfig();

  $no_dev = drush_get_option('no-dev');

  $project_types = array('projects', 'devProjects');

  foreach($project_types as $project_type) {

    if (($project_type == 'devProjects') && ($no_dev)) {
      break;
    }

    if (isset($config->$project_type)) {
      foreach($config->$project_type as $type => $projects) {

        $group = $type . 's';

        if ($project_type == 'devProjects') {
          $group = 'Development ' . $group;
        }

        drush_print(ucwords($group) . ':');
        foreach($projects as $name => $version) {
          drush_print($name . '-' . $version);
        }
        drush_print();
      }
    }

  }

}


/**
 *
 */
function _drush_surf_add() {

  $names = func_get_args();

  if (empty($names)) {
    return drush_set_error(
      'DRUSH_SURF_PROJECT_ERROR',
      'You must pass atleast one project.'
    );
  }

  $config = Site::loadConfig();

  $project_type = 'projects';
  if(!isset($config->projects)){
    $config['projects'] = array();
  }

  if (drush_get_option('dev')) {
    $project_type = 'devProjects';
    if(!isset($config->devProjects)){
      $config['devProjects'] = array();
    }
  }

  foreach($names as $name) {


    preg_match("/([a-z_-]*)(@(.*))?$/", $name, $matches);

    $project_version = isset($matches[3]) ? $matches[3] : null;
    $name = $matches[1];
    try {
      $info = Project::getInfo($name, $config->core);
    }
    catch (Exception $e) {
      drush_set_error(
        'DRUSH_SURF_PROJECT_ERROR',
        $e->getMessage()
      );
      continue;
    }

    $type = str_replace('project_', '', strval($info->type) );

    if (!$project_version) {
      $recommended = $info->releases->release[0];

      if (!isset($config->$project_type->$type)) {
        $config[$project_type][$type] = array();
      }

      $project_version = $recommended->version_major . '.' . $recommended->version_patch;

      if (isset($recommended->version_extra)) {
        $project_version .= "-" . $recommended->version_extra;
      }
    }

    drush_print('Adding '  . $info->title . ' (' . $name . '-' . $project_version . ')');

    $config->$project_type->$type->$name = $project_version;

    //ksort($config->$project_type->$type);
  }
  //ksort($config->$project_type);
  Site::saveConfig($config);

}

function _drush_surf_build() {

  $site = new Site();
  $config = $site->getConfig();

  if (drush_get_option('dev')) {
    $options['dev'] = TRUE;
  }

  $make = new Make($site, $options);
  $make->build();

}

/**
 *
 */
function _drush_surf_create() {

  $config = array();

  $config['name'] = drush_prompt(
    'Name',
    basename(getcwd())
  );

  $config['description'] = drush_prompt(
    'Description',
    'My cool new Drupal project'
  );

  $config['version'] = drush_prompt(
    'Project version',
    '0.0.1'
  );

  $config['core'] = drush_prompt(
    'Drupal Core version',
    '7.x'
  );

  $config['docroot'] = 'drupal';

  $scaffold = drush_get_option('scaffold');

  if ($scaffold) {

    $directories = array(
      'config',
      'lib/modules',
      'lib/themes',
      'lib/libraries',
      'lib/profiles',
      'scripts',
    );

    $files = array(
      '.gitignore' => $config['docroot']  . '/',
      'config/README.md' => 'Keep your configuration files here.',
      'config/settings.php' => '<?php\n',
      'lib/README.md' => 'This is where your custom code lives.',
      'lib/modules/README.md' => 'A home for your custom modules',
      'lib/themes/README.md' => 'A home for your custom themes',
      'lib/libraries/README.md' => 'A home for your custom libraries',
      'lib/profiles/README.md' => 'A home for your custom profiles',
      'scripts/pre-build.sh' => "#!/usr/bin/env bash\n\necho 'Starting build'\n",
      'scripts/post-build.sh' => "#!/usr/bin/env bash\n\necho 'Build complete.'\n",
    );

    exec('mkdir -p ' . implode(' ', $directories));

    foreach ($files as $filename => $contents) {
      exec("echo \"$contents\" > $filename");
    }

    chmod('scripts/post-build.sh', 0755);
    chmod('scripts/pre-build.sh', 0755);

    $config['linked'] = array(
      'lib/modules' => 'sites/all/modules/custom',
      'lib/themes' => 'sites/all/themes/custom ',
      'lib/libraries' => 'sites/all/libraries',
      'config/settings.php' => 'sites/default/settings.php',
    );

    if (!file_exists('.git')) {
      $git = drush_confirm("Initialize git?");

      if ($git) {
        exec('git init');

        $githooks = drush_confirm("Add githooks?");

        if ($githooks) {
          exec("cp " . SURF_SRC_PATH . "/scripts/pre-commit .git/hooks");
        }

      }
    }

  }

  drush_print(Site::configToYaml($config));

  $confirm = drush_confirm("Is this correct?");

  if ($confirm) {
    Site::saveConfig($config);

    drush_print('Success!  Happy Drupalling');
  }

}

/**
 *
 */
function _drush_surf_version() {

  $site = new Site();

  $bump = drush_get_option('bump');

  if ($bump) {
    if ($bump === TRUE) {
      $bump = 'patch';
    }
    $site->bumpVersion($bump);
  }
  drush_print($site->getVersion());

}

/**
 *
 */
function _drush_surf_sniff() {

  $command = array(
    VENDOR_PATH . '/bin/phpcs',
    '--standard=' . VENDOR_PATH . '/drupal/coder/coder_sniffer/Drupal',
    'lib',
    '--ignore=*.js,*.css'
  );

  $command = implode(' ', $command);
  exec($command, $output);

  if (!empty($output)) {
    $results = implode("\n", $output);
    drush_print($results);
    return $results;
  }

}
