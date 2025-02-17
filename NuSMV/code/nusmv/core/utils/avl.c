/* ---------------------------------------------------------------------------


  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and FBK-irst. 

  NuSMV version 2 is free software; you can redistribute it and/or 
  modify it under the terms of the GNU Lesser General Public 
  License as published by the Free Software Foundation; either 
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. 

-----------------------------------------------------------------------------*/

/*!
  \author Originated from glu library of VIS
  \brief AVL TREE

  \todo: Missing description

*/


#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/StreamMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/node/NodeMgr.h"
#include <stdio.h>
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/avl.h"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define HEIGHT(node) (node == NIL(avl_node) ? -1 : (node)->height)

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define BALANCE(node) (HEIGHT((node)->right) - HEIGHT((node)->left))

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define compute_height(node) {				\
    int x=HEIGHT(node->left), y=HEIGHT(node->right);	\
    (node)->height = MAX(x,y) + 1;			\
}

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define COMPARE(key, nodekey, compare)	 		\
    ((compare == avl_numcmp) ? 				\
	(long) key - (long) nodekey : 			\
	(*compare)(key, nodekey))

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define STACK_SIZE	50

static int do_check_tree(StreamMgr_ptr streams, avl_node* node, int (*compar)(char*, char*), int* error);
static avl_node* find_rightmost(register avl_node** node_p);
static void do_rebalance(register avl_node*** stack_nodep,
                         register int stack_n);
static int rotate_left(register avl_node** node_p);
static int rotate_right(register avl_node** node_p);
static avl_node* avl_new_node(char* key, char* value);
static int avl_check_tree(avl_tree* tree, StreamMgr_ptr streams);
static void avl_walk_forward(avl_node* node, void (*func)(char*, char*));
static void free_entry(avl_node* node,
                       void (*key_free)(char*),
                       void (*value_free)(char*));

avl_tree *
avl_init_table(int (*compar)(char*, char*))
{
  avl_tree* tree;

  tree = ALLOC(avl_tree, 1);
  tree->root = NIL(avl_node);
  tree->compar = compar;
  tree->num_entries = 0;
  return tree;
}


int avl_lookup(avl_tree* tree, register char* key, char** value_p)
{
  register avl_node *node;
  register int (*compare)(char*, char*) = tree->compar, diff;

  node = tree->root;
  while (node != NIL(avl_node)) {
    diff = COMPARE(key, node->key, compare);
    if (diff == 0) {
	    /* got a match */
	    if (value_p != NIL(char *)) *value_p = node->value;
	    return 1;
    }
    node = (diff < 0) ? node->left : node->right;
  }
  return 0;
}

int avl_first(avl_tree* tree, char** key_p, char** value_p)
{
  register avl_node *node;

  if (tree->root == 0) {
    return 0;		/* no entries */
  } else {
    /* walk down the tree; stop at leftmost leaf */
    for(node = tree->root; node->left != 0; node = node->left) {
    }
    if (key_p != NIL(char *)) *key_p = node->key;
    if (value_p != NIL(char *)) *value_p = node->value;
    return 1;
  }
}

int avl_last(avl_tree* tree, char** key_p, char** value_p)
{
  register avl_node *node;

  if (tree->root == 0) {
    return 0;		/* no entries */
  } else {
    /* walk down the tree; stop at rightmost leaf */
    for(node = tree->root; node->right != 0; node = node->right) {
    }
    if (key_p != NIL(char *)) *key_p = node->key;
    if (value_p != NIL(char *)) *value_p = node->value;
    return 1;
  }
}


int avl_insert(avl_tree* tree, char* key, char* value)
{
  register avl_node **node_p, *node;
  register int stack_n = 0;
  register int (*compare)(char*, char*) = tree->compar;
  avl_node **stack_nodep[STACK_SIZE];
  int diff, status;

  node_p = &tree->root;

  /* walk down the tree (saving the path); stop at insertion point */
  status = 0;
  while ((node = *node_p) != NIL(avl_node)) {
    stack_nodep[stack_n++] = node_p;
    diff = COMPARE(key, node->key, compare);
    if (diff == 0) status = 1;
    node_p = (diff < 0) ? &node->left : &node->right;
  }

  /* insert the item and re-balance the tree */
  *node_p = avl_new_node(key, value);
  do_rebalance(stack_nodep, stack_n);
  tree->num_entries++;
  tree->modified = 1;
  return status;
}


