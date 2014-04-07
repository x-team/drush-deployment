Drush Deployment Module
based on drush_deploy module but concentrated on Tagging not Branch

To deploy a tag
===============

Example 1
drush dtag v.7.26-1.0

Example 2
drush rt v.7.26-1.0

Example 3
drush release-tag v.7.26-1.0


HowTo: Debug

============

To dump array or object use `drush_print_r()` function.

Config:
======

Place your configuration in your drush folder, usually in ~/.drush/. The filename must be `deployment.drushrc.php`.

```
<?php
  $options['deploy-repository'] = 'git://github.com/geraldvillorente/test-drupal.git';
  $options['docroot'] = '/media/Data/www/test';
?>
```

See `drush` for more commands.
