<?php
/**
 * @file
 * Simplefield classes and helper functions used along with the classes.
 *
 * This file is autoloaded, so functions here can only be used once one of the
 * classes has been instantiated.
 */

/**
 * Entity class for Simple Fields.
 */
class SimpleFieldEntity extends Entity {

  // Standard db-backed properties.
  public $id;
  public $type;
  public $field_id;
  public $uid;
  public $field_settings;
  public $instance_settings;
  public $widget_settings;
  public $formatter_settings;
  public $required;

  /**
   * Override default entity label with field's label.
   */
  function defaultLabel() {
    return isset($this->field_settings['label']) ? $this->field_settings['label'] : NULL;
  }

  function defaultUri() {
    return array(
      'path' => 'admin/structure/simple-field/' . $this->identifier(),
      'options' => array(),
    );
  }

  /**
   * Get the type information for this field.
   */
  function getTypeInfo($key = NULL) {
    $info = simple_field_type_info($this->type);
    if (isset($key)) {
      return isset($info[$key]) ? $info[$key] : NULL;
    }
    return $info;
  }

  /**
   * Get the label for this field's type.
   */
  function getTypeLabel() {
    $typeinfo = $this->getTypeInfo();
    return $typeinfo['label'];
  }

  /**
   * Get the settings for this field.
   *
   * @param setting_type
   *   The type of setting ('field', 'instance', 'widget', 'formatter').
   *
   * @return
   *   An array of the given type.
   */
  function getSettings($setting_type) {

    $settings = array();
    // Add settings from the question type, but don't allow question to override
    // the question type settings.
    $setting_key = $setting_type . '_settings';

    if ($this->$setting_key) {
      $settings += $this->$setting_key;
    }

    if ($type_info = $this->getTypeInfo()) {
      $settings += $type_info['defaults'][$setting_key];
    }

    return $settings;
  }

  /**
   * Get the Field API field for this field.
   */
  function getField() {
    return $this->field_id ? field_info_field_by_id($this->field_id) : FALSE;
  }

  /**
   * Get a list of all instances of this field.
   *
   * @param $entity_type
   *   Filter returned instances by entity type.
   * @param $bundle
   *   Filter returned instances by bundle.
   *
   * @return
   *   An array of field instances.
   */
  function getInstances($entity_type = NULL, $bundle = NULL) {
    $field = $this->getField();
    $field_name = $field['field_name'];
    $instances = field_info_instances();

    // Filter the instances by provided type and bundle.
    if (isset($entity_type) && isset($instances[$entity_type])) {
      $instances = array($entity_type => $instances[$entity_type]);
      if (isset($bundle) && isset($instances[$entity_type][$bundle])) {
        $instances[$entity_type] = array($bundle => $instances[$entity_type][$bundle]);
      }
    }

    // Aggregate instances of this field.
    $field_instances = array();
    foreach ($field['bundles'] as $entity_type => $bundles) {
      foreach ($bundles as $bundle) {
        if (isset($instances[$entity_type][$bundle][$field_name])) {
          $field_instances[] = $instances[$entity_type][$bundle][$field_name];
        }
      }
    }
    return $field_instances;
  }

  /**
   * Checks whether this field has data associated with it.
   */
  function hasData() {
    $field = $this->getField();
    return $field && field_has_data($field);
  }




}

/**
 * Entity controller class for Simple Fields.
 */
class SimpleFieldEntityController extends EntityAPIController {

  public function invoke($hook, $entity) {

    if (empty($entity->simple_field_stop_race)) {
      switch ($hook) {
        case 'presave':
          $entity->field_created = !$entity->field_id;
          if (!$entity->field_id) {
            $field = simple_field_build_field($entity);
            $field = field_create_field($field);
            $entity->field_id = $field['id'];
          }
          break;
        case 'update':
          if (!$entity->field_created) {
            $field = $entity->getField();
            // Merge form settings with those already in the field.
            $field['settings'] = array_merge($field['settings'], $entity->getSettings('field'));

            // Mark field as in process of updating.
            simple_field_field_updated_flag_cache('field', $field['id'], TRUE);

            // Update the field with new settings from this simplefield.
            field_update_field($field);

            // Mark field as not in middle of update.
            simple_field_field_updated_flag_cache('field', $field['id'], FALSE);

            foreach ($entity->getInstances() as $instance) {
              $inst = simple_field_build_instance($instance['entity_type'], $instance['bundle'], $entity);
              $inst = array_merge($instance, $inst);
              $inst['settings'] = array_merge($instance['settings'], $inst['settings']);
              $inst['widget']['settings'] = array_merge($instance['widget']['settings'], $inst['widget']['settings']);
              simple_field_field_updated_flag_cache('instance', $inst['id'], TRUE);

              field_update_instance($inst);
              simple_field_field_updated_flag_cache('instance', $inst['id'], FALSE);
            }
          }
          break;
        case 'delete':
          $field = $entity->getField();
          field_delete_field($field['field_name']);
          break;
      }

    }

    return parent::invoke($hook, $entity);
  }


}

// ----------------------------------------------------------------------------
// Helper functions for working with SimpleField fields and instances.

