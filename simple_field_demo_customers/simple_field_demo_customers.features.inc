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
  $items['sf_address'] = entity_import('simple_field', '{
    "type" : "simple_field_long_answer",
    "uid" : "1",
    "field_settings" : { "label" : "Address" },
    "instance_settings" : { "description" : "" },
    "widget_settings" : null,
    "formatter_settings" : null,
    "required" : "0",
    "field_name" : "sf_address",
    "rdf_mapping" : [  ]
  }');
  $items['sf_availability'] = entity_import('simple_field', '{
    "type" : "simple_field_multichoice_multi",
    "uid" : "1",
    "field_settings" : {
      "allowed_values" : {
        "Morning" : "Morning",
        "Afternoon" : "Afternoon",
        "Evening" : "Evening"
      },
      "label" : "Availability"
    },
    "instance_settings" : { "description" : "" },
    "widget_settings" : null,
    "formatter_settings" : null,
    "required" : "0",
    "field_name" : "sf_availability",
    "rdf_mapping" : [  ]
  }');
  $items['sf_datelastcontacted'] = entity_import('simple_field', '{
    "type" : "simple_field_single_date",
    "uid" : "1",
    "field_settings" : { "label" : "Date last contacted" },
    "instance_settings" : { "description" : "" },
    "widget_settings" : null,
    "formatter_settings" : null,
    "required" : "0",
    "field_name" : "sf_datelastcontacted",
    "rdf_mapping" : [  ]
  }');
  $items['sf_doyouwanttobesentemails'] = entity_import('simple_field', '{
    "type" : "simple_field_short_answer",
    "uid" : "3",
    "field_settings" : { "label" : "Do you want to be sent emails?" },
    "instance_settings" : { "description" : "" },
    "widget_settings" : null,
    "formatter_settings" : null,
    "required" : "0",
    "field_name" : "sf_doyouwanttobesentemails",
    "rdf_mapping" : [  ]
  }');
  $items['sf_language'] = entity_import('simple_field', '{
    "type" : "simple_field_multichoice_single",
    "uid" : "1",
    "field_settings" : {
      "allowed_values" : { "French" : "French", "English" : "English" },
      "label" : "Language"
    },
    "instance_settings" : { "description" : "" },
    "widget_settings" : null,
    "formatter_settings" : null,
    "required" : "0",
    "field_name" : "sf_language",
    "rdf_mapping" : [  ]
  }');
  $items['sf_numberoftimescontacted'] = entity_import('simple_field', '{
    "type" : "simple_field_integer",
    "uid" : "1",
    "field_settings" : { "label" : "Number of times contacted" },
    "instance_settings" : { "description" : "" },
    "widget_settings" : null,
    "formatter_settings" : null,
    "required" : "0",
    "field_name" : "sf_numberoftimescontacted",
    "rdf_mapping" : [  ]
  }');
  $items['sf_phonenumber'] = entity_import('simple_field', '{
    "type" : "simple_field_short_answer",
    "uid" : "1",
    "field_settings" : { "label" : "Phone Number" },
    "instance_settings" : { "description" : "" },
    "widget_settings" : null,
    "formatter_settings" : null,
    "required" : "0",
    "field_name" : "sf_phonenumber",
    "rdf_mapping" : [  ]
  }');
  $items['sf_productsofinterest'] = entity_import('simple_field', '{
    "type" : "simple_field_multichoice_multi",
    "uid" : "1",
    "field_settings" : {
      "allowed_values" : {
        "Tupperware" : "Tupperware",
        "Kitchen Supplies" : "Kitchen Supplies",
        "Knives" : "Knives",
        "Fancy Soaps" : "Fancy Soaps"
      },
      "label" : "Products of Interest"
    },
    "instance_settings" : { "description" : "" },
    "widget_settings" : null,
    "formatter_settings" : null,
    "required" : "0",
    "field_name" : "sf_productsofinterest",
    "rdf_mapping" : [  ]
  }');
  return $items;
}
