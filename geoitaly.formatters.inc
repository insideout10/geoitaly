<?php

/**
 * Set up the GeoNames feature formatter.
 *
 * @since 1.0.0
 *
 * @return array An array of formatters.
 */
function geoitaly_field_formatter_info() {

  return array(
    'geonames_feature_formatter' => array(
      'label'       => t('GeoNames feature formatter'),
      'field types' => array('geonames_feature'),
    )
  );
}

/**
 * Prepare the GeoNames feature value for display, by iterating its parent and
 * creating a string like 'Italy > Region > Province > Town'.
 *
 * @since 1.0.0
 */
function geoitaly_field_formatter_prepare_view($entity_type, $entities, $field, $instances, $langcode, &$items, $displays) {

  // Set the default prefix.
  $prefix = t('Italy');

  // Collect every possible term attached to any of the fieldable entities.
  foreach ($entities as $id => $entity) {
    foreach ($items[$id] as $delta => $item) {

      $parent_id = $item;

      $suffix = '';

      while (!empty($row = db_select('geoitaly', 'g')
        ->fields('g', array('parent_id', 'name', 'feature_code'))
        ->condition('geonames_id', $parent_id, '=')
        ->execute()
        ->fetchAssoc())) {

        $suffix    = ' > ' . $row['name'] . $suffix;
        $parent_id = $row['parent_id'];
      }

      $items[$id][0]['value'] = $prefix . $suffix;

    }

  }

}

/**
 * Display the field value. Values are preloaded in the {@see geoitaly_field_formatter_prepare_view} function.
 *
 * @since 1.0.0
 */
function geoitaly_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {

  $element = array();

  foreach ($items as $delta => $item) {
    $element[$delta] = array(
      '#markup' => $item['value'],
    );
  }

  return $element;
}