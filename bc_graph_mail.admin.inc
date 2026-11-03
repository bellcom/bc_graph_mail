<?php
/** Config + isolated test; credentials are supplied exclusively in settings.php. */
function bc_graph_mail_settings_form($form, &$form_state) {
  $c = bc_graph_mail_config();
  $form['info'] = array('#markup' => '<p>Credentials must be set in settings.php. Enabling this module does not change the active mail transport.</p>');
  foreach (array('tenant_id', 'client_id', 'sender', 'sender_name') as $key) {
    $form[$key] = array('#type' => 'item', '#title' => check_plain($key), '#markup' => !empty($c[$key]) ? check_plain($c[$key]) : '<em>Not configured</em>');
  }
  $form['secret_status'] = array('#type' => 'item', '#title' => t('Client secret'), '#markup' => empty($c['client_secret']) ? t('Not configured') : t('Configured (hidden)'));
  $form['recipient'] = array('#type' => 'textfield', '#title' => t('Test recipient'), '#required' => TRUE, '#description' => t('Use a controlled test mailbox only.'));
  $form['actions']['send'] = array('#type' => 'submit', '#value' => t('Send test message'));
  return $form;
}
function bc_graph_mail_settings_form_validate($form, &$form_state) {
  if (!valid_email_address($form_state['values']['recipient'])) {
    form_set_error('recipient', t('Invalid email address.'));
  }
}
function bc_graph_mail_settings_form_submit($form, &$form_state) {
  $c = bc_graph_mail_config();
  $recipient = $form_state['values']['recipient'];
  $message = array(
    'to' => $recipient,
    'from' => isset($c['sender']) ? $c['sender'] : '',
    'subject' => 'Broen7000 Graph Mail test',
    'headers' => array('Content-Type' => 'text/plain; charset=UTF-8'),
    'body' => "This is an isolated Graph API test from Drupal 7.\n",
  );
  if (bc_graph_mail_send($message, $recipient)) {
    drupal_set_message(t('Microsoft Graph accepted the test message (202). Verify actual delivery in the recipient mailbox.'));
  }
  else {
    drupal_set_message(t('Microsoft Graph rejected the test message. See recent log messages.'), 'error');
  }
}
