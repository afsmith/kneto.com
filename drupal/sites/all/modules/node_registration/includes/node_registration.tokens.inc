<?php

/**
 * @file
 * Token hooks and callbacks.
 */

/**
 * Implements hook_token_info().
 *
 * Defines tokens that don't fit into the entity info alter.
 */
function node_registration_token_info() {
  return array(
    'tokens' => array(
      'node' => array(
        'event-start' => array(
          'name' => t('Event start'),
          'description' => t('Date object of event start date + time, according to Registration settings.'),
          'type' => 'date',
        ),
        'time-left' => array(
          'name' => t('Time left'),
          'description' => t("A la drupal's %function", array('%function' => 'format_interval')),
        ),
      ),
    ),
  );
}

/**
 * Implements hook_tokens().
 */
function node_registration_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $url_options = array('absolute' => TRUE);
  if (isset($options['language'])) {
    $url_options['language'] = $options['language'];
  }

  $replacements = array();

  if (!in_array($type, array('node', 'node-registration'))) {
    return array();
  }

  $node_tokens = array();
  foreach ($tokens as $token => $raw_token) {
    // Node: event-start.
    if ('node' == $type && 0 === strpos($token, 'event-start')) {
      if ($date = $data['node']->registration->date()) {
        $time = strtotime($date);
        $token_name = drupal_substr($token, 12) ?: 'short';
        $tokenized = system_tokens('date', array($token_name => $raw_token), array('date' => $time), $options);
        $replacements += $tokenized;
      }
    }
    // Node: time-left.
    elseif ('node' == $type && 'time-left' == $token) {
      if ($date = $data['node']->registration->date()) {
        $time = strtotime($date);
        $time_left = $time - time();
        $time_left = format_interval($time_left);
        $replacements[$raw_token] = $time_left;
      }
    }
  }

  return $replacements;
}
