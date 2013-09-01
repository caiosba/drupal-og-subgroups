<?php
/**
 * @file
 * Display a inherited group title inside group members block.
 *
 * Available variables:
 * - $group: The inherited group.
 *
 * @ingroup themeable
 */
?>

<h3 class="og-subgroups-members-subtitle">
  <?php print t('Members inherited from group %title', array('%title' => $group['entity']->title)); ?>
</h3>
