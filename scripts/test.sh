#!/usr/bin/env bash

composer install --dev

./vendor/bin/phpunit --bootstrap=./vendor/autoload.php --debug tests/
