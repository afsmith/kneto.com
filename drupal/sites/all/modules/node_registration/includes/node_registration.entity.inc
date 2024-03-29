<?php

/**
 * @file
 * Entity hooks and callbacks for registrations.
 */

/**
 * Implements hook_entity_info().
 */
function node_registration_entity_info() {
  $module_path = _node_registration_type_to_uri('node_registration');

  $bundles = array();

  // Node types + specific nodes.
  foreach (_node_registration_bundles() as $type => $name) {
    $bundles[$type] = array(
      'label' => $name . ' registration',
      'name' => $name,
      'type' => $type,
      'admin' => array(
        'path' => 'admin/structure/' . $module_path . '/manage/%node_registration_type',
        'real path' => 'admin/structure/' . $module_path . '/manage/' . _node_registration_type_to_uri($type),
        'bundle argument' => 4,
        'access arguments' => array('administer node registration'),
      ),
    );
  }

  return array(
    'node_registration' => array(
      'module' => 'node_registration',
      'label' => t('Node registration'),
      'controller class' => 'NodeRegistrationController',
      'entity class' => 'NodeRegistrationEntityClass',
      'base table' => 'node_registration',
      'fieldable' => TRUE,
      'entity keys' => array(
        'id' => 'registration_id',
        'bundle' => 'type',
      ),
      'bundle keys' => array(
        'bundle' => 'type',
      ),
      'bundles' => $bundles,
      'view modes' => array(
        'full' => array(
          'label' => t('Full Registration'),
          'custom settings' => FALSE,
        ),
        'simple' => array(
          'label' => t('Simple Registration'),
          'custom settings' => FALSE,
        ),
      ),
      'uri callback' => 'node_registration_uri',
      'token type' => 'node-registration',
    ),
  );
}

/**
 * Retrieves a node registration bundle by its name.
 */
function node_registration_type_load($bundle_name) {
  $bundles = field_info_bundles('node_registration');

  $bundle_name = _node_registration_uri_to_type($bundle_name);
  if (isset($bundles[$bundle_name])) {
    return (object) $bundles[$bundle_name];
  }

  return FALSE;
}

/**
 * Page title callback for the Node registration type (admin) pages.
 */
function node_registration_type_page_title($bundle) {
  return $bundle['label'];
}

/**
 * URI callback for the Node registration type.
 */
function node_registration_uri($registration) {
  $registration_id = is_scalar($registration) ? (int) $registration : (int) $registration->registration_id;
  return array(
    'path' => 'registration/' . $registration_id,
  );
}

/**
 * Controller class Node Registrations
 */
class NodeRegistrationController extends EntityAPIController {

  /**
   * Override create() to add registration properties.
   */
  public function create(array $values = array()) {
    $node = node_load($values['nid']);
    $values['node'] = $node;

    // The dynamic type: fields per registration type or per node.
    $values['type'] = $node->registration->private_fields ? 'node_' . $node->nid : $node->type;
    // The node type.
    $values['node_type'] = $node->type;

    return parent::create($values);
  }

}

/**
 * Entity class for Node Registrations
 */
class NodeRegistrationEntityClass extends Entity {

  public $is_new = 0;
  public $registration_id = 0;
  public $language = LANGUAGE_NONE;
  public $slots = 1;
  public $attended = 0;
  public $verified = 0;
  public $cancelled = 0;
  public $waitinglist = 0;

  /**
   * Override buildContent() to add registration properties.
   */
  public function buildContent($view_mode = 'full', $langcode = NULL) {
    $content = parent::buildContent($view_mode, $langcode);

    $node = node_load($this->nid);

    return array(
      '#theme' => 'node_registration',
      '#registration' => $this,
      '#content' => $content,
      '#node' => $node,
    );
  }

  /**
   * Override save() to ensure the right ->type
   */
  public function save() {
    return parent::save();
  }

}

/**
 * Implements hook_entity_property_info_alter().
 */
