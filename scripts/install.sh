#!/usr/bin/env bash

command -v composer >/dev/null 2>&1 || { echo >&2 "Please install composer"; exit 1; }
command -v drush >/dev/null 2>&1 || { echo >&2 "Please install drush"; exit 1; }

rm -rf $HOME/.drush/surf

composer_bin=$(which composer)

$composer_bin create-project desmondmorris/surf $HOME/.drush/surf -s dev --no-dev -n

TEMPLATE=$(cat <<EOF
# SURF
\$project_root = drush_get_option('root') ? drush_get_option('root') : getcwd();
if (file_exists(\$project_root . '/surf.json')) {
  \$option_keys = array('config', 'include', 'alias-path');
  foreach(\$option_keys as \$option_key) {
    if (isset(\$options[\$option_key]) && !is_array(\$options[\$option_key])) {
      \$options[\$option_key] = array(\$options[\$option_key]);
    }
  }
  \$options['config'][] = \$project_root . '/config/drushrc.php';
  \$options['include'][] = \$project_root . '/lib/commands';
  \$options['include'][] = \$project_root . '/vendor/drush-commands';
  \$options['alias-path'][] = \$project_root . '/config/aliases';
}
EOF
);

if [ ! -f ~/.drushrc.php ]; then
  echo "Creating a drush rc file";
  echo "<?php $TEMPLATE" > ~/.drushrc.php;
else
  exists=`grep -i "# SURF" ~/.drushrc.php`
  if [ -z "$exists" ]; then
    echo "Installing drushrc config"
    echo "$TEMPLATE" >> ~/.drushrc.php;
  fi
fi

drush cc drush
