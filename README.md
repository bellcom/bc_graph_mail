# BC Graph Mail 7.x-1.0-beta1

Target: Drupal 7 + PHP 7.4, Mail System 7.x-2.x, Mime Mail 7.x-1.x.
This is a site-specific beta implementation, not a Drupal.org community module. **Test on a copy first.**

## Installation
Copy directory to `sites/all/modules/custom/bc_graph_mail`; enable with `drush en bc_graph_mail -y` and clear caches.
**Enabling does not switch mail transport.** SMTP remains unchanged.

## Microsoft 365
Register an Entra app with Graph **Application** permission `Mail.Send` and grant admin consent. Restrict application access to the intended mailbox through appropriate Exchange Online application RBAC or equivalent. The sender must be a real Exchange Online mailbox or permitted sender identity.

## Secret configuration
In `sites/default/settings.php` (or an include outside webroot), configure:

```php
$GLOBALS['bc_graph_mail'] = array(
  'tenant_id' => 'YOUR-TENANT-GUID',
  'client_id' => 'YOUR-APP-CLIENT-ID',
  'client_secret' => 'YOUR-SECRET',
  'sender' => 'broen7000@fredericia.dk',
  'sender_name' => 'Broen 7000',
);
```

Prefer a protected PHP configuration include **outside the web root**, with permissions limited to root and PHP-FPM user/group. Never check the secret into Git or paste it into support chats.

Visit `/admin/config/system/bc-graph-mail` and send a test to a controlled mailbox. Confirm actual delivery; 202 only means accepted for processing.

## Cutover
Before cutover, back up the `mail_system` and `mimemail_engine` Drupal variables. Current observed baseline:

- mail_system default-system/mimemail/print_mail_sendpage: `MimeMailSystem__SmtpMailSystem`
- mimemail_engine: `smtp`

After successful tests, change **all three** mappings to `MimeMailSystem__BcGraphMailSystem` and `mimemail_engine` to `bc_graph_mail`. Cache-clear after switch. For rollback restore exact saved values and cache-clear. Do **not** disable SMTP during testing.

The Mime Mail fallback formatter constructs multipart/related, multipart/mixed bodies and attachments. This transport forwards that MIME without remodelling it, using Graph sendMail MIME mode. Some uncommon MIME header structures may need compatibility work; test real notification, CC/BCC, inline image and attachments before production deployment. Unlike the previous SMTP module, this beta does not implement SMTP reroute or queue policies. Avoid accidental user mail from staging.

## Known cautions
- MIME formatter/header interoperability needs live testing against your exact installation.
- Recipient override test deliberately bypasses Drupal mail routing.
- Token is held in Drupal's cache (ensure DB/cache access is restricted).
- The provider's `202 Accepted` is not final delivery confirmation.
- Graph/Microsoft throttling and transient error retries are not implemented in this beta. Do not switch heavy-volume production notifications without these safeguards.
