<?php

/**
 * @file
 * Provides support for the Views module.
 */

/**
 * Implements hook_views_pre_render().
 */
function og_subgroups_views_pre_render(&$view) {
  if ($view->name == 'og_members_admin') {
    // Add an overview of the users that are inherited from parent groups.
    $inherited_users = _og_subgroups_get_inherited_users($view->args[0], $view->args[1]);
    $header = array('Name', 'Inherited from');
    $rows = array();
    $membership_states = og_group_content_states();
    if (!empty($inherited_users)) {
      foreach ($inherited_users as $inherited_uid => $data) {
        $row = array();
        $row[] = l($data[0]['user']->name, 'user/' . $inherited_uid);
        $inherited_from = array();
        foreach ($data as $inheritance) {
          $inherited_from[] = t('!parent (@roles)', array(
            '!parent' => l(entity_label($inheritance['inherited_from']['type'], $inheritance['inherited_from']['entity']), 'group/' . $inheritance['inherited_from']['type'] . '/' . $inheritance['inherited_from']['id'] . '/admin/people'),
            '@roles' => implode(', ', $inheritance['roles']),
          ));
        }
        $row[] = implode(', ', $inherited_from);
        $rows[] = $row;
      }
    }
    $view->attachment_after = theme('table', array(
      'caption' => t('Inherited users'),
      'header' => $header,
      'rows' => $rows,
      'empty' => t('No inherited users.'),
    ));
  }

  // Extend og_members block by listing inherited users, as a single list
  if ($view->name == 'og_members' && $view->current_display == 'block_members_single') {
    $group_id = $view->args[0];
    $group_type = $view->args[1];

    $members = array();
    foreach ($view->result as $index => $member) {
      $members[] = $member->uid;
    }

    $groups = _og_subgroups_get_unique_parents($group_type, $group_id) + _og_subgroups_get_unique_children($group_type, $group_id);
    if (!empty($groups)) {
      foreach ($groups as $group) {
        $inherited_view = views_get_view('og_members');
        $inherited_view->set_arguments(array($group['id']));
        $inherited_view->execute('block_1');
        foreach ($inherited_view->result as $inherited_member) {
          if (!in_array($inherited_member->uid, $members)) {
            $members[] = $inherited_member->uid;
            $view->result[] = $inherited_member;
          }
        }
        $inherited_view->destroy();
      }
    }
  }
}

/**
 * Implements hook_views_post_render().
 */
function og_subgroups_views_post_render(&$view, &$output, &$cache) {
  // Extend og_members block by listing inherited users, separated by group
  if ($view->name == 'og_members' && $view->current_display == 'block_members_separated') {
    $group_id = $view->args[0];
    $group_type = $view->args[1];
    $groups = _og_subgroups_get_unique_parents($group_type, $group_id) + _og_subgroups_get_unique_children($group_type, $group_id);
    if (!empty($groups)) {
      foreach ($groups as $group) {
        if (!isset($group['entity'])) {
          $group['entity'] = current(entity_load($group['type'], array($group['id'])));
        }
        $inherited_view = views_get_view('og_members');
        $inherited_view->set_arguments(array($group['id']));
        $inherited_view->dom_id = $view->dom_id; // Avoid a warning
        $output .= theme('og_subgroups_subtitles', array('group' => $group));
        $output .= $inherited_view->render('block_1');
        $inherited_view->destroy();
      }
    }
  }
}

/**
 * Implements hook_views_default_views_alter().
 */
function og_subgroups_views_default_views_alter(&$views) {
  if (isset($views['og_members'])) {
    $view = $views['og_members'];

    // Contextual filter: OG membership: Group ID
    $handler =& $view->display['default']->handler;
    $handler->display->display_options['arguments']['gid']['id'] = 'gid';
    $handler->display->display_options['arguments']['gid']['table'] = 'og_membership';
    $handler->display->display_options['arguments']['gid']['field'] = 'gid';
    $handler->display->display_options['arguments']['gid']['relationship'] = 'og_membership_rel';
    $handler->display->display_options['arguments']['gid']['default_action'] = 'default';
    $handler->display->display_options['arguments']['gid']['title_enable'] = TRUE;
    $handler->display->display_options['arguments']['gid']['title'] = '%1 members';
    $handler->display->display_options['arguments']['gid']['default_argument_type'] = 'node';
    $handler->display->display_options['arguments']['gid']['summary']['number_of_records'] = '0';
    $handler->display->display_options['arguments']['gid']['summary']['format'] = 'default_summary';
    $handler->display->display_options['arguments']['gid']['summary_options']['items_per_page'] = '25';
    $handler->display->display_options['arguments']['gid']['specify_validation'] = TRUE;
    $handler->display->display_options['arguments']['gid']['validate']['type'] = 'og';
    
    // Contextual filter: OG membership: Group_type
    $handler->display->display_options['arguments']['group_type']['id'] = 'group_type';
    $handler->display->display_options['arguments']['group_type']['table'] = 'og_membership';
    $handler->display->display_options['arguments']['group_type']['field'] = 'group_type';
    $handler->display->display_options['arguments']['group_type']['relationship'] = 'og_membership_rel';
    $handler->display->display_options['arguments']['group_type']['default_action'] = 'default';
    $handler->display->display_options['arguments']['group_type']['default_argument_type'] = 'raw';
    $handler->display->display_options['arguments']['group_type']['default_argument_options']['index'] = '0';
    $handler->display->display_options['arguments']['group_type']['summary']['number_of_records'] = '0';
    $handler->display->display_options['arguments']['group_type']['summary']['format'] = 'default_summary';
    $handler->display->display_options['arguments']['group_type']['summary_options']['items_per_page'] = '25';
    $handler->display->display_options['arguments']['group_type']['limit'] = '0';

    // Add two new displays that inherit all other configurations from the default display
    $handler = $view->new_display('block', t('Block (separated by group)'), 'block_members_separated');
    $handler = $view->new_display('block', t('Block (single list)'), 'block_members_single');
  }
}
