<?php
/** Drupal MailSystemInterface sender; no changes on module enable. */
class BcGraphMailSystem implements MailSystemInterface {
  public function format(array $message) {
    // Normalise using the existing MIME Mail formatter.
    $formatter = new MimeMailSystem();
    return $formatter->format($message);
  }
  public function mail(array $message) {
    return bc_graph_mail_send($message);
  }
}