/**
 * Build a field object for a simple_field entity.
 */
function simple_field_build_field($simplefield) {

  $type_info = $simplefield->getTypeInfo();
  $settings = $simplefield->getSettings('field');

  return array(
    'settings' => $settings,
    'field_name' => simple_field_unique_field_name($settings['label'], 'sf_'),
    'translatable' => FALSE,
    'type' => $type_info['field'],
    'cardinality' => $type_info['cardinality'],
  );
}

function simple_field_build_instance($entity_type, $bundle, $simplefield) {

  $field = $simplefield->getField();
  $type_info = $simplefield->getTypeInfo();
  $field_settings = $simplefield->getSettings('field');
  $instance_settings = $simplefield->getSettings('instance');
  $widget_settings = $simplefield->getSettings('widget');

  $instance = array(
    'field_name' => $field['field_name'],
    'entity_type' => $entity_type,
    'bundle' => $bundle,
    'label' => $field_settings['label'],
    'settings' => $instance_settings,
    'required' => $simplefield->required,
    'description' => $instance_settings['description'],
    'widget' => array(
      'type' => $type_info['type'],
      'settings' => $widget_settings,
      'weight' => 0,
    ),
    'display' => array(),
  );

  return $instance;
}

/**
 * Helper to take a field label and come up with a semi-readable field name.
 */
function simple_field_unique_field_name($label, $prefix = 'field_') {
  // Make plaintext name from label and limit length.
  $label = $prefix . preg_replace('/(^[0-9])|([^A-Za-z0-9])/', '', $label);
  $label = drupal_strtolower($label); // field names must be lower case
  $label = drupal_substr($label, 0, 32);

  // Append numbers to end of name until unique field found.
  $fields = field_info_fields();
  $name = $label;
  for ($i = 0; isset($fields[$name]) && drupal_strlen($i) < 25; $i++) {
    $name = drupal_substr($label, 0, 32 - drupal_strlen($i)) . $i;
  }

  return $name;
}


// ----------------------------------------------------------------------------
// Helper hooks to maintain data integrity of fields and instances.

/**
 * Implements hook_field_update_field().
 *
 * Proxied from hook in module file.
 */
function _simple_field_field_update_field($field, $prior_field, $has_data) {

  if ($simplefield = simple_field_load_by_field($field['id'])) {
    // Prevent recursive loop by checkif if field is already being updated.
    if (!simple_field_field_updated_flag_cache('field', $field['id'])) {

      if (isset($field['settings'])) {
        // Update the settings on the simplefield.
        $simplefield->field_settings = $field['settings'];
      }

      // Mark simplefield as updating to stop race.
      $simplefield->simple_field_stop_race = TRUE;

      // Save the question.
      $simplefield->save();

      // Mark simplefield as done updating.
      unset($simplefield->simple_field_stop_race);
    }
  }
}

/**
 * Implements hook_field_delete_field().
 *
 * Proxied from hook in module file.
 */
function _simple_field_field_delete_field($field) {
  if ($simplefield = simple_field_load_by_field($field['id'])) {
    // Create a new field from default settings and set field_name the same.
    $newfield = simple_field_build_field($simplefield);
    $newfield['field_name'] = $field['field_name'];

    // Save the new field and set the field_id on the simplefield.
    $newfield = field_create_field($newfield);
    $simplefield->field_id = $newfield['id'];

    // Save simplefield with new field_id.
    $simplefield->save();
  }
}

/**
 * Implements hook_field_update_instance().
 *
 * Proxied from hook in module file.
 */
function _simple_field_field_update_instance($instance, $prior_instance) {

  $field = field_info_field($instance['field_name']);
  if ($simplefield = simple_field_load_by_field($field['id'])) {
    if (simple_field_field_updated_flag_cache('instance', $instance['id'])) {
      if (isset($instance['settings'])) {
        $simplefield->instance_settings = $instance['settings'];
      }
      if (isset($instance['widget']['settings'])) {
        $simplefield->widget_settings = $instance['widget']['settings'];
      }
      if (isset($instance['display']['default']['settings'])) {
        $simplefield->formatter_settings = $instance['display']['default']['settings'];
      }

      $simplefield->required = $instance['required'];
      $simplefield->instance_settings['description'] = $instance['description'];

      $simplefield->simple_field_stop_race = TRUE;
      $simplefield->save();
      unset($simplefield->simple_field_stop_race);
    }
  }
}

/**
 * Cache function to mark fields and instances as being saved in our code.
 *
 * Similar to the 'stop_race' flag on the entity. Here we must use a separate
 * cache function because we cannot add it directly to the instance or field
 * because then it would get serialized in the DB
 *
 * This and stop_race are meant to stop race conditions where updating the question
 * triggers a field update, and then the field update triggering a question
 * update and so on.
 */
function simple_field_field_updated_flag_cache($type, $id, $val = NULL) {
  $values = &drupal_static(__FUNCTION__);

  if (isset($val)) {
    $values[$type][$id] = $val;
  }

  if (isset($values[$type][$id])) {
    return $values[$type][$id];
  }
  return NULL;
}
