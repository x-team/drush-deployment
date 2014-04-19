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
    case 'drush:tag-notes':
      return dt('Display commit notes between tags');

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
      'registry' => 'Clear entire registry.',
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

  $items['tag-notes'] = array(
    'description' => 'Generate release notes using all commits between two tags',
    'arguments' => array(
      'tag 1' => 'The first tag, the starting point for the log.',
      'tag 2' => 'The second tag, the ending point for the log. ',
    ),
    'callback' => '_drush_tag_notes',
    'aliases' => array('rn', 'relnotes'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
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

      case 'registry':
        registry_rebuild();
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
        variable_set('maintenance_mode', 1);
        drush_log(dt('Site is now offline.'), 'success');
        break;

      case 'off':
        variable_set('maintenance_mode', 0);
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


/**
 * Displays commits. 
 */
function _drush_tag_notes($tag1n = NULL, $tag2n = NULL) {
  $git = drush_get_option('git', 'git');
  // Make sure we are in the working directory started.
  _drush_git_check_target_dir();

  // Fill in calculated tags if both are not given.
  if (!isset($tag2n)) {
    // Get all the defined tags in this repository and sort them.
    drush_shell_exec('%s name-rev HEAD', $git);
    $branch = substr(array_shift(drush_shell_exec_output()), 5, -1);
    drush_shell_exec('%s tag -l %s*', $git, $branch);
    $tags = drush_shell_exec_output();
    usort($tags, 'version_compare');

    if (!isset($tag1n) && count($tags)) {
      // If no tags are provided, use the two most recent ones.
      $tag2n = array_pop($tags);

      $tag1n = count($tags) ? array_pop($tags) : $tag2n;
    }
    else {
      // If only one tag is given, it is considered to be <1st> and <2nd> is
      // taken to be one tag before it.

      $key = array_search($tag1n, $tags);
      if (is_numeric($key)) {
        if ($key > 0) {
          // Rearrange our tags: the given tag is in fact tag 2.
          $tag2n = $tag1n;
          // The <1st> tag is one before the given <2nd> tag.
          $tag1n = $tags[$key - 1];
        }
        else {
          return drush_set_error('DRUSH_INVALID_TAG', dt('!tag is the first in the branch.', array('!tag' => $tag1n)));
        }
      }
      else {
        return drush_set_error('DRUSH_INVALID_TAG', dt('!tag is not a valid Git tag.', array('!tag' => $tag1n)));
      }
    }
  }

  // '^' is the escape character on Windows (like '\' on *nix) - has to be
  // contained in the escaped shell argument string ("%s").
  if (!drush_shell_exec('%s show -s --pretty=format:%%H %s', $git, $tag1n . '^{commit}')) {
    return drush_set_error('DRUSH_INVALID_TAG', dt('!tag is not a valid Git tag.', array('!tag' => $tag1n)));
  }
  $tag1 = drush_shell_exec_output();
  // '^' is the escape character on Windows (like '\' on *nix) - has to be
  // contained in the escaped shell argument string ("%s").
  if (!drush_shell_exec('%s show -s --pretty=format:%%H %s', $git, $tag2n . '^{commit}')) {
    return drush_set_error('DRUSH_INVALID_TAG', dt('!tag is not a valid Git tag.', array('!tag' => $tag2n)), 'error');
  }
  $tag2 = drush_shell_exec_output();
  $changes = _drush_get_changes($tag1[0], $tag2[0], $git);
  $items = _drush_get_items_array($changes);
  if (drush_get_option('changelog', FALSE)) {
    $formatted_items = _drush_format_changelog($items, $tag2n);
  }
  else {
    $formatted_items = _drush_format_changes($items, $tag1n, $tag1[0], $tag2[0], $git);
  }
  if (drush_get_context('DRUSH_PIPE')) {
    drush_print_pipe(array_keys($formatted_items['raw']));
  }
  else {
    drush_print($formatted_items['rendered']);
  }

  return TRUE;
}

// Other functions

/**
 * Generates the output.
 */
function _drush_format_changes($items, $prev_tag, $tag1, $tag2, $git) {
  $rendered = "Changes since $prev_tag";
  if (drush_get_option('commit-count')) {
    $rendered .= ' (' . trim(drush_get_option('commit-count')) . ' commits)';
  }
  $rendered .= ":\n";

  if (!empty($items)) {
    $rendered .= "\n  " . implode("\n  ", $items) . "\n";
  }

  return array('rendered' => $rendered, 'raw' => $items);
}

function _drush_format_changelog($items, $tag) {
  drush_shell_exec("more *.info | grep name");
  $name = "";
  $info = drush_shell_exec_output();
  $info = explode("=", $info[0]);
  if (isset($info[1])) {
    $name = trim($info[1]);
  }
  drush_shell_exec("git show -s --pretty=format:%%ad --date=short $tag");
  $date = drush_shell_exec_output();
  $changelog = $name . " " . $tag . ", " . $date[0] . "\n";
  $changelog .= str_pad("", strlen($changelog), "-");
  $changelog .= "\n";
  foreach ($items as $raw => $html) {
    $changelog .= '- ';
    $line = ucfirst(trim((strpos($raw, "#") === 0) ? substr(strstr($raw, ':'), 1) : $raw));
    $changelog .= substr($line, -1) == "." ? $line : $line . ".";
    $changelog .= "\n";
  }
  return array('raw' => $items, 'rendered' => $changelog);
}

/**
 * Gets the changes and returns them in an array.
 */
function _drush_get_changes($tag1, $tag2, $git) {
  $changes = array();

  $reverse = drush_get_option('reverse', FALSE) ? '--reverse' : '';
  if (!drush_shell_exec("%s log -s --pretty=format:%%s %s %s..%s", $git, $reverse, $tag1, $tag2)) {
    return drush_set_error('DRUSH_GIT_LOG_ERROR', 'git log returned an error.');
  }
  $output = drush_shell_exec_output();
  $changes[] = $output[0]; // Otherwise, next() would skip first
  while (($line = next($output)) !== FALSE) {
    if (empty($line)) {
      // Skip blank lines that are left behind in the messages.
      continue;
    }
    $changes[] = $line;
  }
  if (drush_get_option('commit-count')) {
    drush_set_option('commit-count', count($changes));
  }
  return $changes;
}

function _drush_get_items_array($issues) {
  $baseurl = drush_get_option('baseurl', '/node/');
  if (strpos($baseurl, '%s') == FALSE) {
    $baseurl .= '%s';
  }

  $items = array();
  foreach ($issues as $number => $line) {
    // Clean up commit log.
    $item = preg_replace('/^(Patch |- |Issue ){0,3}/', '', $line);
    // Add issue links.
    $items[$item] = preg_replace('/#(\d+)/S', '<a href="' . str_replace('%s', '$1', $baseurl) . '">#$1</a>', $item);
  }
  return $items;
}

