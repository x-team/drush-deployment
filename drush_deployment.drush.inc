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
 */
function _drush_init_elephant() {
  $git_dir = _drush_get_docroot();
  return new Repository($git_dir);
}


/**
 * Get document root.
 */
function _drush_get_docroot() {
  return drush_get_option('docroot');
}


/**
 *  Get Git url.
 */
function _drush_git_url() {
  return drush_get_option('deploy-repository');
}


/**
 * Get config options.
 */
function _drush_get_options() {
  $options = array();
  $options['document_root'] = _drush_get_docroot();
  $options['git_url'] = _drush_git_url();

  return drush_print_r($options);
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
 */
function drush_deployment_drush_command() {
  $items = array();

  $items['deployment-config'] = array(
    'aliases' => array('getconf'),
    'description' => 'Return an array of configuration.',
    'callback' => '_drush_get_options',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'config' => 'deployment',
  );

  $items['release-check-target'] = array(
    'aliases' => array('rct', 'release-ct'),
    'description' => 'Check the target directory is a valid git repository.',
    'callback' => '_drush_git_check_target_dir',
  );

  $items['git-init'] = array(
    'description' => 'Initialize git in target directory.',
    'callback' => '_drush_git_init',
    'aliases' => array('giti', 'gi'),
  );

  $items['release-tag'] = array(
    'description' => 'Checkout/deploy a tag.',
    'callback' => '_drush_checkout_tag',
    'aliases' => array('rt', 'dtag'),
  );

  $items['clone-repo'] = array(
    'description' => 'Clone a repository.',
    'callback' => '_drush_clone_repo',
    'aliases' => array('cgr', 'cr'),
  );

  $items['list-tags'] = array(
    'description' => 'List all tags.',
    'callback' => '_drush_list_tags',
    'aliases' => array('ls-tags'),
  );

  $items['latest-tag'] = array(
    'description' => 'Get the latest tag.',
    'callback' => '_drush_get_latest_tag',
    'aliases' => array('glt'),
  );

  return $items;
}


/**
 * Check target directory.
 */
function _drush_git_check_target_dir() {
  // Target repository.
  $repo = _drush_init_elephant();

  try {
    $repo->getStatus();
    drush_print(dt('@docroot is a valid git repository.', array('@docroot' => _drush_get_docroot())));
  }
  catch (Exception $e) {
    // Initialize git init.
    drush_print(dt('@docroot is not a valid git repository. Run "drush git-init" to initialize.', array('@docroot' => _drush_get_docroot())));
  }
}


/**
 * Initialize a new git repository.
 */
function _drush_git_init() {
  // Target repository.
  $repo = _drush_init_elephant();
  $repo->init();
  drush_print(dt('@docroot is now a valid git repository.', array('@docroot' => _drush_get_docroot())));
}


/**
 * Deploy target tag.
 */
function _drush_checkout_tag() {
  $args = func_get_args();
  // Get the tag name.
  $tag_name = (!empty($args[0])) ? $args[0] : FALSE;

  $repo = _drush_init_elephant();

  try {
    $repo->checkout($repo->getTag($tag_name));
    drush_print(dt('You just checkout @tag_name tag.', array('@tag_name' => $tag_name)));
  }
  catch (Exception $e) {
    drush_print(dt('The tag you specified does not exist. Make sure that you cloned the source repository.'));
  }
}


/**
 * Clone repository.
 */
function _drush_clone_repo() {
  $repo = _drush_init_elephant();
  drush_print(dt('This may take some time depending on how big is your repository.'));
  $repo->cloneFrom(_drush_git_url(), _drush_get_docroot());
  drush_print(dt('You just cloned a repository.'));
}


/**
 * List all tags.
 */
function _drush_list_tags() {
  $repo = _drush_init_elephant();
  drush_print($repo->listTags());
}


/**
 * Get latest tag.
 */
function _drush_get_latest_tag() {
  $repo = _drush_init_elephant();
  return $repo->getLastTag();
}
