<?php

if (!class_exists('\LCache\NullL1')) {
  require_once 'vendor/autoload.php';
}

function lcache_get_pdo_handle() {
  static $dbh = NULL;
  if (is_null($dbh)) {
    $dsn = 'mysql:host='. $_ENV['DB_HOST']. ';port='. $_ENV['DB_PORT'] .';dbname='. $_ENV['DB_NAME'];
    $options = array(PDO::ATTR_TIMEOUT => 2, PDO::MYSQL_ATTR_INIT_COMMAND => 'SET sql_mode="ANSI_QUOTES"');
    $dbh = new PDO($dsn, $_ENV['DB_USER'], $_ENV['DB_PASSWORD'], $options);
    $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  }
  return $dbh;
}

/**
 * Defines an L1/L2-based cache implementation.
 *
 * This cache is based on local APCu data stores and the database.
 * Every time a cache item gets updated, we write it to the local cache and the DB.
 */
class LCache implements DrupalCacheInterface {

  /**
   * The cache bin.
   */
  protected $bin;

  /**
   * The integrated L1/L2 cache implementation.
   */
  protected $integrated;

  /**
   * Constructs an LCache object.
   *
   * @param $bin
   *   The cache bin for which the object is created. This will be mostly for the DB cache.
   */
  public function __construct($bin=NULL) {
    $this->bin = $bin;

    // Use the Null L1 cache for the CLI.
    $l1 = new \LCache\NullL1();
    if (php_sapi_name() !== 'cli') {
      $l1 = new \LCache\APCuL1();
    }

    $l2 = new \LCache\DatabaseL2(lcache_get_pdo_handle());
    //$l2 = new \LCache\StaticL2();

    $this->integrated = new \LCache\Integrated($l1, $l2, 100);
  }

  protected function getAddress($cid=NULL) {
    return new \LCache\Address($this->bin, $cid);
  }

  /**
   * Implements DrupalCacheInterface::get().
   */
  public function get($cid) {
    $address = $this->getAddress($cid);
    $entry = $this->integrated->getEntry($address);

    if (is_null($entry)) {
      return FALSE;
    }

    $response = new stdClass();
    $response->cid = $cid;
    $response->data = $entry->value;
    $response->created = $entry->created;
    $response->expire = $entry->expiration;
    return $response;
  }

  /**
   * Implements DrupalCacheInterface::getMultiple().
   */
  public function getMultiple(&$cids) {
    $cache = array();
    foreach ($cids as $cid) {
      $c = $this->get($cid);
      if (!empty($c)) {
        $cache[$cid] = $c;
      }
    }
    $cids = array_diff($cids, array_keys($cache));
    return $cache;
  }

  /**
   * Implements DrupalCacheInterface::set().
   */
  public function set($cid, $data, $expire = CACHE_PERMANENT) {
    $address = $this->getAddress($cid);
    $ttl = NULL;
    if ($expire === CACHE_TEMPORARY) {
      $ttl = 86400;  // @TODO: Use a configurable value.
    }
    else if ($expire !== CACHE_PERMANENT) {
      $ttl = $expire - REQUEST_TIME;
    }
    $this->integrated->set($address, $data, $ttl);
  }

  /**
   * Implements DrupalCacheInterface::clear().
   */
  public function clear($cid = NULL, $wildcard = FALSE) {
    if (empty($cid)) {
      // Flush expired, temporary cache entries now.
      $this->integrated->collectGarbage();
    }
    else {
      if ($wildcard) {
        $address = $this->getAddress();
        $this->integrated->delete($address);
      }
      elseif (is_array($cid)) {
        foreach ($cid as $cid_entry) {
          $address = $this->getAddress($cid_entry);
          $this->integrated->delete($address);
        }
      }
      else {
        $address = $this->getAddress($cid);
        $this->integrated->delete($address);
      }
    }
  }

  /**
   * Implements DrupalCacheInterface::isEmpty().
   */
  public function isEmpty() {
    return FALSE;
  }

  /**
   * Garbage collection for get() and getMultiple().
   *
   * @param $bin
   *   The bin being requested.
   */
  protected function garbageCollection() {
  }

  public function synchronize() {
    $this->integrated->synchronize();
  }
}

$lcache = new LCache();
$applied = $lcache->synchronize();
header('LCache-Changes-Applied: ' . $applied ? $applied : 'NULL');
