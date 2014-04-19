<?php
/**
 * @file
 *
 * @author Paul de Paula aka fusionx1
 *
 */

require 'DrushAutoLoader.php';
require 'upstream.inc';

// Load class include file.
require_once __DIR__ . '/lib/vendor' . '/autoload.php';
use GitElephant\Repository;


/**
 * Instantiate the Elephant.
 *
 * @return object
 *   An object of the specified repository.
 */
function _drush_init_elephant() {
  $git_dir = _drush_get_docroot();
  return new Repository($git_dir);
}


/**
 * Implements hook_drush_init().
 */
function drush_deployment_drush_init() {
  // Load deploy config files.
  foreach (drush_context_names() as $context) {
    drush_load_config_file($context, _drush_config_file($context, 'deployment'));
  }
}


/**
 * Implements hook_drush_help().
 */
function drush_deployment_drush_help($section) {
  switch ($section) {
    case 'drush:deployment-config':
      return dt('Return an array of configuration.');
    case 'drush:release-check-target':
      return dt('Check the target directory is a valid git repository.');
    case 'drush:git-init':
      return dt('Initialize git in target directory.');
    case 'drush:release-tag':
      return dt('Checkout/deploy a tag.');
    case 'drush:clone-repo':
      return dt('Clone a repository.');
    case 'drush:list-tags':
      return dt('List all tags.');
    case 'drush:latest-tag':
      return dt('Get the latest tag.');
  }
}


/**
 * Implements hook_drush_command().
 *
 * @return array
 *   A list of drush commands.
 */
function drush_deployment_drush_command() {
  $items = array();

  $items['deployment-config'] = array(
    'aliases' => array('dm-conf'),
    'description' => 'Return an array of configuration.',
    'callback' => '_drush_get_options',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'config' => 'deployment',
  );

  $items['check-repo'] = array(
    'aliases' => array('dm-check'),
    'description' => 'Check the target directory is a valid git repository.',
    'callback' => '_drush_git_check_target_dir',
  );

  $items['git-init'] = array(
    'description' => 'Initialize git in target directory.',
    'callback' => '_drush_git_init',
    'aliases' => array('dm-gi'),
  );

  $items['release-tag'] = array(
    'description' => 'Checkout/deploy a tag.',
    'callback' => '_drush_checkout_tag',
    'aliases' => array('dm-rt', 'dm-dtag'),
    'options' => array(
      'maintenance' => 'Manage site maintenance page.',
    ),
    'examples' => array(
      'drush release-tag v.1.0' => 'Deploy v.1.0 tag.',
      'drush release-tag v.1.0 --maintenance on' => 'Put the site in maintenance mode before deploying the tag.',
    ),
  );

  $items['clone-repo'] = array(
    'description' => 'Clone a repository.',
    'callback' => '_drush_clone_repo',
    'aliases' => array('dm-cr'),
  );

  $items['list-tags'] = array(
    'description' => 'List all tags.',
    'callback' => '_drush_list_tags',
    'aliases' => array('dm-lts'),
  );

  $items['latest-tag'] = array(
    'description' => 'Get the latest tag.',
    'callback' => '_drush_get_latest_tag',
    'aliases' => array('dm-lt'),
  );

  $items['rebuild-cache'] = array(
    'description' => '',
    'callback' => '_drush_rebuild_cache',
    'options' => array(
      'clear' => 'Clear and rebuild the cache after deployment.',
    ),
    'arguments' => array(
      'all' => 'Clear all cache.',
      'js-css' => 'Clear js and css cache.',
      'theme-registry' => 'Reset theme-registry.',
      'menu' => 'Clear menu cache.',
      'block' => 'Clear block cache.',
      'module-list' => 'Clear module list cache.',
      'views' => 'Clear views cache.',
    ),
    'examples' => array(
      'drush rebuild-cache --clear all' => 'Clear and rebuild the cache after deployment.',
      'drush rebuild-cache --clear views' => 'Clear and rebuild the views cache after deployment.',
      'drush rebuild-cache --clear block' => 'Clear and rebuild the block cache after deployment.',
    ),
    'aliases' => array('dm-rc'),
  );

  $items['list-branches'] = array(
    'description' => 'List all branches.',
    'callback' => '_drush_get_branches',
    'aliases' => array('dm-lsb'),
  );

  $items['maintenance'] = array(
    'description' => 'Manage site maintenance page.',
    'callback' => '_drush_maintenance',
    'aliases' => array('dm-m'),
    'arguments' => array(
      'on' => 'Turn on maintenance mode.',
      'off' => 'Turn off maintenance mode.',
    ),
    'examples' => array(
      'drush maintenance on' => 'Put the site offline.',
      'drush maintenance off' => 'Put the site online.',
    ),
  );

  return $items;
}


/**
 * Get document root.
 *
 * @return string
 *   The path of the repository.
 */
function _drush_get_docroot() {
  return drush_get_option('docroot');
}


/**
 * Get Git url.
 *
 * @return string
 *   The url of the remote repository.
 */
function _drush_git_url() {
  return drush_get_option('deploy-repository');
}


