<?php
/**
 * @file
 * simple_field_demo_customers.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function simple_field_demo_customers_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}

/**
 * Implements hook_views_api().
 */
function simple_field_demo_customers_views_api() {
  list($module, $api) = func_get_args();
  if ($module == "views" && $api == "views_default") {
    return array("version" => "3.0");
  }
}

/**
 * Implements hook_node_info().
 */
function simple_field_demo_customers_node_info() {
  $items = array(
    'customer' => array(
      'name' => t('Customer'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Name'),
      'help' => '',
    ),
  );
  return $items;
}

/**
 * Implements hook_default_simple_field().
 */
function simple_field_demo_customers_default_simple_field() {
  $items = array();
  $items['sf_doyouwanttoaddthis'] = entity_import('simple_field', '{
    "type" : "simple_field_yesno",
    "uid" : "1",
    "field_settings" : {
      "allowed_values" : { "1" : "Yes", "0" : "No" },
      "allowed_values_function" : "",
      "label" : "Do you want to add this?"
    },
    "instance_settings" : {
      "allowed_values" : [  ],
      "allowed_values_function" : "",
      "description" : "",
      "user_register_form" : false
    },
    "widget_settings" : [  ],
    "formatter_settings" : [  ],
    "required" : "0",
    "field_name" : "sf_doyouwanttoaddthis",
    "rdf_mapping" : [  ]
  }');
  return $items;
}
