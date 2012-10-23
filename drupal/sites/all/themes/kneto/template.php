<?php

// Altering the user login block
function kneto_form_alter(&$form, &$form_state, $form_id) {
  if ($form_id == 'user_login_block') {
	$form['#action'] = url(current_path(), array('query' => drupal_get_destination(), 'external' => FALSE));
	$form['name'] = array(
	  '#type' => 'textfield',	
	  '#title' => NULL, // Remove label
	  '#maxlength' => USERNAME_MAX_LENGTH, 
	  '#size' => 15, 
	  '#required' => TRUE,
	);
	$form['name']['#attributes']['placeholder'] = t('Membername'); // Add placeholder for Username field
	$form['pass'] = array(
	  '#type' => 'password', 
	  '#title' => NULL, // Remove label
	  '#maxlength' => 60, 
	  '#size' => 15, 
	  '#required' => TRUE,
	);
	$form['pass']['#attributes']['placeholder'] = t('Password'); // Add placeholder for Password field
	$form['links'] = NULL; // Remove links from User login block
  }
}

// Add title of current page to breadcrumb
function kneto_breadcrumb($variables) {
  $breadcrumb = $variables['breadcrumb'];
  if (!empty($breadcrumb)) {
    // Adding the title of the current page to the breadcrumb.
    $breadcrumb[] = drupal_get_title();
   
    // Provide a navigational heading to give context for breadcrumb links to
    // screen-reader users. Make the heading invisible with .element-invisible.
    $output = '<h2 class="element-invisible">' . t('You are here') . '</h2>';

    $output .= '<div class="breadcrumb">' . implode(' » ', $breadcrumb) . '</div>';
    return $output;
  }
}

// Add delimiters to main menu
function kneto_links($variables) {
  $links = $variables['links'];
  $attributes = $variables['attributes'];
  $heading = $variables['heading'];
  global $language_url;
  $output = '';
  $delimiter = '';
  
 if (isset($attributes['id'])) {
    if ($attributes['id'] == 'main-menu') {
      // 1. Change the delimiter to what you want.
      $delimiter = '<li class="delimiter"></li>';
        }
     }

  if (count($links) > 0) {
    $output = '';

    // Treat the heading first if it is present to prepend it to the
    // list of links.
    if (!empty($heading)) {
      if (is_string($heading)) {
        // Prepare the array that will be used when the passed heading
        // is a string.
        $heading = array(
          'text' => $heading,
          // Set the default level of the heading. 
          'level' => 'h2',
        );
      }
      $output .= '<' . $heading['level'];
      if (!empty($heading['class'])) {
        $output .= drupal_attributes(array('class' => $heading['class']));
      }
      $output .= '>' . check_plain($heading['text']) . '</' . $heading['level'] . '>';
    }

    $output .= '<ul' . drupal_attributes($attributes) . '>';

    $num_links = count($links);
    $i = 1;

    foreach ($links as $key => $link) {
      $class = array($key);

      // Add first, last and active classes to the list of links to help out themers.
      if ($i == 1) {
        $class[] = 'first';
      }
      if ($i == $num_links) {
        $class[] = 'last';
      }
      if (isset($link['href']) && ($link['href'] == $_GET['q'] || ($link['href'] == '<front>' && drupal_is_front_page()))
           && (empty($link['language']) || $link['language']->language == $language_url->language)) {
        $class[] = 'active';
      }
      $output .= '<li' . drupal_attributes(array('class' => $class)) . '>';

      if (isset($link['href'])) {
        // Pass in $link as $options, they share the same keys.
        $output .= l($link['title'], $link['href'], $link);
      }
      elseif (!empty($link['title'])) {
        // Some links are actually not links, but we wrap these in <span> for adding title and class attributes.
        if (empty($link['html'])) {
          $link['title'] = check_plain($link['title']);
        }
        $span_attributes = '';
        if (isset($link['attributes'])) {
          $span_attributes = drupal_attributes($link['attributes']);
        }
        $output .= '<span' . $span_attributes . '>' . $link['title'] . '</span>';
      }

    
        if ($i == $num_links) {
          $delimiter = '';
        }
        
      $i++;
      $output .= "</li>" . $delimiter . "\n";
    }

    $output .= '</ul>';
  }

  return $output;
}

?>