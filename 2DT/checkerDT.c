/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dynarray.h"
#include "checkerDT.h"


/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T n) {
   Node_T parent;
   const char* npath;
   const char* ppath;
   const char* rest;
   size_t i;
   size_t childCount;

   /* Sample check: a NULL pointer is not a valid node */
   if(n == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   parent = Node_getParent(n);
   if(parent != NULL) {
      npath = Node_getPath(n);

      /* Sample check that parent's path must be prefix of n's path */
      ppath = Node_getPath(parent);
      i = strlen(ppath);
      if(strncmp(npath, ppath, i)) {
         fprintf(stderr, "P's path is not a prefix of C's path\n");
         return FALSE;
      }
      /* Child and parent cannot share same path */
      if(strcmp(npath, ppath)) {
         fprintf(stderr, "P and C have same path\n");
         return FALSE;
      }
      /* Check that next char after parent's path is '/' */
      rest = npath + i;
      if (strstr(rest, "/") != rest) {
         fprintf(stderr, "C is not child of P\n");
         return FALSE;
      }
      /* Sample check that n's path after parent's path + '/'
         must have no further '/' characters */
      rest++;
      if(strstr(rest, "/") != NULL) {
         fprintf(stderr, "C's path has grandchild of P's path\n");
         return FALSE;
      }
      /* Check that child's path continues after parent's */
      if(strstr(rest, "\0") != rest) {
         fprintf(stderr, "C's path does not continue after P's path\n");
         return FALSE;
      }
   }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at n.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise. Specifically checks if
   Node_T invariants are followed and if DT links are 
   correct.
*/
static boolean CheckerDT_treeCheck(Node_T n) {
   size_t c;

   if(n != NULL) {

      /* Sample check on each non-root node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(n))
         return FALSE;


      for(c = 0; c < Node_getNumChildren(n); c++)
      {
         Node_T child = Node_getChild(n, c);

         /* Check that Parent-Child links are correct */
         if (Node_getParent(child) != n) {
            fprintf(stderr, "Children does not have link to correct
            parent\n");
                        return FALSE
                           }

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(child))
            return FALSE;
      }
   }
   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at n.
   Returns FALSE if the number of nodes in the tree does 
   not match count, and returns TRUE otherwise.
*/
static boolean CheckerDT_treeCountCheck(Node_T n, size_t count) {
   size_t c = CheckerDT_nodeCount(n);
   if (c == count) return TRUE;
   fprintf(stderr, "count does not match number of nodes in tree\n");
   else return FALSE;
}

/* 
   Performs a pre-order traversal of the tree rooted at n.
   Returns the number of nodes within that tree.
*/
static size_t CheckerDT_nodeCount(Node_T n) {
   size_t count;
   if (n = NULL) return 0;
   else count = 1;

   for(c = 0; c < Node_getNumChildren(n); c++) {
      Node_T child = Node_getChild(n, c);

      count += CheckerDT_nodeCount(child);
   }

   return count;
}

/* 
   Performs a pre-order traversal of the tree rooted at n.
   Checks that the children of each node are ordered correctly.
   Returns TRUE if lexicographic order is followed, FALSE 
   otherwise.
*/
static size_t CheckerDT_orderCheck(Node_T n) {
   size_t c;
   int compare;

   if(n != NULL) {

      for(c = 0; c < Node_getNumChildren(n) - 1; c++)
      {
         compare = Node_compare(Node_getChild(n, c),
                                Node_getChild(n, c + 1));

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(compare >= 0) {
            fprintf(stderr, "Children do not follow lexicographic 
            order\n");
            return FALSE;
         }
      }

      /* Run pre-order traversal */
      for(c = 0; c < Node_getNumChildren(n); c++) {
         CheckerDT_orderCheck(Node_getChild(n, c));
      }
   }
   return TRUE;
}

/*
   Calls multiple other checks on the tree rooted at x.
   Returns FALSE if any test fails, and TRUE otherwise.
*/
static boolean CheckerDT_treeFullCheck(Node_T n, size_t count) {
   if (!CheckerDT_treeCountCheck(n, count)) return FALSE;
   if (!CheckerDT_treeCheck(n)) return FALSE;
   if (!CheckerDT_orderCheck(n)) return FALSE;
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean isInit, Node_T root, size_t count) {

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!isInit)
      if(count != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }

   /* if the count is 0 or DT not initialized, root should be NULL */
   if(count == 0)
      if (root != NULL) {
         fprintf(stderr, "Empty, but root is not NULL\n");
      }
   if(!isInit)
      if (root != NULL) {
         fprintf(stderr, "Not initialized, but root is not NULL\n");
      }

   /* Now checks invariants recursively at each node from the root. */
   return CheckerDT_treeFullCheck(root, count);
}
