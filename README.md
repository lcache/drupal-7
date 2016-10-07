# LCache

This module provides a combination L1/L2 cache using a combination
of APCu as L1 with a database as L2 (and for coherency management
among the L1 caches).

Currently only supported on Pantheon, but there's nothing that
inherently relies on anything Pantheon-specific.

Upstream library: https://github.com/lcache/lcache

## Usage

 1. Upload the module to your site.
     a. If using Composer to manage your site's modules:

        composer require drupal/lcache

     b. If not using Composer:

        drush dl lcache
        cd sites/all/modules/lcache
        composer install --no-dev

     c. If you are pushing to a remote location (e.g. Pantheon) you will need to
        force-commit the 'vendor' directory so that the core lcache library is
        pushed along with the module:

        git add --force vendor
        git push origin master

 2. Install the module (so Drupal creates the database schema).
 3. Configure `settings.php` to use the cache:

        $conf['cache_backends'][] = 'sites/all/modules/lcache/lcache.cache.inc';
        $conf['cache_default_class'] = 'LCache';
        // The 'cache_form' bin must be assigned to non-volatile storage.
        // And if you use page cache, you probably don't want it in LCache.
        $conf['cache_class_cache_form'] = 'DrupalDatabaseCache';
        $conf['cache_class_cache_page'] = 'DrupalDatabaseCache';

### With Composer Manager

Composer Manager is not officially supported for use with LCache, but there
are reports of it working properly by adding the following early (before LCache
is included) in `settings.php`:

    require_once DRUPAL_ROOT . '/sites/all/modules/composer_manager/composer_manager.module';
    composer_manager_register_autoloader();
