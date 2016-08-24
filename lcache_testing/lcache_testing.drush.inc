<?php

/**
 * Implements hook_drush_command().
 */
function lcache_testing_drush_command() {

  $items['lcache-test'] = array(
    'description' => 'Run some tests - the ugly way.',
  );

  return $items;
}

/**
 * Callback for the drush-demo-command command
 */
function drush_lcache_testing_lcache_test() {
  $apcuTest = new LCacheTesting();
  $methods = get_class_methods($apcuTest);
  
  $passed = 0;
  $failed = 0;
  $tests = array();
  foreach ($methods as $method) {
    if (substr($method, 0, 4) === 'test') {
      // Get the test name in a more readable format.
      $test_name = preg_split('/(?=[A-Z])/', $method);
      $test_name = implode(' ', $test_name);

      if ($apcuTest->$method()) {
        $tests['passed'][] = $test_name;
        $passed++;
        print(".");
      }
      else {
        $tests['failed'][] = $test_name;
        $failed++;
        drush_set_error("TEST FAILED: " . $method . " - " . $test_name);
      }
    }  
  }

  drush_print('');
  drush_print('Tests PASSED: ' . $passed);
  drush_print('Tests FAILED: ' . $failed);

  if (!empty($tests['passed']) && is_array($tests['passed'])) {
    drush_print('');
    drush_print('PASSED:');
    foreach ($tests['passed'] as $test) {
      drush_print('  - ' . $test);
    }
  }

  if (!empty($tests['failed']) && is_array($tests['failed'])) {
    drush_print('');
    drush_print('FAILED:');
    foreach ($tests['failed'] as $test) {
      drush_print('  - ' . $test);
    }
  }
}


class LCacheTesting {

  private $_bin = 'cache_testing';

  private $_cache = NULL;

  public function __construct() {
    $this->_cache = new LCache($this->_bin, TRUE);
  }

  public function __destruct() {
    //$this->_resetCache();
  }

  /**
   * Reset cache.
   */
  private function _resetCache($cid, $apc=true, $db=true) {
    if ($apc) {
      apcu_delete('lcache_' . $this->_bin . ':' . $cid);
    }
    if ($db) {
      db_query('TRUNCATE TABLE cache_testing');
    }
  }

  /**
   * Test get string from APC
   */
  public function testGetStringFromApcu() {
    $cid = 'apcu_testGetFromApcu';
    $data = 'apcu_testGetFromApcu';

    $this->_resetCache($cid);

    $this->_cache->set($cid, $data);
    $data_test = $this->_cache->get($cid);

    $return = ($data == $data_test->data && $data_test->source == 'apcu') ? TRUE : FALSE;
    return $return;
  }

  /**
   * Test get array from APC
   */
  public function testGetArrayFromApcu() {
    $cid = 'apcu_testGetFromApcu';
    $data = array(1,2,3,4,5,6,7,8);

    $this->_resetCache($cid);

    $this->_cache->set($cid, $data);
    $data_test = $this->_cache->get($cid);

    $return = ($data == $data_test->data && $data_test->source == 'apcu') ? TRUE : FALSE;
    return $return;
  }

  /**
   * Test get multiple strings from APC
   */
  public function testGetMultipleStringsFromApcu() {
    $cache_entries = array(
      'apcu_a' => md5('apcu_a'),
      'apcu_b' => md5('apcu_b'),
      'apcu_c' => md5('apcu_c'),
    );

    $this->_resetCache('apcu_a');
    $this->_resetCache('apcu_b');
    $this->_resetCache('apcu_c');
    
    foreach ($cache_entries as $cid => $data) {
      $this->_cache->set($cid, $data);
    }

    $cids = array('apcu_a', 'apcu_b', 'apcu_c');
    $data_test = $this->_cache->getMultiple($cids);

    $result = ($data_test['apcu_a']->data == $cache_entries['apcu_a'] && $data_test['apcu_a']->source == 'apcu') &&
              ($data_test['apcu_b']->data == $cache_entries['apcu_b'] && $data_test['apcu_b']->source == 'apcu') &&
              ($data_test['apcu_c']->data == $cache_entries['apcu_c'] && $data_test['apcu_c']->source == 'apcu');

    return $result;
  }

  /**
   * Test get multiple arrays from APCu
   */
  public function testGetMultipleArraysFromApcu() {
    $cache_entries = array(
      'apcu_a' => array(1,2,3),
      'apcu_b' => array(4,5,6),
      'apcu_c' => array(7,8,9),
    );

    $this->_resetCache('apcu_a');
    $this->_resetCache('apcu_b');
    $this->_resetCache('apcu_c');
    
    foreach ($cache_entries as $cid => $data) {
      $this->_cache->set($cid, $data);      
    }

    $cids = array('apcu_a', 'apcu_b', 'apcu_c');
    $data_test = $this->_cache->getMultiple($cids);

    $result = ($data_test['apcu_a']->data == $cache_entries['apcu_a'] && $data_test['apcu_a']->source == 'apcu') &&
              ($data_test['apcu_b']->data == $cache_entries['apcu_b'] && $data_test['apcu_b']->source == 'apcu') &&
              ($data_test['apcu_c']->data == $cache_entries['apcu_c'] && $data_test['apcu_c']->source == 'apcu');

    return $result;
  }

