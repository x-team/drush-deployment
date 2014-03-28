Drush Deployment Module 
based on drush_deploy module but concentrated on Tagging not Branch

ex.1
drush deployment release --tag=ver1

ex.2
drush deployment rollback --tag=ver1

ex.3
drush deployment pre-release-check --tag=ver1



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
 

