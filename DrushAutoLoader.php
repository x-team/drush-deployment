<?php

/*
 * This file is part of the Symfony package.
 *
 * (c) Fabien Potencier <fabien@symfony.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

/**

 * @api
 */
class DrushAutoLoader {
  private $namespaces = array();
  private $prefixes = array();
  private $namespaceFallbacks = array();
  private $prefixFallbacks = array();
  private $cache_prefix;

  private static $apc = NULL;


  /**
   * Constructor.
   *
   * @param string $prefix A prefix to create a namespace in APC
   *
   * @api
   */
  public function __construct($cache_prefix = '') {
    if (!isset(self::$apc)) {
      self::$apc = extension_loaded('apc');
    }
    $this->prefix = $cache_prefix;
  }

  /**
   * Gets the configured namespaces.
   *
   * @return array A hash with namespaces as keys and directories as values
   */
  public function getNamespaces()
  {
    return $this->namespaces;
  }

  /**
   * Gets the configured class prefixes.
   *
   * @return array A hash with class prefixes as keys and directories as values
   */
  public function getPrefixes()
  {
    return $this->prefixes;
  }

  /**
   * Gets the directory(ies) to use as a fallback for namespaces.
   *
   * @return array An array of directories
   */
  public function getNamespaceFallbacks()
  {
    return $this->namespaceFallbacks;
  }

  /**
   * Gets the directory(ies) to use as a fallback for class prefixes.
   *
   * @return array An array of directories
   */
  public function getPrefixFallbacks()
  {
    return $this->prefixFallbacks;
  }

  /**
   * Registers the directory to use as a fallback for namespaces.
   *
   * @param array $dirs An array of directories
   *
   * @api
   */
  public function registerNamespaceFallbacks(array $dirs)
  {
    $this->namespaceFallbacks = $dirs;
  }

  /**
   * Registers the directory to use as a fallback for class prefixes.
   *
   * @param array $dirs An array of directories
   *
   * @api
   */
  public function registerPrefixFallbacks(array $dirs)
  {
    $this->prefixFallbacks = $dirs;
  }

  /**
   * Registers an array of namespaces
   *
   * @param array $namespaces An array of namespaces (namespaces as keys and locations as values)
   *
   * @api
   */
  public function registerNamespaces(array $namespaces)
  {
    foreach ($namespaces as $namespace => $locations) {
      $this->namespaces[$namespace] = (array) $locations;
    }
  }

  /**
   * Registers a namespace.
   *
   * @param string       $namespace The namespace
   * @param array|string $paths     The location(s) of the namespace
   *
   * @api
   */
  public function registerNamespace($namespace, $paths) {
    $this->namespaces[$namespace] = (array)$paths;
  }

  /**
   * Registers an array of classes using the PEAR naming convention.
   *
   * @param array $classes An array of classes (prefixes as keys and locations as values)
   *
   * @api
   */
  public function registerPrefixes(array $classes)
  {
    foreach ($classes as $prefix => $locations) {
      $this->prefixes[$prefix] = (array) $locations;
    }
  }

  /**
   * Registers a set of classes using the PEAR naming convention.
   *
   * @param string       $prefix  The classes prefix
   * @param array|string $paths   The location(s) of the classes
   *
   * @api
   */
  public function registerPrefix($prefix, $paths)
  {
    $this->prefixes[$prefix] = (array) $paths;
  }

  /**
   * Registers this instance as an autoloader.
   *
   * @param Boolean $prepend Whether to prepend the autoloader or not
   *
   * @api
   */
  public function register($prepend = false) {
    if (!isset(self::$apc)) {
      self::$apc = extension_loaded('apc');
    }
    spl_autoload_register(array($this, 'loadClass'), true, $prepend);
  }

  /**
   * Loads the given class or interface.
   *
   * @param string $class The name of the class
   */
  public function loadClass($class) {
    if ($file = $this->findFileCached($class)) {
      require $file;
    }
  }

  /**
   * Finds a file by class name while caching lookups to APC.
   *
   * @param string $class A class name to resolve to file
   */
  public function findFileCached($class) {
    if (self::$apc) {
      if (FALSE === $file = apc_fetch($this->prefix.$class)) {
        apc_store($this->prefix.$class, $file = $this->findFile($class));
      }
    }
    else {
      $file = $this->findFile($class);
    }

    return $file;
  }

  /**
   * Finds the path to the file where the class is defined.
   *
   * @param string $class The name of the class
   *
   * @return string|null The path, if found
   */
  public function findFile($class) {
    if ('\\' == $class[0]) {
      $class = substr($class, 1);
    }

    if (false !== $pos = strrpos($class, '\\')) {
      // namespaced class name
      $namespace = substr($class, 0, $pos);
      foreach ($this->namespaces as $ns => $dirs) {
        foreach ($dirs as $dir) {
          if (0 === strpos($namespace, $ns)) {
            $className = substr($class, $pos + 1);
            $file = $dir.DIRECTORY_SEPARATOR.str_replace('\\', DIRECTORY_SEPARATOR, $namespace).DIRECTORY_SEPARATOR.str_replace('_', DIRECTORY_SEPARATOR, $className).'.php';
            if (file_exists($file)) {
              return $file;
            }
          }
        }
      }

      foreach ($this->namespaceFallbacks as $dir) {
        $file = $dir.DIRECTORY_SEPARATOR.str_replace('\\', DIRECTORY_SEPARATOR, $class).'.php';
        if (file_exists($file)) {
          return $file;
        }
      }
    }
    else {
      // PEAR-like class name
      foreach ($this->prefixes as $prefix => $dirs) {
        foreach ($dirs as $dir) {
          if (0 === strpos($class, $prefix)) {
            $file = $dir.DIRECTORY_SEPARATOR.str_replace('_', DIRECTORY_SEPARATOR, $class).'.php';
            if (file_exists($file)) {
              return $file;
            }
          }
        }
      }

      foreach ($this->prefixFallbacks as $dir) {
        $file = $dir.DIRECTORY_SEPARATOR.str_replace('_', DIRECTORY_SEPARATOR, $class).'.php';
        if (file_exists($file)) {
          return $file;
        }
      }
    }
  }
}

