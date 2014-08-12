#!/usr/bin/env bash

OUTPUT=$($PWD/vendor/bin/phpcs --standard=$PWD/vendor/drupal/coder/coder_sniffer/Drupal $PWD/includes $PWD/surf.drush.inc $PWD/tests/)

if [[ ! -z "$OUTPUT" ]]
  then
  echo "$OUTPUT"
  exit 1
fi

exit 0;
