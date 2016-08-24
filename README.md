# LCache

This module provides a combination L1/L2 cache using a combination
of APCu as L1 with a database as L2 (and for coherency management
among the L1 caches).

Currently only supported on Pantheon, but there's nothing that
inherently relies on anything Pantheon-specific.

See also: https://github.com/lcache/lcache

## Usage

### Drupal 7

 1. Upload the module to your site.
    a. If using Composer to manage your sites modules:  
       composer require drupal/lcache
    b. If not using Composer:
       drush dl lcache
       cd sites/all/modules/lcache
       composer install
 2. Install the module (so Drupal creates the schema).
 3. Configure `settings.php` to use the cache:

    $conf['cache_backends'][] = 'sites/all/modules/lcache/lcache.cache.inc';
    $conf['cache_default_class'] = 'LCache';

### Drupal 8

Coming soon.

### WordPress

Coming soon.

## Testing (on Fedora)

 1. Install packages:

    sudo dnf install -y php-cli php-phpunit-PHPUnit php-phpunit-DbUnit php-pecl-apcu

 2. Enable APCu caching for the CLI:

    echo "apc.enable_cli=1" | sudo tee -a /etc/php.d/40-apcu.ini

 3. From the "lcache" directory:

    composer install
    composer test