/**
 * Get config options.
 *
 * @return array
 *   Settings of deployment.drushrc.php.
 */
function _drush_get_options() {
  $options = array();
  $options['document_root'] = _drush_get_docroot();
  $options['git_url'] = _drush_git_url();

  return drush_print_r($options);
}


/**
 * Check target directory.
 */
function _drush_git_check_target_dir() {
  // Target repository.
  $repo = _drush_init_elephant();

  try {
    $repo->getStatus();
    drush_print(dt('Directory @docroot is a valid git repository.', array('@docroot' => _drush_get_docroot())));
  }
  catch (Exception $e) {
    // Initialize git init.
    drush_log(dt('@docroot is not a valid git repository. Run "drush git-init" to initialize.', array('@docroot' => _drush_get_docroot())), 'failed');
  }
}


/**
 * Initialize a new git repository.
 */
function _drush_git_init() {
  // Target repository.
  $repo = _drush_init_elephant();
  drush_confirm('Do you really want to continue?');
  $repo->init();
  drush_log(dt('@docroot is now a valid git repository.', array('@docroot' => _drush_get_docroot())), 'success');
}


/**
 * Deploy target tag.
 */
function _drush_checkout_tag() {
  $args = func_get_args();
  // Get the tag name.
  $tag_name = (!empty($args[0])) ? $args[0] : FALSE;

  $repo = _drush_init_elephant();

  // Verify if the tag is correct.
  $tags = $repo->listTags($array = TRUE);
  if (!in_array($tag_name, $tags)) {
    drush_log(dt('Tag @tag_name does not exist.', array('@tag_name' => $tag_name)), 'error');
    exit;
  }

  // Put the site offline.
  if (isset($args[1]) && $args[1] === 'on') {
    variable_set('maintenance_mode', 1);
    drush_log(dt('Site is now offline.'), 'success');
  }
  else {
    drush_log(dt('Invalid argument for maintenance mode!'), 'error');
    exit;
  }

  try {
    drush_confirm('Do you really want to continue?');
    $repo->checkout($repo->getTag($tag_name));
    drush_log(dt('You just checkout @tag_name tag.', array('@tag_name' => $tag_name)), 'success');
  }
  catch (Exception $e) {
    drush_log(dt('The tag you specified does not exist. Make sure that you cloned the source repository.'), 'error');
  }
}


/**
 * Clone repository.
 */
function _drush_clone_repo() {
  $repo = _drush_init_elephant();
  drush_confirm('Do you really want to continue?');
  drush_print(dt('This may take some time depending on how big is your repository.'));
  $repo->cloneFrom(_drush_git_url(), _drush_get_docroot());
  drush_log(dt('You just cloned a repository.'), 'success');
}


/**
 * List all tags.
 */
function _drush_list_tags() {
  $repo = _drush_init_elephant();
  drush_print_r($repo->listTags());
}


/**
 * Get latest tag.
 *
 * @return string
 *   The latest tag.
 */
function _drush_get_latest_tag() {
  $repo = _drush_init_elephant();
  return $repo->getLastTag();
}


/**
 * List all branches.
 */
function _drush_get_branches() {
  $repo = _drush_init_elephant();
  $branches = $repo->getBranches($namesOnly = TRUE, $all = TRUE);
  foreach ($branches as $branch) {
    drush_print('  ' . $branch);
  }
}


/**
 * Clear and rebuild the cache.
 */
function _drush_rebuild_cache() {
  $args = func_get_args();
  if (isset($args[0])) {
    drush_confirm('Do you really want to continue?');
    _drush_clear_cache($args[0]);
    _drush_rebuild_drupal_cache();
    drush_log(dt('Cache has been primed.'), 'success');
  }
  else {
    drush_log(dt('Command failed. Check the supplied parameters.'), 'error');
  }
}


/**
 * Clear cache.
 *
 * @param $arg string
 *   The cache type to be cleared.
 */
function _drush_clear_cache($arg) {
  if (function_exists('exec')) {
    exec('drush cc ' . $arg . ' -y');
  }
  else {
    // Native Drupal methods here.
    switch ($arg) {
      case 'all':
        drupal_flush_all_caches();
        break;

      case 'js-css':
        // Change query-strings on css/js files to enforce reload for all users.
        _drupal_flush_css_js();
        drupal_clear_css_cache();
        drupal_clear_js_cache();
        break;

      case 'menu':
        menu_rebuild();
        break;

      default:
        break;
    }
  }
}


/**
 * Manage maintenance page.
 */
function _drush_maintenance() {
  $args = func_get_args();
  if (isset($args[0])) {
    drush_confirm('Do you really want to continue?');
    switch ($args[0]) {
      case 'on':
        variable_set('site_offline', '1');
        drush_log(dt('Site is now offline.'), 'success');
        break;

      case 'off':
        variable_set('site_offline', '0');
        drush_log(dt('Site is now online.'), 'success');
        break;
    }
  }
}


/**
 * Rebuild the cache.
 */
function _drush_rebuild_drupal_cache() {
  // Warm homepage cache.
  // There is a module called Cache Warmer by Perusio. Worth checking it.
}
