Drush Deployment Module
=======================

Is based on [drush_deploy](http://drupal.org/project/drush_deploy) module written by [Mark Sonnabaum](https://drupal.org/user/75278), but concentrated on tagging not branching.

Examples
========

ex.1
drush release-code --tag=ver2

ex.2
drush release-rollback --tag=ver1

ex.3
drush release-pre-check --tag=ver2

ex.4
drush release-post-check {prod|stage|dev}


HowTo: Debug
============

To dump array or object use `drush_print_r()` function.

Config Sample
=============

```
<?php
$options['application'] = 'drupal';
// Source Repository
$options['deploy-repository'] = 'git@github.com:geraldvillorente/test-drupal.git';
$options['keep-releases'] = 1;
$options['deploy-via'] = 'RemoteCache';
// Drupal root
$options['docroot'] = '/var/www/';
?>
```
