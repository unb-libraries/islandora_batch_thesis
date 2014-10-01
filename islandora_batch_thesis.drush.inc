<?php

/**
 * @file
 * Implementation of Drush hooks.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_batch_thesis_drush_command() {
  $items = array();

  $items['islandora_thesis_batch_preprocess'] = array(
    'aliases' => array('itbp'),
    'description' => 'Import theses into collections.',
    'drupal dependencies' => array(
      'islandora_batch',
      'islandora_scholar',
      'islandora_batch_thesis',
    ),
    'options' => array(
      'type' => array(
        'description' => 'Either "directory" or "zip".',
        'required' => TRUE,
      ),
      'target' => array(
        'description' => 'The target to directory or zip file to scan.',
        'required' => TRUE,
      ),
      'namespace' => array(
        'description' => 'The namespace for objects created. Defaults to namespace set in fedora config.',
        'required' => FALSE,
      ),
      'content_models' => array(
        'description' => 'A comma-separated list of content models to assign',
        'value' => 'optional',
      ),
      'parent' => array(
        'description' => 'The collection to which the generated items should be added',
        'value' => 'optional',
      ),
      'parent_relationship_uri' => array(
        'description' => 'URI of the parent relationship, defaults to "info:fedora/fedora-system:def/relations-external#".',
        'value' => 'optional',
      ),
      'parent_relationship_pred' => array(
        'description' => 'The predicate of the relationship to the parent. Defaults to "isMemberOfCollection".',
        'value' => 'optional',
      ),
      'do_not_generate_ocr' => array(
        'description' => 'A flag to allow for conditional OCR generation.',
        'value' => 'optional',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );

  return $items;
}

/**
 * Implements drush_file_command().
 */
function drush_islandora_batch_thesis_islandora_thesis_batch_preprocess() {
  // XXX: Due to how Drush bootstrapping works, the connection may be created
  // without credentials (when your site's front page is
  // 'islandora/object/some:object', for example). Resetting to ensure a new
  // connection gets created should fix it.
  drupal_static_reset('islandora_get_tuque_connection');
  $connection = islandora_get_tuque_connection();
  $parameters = array(
    'type' => drush_get_option('type'),
    'namespace' => drush_get_option('namespace'),
    'target' => drush_get_option('target'),
    'parent' => drush_get_option('parent', 'ir:thesisCModel'),
    'parent_relationship_uri' => drush_get_option('parent_relationship_uri', 'info:fedora/fedora-system:def/relations-external#'),
    'parent_relationship_pred' => drush_get_option('parent_relationship_pred', 'isMemberOfCollection'),
    'wait_for_metadata' => FALSE,
    'directory_dedup' => FALSE,
  );

  if ($content_models = drush_get_option('content_models', FALSE)) {
    $parameters['content_models'] = explode(',', $content_models);
  }
  else {
    $parameters['content_models'] = array('ir:thesisCModel');
  }

  if ($do_not_generate = drush_get_option('do_not_generate_ocr', FALSE)) {
    $parameters['generate_ocr'] = FALSE;
  }
  else {
    $parameters['generate_ocr'] = TRUE;
  }

  $preprocessor = new ThesisBatchObject($connection, $parameters);

  // Pass the preprocessor off to run.
  $preprocessed = islandora_batch_handle_preprocessor($preprocessor);
}
