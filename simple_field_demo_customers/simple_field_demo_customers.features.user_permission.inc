<?php
/**
 * @file
 * simple_field_demo_customers.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function simple_field_demo_customers_user_default_permissions() {
  $permissions = array();

  // Exported permission: administer simple fields
  $permissions['administer simple fields'] = array(
    'name' => 'administer simple fields',
    'roles' => array(
      0 => 'administrator',
      1 => 'customer service rep',
    ),
    'module' => 'simple_field',
  );

  // Exported permission: create simple field instances
  $permissions['create simple field instances'] = array(
    'name' => 'create simple field instances',
    'roles' => array(
      0 => 'customer service rep',
    ),
    'module' => 'simple_field',
  );

  // Exported permission: create simple fields
  $permissions['create simple fields'] = array(
    'name' => 'create simple fields',
    'roles' => array(
      0 => 'customer service rep',
    ),
    'module' => 'simple_field',
  );

  // Exported permission: delete simple field instances
  $permissions['delete simple field instances'] = array(
    'name' => 'delete simple field instances',
    'roles' => array(
      0 => 'customer service rep',
    ),
    'module' => 'simple_field',
  );

  // Exported permission: delete simple fields
  $permissions['delete simple fields'] = array(
    'name' => 'delete simple fields',
    'roles' => array(
      0 => 'customer service rep',
    ),
    'module' => 'simple_field',
  );

  // Exported permission: edit simple fields
  $permissions['edit simple fields'] = array(
    'name' => 'edit simple fields',
    'roles' => array(
      0 => 'customer service rep',
    ),
    'module' => 'simple_field',
  );

  // Exported permission: view simple fields
  $permissions['view simple fields'] = array(
    'name' => 'view simple fields',
    'roles' => array(
      0 => 'customer service rep',
    ),
    'module' => 'simple_field',
  );

  return $permissions;
}
