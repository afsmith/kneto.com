<?php

/**
 * @file
 * Node settings class.
 *
 * Its objects are created in hook_node_load() and assigned to $node->registration.
 */

/**
 * Node settings class.
 *
 * Will be available in EVERY node->registration.
 */
class NodeRegistrationNodeSettings {

  public $nid = 0;
  public $node_type_settings;
  public $node;
  public $more_settings = array();
  public $cache = array();


  /**
   * Init the node registration node settings object.
   */
  public function __construct( $node ) {
    $nid = (int) $node->nid;
    $type = $node->type;

    // Save parent/owner.
    $this->nid = $nid;
    $this->node = $node;

    // Don't do anything for disabled content types.
    if (!_node_registration_node_type_enabled($type)) {
      return;
    }

    // Node type settings.
    $node_type_settings = _node_registration_node_type_settings($type);
    $this->_fill($node_type_settings);

    // Fetch node settings.
    $settings = db_select('node_registration_node', 'rn')
      ->fields('rn')
      ->condition('nid', $nid, '=')
      ->execute()
      ->fetchAssoc();

    if ($settings) {
      // Store node settings.
      $this->_fill($settings);

      // Serialized settings.
      if ($more_settings = @unserialize($settings['settings'])) {
        $this->more_settings = $more_settings;
        $this->_fill($more_settings);
      }
    }

    // Shortcuts for anonymous and authenticated access by # slots.
    $this->allow_anonymous = 0 < $this->max_slots_per_registration_anonymous;
    $this->allow_authenticated = 0 < $this->max_slots_per_registration_authenticated;
  }


  /**
   * Whether this event's registration is enabled.
   */
  public function enabled() {
    return 1 == (int) $this->setting('status', 0);
  }


  /**
   * The capacity for this event.
   */
  public function capacity() {
    return $this->_cache(__FUNCTION__, function($settings) {
      // Use capacity from node field.
      if ($capacity_field = $settings->capacity_field) {
        $field = @$settings->node->$capacity_field;
        if (isset($field[LANGUAGE_NONE][0]['value'])) {
          return max(0, (int) $field[LANGUAGE_NONE][0]['value']);
        }

        // No max.
        return 0;
      }

      // Use capacity from node settings or registration type settings.
      return max(0, (int) $settings->setting('capacity', 0));
    });
  }


  /**
   * The event's start date + time.
   */
  public function date() {
    if (!isset($this->date_value)) {
      $this->date_value = FALSE;
      if ($date_field = $this->date_field) {
        if ($date = $this->node->$date_field) {
          $date = $date[LANGUAGE_NONE][0]['value'];
          is_numeric($date) && $date = date('c', $date);

          $this->date_value = $date;
        }
      }
    }

    return $this->date_value;
  }


  /**
   * Returns the event's end date + time.
   *
   * @see NodeRegistrationNodeSettings::max_registration_time()
   */
  public function get_end_date() {
    return $this->max_registration_time();
  }


  /**
   * Retrieves the time for a given date field in this node.
   */
  public function time_from_date_field($date_field) {
    if ($date_field) {
      if ($date = $this->node->$date_field) {
        $time = $date[LANGUAGE_NONE][0]['value'];
        is_numeric($time) || $time = strtotime($time);

        return $time;
      }
    }
  }


  /**
   * Retrieves the minimum registration time for this event.
   *
   * The time source is dubious: node field value or node settings `start_date`.
   */
  public function min_registration_time(&$origin = NULL) {
    // Get setting from node field values.
    $date_field = $this->min_registration_date_field;
    if ($time = $this->time_from_date_field($date_field)) {
      $origin = 'node field value (' . $date_field . ')';
      return $time;
    }

    // DEPRECATED: Get setting from node settings.
    if ($this->start_date) {
      $time = strtotime($this->start_date);

      $origin = 'registration node settings: ' . $this->start_date;
      return $time;
    }

    // This event has no minimum registration time.
    $origin = '';
  }


