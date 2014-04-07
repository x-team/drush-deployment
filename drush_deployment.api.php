<?php

/**
 * Perform a single rollback.
 *
 * If in any case the deployment went bad and you want to revert the DB record
 * on its previous state, add a new hook_rollback_N(), which will be called
 * by "drush deploy-rollback" command.
 *
 * Implementations of hook_rollback_N() are named (module name)_rollback_(previous_number).
 * The numbers are composed of three parts:
 * - 1 digit for Drupal core compatibility.
 * - 1 digit for your module's major release version (e.g., is this the 7.x-1.*
 *   (1) or 7.x-2.* (2) series of your module?). This digit should be 0 for
 *   initial porting of your module to a new Drupal core API.
 * - 2 digits for sequential counting, starting with 00.
 *
 * Example:
 * - mymodule_rollback_7105(): Rollback to this number.
 *
 * It is advisable to use this API alongside of every hook_update_N() to
 * minimize the problem if in any case the release failed.
 *
 * Implementations of this hook should be placed in a mymodule.install file in
 * the same directory as mymodule.module.
 *
 * Not all module functions are available from within a hook_rollback_N() function.
 * In order to call a function from your mymodule.module or an include file,
 * you need to explicitly load that file first.
 *
 * During database updates the schema of any module could be out of date. For
 * this reason, caution is needed when using any API function within an update
 * function - particularly CRUD functions, functions that depend on the schema
 * (for example by using drupal_write_record()), and any functions that invoke
 * hooks. See @link update_api Update versions of API functions @endlink for
 * details.
 *
 * If your update task is potentially time-consuming, you'll need to implement a
 * multipass update to avoid PHP timeouts. Multipass updates use the $sandbox
 * parameter provided by the batch API (normally, $context['sandbox']) to store
 * information between successive calls, and the $sandbox['#finished'] value
 * to provide feedback regarding completion level.
 *
 * See the batch operations page for more information on how to use the
 * @link http://drupal.org/node/180528 Batch API. @endlink
 *
 * @param $sandbox
 *   Stores information for multipass updates. See above for more information.
 *
 * @throws DrupalUpdateException, PDOException
 *   In case of error, update hooks should throw an instance of DrupalUpdateException
 *   with a meaningful message for the user. If a database query fails for whatever
 *   reason, it will throw a PDOException.
 *
 * @return
 *   Optionally, update hooks may return a translated string that will be
 *   displayed to the user after the update has completed. If no message is
 *   returned, no message will be presented to the user.
 *
 * @see batch
 * @see schemaapi
 * @see update_api
 * @see hook_update_last_removed()
 * @see update_get_update_list()
 */
function hook_rollback_N(&$sandbox) {
  // For non-multipass updates, the signature can simply be;
  // function hook_rollback_N() {

  // For most updates, the following is sufficient.
  db_add_field('mytable1', 'newcol', array('type' => 'int', 'not null' => TRUE, 'description' => 'My new integer column.'));

  // However, for more complex operations that may take a long time,
  // you may hook into Batch API as in the following example.

  // Update 3 users at a time to have an exclamation point after their names.
  // (They're really happy that we can do batch API in this hook!)
  if (!isset($sandbox['progress'])) {
    $sandbox['progress'] = 0;
    $sandbox['current_uid'] = 0;
    // We'll -1 to disregard the uid 0...
    $sandbox['max'] = db_query('SELECT COUNT(DISTINCT uid) FROM {users}')->fetchField() - 1;
  }

  $users = db_select('users', 'u')
    ->fields('u', array('uid', 'name'))
    ->condition('uid', $sandbox['current_uid'], '>')
    ->range(0, 3)
    ->orderBy('uid', 'ASC')
    ->execute();

  foreach ($users as $user) {
    $user->name .= '!';
    db_update('users')
      ->fields(array('name' => $user->name))
      ->condition('uid', $user->uid)
      ->execute();

    $sandbox['progress']++;
    $sandbox['current_uid'] = $user->uid;
  }

  $sandbox['#finished'] = empty($sandbox['max']) ? 1 : ($sandbox['progress'] / $sandbox['max']);

  // To display a message to the user when the rollback is completed, return it.
  // If you do not want to display a completion message, simply return nothing.
  return t('The rollback did what it was supposed to do.');

  // In case of an error, simply throw an exception with an error message.
  throw new DrupalUpdateException('Something went wrong; here is what you should do.');
}
