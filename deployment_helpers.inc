<?php

function deployment_task() {
  $args = func_get_args();
  $name = array_shift($args);
  if ($args[count($args) - 1] instanceof Closure) {
    $work = array_pop($args);
  }
  else {
    $work = null;
  }
  DrushDeployment\Deployment::$tasks[$name] = $work;
}

function deployment_before($before_task, $task) {
  DrushDeployment\Deployment::$before[$before_task][] = $task;
}

function deployment_after($after_task, $task) {
  DrushDeployment\Deployment::$after[$after_task][] = $task;
}
