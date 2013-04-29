<?php

function convio_signup_admin() {
  $header = array(
    'id'   => t('Block ID'),
    'name' => t('Name'),
    'desc' => t('Description'),
    'ops'  => t('Operations'),
  );

  $results = db_query("SELECT cid,title,description FROM {convio_signup}");
  
  foreach ($results as $row) {
    $actions = array(
      'configure' => l(t('Configure'), sprintf('admin/structure/block/manage/convio_signup/%d/configure', $row->cid)),
      'delete'    => l(t('Delete'), sprintf('admin/structure/convio-signup/%d/delete', $row->cid)),
    );

    $rows[] = array(
      'id'   => $row->cid,
      'name' => $row->title,
      'description' => $row->description,
      'ops'  => implode(' | ', $actions),
    );
  }

  $table = '';
  if (!empty($rows)) {
    $table = theme('table', array('header' => $header, 'rows' => $rows));
  }

  return theme('convio_signup_admin', array('table' => $table));  
}

function convio_signup_add($form, &$form_state) {
  $form['convio_signup'] = array(
    '#type' => 'fieldset'
  );
  $form['convio_signup']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#size' => 60,
    '#maxlength' => 60
  );
  $form['convio_signup']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('A brief description of this form.'),
    '#size' => 120,
    '#maxlength' => 1024,
    '#required' => TRUE
  );
  $form['convio_signup']['before'] = array(
    '#type' => 'text_format',
    '#title' => 'Form Prefix',
    '#description' => t('Markup to be displayed above the form.')
  );
  $form['convio_signup']['display_fields'] = array(
    '#type' => 'checkboxes',
    '#options' => convio_signup_get_fields(),
    '#title' => t('Additional form fields to be displayed. (Email will always be displayed and required)')
  );
  $form['convio_signup']['required_fields'] = array(
    '#type' => 'checkboxes',
    '#options' => convio_signup_get_fields(),
    '#title' => t('Additional form fields to be marked as required.')
  );
  $form['convio_signup']['interest_ids'] = array(
    '#type' => 'textarea',
    '#title' => t('Interest Ids'),
    '#description' => t('List of comma separated (Interest ID, Friendly Name) pairs that will be listed as subscription options on this form. Adding ",1" to the end of a row will make that option selected by default. Multiple interest ID\'s should be listed on a separate line. Example: 13532, Programming Newsletter, 1'),
    '#element_validate' => array('_convio_signup_id_validate'),
    '#size' => 60,
    '#maxlength' => 60
  );
  $form['convio_signup']['group_ids'] = array(
    '#type' => 'textfield',
    '#title' => t('Group Ids'),
    '#description' => t('A comma separated list of Group IDs to be added to an account when when the form is submitted.'),
    '#element_validate' => array('_convio_signup_id_validate'),
    '#size' => 60,
    '#maxlength' => 60
  );
  $form['convio_signup']['submit_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Submit Button Text'),
    '#description' => t('Label to be applied to the submit button for this form. Example: "Submit" or "Sign up"'),
    '#default_value' => 'Submit',
    '#size' => 9,
    '#maxlength' => 16
  );
  
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['actions']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/structure/convio-signup'),
  );
 
  return $form;
}

function convio_signup_add_submit($form, &$form_state) {
  $values = $form_state['values'];
  $data = array(
    'title' => ($values['title']) ? ($values['title']) : NULL,
    'description' => ($values['description']) ? ($values['description']) : NULL,
    'interest_ids' => ($values['interest_ids']) ? $values['interest_ids'] : NULL,
    'group_ids' => ($values['group_ids']) ? $values['group_ids'] : NULL,
    'fields_displayed' => ($values['display_fields']) ? $values['display_fields'] : NULL,
    'fields_required' => ($values['required_fields']) ? $values['required_fields'] : NULL,
    'before_markup' => ($values['before']['value']) ? $values['before']['value'] : NULL,
    'before_format' => ($values['before']['format']) ? $values['before']['format'] : NULL,
    'submit' => ($values['submit_text']) ? $values['submit_text'] : NULL, 
  );
  
  $result = drupal_write_record('convio_signup', $data);
  
  if ($result) {
    drupal_set_message('Your signup form has been created');
  }
  else {
    drupal_set_message('There was an error creating your signup form', 'error');
  }
}

function convio_signup_delete($form, &$form_state, $cid) {
  $result = db_query("SELECT title,description FROM {convio_signup} where cid = :cid", array(':cid' => $cid))->fetchObject();
  $form['warning'] = array(
    '#markup' => '<h2>You are about to delete the Convio signup form "' . $result->description . '".</h2><h2>This cannot be undone!</h2>'
  );
  
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  $form['actions']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/structure/convio-signup'),
  );
  return $form;
}

function convio_signup_delete_submit($form, &$form_state) {
  $cid = $form_state['build_info']['args'][0];
  convio_signup_remove($cid);
  drupal_goto('admin/structure/convio-signup');
}

function convio_signup_remove($cid) {
  $res = db_query('DELETE from {convio_signup} WHERE cid = :cid', array(':cid' => $cid));
  if ($res) {
    drupal_set_message('The Convio signup form has been deleted.');
  }
  else {
    drupal_set_message('There was an error deleting the signup form.', 'error');
  }
  $res = db_query("DELETE from {block} WHERE module = 'convio_signup' AND delta = :cid", array('cid' => $cid));
  if (!$res) {
    drupal_set_message('There was an error deleting the form from the blocks table.', 'error');
  }
  
}