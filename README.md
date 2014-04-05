Drush Deployment Module
=======================

Is based on [drush_deploy](http://drupal.org/project/drush_deploy) module written by [Mark Sonnabaum](https://drupal.org/user/75278), but concentrated on tagging not branching.

ex.1
drush release-code --tag=ver2

ex.2
drush release-rollback --tag=ver1

ex.3
drush release-pre-check --tag=ver2

ex.4
drush release-post-check {prod|stage|dev}


```
/**
 * Implementation of hook_drush_command().
 */
function deployment_drush_command() {
  $items = array();
  $items['deployment'] = array(
    'description' => '',
    'arguments' => array(
      'filling' => 'e.g. release rollback pre-release-check'
    ),
    'options' => array(
      'tag' => 'e.g. ver1, ver2, tagname'
    ),
    'examples' => array(
      'drush deployment release --tag=ver1' => 'sample drush deployment using release with tagname'
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'config' => 'deployment',
  );
}
```

HowTo: Debug
============

To dump array or object use `drush_print_r()` function.
