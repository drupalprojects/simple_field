<?php
/**
 * @file
 * Page and form callbacks for various pages.
 */

/**
 * Page callback for the simple field details page.
 */
function simple_field_view($simplefield) {

  $type_info = $simplefield->getTypeInfo();
  $field_settings = $simplefield->getSettings('field');
  $instance_settings = $simplefield->getSettings('instance');


  $description = $instance_settings['description'];

  $fields = array(
    t('Field Text') => $field_settings['label'],
    t('Field Type') => $type_info['label'],
    t('Required') => $simplefield->required ? t('yes') : t('no'),
    t('Editable') => $simplefield->hasData() ? t('no (there is data stored in this field)') : t('yes'),
    t('Description') => !empty($description) ? $description : t('None'),
  );

  $fieldset = array(
    '#type' => 'fieldset',
  );

  foreach ($fields as $field_label => $field_value) {
    $fieldset[] = array(
      '#type' => 'item',
      '#title' => $field_label,
      '#markup' => $field_value,
    );
  }

  // Allowed Values:
  if (!empty($field_settings['allowed_values'])) {
    $fieldset['allowed-values'] = array(
      '#title' => t('Allowed Values'),
      '#theme' => 'item_list',
      '#items' => array(),
    );
    foreach ($field_settings['allowed_values'] as $allowed_value) {
      $fieldset['allowed-values']['#items'][] = check_plain($allowed_value);
    }
  }

  return $fieldset;
}

/**
 * Page callback for simple field add/edit/clone page.
 *
 * Using custom page callback to handle processing type information.
 */
function simple_field_add_page($type_info, $form_state = array()) {

  $entity = entity_create('simple_field', array(
    'type' => $type_info['type'],
  ));
  return entity_ui_get_form('simple_field', $entity, 'add', $form_state);
}

// ----------------------------------------------------------------------------
// Form functions for Entity API module UI controller.

/**
 * Add/Edit/Clone form entity API module.
 */
function simple_field_form($form, &$form_state, $entity, $op = 'edit') {

  $form = array('#tree' => TRUE);

  if (empty($entity->type)) {
    drupal_set_message(t('You must choose a type first'));
    return;
  }

  $form['type'] = array(
    '#type' => 'value',
    '#value' => $entity->type,
  );

  $form['type-item'] = array(
    '#type' => 'item',
    '#title' => t('Field Type'),
    '#markup' => $entity->getTypeInfo('label') . '<br /><em>' . $entity->getTypeInfo('help') . '</em>',
  );

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Simple Field Text'),
    '#required' => TRUE,
    '#default_value' => isset($entity->field_settings['label']) ? $entity->field_settings['label'] : '',
  );

  $form['required'] = array(
    '#type' => 'checkbox',
    '#title' => t('Required'),
    '#default_value' => $entity->required,
  );

  if ($subform = simple_field_type_form($entity)) {
    $form += $subform;
  }

  $form['instance_settings']['description'] = array(
    '#weight' => -100,
    '#type' => 'textarea',
    '#title' => t('Help Text'),
    '#default_value' => isset($simple field->instance_settings['description']) ? $simple field->instance_settings['description'] : '',
  );

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
    ),
  );
  return $form;
}

/**
 * Submit function for simple field entity form.
 */
function simple_field_form_submit($form, &$form_state) {
  global $user;

  $form_state['values']['field_settings']['label'] = $form_state['values']['label'];

  $simplefield = entity_ui_form_submit_build_entity($form, $form_state);

  if (!isset($simplefield->uid)) {
    $simplefield->uid = $user->uid;
  }

  $simplefield->save();

  if (!empty($simplefield->first_instance)) {
    $entity_type = $simplefield->first_instance['entity_type'];
    $bundle = $simplefield->first_instance['bundle'];
    $inst = simple_field_build_instance($entity_type, $bundle, $simplefield);
    field_create_instance($inst);
  }
  $form_state['redirect'] = 'admin/structure/simple-field';
}

// ----------------------------------------------------------------------------
// Form functions for Simple Field type selection page.

/**
 * Form function for the simple field type selection form.
 *
 * Form is used to select a type for the simple_field. Once the type has been
 * selected, the form redirects to the standard add form with the type in the
 * url.
 */
function simple_field_type_select_form($form, &$form_state) {
  $types = simple_field_type_info();
  $options = array();
  foreach ($types as $type => $type_info) {
    if (simple_field_type_access('create', $type)) {
      $options[$type] = $type_info['label'] . ($type_info['help'] ? ' - <em>' . $type_info['help'] . '</em>' : '');
    }
  }

  $form['type'] = array(
    '#title' => 'Simple Field Type',
    '#type' => 'radios',
    '#options' => $options,
    '#required' => TRUE,
  );

  $form['actions'] = array(
    '#type' => 'actions',
    'next' => array(
      '#type' => 'submit',
      '#value' => t('Next'),
    ),
  );

  return $form;

}

