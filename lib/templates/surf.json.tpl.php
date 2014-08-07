{
    "name": "<?= $name; ?>",
    "title": "<?= $title; ?>",
    "type": "<?= $type; ?>",
    "description": "<?= $description; ?>",
    "version": "<?= $version; ?>",
    "core": "<?= $core; ?>",
    "docroot": "<?= $docroot; ?>",
    "directories": {
        "commands": "lib/commands",
        "config": "config",
        "data": "data",
        "docs": "docs",
        "libraries": "lib/libraries",
        "modules": "lib/modules",
        "profiles": "lib/profiles",
        "themes": "lib/themes"
    },
    "linked": {
      "modules": "sites/all/modules/custom",
      "themes": "sites/all/themes"
    },
    "commands": {
      "drush-casperjs": "git@bitbucket.org:davereid/drush-casperjs.git"
    }
}