function node_registration_entity_property_info_alter(&$info) {
  // NODE properties.
  $properties = &$info['node']['properties'];

  // Event start.
  /*$properties['event_start'] = array(
    'label' => t("Event start"),
    'description' => t("Event start according to node type setup."),
    'type' => 'date',
    'getter callback' => 'node_registration_metadata_entity_get_properties',
  );

  // Time left.
  $properties['time_left'] = array(
    'label' => t("Time left"),
    'description' => t("Time left according to node type setup."),
    'type' => 'text',
    'getter callback' => 'node_registration_metadata_entity_get_properties',
  );*/

  // Unset pointer. I'm paranoid about PHP references.
  unset($properties);

  // REGISTRATION properties.
  $properties = &$info['node_registration']['properties'];

  // E-mail.
  $properties['email'] = array(
    'label' => t("E-mail address"),
    'description' => t("The registree's e-mail address."),
    'type' => 'text',
    'schema field' => 'email',
  );

  // Node type.
  $properties['node_type'] = array(
    'label' => t("Node type"),
    'description' => t("The node type's machine name."),
    'type' => 'text',
    'schema field' => 'node_type',
  );

  // Slots.
  $properties['slots'] = array(
    'label' => t("Slots"),
    'description' => t("The number of slots consumed by this user."),
    'type' => 'integer',
    'schema field' => 'slots',
  );

  // Created.
  $properties['created'] = array(
    'label' => t("Created"),
    'description' => t("The date the registration was created."),
    'type' => 'date',
    'schema field' => 'created',
  );

  // Updated.
  $properties['updated'] = array(
    'label' => t("Updated"),
    'description' => t("The date the registration was updated."),
    'type' => 'date',
    'schema field' => 'updated',
  );

  // Node entity.
  $properties['nid'] = array('label' => t('Node nid'), 'type' => 'integer');
  $properties['node'] = array(
    'label' => t("Node"),
    'description' => t("The node this registration is associated with."),
    'type' => 'node',
    'schema field' => 'nid',
  );

  // Author user entity.
  $properties['author_uid'] = array('label' => t('Author uid'), 'type' => 'integer');
  $properties['author'] = array(
    'label' => t("Author"),
    'description' => t("The user that <strong>made</strong> this registration."),
    'type' => 'user',
    'schema field' => 'author_uid',
  );

  // Registree user entity.
  $properties['uid'] = array('label' => t('User uid'), 'type' => 'integer');
  $properties['user'] = array(
    'label' => t("User"),
    'description' => t("The user that's registered."),
    'type' => 'user',
    'schema field' => 'uid',
  );

  // View URL.
  $properties['url'] = array(
    'label' => t("View URL"),
    'description' => t("The URL to view the registration."),
    'type' => 'uri',
    'getter callback' => 'node_registration_metadata_entity_get_properties',
  );

  // Edit URL.
  $properties['edit_url'] = array(
    'label' => t("Edit URL"),
    'description' => t("The URL to edit the registration."),
    'type' => 'uri',
    'getter callback' => 'node_registration_metadata_entity_get_properties',
  );

  // Booleans.
  $properties['attended'] = array(
    'label' => t("Attended"),
    'description' => t("Whether the registree attended the event."),
    'type' => 'boolean',
    'schema field' => 'attended',
  );
  $properties['verified'] = array(
    'label' => t("Verified"),
    'description' => t("Whether the registration was verified."),
    'type' => 'boolean',
    'schema field' => 'verified',
  );
  $properties['cancelled'] = array(
    'label' => t("Cancelled"),
    'description' => t("Whether the registration was cancelled."),
    'type' => 'boolean',
    'schema field' => 'cancelled',
  );
  $properties['reminder_sent'] = array(
    'label' => t("Reminder sent"),
    'description' => t("Whether the registree was sent a reminder."),
    'type' => 'boolean',
    'schema field' => 'reminder_sent',
  );
  $properties['waitinglist'] = array(
    'label' => t("Waiting list"),
    'description' => t("Whether the registration is on the waiting list."),
    'type' => 'boolean',
    'schema field' => 'waitinglist',
  );
}

/**
 * Callback for getting properties of an entity.
 */
function node_registration_metadata_entity_get_properties($entity, array $options, $name, $entity_type) {
  switch ($entity_type) {
    // REGISTRATION properties.
    case 'node_registration':
      switch ($name) {
        // View URL.
        case 'url':
          $return = entity_uri($entity_type, $entity);
          return url($return['path'], $return['options'] + $options);

        // Edit URL.
        case 'edit_url':
          $return = entity_uri($entity_type, $entity);
          $return['path'] .= '/edit';
          if (!$entity->uid) {
            $return['options']['query'] = array('secret' => $entity->secret);
          }
          return url($return['path'], $return['options'] + $options);
      }
      break;
  }
}

/**
 * Loads a registration by ID.
 */
function node_registration_load($registration_id, $reset = FALSE) {
  if (empty($registration_id)) {
    return FALSE;
  }

  $registrations = node_registration_load_multiple(array($registration_id), array(), $reset);
  return $registrations ? reset($registrations) : FALSE;
}

/**
 * Loads multiple registrations by ID or based on a set of matching conditions.
 */
function node_registration_load_multiple($registration_ids = FALSE, $conditions = array(), $reset = FALSE) {
  if (empty($registration_ids) && empty($conditions)) {
    return array();
  }

  return entity_load('node_registration', $registration_ids, $conditions, $reset);
}

/**
 * Deletes multiple registrations by ID.
 */
function node_registration_delete_multiple($registration_ids) {
  return entity_get_controller('node_registration')->delete($registration_ids);
}

/**
 * Saves a registration.
 */
function node_registration_save($registration) {
  // Updated NOW.
  $registration->updated = REQUEST_TIME;

  // Created NOW only if new.
  if (empty($registration->created)) {
    $registration->created = REQUEST_TIME;
  }

  // Do entity callback.
  return entity_get_controller('node_registration')->save($registration);
}

/**
 * Proxy for the fields widget handlers.
 */
function _node_registration_fields_callback($callback, $registration, &$form, &$form_state) {
  // Do field callback.
  return $callback('node_registration', $registration, $form, $form_state);
}
