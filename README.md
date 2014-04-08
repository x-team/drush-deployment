# Drush Deployment Module

![Banner](assets/banner.jpg)

This module is based on [drush_deploy](http://drupal.org/project/drush_deploy) module written by [Mark Sonnabaum](https://drupal.org/user/75278), but concentrated on tagging not branching.

**Contributors:** [X-Team](https://github.com/x-team), [Paul Jebulan de Paula](https://github.com/fusionx1), [Gerald Villorente](https://github.com/geraldvillorente)  
**Requires at least:** 7.x  
**Tested on:** 7.x   
**Stable tag:** trunk (master)  
**License:** [GPLv2 or later](http://www.gnu.org/licenses/gpl-2.0.html)  


## Installation ##

Install like a typical Drupal module.

1. `drush en drush_deployment`

2. Or go to Drupal module page and look for `Enhanced Drush Deployment` and
enable it by checking the checkbox

## Configuration ##

Place your configuration in your drush folder, usually in ~/.drush/. The filename must be `deployment.drushrc.php`.

```php
<?php
  $options['deploy-repository'] = 'git://github.com/geraldvillorente/test-drupal.git';
  $options['docroot'] = '/media/Data/www/test';
```

## To deploy a tag ##

Example 1

`drush dtag v.7.26-1.0`

Example 2

`drush rt v.7.26-1.0`

Example 3

`drush release-tag v.7.26-1.0`

ex.5
drush release-notes ver1.0 ver1.1
list down all diff between tags

## HowTo: Debug ##

To dump array or object use `drush_print_r()` function.

See `drush` for more commands.
