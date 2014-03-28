<?php
/**
 * New or modified drush functionality that should eventually go upstream.
 */

/**
 * Async version of _drush_proc_open.
 *
 * @param $cmd
 *    The command to execute. This command already needs to be properly escaped.
 * @param null $post_options
 * @param null $context
 *    An associative array that will be JSON encoded and passed to the script being called.
 *    Objects are not allowed, as they do not json_decode gracefully.
 * @param array $backend_options
 * @return array
 *   False if the command could not be executed, or did not return any output.
 *   If it executed successfully, it returns an associative array containing the command
 *   called, the output of the command, and the error code of the command.
 */
function _drush_deploy_proc_open($cmd, $post_options = NULL, $context = NULL, $backend_options = array()) {

  $descriptorspec = array(
    0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
    1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
    2 => array("pipe", "w")   // stderr is a pipe the child will write to
  );
  $open_processes = array();
  $bucket = array();

  // Initiate processes for each command.
  foreach ($cmd as $site => $c) {
    $process = array();
    drush_log($backend_options[$site]['output-label'] . $c);
    $process['process'] = proc_open($c, $descriptorspec, $process['pipes'], null, null, array('context' => $context));

    if (is_resource($process['process'])) {
      if ($post_options) {
        fwrite($process['pipes'][0], json_encode($post_options)); // pass the data array in a JSON encoded string
      }
      // If we do not close stdin here, then we cause a deadlock;
      // see: http://drupal.org/node/766080#comment-4309936
      // If we reimplement interactive commands to also use
      // _drush_proc_open, then clearly we would need to keep
      // this open longer.
      //fclose($current_process['pipes'][0]);
      fclose($process['pipes'][0]);

      $process['info'] = stream_get_meta_data($process['pipes'][1]);
      stream_set_blocking($process['pipes'][1], TRUE);
      stream_set_timeout($process['pipes'][1], 1);
      $bucket[$site]['cmd'] = $c;
      $bucket[$site]['backend_opts'] = $backend_options[$site];
      $open_processes[$site] = $process;
    }
  }

  while (sizeof($open_processes)) {
    foreach ($open_processes as $site => &$current_process) {
      $bucket[$site]['output'] = '';
      if (isset($current_process['pipes'][1]) || isset($current_process['pipes'][2])) {
        foreach (array(1, 2) as $pipe) {
          if (isset($current_process['pipes'][$pipe])) {
            $bucket[$site][$pipe] = '';
            $info = stream_get_meta_data($current_process['pipes'][$pipe]);
            //$output = '';
            $end_of_output = FALSE;
            $outputted = FALSE;

            if (!feof($current_process['pipes'][$pipe]) && !$info['timed_out']) {
              $string = fread($current_process['pipes'][$pipe], 4096);
              $output_end_pos = strpos($string, 'DRUSH_BACKEND_OUTPUT_START>>>');
              if ($output_end_pos !== FALSE) {
                $trailing_string = substr($string, 0, $output_end_pos);
                drush_backend_parse_packets($trailing_string, $bucket[$site]['backend_opts']);
                _drush_backend_print_output($trailing_string, $bucket[$site]['backend_opts']);
                $end_of_output = TRUE;
              }
              if (!$end_of_output) {
                drush_backend_parse_packets($string, $bucket[$site]['backend_opts']);
                // Pass output through.
                _drush_backend_print_output($string, $bucket[$site]['backend_opts']);
                $outputted = TRUE;
              }
              $bucket[$site][$pipe] .= $string;
              $bucket[$site]['output'] .= $string;
              $info = stream_get_meta_data($current_process['pipes'][$pipe]);
              flush();
            }
            else {
              fclose($current_process['pipes'][$pipe]);
              unset($current_process['pipes'][$pipe]);
              // close the pipe , set a marker
            }
          }
        }
      }
      else {
        // if both pipes are closed for the process, remove it from active loop and add a new process to open.
        $bucket[$site]['code'] = proc_close($current_process['process']);
        unset($open_processes[$site]);
      }
    }
  }
  return $bucket;
  // TODO: Handle bad proc handles
  //}
  //return FALSE;
}


if (!function_exists('drush_backend_parse_packets')) {
  /**
   * Parse out and remove backend packet from the supplied string and
   * invoke the commands.
   */
  function drush_backend_parse_packets(&$string, $backend_options) {
    $packet_regex = strtr(sprintf(DRUSH_BACKEND_PACKET_PATTERN, "([^\0]*)"), array("\0" => "\\0"));
    if (preg_match_all("/$packet_regex/s", $string, $match, PREG_PATTERN_ORDER)) {
      foreach ($match[1] as $packet_data) {
        $entry = (array) json_decode($packet_data);
        if (is_array($entry) && isset($entry['packet'])) {
          $function = 'drush_backend_packet_' . $entry['packet'];
          if (function_exists($function)) {
            $function($entry, $backend_options);
          }
          else {
            drush_log(dt("Unknown backend packet @packet", array('@packet' => $entry['packet'])), 'notice');
          }
        }
        else {
          drush_log(dt("Malformed backend packet"), 'error');
          drush_log(dt("Bad packet: @packet", array('@packet' => print_r($entry, TRUE))), 'debug');
          drush_log(dt("String is: @str", array('@str' => $packet_data), 'debug'));
        }
      }

      $string = trim(preg_replace("/$packet_regex/s", '', $string));
    }
  }
}

if (!function_exists('_drush_backend_print_output')) {
  /**
   * Print the output received from a call to backend invoke,
   * adding the label to the head of each line if necessary.
   */
  function _drush_backend_print_output($output_string, $backend_options) {
    if ($backend_options['output'] && !empty($output_string)) {
      $output_label = array_key_exists('output-label', $backend_options) ? $backend_options['output-label'] : FALSE;
      if ($output_label) {
        // Remove one, and only one newline from the end of the
        // string. Else we'll get an extra 'empty' line.
        foreach (explode("\n", preg_replace('/\\n$/', '', $output_string)) as $line) {
          fwrite(STDOUT, $output_label . rtrim($line) . "\n");
        }
      }
      else {
        fwrite(STDOUT, $output_string);
      }
    }
  }
}
