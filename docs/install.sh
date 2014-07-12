#!/usr/bin/env bash

command -v composer >/dev/null 2>&1 || { echo >&2 "Please install composer"; exit 1; }
command -v drush >/dev/null 2>&1 || { echo >&2 "Please install drush"; exit 1; }

rm -rf $HOME/.drush/surf

/usr/local/bin/composer create-project desmondmorris/surf $HOME/.drush/surf -s dev --no-dev -n

drush cc drush
