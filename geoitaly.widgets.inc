<?php

/**
 * Provide the widget for the GeoNames feature field.
 *
 * @since 1.0.0
 * @return array Return an array with the GeoNames hierarchical selector form.
 */
function geoitaly_field_widget_info() {

  return array(
    'geoitaly_geonames_feature' => array(
      'label'       => t('GeoNames hierarchical selector'),
      'field types' => array('geonames_feature'),
    )
  );
}

/**
 * This function is called by Drupal to display the GeoNames hierarchical selector form.
 *
 * @since 1.0.0
 */
function geoitaly_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

  // Get the field name.
  $field_name = $instance['field_name'];

  // Preset the values to NULL (not set either manually by the user nor in the db).
  $values = array(
    'ADM1' => NULL,
    'ADM2' => NULL,
    'ADM3' => NULL,
  );

  // If values have been set by the user then load them.
  if (isset($form_state['values'])) {
    foreach ($values as $key => $value) {
      $values[$key] = $form_state['values'][$field_name][LANGUAGE_NONE][0]['adm'][$key];
    }
  }
  // Else if values have been loaded from the database, get them.
  elseif (!empty($items)) {

    // Get the value set.
    $parent_id = $items[0]['value'];

    // Load the parents until we find none.
    while (!empty($row = db_select('geoitaly', 'g')
      ->fields('g', array('geonames_id', 'parent_id', 'feature_code'))
      ->condition('geonames_id', $parent_id, '=')
      ->execute()
      ->fetchAssoc())) {

      // Load the data into the values array.
      $parent_id             = $row['parent_id'];
      $feature_code          = $row['feature_code'];
      $values[$feature_code] = $row['geonames_id'];
    }

  }


  dd("geoitaly_field_widget_form [ field name :: $field_name ][ adm1 :: {$values['ADM1']}  ][ adm2 :: {$values['ADM2']} ][ adm3 :: {$values['ADM3']} ]");

  // Generate a unique fieldset id that we'll use to replace dynamically the content
  // when the user selects a value.
  $element_id = uniqid('adm-');

  // Set a custom call back to reduce the multiple values set here to one single
  // value.
  // ATTN: the order of the fields is very important here, as the validate function
  // expects them to be from the least detailed to the most details (adm1 > adm2 > adm3).
  $element['#element_validate'] = array('_geoitaly_widget_validate');

  // Create the fieldset container, using the unique element id as id.
  $element['adm'] = array(
    '#type'       => 'fieldset',
    '#title'      => $element['#title'],
    '#attributes' => array('id' => $element_id)
  );

  // Prepare the ajax callback for adm1 and adm2.
  $ajax_callback = array(
    'event'    => 'change',
    'callback' => '_geoitaly_adm_selected',
    'wrapper'  => $element_id,
    'method'   => 'replace',
  );

  // The region.
  $element['adm']['ADM1'] = array(
    '#type'          => 'select',
    '#title'         => t('Region'),
    '#description'   => t('Select the region, if any'),
    '#options'       => _geoitaly_options('ADM1', GEOITALY_COUNTRY_PARENT_ID),
    '#default_value' => $values['ADM1'],
    '#ajax'          => $ajax_callback
  );

  // The province.
  $element['adm']['ADM2'] = array(
    '#type'          => 'select',
    '#title'         => t('Province'),
    '#description'   => t('Select the province, if any'),
    '#options'       => _geoitaly_options('ADM2', $values['ADM1']),
    '#default_value' => $values['ADM2'],
    '#ajax'          => $ajax_callback
  );

  // The town.
  $element['adm']['ADM3'] = array(
    '#type'          => 'select',
    '#title'         => t('Town'),
    '#description'   => t('Select the town, if any'),
    '#options'       => _geoitaly_options('ADM3', $values['ADM2']),
    '#default_value' => $values['ADM3']
  );

  return $element;
}

/**
 * Validate the widget entry, by reducing the 3 optionally selected values (adm1/2/3)
 * and returning the most detailed. This function is called by Drupal using the '#element_validate'
 * attribute on the main widget element in {@see geoitaly_field_widget_form}.
 *
 * @since 1.0.0
 *
 * @param array $element The element data containing the field name.
 * @param array $form_state The form state containing the set values.
 * @param array $form The form configuration.
 */
function _geoitaly_widget_validate($element, &$form_state, $form) {

  // Get the field name.
  $field_name = $element['#field_name'];

  // Get the values set for adm1/2/3.
  $values = array_values($form_state['values'][$field_name][LANGUAGE_NONE][0]['adm']);

  // Set the value by iterating into the values array and returning the lower value set,
  // starting with the default value for the country.
  $form_state['values'][$field_name][LANGUAGE_NONE][0]['value'] = array_reduce($values, function ($carry, $item) {
    return $item ?: $carry;
  }, GEOITALY_COUNTRY_PARENT_ID);

}

/**
 * When an ADM select is changed, update the form accordingly.
 *
 * @since 1.0.0
 *
 * @param array $form The form configuration.
 * @param array $form_state The form state.
 *
 * @return mixed The form field to update.
 */
function _geoitaly_adm_selected($form, $form_state) {

  // Get the field name to update.
  $field_name = $form_state['triggering_element']['#parents'][0];

  return $form[$field_name];
}

/**
 * Load the options by selecting the features matching the specified feature code
 * and parent id.
 *
 * @param string $feature_code The feature code, e.g. ADM1.
 * @param number|NULL $parent_id The feature parent id. If NULL, an empty list is returned.
 *
 * @return array An array of options where the key is the GeoNames ID and the value is the name.
 */
function _geoitaly_options($feature_code, $parent_id = NULL) {

  dd("_geoitaly_options [ feature code :: $feature_code ][ parent id :: $parent_id ]");

  $options = array('' => '-');

  // Return an empty array if the parent id hasn't been selected.
  if (NULL === $parent_id) {
    return $options;
  }

  $rows = db_select('geoitaly', 'g')
    ->fields('g', array('geonames_id', 'name'))
    ->condition('feature_class', 'A', '=')
    ->condition('feature_code', $feature_code, '=')
    ->condition('parent_id', $parent_id, '=')
    ->orderBy('name')
    ->execute();

  foreach ($rows as $row) {
    $options[$row->geonames_id] = $row->name;
  }

  return $options;
}
