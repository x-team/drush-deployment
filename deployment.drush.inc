<?php
/**
 * @file
 *
 * @author Paul de Paula aka fusionx1
 *
 */

require 'DrushAutoLoader.php';
require 'upstream.inc';

/**
 * Ipmlementation of hook_drush_help().
 */
function deployment_drush_help($section) {
}

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



  $loader = new DrushAutoLoader();
  $loader->registerNamespace('Drush', __DIR__);
  $loader->registerNamespace('Drush\\Deployment', __DIR__);
  $loader->register();

  foreach(\Drush\Deployment\Deployment::getCommands() as $command) {
    $items['deployment-' . $command] = array(
      'description' => '',
      'arguments' => array(),
      'options' => array(),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUSH, // No bootstrap at all.
    );
  }

  foreach ($items as $command => &$item) {
    $item['callback'] = 'drush_deployment_callback';
    $item['callback arguments'] = array($command);
  }

  return $items;
}

/**
 * Ipmlementation of hook_drush_init().
 */
function deployment_drush_init() {
  // Load deploy config files.
  foreach (drush_context_names() as $context) {
    drush_load_config_file($context, _drush_config_file($context, 'deployment'));
  }
  $tasks = \Drush\Deployment\Deployment::getTasks();
}

function deployment_task() {
  $args = func_get_args();
  $name = array_shift($args);
  if ($args[count($args) - 1] instanceof Closure) {
    $work = array_pop($args);
  }
  else {
    $work = null;
  }
  Drush\Deployment\Deployment::$tasks[$name] = $work;
}

/**
 * Implementation of drush_hook_COMMAND().
 */
function drush_deployment_callback($command, $alias = NULL) {
  if (empty($alias)) {
    drush_set_error('DRUSH_DEPLOY_ERROR', dt("Must specify a site alias or group of site aliases!"));
    return;
  }
  $alias_settings = drush_sitealias_get_record($alias);
  $method = strpos($command, 'deployment-') === 0 ? substr($command, 7) : $command;
  $sites = array();
  if (isset($alias_settings['site-list'])) {
    foreach ($alias_settings['site-list'] as $alias) {
      $sites[] = drush_sitealias_get_record($alias);
    }
  }
  else {
    $sites[] = $alias_settings;
  }

  foreach ($sites as $site) {
    drush_set_config_options('alias', $site);
  }

  $deploy = new \Drush\Deployment\Deployment($sites);
  try {
    $deploy->runCommand($deploy, $method);
  }
  catch (CommandException $e) {
    drush_set_error($e->getMessage());
  }
}

function drush_deployment_transaction(&$obj = NULL, $commands) {
  if ($obj) {
    foreach ($commands as $i => $cmd) {
      try {
        $obj->runCommand($obj, $cmd);
      }
      //catch (CommandException $e) {
      catch (Exception $e) {
        foreach ($commands as $ri => $rcmd) {
          try {
            $obj->runCommand($obj, $rcmd . '-rollback');
          }
          //catch (CommandException $e) {
          catch (Exception $e) {
            drush_set_error($e);
          }
          if ($ri >= $i) break;
          $ri++;
        }
        break;
      }
    }
  }
}
