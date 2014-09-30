<?php

/**
 * @file
 * Form to kickoff a thesis batch ingest from the front-end.
 */

/**
 * Form building function.
 */
function islandora_thesis_batch_form($form, &$form_state, $object) {
  form_load_include($form_state, 'inc', 'islandora_book_batch', 'includes/batch.form');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $default_namespace = islandora_get_namespace($object->id);

  $form['target'] = array(
    '#type' => 'managed_file',
    '#title' => t('Zip file'),
    '#description' => t('A Zip file containing a number of theses. Each thesis has an XML record in MODS format, with a corresponding same-named PDF.'),
    '#required' => TRUE,
    '#upload_location' => 'public://',
    '#upload_validators' => array(
      'file_validate_extensions' => array('zip'),
    ),
  );

  $form['parent'] = array(
    '#type' => 'value',
    '#value' => $object->id,
  );

  $form['namespace'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace for created objects?'),
    '#description' => t('Thesis objects will be constructed within this namespace.'),
    "#default_value" => $default_namespace,
    '#size' => 15,
  );

  if (module_exists('islandora_ocr')) {
    module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
    $derive = islandora_paged_content_get_enabled_ingest_derivatives('islandora_book');
    if ($derive['ocr']) {
      $form['generate_ocr'] = array(
        '#type' => 'checkbox',
        '#title' => t('Generate OCR?'),
        '#description' => t('Whether or not we should generate OCR for the thesis.'),
        '#default_value' => TRUE,
      );
    }
  }

  $form['ingest_set_immediately'] = array(
    '#type' => 'checkbox',
    '#title' => t('Ingest immediately?'),
    '#description' => t('If not selected, objects will just be preprocessed into the queue of objects to be ingested, to be fully-processed (and ingested) later.'),
    '#default_value' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Form submit handler.
 *
 * May kick off a batch process.
 */
function islandora_thesis_batch_form_submit($form, $form_state) {
  $target = file_load($form_state['values']['target']);

  $form_state['values']['target'] = $target->uri;

  $connection = islandora_get_tuque_connection();
  $parameters = $form_state['values'] + array(
    'type' => 'zip',
    'parent_relationship_uri' => 'info:fedora/fedora-system:def/relations-external#',
    'parent_relationship_pred' => 'isMemberOfCollection',
    'wait_for_metadata' => FALSE,
    'directory_dedup' => FALSE,
    'content_models' => array('ir:thesisCModel'),
  );

  $preprocessor = new ThesisBatchObject($connection, $parameters);
  file_usage_add($target, 'islandora_batch', 'islandora_batch_set', $preprocessor->getSetId());

  if ($form_state['values']['ingest_set_immediately']) {
    islandora_batch_ingest(array(
        'preprocessor' => $preprocessor,
      ));
  }
  else {
    $preprocessed = islandora_batch_handle_preprocessor($preprocessor);
  }
}