/**
 * Submit function for the simple field type selection form.
 */
function simple_field_type_select_form_submit($form, &$form_state) {
  $options = array();
  $path = 'admin/structure/simple-field/add/' . $form_state['values']['type'];

  // If there is a destination, we preserve it for the next page.
  if (isset($_GET['destination'])) {
    $options['query']['destination'] = $_GET['destination'];
    unset($_GET['destination']);
  }

  $form_state['redirect'] = array($path, $options);
}


// ----------------------------------------------------------------------------
// Form functions for Simple Field instance listing pages.

/**
 * Form callback for a simple_field overview form for a specific entity bundle.
 */
function simple_field_instance_overview_form($form, &$form_state, $entity_type, $bundle) {
  $form['#entity_type'] = $entity_type;
  $form['#bundle'] = $bundle = field_extract_bundle($entity_type, $bundle);

  $admin_path = _field_ui_bundle_admin_path($entity_type, $bundle);

  $simplefields = simple_field_entity_fields($entity_type, $bundle);
  $field_by_weight = array();
  foreach ($simplefields as $simplefield) {
    $field = $simplefield->getField();
    $instance = field_info_instance($entity_type, $field['field_name'], $bundle);
    $field_by_weight[] = array(
      'weight' => $instance['widget']['weight'],
      'simplefield' => $simplefield,
    );
  }
  uasort($field_by_weight, 'drupal_sort_weight');

  $form['weight'] = array('#tree' => TRUE);
  $rows = array();
  $i = 0;
  foreach ($field_by_weight as $info) {
    $simplefield = $info['simplefield'];

    $form['weight'][$simplefield->id] = array(
      '#type' => 'textfield',
      '#size' => 4,
      '#default_value' => $i++,
      '#attributes' => array(
        'class' => array('weight-item'),
      ),
    );

    $field = $simplefield->getField();
    $row = array(
      'data' => array(),
      'class' => array('draggable'),
    );

    $row['data'][] = check_plain($simplefield->label());
    $row['data'][] = check_plain($simplefield->getTypeInfo('label'));
    $row['data'][] = l(
      t('delete'),
      $admin_path . '/simple-fields/instance-remove/' . $simplefield->identifier(),
      array('query' => array(
        'token' => drupal_get_token('delete' . $entity_type . $bundle . $simplefield->identifier()),
      ))
    );
    $rows[$simplefield->id] = $row;

  }
  $form['#rows'] = $rows;

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Weight'),
    '#access' => !empty($rows),
  );

  $form['#headers'] = array(
    t('Label'),
    t('Type'),
    array(
      'data' => t('Operations'),
      'colspan' => 1,
    ),
  );

  $form['#attached']['drupal_add_tabledrag'][] = array('simple-field-instance-list', 'order', 'sibling', 'weight-item');

  return $form;
}

function simple_field_instance_overview_form_submit($form, &$form_state) {
  $entity_type = $form['#entity_type'];
  $bundle = $form['#bundle'];

  $settings = field_bundle_settings($entity_type, $bundle);
  $base = 0;
  if (isset($settings['extra_fields']['form']['simple_fields']['weight'])) {
    $base = $settings['extra_fields']['form']['simple_fields']['weight'];
  }

  if (!empty($form_state['values']['weight'])) {
    asort($form_state['values']['weight']);
    $max = count($form_state['values']['weight']);
    $max = (floor($max/10) + 1)*10;

    $i = 1;
    $simplefields = entity_load('simple_field', array_keys($form_state['values']['weight']));
    foreach ($form_state['values']['weight'] as $id => $weight) {
      $field = $simplefields[$id]->getField();
      $instance = field_info_instance($entity_type, $field['field_name'], $bundle);
      $instance['widget']['weight'] = $base + $i / $max;
      field_update_instance($instance);
      $i++;
    }
  }
}

/**
 * Theme function for simple field instance listing form.
 */
function theme_simple_field_instance_overview_form(&$variables) {
  $form = $variables['form'];
  $header = $form['#headers'];
  $rows = $form['#rows'];

  $header[] = t('Weight');
  foreach ($rows as $id => &$row) {
    $row['data'][] = drupal_render($form['weight'][$id]);
  }
  unset($row); // clear reference

  return theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'simple-field-instance-list',
    ),
    'empty' => t('No fields have been added yet.'),
  )) . drupal_render_children($form);
}


// ----------------------------------------------------------------------------
// Form functions for Simple Field instance adding page.

/**
 * Form callback for the add page that shows fields that can be added.
 */
