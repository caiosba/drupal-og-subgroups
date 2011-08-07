<?php

/**
 * Plugin defenition
 */
if (module_exists('graphapi')) {
  $plugin = array (
    'title' => t('OG Group hierarchy graph'),
    'description' => t('Display a graph of group hierarchy starting from a given group.'),
    'required context' => new ctools_context_required(t('Group'), 'entity:group'),
    'category' => t('OG subrgoups'),
  );
}

function og_subgroups_graph_content_type_render($subtype, $conf, $args, $context) {
  if (empty($context)) {
    return;
  }

  $block = new stdClass();
  $block->module = 'og_subgroups';
  $block->title = t('OG Group hierarchy graph');

  $group = clone $context->data;
  $graph = graphapi_new_graph();

  og_subgroups_get_reverse_hierarchy_tree_perm(array($group->gid), '' , NULL, $structure, $graph);


  // TODO: Add settings for the options.
  $options = array(
    'width' => 550,
    'height' => 400,
    'item-width' => 50,
  );
  $block->content = graphapi_container($graph, $options);

  return $block;
}

function og_subgroups_graph_content_type_edit_form($form, &$form_state) {
  // provide a blank form so we have a place to have context setting.
  return $form;
}