int avl_find_or_add(avl_tree* tree, char* key, char*** slot_p)
{
  register avl_node **node_p, *node;
  register int stack_n = 0;
  register int (*compare)(char*, char*) = tree->compar;
  avl_node **stack_nodep[STACK_SIZE];
  int diff;

  node_p = &tree->root;

  /* walk down the tree (saving the path); stop at insertion point */
  while ((node = *node_p) != NIL(avl_node)) {
    stack_nodep[stack_n++] = node_p;
    diff = COMPARE(key, node->key, compare);
    if (diff == 0) {
	    if (slot_p != 0) *slot_p = &node->value;
	    return 1;		/* found */
    }
    node_p = (diff < 0) ? &node->left : &node->right;
  }

  /* insert the item and re-balance the tree */
  *node_p = avl_new_node(key, NIL(char));
  do_rebalance(stack_nodep, stack_n);
  tree->num_entries++;
  tree->modified = 1;
  if (slot_p != 0) *slot_p = &node->value;
  return 0;			/* not already in tree */
}


int avl_delete(avl_tree* tree, char** key_p, char** value_p)
{
  register avl_node **node_p, *node, *rightmost;
  register int stack_n = 0;
  char *key = *key_p;
  int (*compare)(char*, char*) = tree->compar, diff;
  avl_node **stack_nodep[STACK_SIZE];
    
  node_p = &tree->root;

  /* Walk down the tree saving the path; return if not found */
  while ((node = *node_p) != NIL(avl_node)) {
    diff = COMPARE(key, node->key, compare);
    if (diff == 0) goto delete_item;
    stack_nodep[stack_n++] = node_p;
    node_p = (diff < 0) ? &node->left : &node->right;
  }
  return 0;		/* not found */

  /* prepare to delete node and replace it with rightmost of left tree */
 delete_item:
  *key_p = node->key;
  if (value_p != 0) *value_p = node->value;
  if (node->left == NIL(avl_node)) {
    *node_p = node->right;
  } else {
    rightmost = find_rightmost(&node->left);
    rightmost->left = node->left;
    rightmost->right = node->right;
    rightmost->height = -2; 	/* mark bogus height for do_rebal */
    *node_p = rightmost;
    stack_nodep[stack_n++] = node_p;
  }
  FREE(node);

  /* work our way back up, re-balancing the tree */
  do_rebalance(stack_nodep, stack_n);
  tree->num_entries--;
  tree->modified = 1;
  return 1;
}


static void avl_record_gen_forward(avl_node* node, avl_generator* gen)
{
  if (node != NIL(avl_node)) {
    avl_record_gen_forward(node->left, gen);
    gen->nodelist[gen->count++] = node;
    avl_record_gen_forward(node->right, gen);
  }
}


static void avl_record_gen_backward(avl_node* node, avl_generator* gen)
{
  if (node != NIL(avl_node)) {
    avl_record_gen_backward(node->right, gen);
    gen->nodelist[gen->count++] = node;
    avl_record_gen_backward(node->left, gen);
  }
}


avl_generator* avl_init_gen(avl_tree* tree, int dir)
{
  avl_generator *gen;

  /* what a hack */
  gen = ALLOC(avl_generator, 1);
  gen->tree = tree;
  gen->nodelist = ALLOC(avl_node *, avl_count(tree));
  gen->count = 0;
  if (dir == AVL_FORWARD) {
    avl_record_gen_forward(tree->root, gen);
  } else {
    avl_record_gen_backward(tree->root, gen);
  }
  gen->count = 0;

  /* catch any attempt to modify the tree while we generate */
  tree->modified = 0;
  return gen;
}


int avl_gen(avl_generator* gen, char** key_p, char** value_p)
{
  avl_node *node;

  if (gen->count == gen->tree->num_entries) {
    return 0;
  } else {
    node = gen->nodelist[gen->count++];
    if (key_p != NIL(char *)) *key_p = node->key;
    if (value_p != NIL(char *)) *value_p = node->value;
    return 1;
  }
}


void avl_free_gen(avl_generator* gen)
{
  FREE(gen->nodelist);
  FREE(gen);
}


static avl_node* find_rightmost(register avl_node** node_p)
{
  register avl_node *node;
  register int stack_n = 0;
  avl_node **stack_nodep[STACK_SIZE];

  node = *node_p;
  while (node->right != NIL(avl_node)) {
    stack_nodep[stack_n++] = node_p;
    node_p = &node->right;
    node = *node_p;
  }
  *node_p = node->left;

  do_rebalance(stack_nodep, stack_n);
  return node;
}


static void do_rebalance(register avl_node*** stack_nodep, 
                         register int stack_n)
{
  register avl_node **node_p, *node;
  register int hl, hr;
  int height;

  /* work our way back up, re-balancing the tree */
  while (--stack_n >= 0) {
    node_p = stack_nodep[stack_n];
    node = *node_p;
    hl = HEIGHT(node->left);		/* watch for NIL */
    hr = HEIGHT(node->right);		/* watch for NIL */
    if ((hr - hl) < -1) {
	    rotate_right(node_p);
    } else if ((hr - hl) > 1) {
	    rotate_left(node_p);
    } else {
	    height = MAX(hl, hr) + 1;
	    if (height == node->height) break;
	    node->height = height;
    }
  }
}