function simple_field_instance_add_form($form, &$form_state, $entity_type, $bundle) {

  $bundle = field_extract_bundle($entity_type, $bundle);
  $instances = field_info_instances($entity_type, $bundle);
  $admin_path = _field_ui_bundle_admin_path($entity_type, $bundle);
  $simplefields = entity_load('simple_field');

  $headers = array(
    t('Label'),
    t('Type'),
    t('Operation'),
  );

  $rows = array();
  foreach ($simplefields as $simplefield) {
    $field = $simplefield->getField();
    $exists = isset($instances[$field['field_name']]);
    $row = array();

    $row[] = check_plain($simplefield->label());
    $row[] = check_plain($simplefield->getTypeInfo('label'));
    $row[] = $exists ?
      t('Already added') :
      l(t('Add this field'), $admin_path . '/simple-fields/instance-add/' . $simplefield->identifier(),
        array('query' => array(
          'token' => drupal_get_token('create' . $entity_type . $bundle . $simplefield->identifier()),
        ))
      );

    $rows[] = $row;
  }

  return array(
    '#theme' => 'table',
    '#header' => $headers,
    '#rows' => $rows,
    '#empty' => t('There are no fields to add. Please !create.', array('!create' => l(t('Create a field'), 'admin/structure/simple-field/choose-type'))),
  );
}

/**
 * Menu callback to add a field instance for a given bundle and field.
 */
function simple_field_instance_add_callback($entity_type, $bundle, $simplefield) {
  $bundle = field_extract_bundle($entity_type, $bundle);
  $admin_path = _field_ui_bundle_admin_path($entity_type, $bundle);
  if (isset($_GET['token']) && drupal_valid_token($_GET['token'], 'create' . $entity_type . $bundle . $simplefield->identifier())) {
    $instance = simple_field_build_instance($entity_type, $bundle, $simplefield);
    try {
      field_create_instance($instance);
      drupal_set_message(t('Field instance successfully added.'));
    }
    catch (FieldException $e) {
      drupal_set_message(t('The field instance was not create because there was an error.'), 'error');
    }
  }
  else {
    drupal_set_message(t('There was an error processing the request.'), 'error');
  }
  drupal_goto($admin_path . '/simple-fields');
}

/**
 * Menu callback to remove a field instance from a given bundle.
 */
function simple_field_instance_delete_callback($entity_type, $bundle, $simplefield) {
  $bundle = field_extract_bundle($entity_type, $bundle);
  $admin_path = _field_ui_bundle_admin_path($entity_type, $bundle);
  if (isset($_GET['token']) && drupal_valid_token($_GET['token'], 'delete' . $entity_type . $bundle . $simplefield->identifier())) {
    $field = $simplefield->getField();
    $instance = field_info_instance($entity_type, $field['field_name'], $bundle);
    field_delete_instance($instance, FALSE);
    drupal_set_message(t('Field instance successfully removed.'));
  }
  else {
    drupal_set_message(t('There was an error processing the request.'), 'error');
  }
  drupal_goto($admin_path . '/simple-fields');
}

/**
 * Form callback for type selection page for specific bundle.
 */
function simple_field_add_with_instance_type_select_form($form, &$form_state, $entity_type, $bundle) {
  $bundle = field_extract_bundle($entity_type, $bundle);
  $form['entity_type'] = array(
    '#type' => 'value',
    '#value' => $entity_type,
  );
  $form['bundle'] = array(
    '#type' => 'value',
    '#value' => $bundle,
  );

  return simple_field_type_select_form($form, $form_state);
}

/**
 * Submit handler for type selection for specific bundle.
 */
function simple_field_add_with_instance_type_select_form_submit($form, &$form_state) {
  $entity_info = entity_get_info($form_state['values']['entity_type']);
  $bundle_info = $entity_info['bundles'][$form_state['values']['bundle']];

  $options = array();
  $path = $bundle_info['admin']['real path'] . '/simple-fields/add/' . $form_state['values']['type'];

  // If there is a destination, we preserve it for the next page.
  if (isset($_GET['destination'])) {
    $options['query']['destination'] = $_GET['destination'];
    unset($_GET['destination']);
  }
  else {
    $options['query']['destination'] = $bundle_info['admin']['real path'] . '/simple-fields';
  }

  $form_state['redirect'] = array($path, $options);
}

/**
 * Page callback for entity creation page including first instance.
 */
function simple_field_add_with_instance_page($entity_type, $bundle, $type_info, $form_state = array()) {
  $bundle = field_extract_bundle($entity_type, $bundle);

  $entity = entity_create('simple_field', array(
    'type' => $type_info['type'],
    'first_instance' => array(
      'entity_type' => $entity_type,
      'bundle' => $bundle,
    ),
  ));
  return entity_ui_get_form('simple_field', $entity, 'add', $form_state);
}

