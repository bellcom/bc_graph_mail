# BC Graph Mail (Drupal 7)

Microsoft Graph OAuth2 mail transport for Drupal 7 using the existing Mail System and Mime Mail modules.

**Status:** beta. Tested on Broen7000 with PHP 7.4, Mail System 7.x-2.35 and Mime Mail 7.x-1.2. A direct Graph test and a direct call to the `BcGraphMailSystem` formatter/transport succeeded. A real application notification through `drupal_mail()` has not yet been verified.

## Requirements

- Drupal 7, PHP >= 7.4
- Mail System 7.x-2.x and Mime Mail 7.x-1.x installed and enabled
- Microsoft Entra application with Microsoft Graph **Application** permission `Mail.Send` and admin consent
- Exchange Online sender mailbox and application access restricted to permitted mailbox(es), e.g. via Exchange application RBAC

## Composer installation

This repository is public, but **not registered with Packagist**. Add it as a VCS repository to your Composer project.

For an existing legacy Drupal 7 site, consider a **separate Composer project** so Composer does not upgrade unrelated legacy dependencies. Example for a project at `/var/www/EXAMPLE/bc-modules-composer` with the Drupal document root at `/var/www/EXAMPLE/public_html`:

```json
{
  "name": "example/drupal7-custom-modules",
  "type": "project",
  "repositories": [
    {
      "type": "vcs",
      "url": "https://github.com/bellcom/bc_graph_mail"
    }
  ],
  "require": {
    "php": ">=7.4",
    "composer/installers": "^2.0",
    "bellcom/bc-graph-mail": "1.0.0-beta1"
  },
  "config": {
    "allow-plugins": {
      "composer/installers": true
    }
  },
  "extra": {
    "installer-paths": {
      "../public_html/sites/all/modules/custom/bc_graph_mail": [
        "bellcom/bc-graph-mail"
      ]
    }
  }
}
```

Adjust the installer path to your actual document root. Run `composer install` as the intended deployment user (not root), then retain `composer.json` and `composer.lock` for reproducible deployments.

In the Drupal document root, enable the module and clear caches:

```bash
drush --uri=https://EXAMPLE.DOMAIN en bc_graph_mail -y
drush --uri=https://EXAMPLE.DOMAIN cc all -y
```

Enabling the module **does not** change the active mail transport.

## settings.php configuration (copy and edit)

Add the following to your Drupal 7 site's `sites/default/settings.php`, inside its existing PHP code. **Do not add another `<?php` tag.** Replace the example values, including sender and sender name, with values for your installation.

```php
/**
 * BC Graph Mail - Microsoft Graph OAuth2.
 * Keep secrets in local settings.php, never in Git.
 */
$GLOBALS['bc_graph_mail'] = array(
  'tenant_id' => 'YOUR-TENANT-GUID',
  'client_id' => 'YOUR-APP-CLIENT-ID',
  'client_secret' => 'YOUR-CLIENT-SECRET',
  'sender' => 'sender@example.com',
  'sender_name' => 'Example sender',
);
```

The `client_secret` is the **secret value**, not the secret ID. Restrict access to `settings.php` and exclude it from version control. Alternatively, include a protected PHP file outside the document root, readable by the PHP runtime.

Validate syntax without displaying secrets:

```bash
php -l sites/default/settings.php
```

From the Drupal root, check whether configuration is loaded (without printing values):

```bash
drush --uri=https://EXAMPLE.DOMAIN php-eval '
$c = isset($GLOBALS["bc_graph_mail"]) ? $GLOBALS["bc_graph_mail"] : array();
foreach (array("tenant_id", "client_id", "client_secret", "sender", "sender_name") as $key) {
  echo $key . ": " . (empty($c[$key]) ? "MANGLER" : "OK") . PHP_EOL;
}
'
```

## Test without switching mail transport

Open `/admin/config/system/bc-graph-mail` on your Drupal site and send a test to a controlled recipient. Verify actual delivery. An HTTP `202 Accepted` response from Graph means accepted for processing, **not** delivered.

## Cutover from SMTP

Back up both Drupal variables `mail_system` and `mimemail_engine` before changing anything. A previously observed SMTP setup was:

- `default-system`, `mimemail`, `print_mail_sendpage`: `MimeMailSystem__SmtpMailSystem`
- `mimemail_engine`: `smtp`

**Use the registered `BcGraphMailSystem` class directly**, not `MimeMailSystem__BcGraphMailSystem`. The latter is not a pre-existing registered class. `BcGraphMailSystem` implements both `format()` (delegates to Mime Mail) and `mail()` (Graph transport).

From your Drupal root, after verifying Graph credentials and backing up current settings:

```bash
drush --uri=https://EXAMPLE.DOMAIN php-eval '
$map = variable_get("mail_system", array());
foreach (array("default-system", "mimemail", "print_mail_sendpage") as $key) {
  $map[$key] = "BcGraphMailSystem";
}
variable_set("mail_system", $map);
variable_set("mimemail_engine", "bc_graph_mail");
'
drush --uri=https://EXAMPLE.DOMAIN cc all -y
```

Read the variables back in a **new Drush process** to confirm the values survive cache clearing. Test the real mail flow with a controlled recipient, including a genuine application notification. Do not disable SMTP until satisfied with the result.

To roll back, restore the **exact** backed-up `mail_system` and `mimemail_engine` values and clear caches. Do not assume every site has the same SMTP settings.

## Limitations and security notes

- Beta implementation; test real notifications, HTML, plain text, CC/BCC, inline images, and attachments before wider deployment.
- Graph MIME mode forwards Mime Mail output; unusual MIME headers may require compatibility fixes.
- Graph throttling and automatic retries for transient errors are not implemented.
- SMTP reroute/queue policies are not reproduced automatically.
- OAuth tokens are cached in Drupal's cache; secure database/cache access.
- Avoid sending unintentional production notifications from staging.
- Never commit OAuth secrets or paste them in issue reports or support chats.
