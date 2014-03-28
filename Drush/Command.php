<?php
/**
 *
 */
namespace Drush;
class Command {
  private static $annotations = array();
  private static $functions = array();
  public static $before = array();
  public static $after = array();
  public static $tasks = array();
  public $classname = NULL;
  public $commands = NULL;

  public function className() {
    if (empty($this->classname)) {
      $class = get_called_class();
      $ref = new \ReflectionClass($class);
      $this->classname = $ref->getShortName();
    }
    return $this->classname;
  }

  public function runCommand($obj, $cmd) {
    static $commands = array();
    if (empty($commands)) {
      $commands = drush_get_context('DRUSH_COMMANDS');
      $commands = !empty($commands) ? array_keys($commands) : $commands;
    }
    $class = $obj->className();
    $class = strtolower($class);
    $before = drush_get_option('before', array());
    $after = drush_get_option('after', array());
    // Full command name.
    $command_name = $cmd == $class ? $cmd : $class . '-' . $cmd;
    // Convert command names to camelcase method names.
    $method = preg_replace("/-([a-z])/e", "strtoupper('\\1')", $cmd);

    // See if there are any before tasks to run before calling the command callback.
    if (isset($before[$command_name])) {
      foreach($before[$command_name] as $task) {
        // If it's in the list of drush commands, call it as one.
        if (in_array($task, $commands)) {
          $this->drush($task);
        }
        else {
          $task($obj);
        }
      }
    }

    // Call command callback.
    if (is_callable(array($obj, $method))) {
      $ret = $obj->{$method}();
    }

    // Call any after tasks.
    if (isset($after[$command_name])) {
      foreach($after[$command_name] as $task) {
        if (in_array($task, $commands)) {
          $this->drush($task);
        }
        else {
          $task($obj);
        }
      }
    }

    return $ret;
  }

  public static function getCommands() {
    $class = get_called_class();
    $ref = new \ReflectionClass($class);
    $class_name = $ref->getShortName();
    $annotations = self::getClassAnnotations($ref);

    $commands = array();
    foreach($annotations[$class_name] as $method_name => $a) {
      if (isset($a['command'])) {
        $commands[] = $method_name;
      }
    }
    return $commands;
  }

  public static function getFunctions($reset = FALSE) {
    if (empty(self::$functions) || $reset === FALSE) {
      $functions = get_defined_functions();
      self::$functions = $functions['user'];
    }
    return self::$functions;
  }

  public static function getTasks() {
    $class = get_called_class();
    $ref = new \ReflectionClass($class);
    $namespace = $ref->getNamespaceName();
    $namespace_match = strtolower($namespace);
    $functions = self::getFunctions();
    $tasks = array();
    foreach ($functions as $f) {
      if (strpos($f, $namespace_match) === 0) {
        $tasks[] = $f;
      }
    }
    $annotations = self::getFunctionAnnotations($tasks);
    foreach ($annotations as $function => $a) {
      if (isset($a['task'])) {
        self::$tasks[] = $function;
      }
      if (isset($a['before'])) {
        self::$before[$a['before']][] = $function;
      }
      if (isset($a['after'])) {
        foreach ($a['after'] as $after_command) {
          self::$before[$after_command][] = $function;
        }
      }
    }
    return $tasks;
  }

  private static function getFunctionAnnotations($functions) {
    foreach ($functions as $f) {
      $ref = new \ReflectionFunction($f);
      if (!isset(self::$annotations[$f])) {
        self::$annotations[$f] = self::parseAnnotations($ref->getDocComment());
      }
    }
    return self::$annotations;
  }

  private static function getClassAnnotations(\ReflectionClass $ref) {
    $class_name = $ref->getShortName();

    if (!isset(self::$annotations[$class_name])) self::$annotations[$class_name] = array();

    foreach ($ref->getMethods() as $method) {
      $method_name = $method->getName();
      if (!isset(self::$annotations[$class_name][$method_name])) {
        self::$annotations[$class_name][$method_name] = self::parseAnnotations($method->getDocComment());
      }
    }
    return self::$annotations;
  }

  /**
   * Stolen from phpunit.
   *
   * @param  string $docblock
   * @return array
   */
  private static function parseAnnotations($docblock) {
    $annotations = array();

    if (preg_match_all('/@(?P<name>[A-Za-z_-]+)(?:[ \t]+(?P<value>.*?))?[ \t]*\r?$/m', $docblock, $matches)) {
      $numMatches = count($matches[0]);

      for ($i = 0; $i < $numMatches; ++$i) {
        $annotations[$matches['name'][$i]][] = $matches['value'][$i];
      }
    }

    return $annotations;
  }
}