static int rotate_left(register avl_node** node_p)
{
  register avl_node *old_root = *node_p, *new_root, *new_right;

  if (BALANCE(old_root->right) >= 0) {
    *node_p = new_root = old_root->right;
    old_root->right = new_root->left;
    new_root->left = old_root;
  } else {
    new_right = old_root->right;
    *node_p = new_root = new_right->left;
    old_root->right = new_root->left;
    new_right->left = new_root->right;
    new_root->right = new_right;
    new_root->left = old_root;
    compute_height(new_right);
  }
  compute_height(old_root);
  compute_height(new_root);

  return 0;
}


static int rotate_right(register avl_node** node_p)
{
  register avl_node *old_root = *node_p, *new_root, *new_left;

  if (BALANCE(old_root->left) <= 0) {
    *node_p = new_root = old_root->left;
    old_root->left = new_root->right;
    new_root->right = old_root;
  } else {
    new_left = old_root->left;
    *node_p = new_root = new_left->right;
    old_root->left = new_root->right;
    new_left->right = new_root->left;
    new_root->left = new_left;
    new_root->right = old_root;
    compute_height(new_left);
  }
  compute_height(old_root);
  compute_height(new_root);

  return 0;
}

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
static void avl_walk_forward(avl_node* node, void (*func)(char*, char*))
{
  if (node != NIL(avl_node)) {
    avl_walk_forward(node->left, func);
    func(node->key, node->value);
    avl_walk_forward(node->right, func);
  }
}


static void avl_walk_backward(avl_node* node, void (*func)(char*, char*))
{
  if (node != NIL(avl_node)) {
    avl_walk_backward(node->right, func);
    (*func)(node->key, node->value);
    avl_walk_backward(node->left, func);
  }
}


void avl_foreach(avl_tree* tree, void (*func)(char*, char*), int direction)
{
  if (direction == AVL_FORWARD) {
    avl_walk_forward(tree->root, func);
  } else {
    avl_walk_backward(tree->root, func);
  }
}

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
static void free_entry(avl_node* node, 
		       void (*key_free)(char*), void (*value_free)(char*))
{
  if (node != NIL(avl_node)) {
    free_entry(node->left, key_free, value_free);
    free_entry(node->right, key_free, value_free);
    if (key_free != 0) (*key_free)(node->key);
    if (value_free != 0) (*value_free)(node->value);
    FREE(node);
  }
}
    

void avl_free_table(avl_tree* tree, 
                    void (*key_free)(char*), void (*value_free)(char*))
{
  free_entry(tree->root, key_free, value_free);
  FREE(tree);
}


int avl_count(avl_tree* tree)
{
  return tree->num_entries;
}

static avl_node* avl_new_node(char* key, char* value)
{
  register avl_node *new;

  new = ALLOC(avl_node, 1);
  new->key = key;
  new->value = value;
  new->height = 0;
  new->left = new->right = NIL(avl_node);
  return new;
}


int avl_numcmp(char* x, char* y)
{
  return (long) x - (long) y;
}

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
static int avl_check_tree(avl_tree* tree, StreamMgr_ptr streams)
{
  int error = 0;
  (void) do_check_tree(streams, tree->root, tree->compar, &error);
  return error;
}

static int do_check_tree(StreamMgr_ptr streams, avl_node* node, int (*compar)(char*, char*), int* error)
{
  int l_height, r_height, comp_height, bal;
    
  if (node == NIL(avl_node)) {
    return -1;
  }

  r_height = do_check_tree(streams, node->right, compar, error);
  l_height = do_check_tree(streams, node->left, compar, error);

  comp_height = MAX(l_height, r_height) + 1;
  bal = r_height - l_height;
    
  if (comp_height != node->height) {
    (void) StreamMgr_print_output(streams, "Bad height for 0x%p: computed=%d stored=%d\n",
                  (void*) node, comp_height, node->height);
    ++*error;
  }

  if (bal > 1 || bal < -1) {
    (void) StreamMgr_print_output(streams, "Out of balance at node 0x%p, balance = %d\n", 
                  (void*) node, bal);
    ++*error;
  }

  if (node->left != NIL(avl_node) && 
      (*compar)(node->left->key, node->key) > 0) {
    (void) StreamMgr_print_output(streams, "Bad ordering between 0x%p and 0x%p", 
                  (void*) node, (void*) node->left);
    ++*error;
  }
    
  if (node->right != NIL(avl_node) && 
      (*compar)(node->key, node->right->key) > 0) {
    (void) StreamMgr_print_output(streams, "Bad ordering between 0x%p and 0x%p", 
                  (void*) node, (void*) node->right);
    ++*error;
  }

  return comp_height;
}