  /**
   * Test get string from DB
   */
  public function testGetStringFromDb() {
    $cid = 'apcu_testStringGetFromDb';
    $data = 'apcu_testStringGetFromDb';

    $this->_resetCache($cid);

    $this->_cache->set($cid, $data);
    $this->_resetCache($cid, true, false);
    $data_test = $this->_cache->get($cid);

    $return = ($data == $data_test->data && $data_test->source == 'db') ? TRUE : FALSE;
    return $return;
  }

  /**
   * Test get array from Db
   */
  public function testGetArrayFromDb() {
    $cid = 'apcu_testGetFromDb';
    $data = array(1,2,3,4,5,6,7,8);

    $this->_resetCache($cid);

    $this->_cache->set($cid, $data);
    $this->_resetCache($cid, true, false);
    $data_test = $this->_cache->get($cid);

    $return = ($data == $data_test->data && $data_test->source == 'db') ? TRUE : FALSE;
    return $return;
  }

   /**
   * Test get multiple strings from Db
   */
  public function testGetMultipleStringsFromDb() {
    $cache_entries = array(
      'apcu_a' => md5('apcu_a'),
      'apcu_b' => md5('apcu_b'),
      'apcu_c' => md5('apcu_c'),
    );

    $this->_resetCache('apcu_a');
    $this->_resetCache('apcu_b');
    $this->_resetCache('apcu_c');
    
    foreach ($cache_entries as $cid => $data) {
      $this->_cache->set($cid, $data);      
    }

    $this->_resetCache('apcu_a', true, false);
    $this->_resetCache('apcu_b', true, false);
    $this->_resetCache('apcu_c', true, false);

    $cids = array('apcu_a', 'apcu_b', 'apcu_c');
    $data_test = $this->_cache->getMultiple($cids);

    $result = ($data_test['apcu_a']->data == $cache_entries['apcu_a'] && $data_test['apcu_a']->source == 'db') &&
              ($data_test['apcu_b']->data == $cache_entries['apcu_b'] && $data_test['apcu_b']->source == 'db') &&
              ($data_test['apcu_c']->data == $cache_entries['apcu_c'] && $data_test['apcu_c']->source == 'db');

    return $result;
  }

  /**
   * Test get multiple arrays from Db
   */
  public function testGetMultipleArraysFromDb() {
    $cache_entries = array(
      'apcu_a' => array(1,2,3),
      'apcu_b' => array(4,5,6),
      'apcu_c' => array(7,8,9),
    );

    $this->_resetCache('apcu_a');
    $this->_resetCache('apcu_b');
    $this->_resetCache('apcu_c');
    
    foreach ($cache_entries as $cid => $data) {
      $this->_cache->set($cid, $data);      
    }

    $this->_resetCache('apcu_a', true, false);
    $this->_resetCache('apcu_b', true, false);
    $this->_resetCache('apcu_c', true, false);

    $cids = array('apcu_a', 'apcu_b', 'apcu_c');
    $data_test = $this->_cache->getMultiple($cids);

    $result = ($data_test['apcu_a']->data == $cache_entries['apcu_a'] && $data_test['apcu_a']->source == 'db') &&
              ($data_test['apcu_b']->data == $cache_entries['apcu_b'] && $data_test['apcu_b']->source == 'db') &&
              ($data_test['apcu_c']->data == $cache_entries['apcu_c'] && $data_test['apcu_c']->source == 'db');
              
    return $result;
  }

  /**
   * Test get an updated string from APCu after sync.
   */
  public function testGetUpdatedStringFromApcuAfterSync() {
    //$this->_resetCache();

    $cid = 'apcu_testGetUpdatedString';
    $data = 'apcu_testGetUpdatedString_a';
    $this->_cache->set($cid, $data);

    // Simulate a cache update by adding an entry in lcache_events for the same $cid with new/updated data.
    // run lcache_synchronize() to sync the local APCu with the Db
    // Get the entry and check if the data has been updated.

    $data = 'apcu_testGetUpdatedString_b';
    $fields = array(
      'pool' => lcache_cache_get_pool(),
      'bin' => $this->_bin,
      'cid' => $cid,
      'created' => REQUEST_TIME,
      'expire' => CACHE_PERMANENT,
      'data' => serialize($data),
    );

    try {
      $eid = db_insert('lcache_events')
                      ->fields($fields)
                      ->execute();

      //print("============ Insert EID: $eid ===============\n\n");
    }
    catch (Exception $e) {
      // The database may not be available, so just return the NULL $eid.
      drush_set_error($e->getMessage());
    }

    $eid = apcu_fetch('lcache_eid_high_water_mark', $success);
    if (!$success) {
      $eid = 0;
    }
    //print("============ APCu EID before sync: $eid ===============\n\n");

    lcache_synchronize(TRUE);

    $eid = apcu_fetch('lcache_eid_high_water_mark', $success);
    if (!$success) {
      $eid = 0;
    }
    //print("============ APCu EID after sync: $eid ===============\n\n");
    
    $data_test = $this->_cache->get($cid);
    //print_r($data_test);

    $return = ($data == $data_test->data && $data_test->source == 'apcu') ? TRUE : FALSE;
    return $return;
  }

}