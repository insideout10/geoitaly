<?php
/**
 * Provide drush commands.
 */

function geoitaly_drush_command() {

  $items = array();

  $items['gn-import-features'] = array(
    'callback' => 'geoitaly_drush_import_features',
    'aliases' => array('gnif')
  );

  $items['gn-import-hierarchy'] = array(
    'callback' => 'geoitaly_drush_import_hierarchy',
    'aliases' => array('gnih')
  );

  return $items;
}

function geoitaly_drush_import_features() {

  drupal_set_message(t('Importing features...'));

  geoitaly_update_geonames_data();

}

function geoitaly_drush_import_hierarchy() {

  drupal_set_message(t('Importing hierarchy...!'));

  geoitaly_drush_import_hierarchy();

}