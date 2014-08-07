# Inspired by the Lullabot Drupal Boilerplate

# Sensitive information #
#########################
# Ignore configuration files that may contain sensitive information.
<?= $docroot; ?>/sites/*/settings*.php
*.local.php

# User Generated Content #
##########################
<?= $docroot; ?>/files
<?= $docroot; ?>/sites/*/files
<?= $docroot; ?>/sites/*/private

# Vagrant Files #
#################
.vagrant

# Compiled source #
###################
*.com
*.class
*.dll
*.exe
*.o
*.so

# Packages #
############
# It's better to unpack these files and commit the raw source, git has its own
# built in compression methods
*.7z
*.dmg
*.gz
*.iso
*.jar
*.rar
*.tar
*.zip

# Logs and databases #
######################
*.log
*.sql
*.sqlite

# OS generated files #
######################
.DS_Store*
ehthumbs.db
Icon\?
Thumbs.db


# Contirbuted Drush Commands #
vendor/

data/tmp/
data/cache/