  /**
   * Whether the minimum registration time has passed.
   */
  public function min_registration_time_passed(&$reason = NULL) {
    if ($min_registration_time = $this->min_registration_time($origin)) {
      if ($min_registration_time > REQUEST_TIME) {
        $reason = 'future: ' . $origin;
        return FALSE;
      }
    }

    $reason = '';
    return TRUE;
  }


  /**
   * Retrieves the maximum registration time for this event.
   */
  public function max_registration_time(&$origin = NULL) {
    // Get setting from node field values.
    $date_field = $this->max_registration_date_field;
    if ($time = $this->time_from_date_field($date_field)) {
      $origin = 'node field value (' . $date_field . ')';
      return $time;
    }

    // Get setting from node or registration type settings.
    if ($this->allow_registration_until) {
      if ($event_start = $this->date()) {
        $max_time = _node_registration_strtotime($event_start, $this->allow_registration_until);

        $origin = 'event date (' . $event_start . ') - allow until (' . $this->allow_registration_until . ')';
        return $max_time;
      }
    }

    // This event has no maximum registration time.
    $origin = '';
  }


  /**
   * Whether the maximum registration time has passed.
   */
  public function max_registration_time_passed(&$reason = NULL) {
    if ($max_registration_time = $this->max_registration_time($origin)) {
      if ($max_registration_time < REQUEST_TIME) {
        $reason = 'passed: ' . $origin;
        return TRUE;
      }
    }

    $reason = '';
    return FALSE;
  }


  /**
   * Whether the maximum cancellation time for this event has passed.
   */
  public function max_cancel_time_passed() {
    if ($event_start = $this->date()) {
      $event_start = strtotime($event_start);

      $before = $this->allow_cancellation_until;
      $max_cancel_time = $before ? _node_registration_strtotime($event_start, $before) : FALSE;

      if ($max_cancel_time && $max_cancel_time <= REQUEST_TIME) {
        return TRUE;
      }
    }
  }


  /**
   * Whether the reminder time for this event has passed.
   */
  public function reminder_time_passed() {
    if ($event_start = $this->date()) {
      $event_start = strtotime($event_start);

      $before = $this->send_reminders_before;
      $reminder_time = $before ? _node_registration_strtotime($event_start, $before) : FALSE;

      if ($reminder_time && $reminder_time <= REQUEST_TIME) {
        return TRUE;
      }
    }
  }


  /**
   * Returns a given setting by name.
   */
  public function setting($name, $alternative = NULL) {
    // Get setting from node settings.
    if (isset($this->$name)) {
      return $this->$name;
    }

    // Get setting from registration type settings.
    elseif (isset($this->node_type_settings->$name)) {
      return $this->node_type_settings->$name;
    }

    return $alternative;
  }


  /**
   * Updates this node's registration settings.
   */
  public function update($settings) {
    $allowed_native = array('nid' => 0, 'capacity' => 1, 'status' => 1, 'private_fields' => 1);
    $allowed_extra = _node_registration_defaults();

    $extra = $this->more_settings;
    $native = array();
    foreach ($settings as $name => $value) {
      if (isset($allowed_native[$name])) {
        if ($allowed_native[$name]) {
          $native[$name] = $value;
        }
      }
      else {
        if (isset($allowed_extra[$name])) {
          $extra[$name] = $value;
        }
      }
    }

    $native['settings'] = serialize($extra);

    return db_merge('node_registration_node')
      ->key(array('nid' => $this->nid))
      ->fields($native)
      ->execute();
  }


  /**
   * Save values into the settings object.
   */
  protected function _fill($values) {
    foreach ($values as $name => $value) {
      if (NULL !== $value) {
        $this->$name = $value;
      }
    }
  }


  /**
   * Fallback for missing settings: NULL instead of a PHP notice.
   */
  public function __get($name) {
    return $this->setting($name);
  }

  /**
   * Internal (per object) cache helper.
   */
  public function _cache($name, $callback) {
    if (!isset($this->cache[$name])) {
      $this->cache[$name] = $callback($this);
    }

    return $this->cache[$name];
  }

}
