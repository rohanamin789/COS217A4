/*--------------------------------------------------------------------*/
/* node.c                                                             */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "dynarray.h"
#include "node.h"


/*
   A node structure represents a directory in the file tree
*/
struct node {
   /* the full path of this directory/file */
   char* path;

   /* the parent directory of this directory
      NULL for the root of the file tree */
   Node_T parent;

   /* Contents of a file, NULL if a directory */ 
   void* contents;

   /* Length of a file, NULL if a directory */
   size_t length;

   /* Status as a file or directory: 
      TRUE if file and FALSE if directory */
   boolean status; 


   /* the subdirectories of this directory
      stored in sorted order by pathname */
   DynArray_T children;
};


/*
  returns a path with contents
  n->path/dir
  or NULL if there is an allocation error.
  Allocates memory for the returned string,
  which is then owned by the caller!
*/
static char* Node_buildPath(Node_T n, const char* dir) {
   char* path;

   assert(dir != NULL);

   if(n == NULL)
      path = malloc(strlen(dir)+1);
   else
      path = malloc(strlen(n->path) + 1 + strlen(dir) + 1);

   if(path == NULL)
      return NULL;
   *path = '\0'; 

   if(n != NULL) {
      strcpy(path, n->path);
      strcat(path, "/");
   }
   strcat(path, dir);
   
   return path;
}

/*
  Compares node1 and node2 based on their paths.
  Returns <0, 0, or >0 if node1 is less than,
  equal to, or greater than node2, respectively.
*/
static int Node_compare(Node_T node1, Node_T node2) {
   assert(node1 != NULL);
   assert(node2 != NULL);
   if (Node_getStatus(node1) && !Node_getStatus(node2))
      return -1;
   if (!Node_getStatus(node1) && Node_getStatus(node2))
       return 1;
   return strcmp(node1->path, node2->path);
}

/*
   Returns 1 if n has a child directory with path,
   0 if it does not have such a child, and -1 if
   there is an allocation error during search.
   If n does have such a child, and childID is not NULL, store the
   child's identifier in *childID. If n does not have such a child,
   store the identifier that such a child would have in *childID.
*/
static int Node_hasChild(Node_T n, const char* path, size_t* childID) {
   size_t index;
   int result;
   Node_T checker;

   assert(n != NULL);
   assert(path != NULL);
   /* To avoid using a variable before its definition */
   index = 0; 

   checker = Node_create(path, NULL);
   if(checker == NULL) {
      return -1;
   }
   result = DynArray_bsearch(n->children, checker, &index,
                             (int (*)(const void*, const void*)) Node_compare);
   (void) Node_destroy(checker);

   if(childID != NULL)
      *childID = index;

   return result;
}

/* see node.h for specification */
void Node_changeFileContents(Node_T n, void* newContents,
                             size_t newLength){
   assert (n != NULL); 
   n->contents = newContents;
   n->length = newLength; 
}



/* see node.h for specification */
boolean Node_getStatus(Node_T n){ 
   assert (n != NULL); 
   return n->status;
}

/* see node.h for specification */
void *Node_getFileContents(Node_T n){
   assert (n != NULL); 
   return n->contents; 
}

/* see node.h for specification */
size_t Node_getFileLength(Node_T n){
   assert (n != NULL); 
   return n->length; 
}

/* see node.h for specification */

Node_T Node_addFile(const char* dir, Node_T parent,
                    void* contents, size_t length){
   Node_T new;
   assert (parent != NULL);
   assert (dir != NULL);
   new = (Node_T) malloc(sizeof(struct node)); 
   if (new == NULL)
      return NULL;
   new->path = Node_buildPath(parent,dir);
   if(new->path == NULL){
      free(new);
      return NULL;
   }
   new->status = TRUE; 
   new->parent = parent;
   new->contents = contents;
   new->length = length;
   new->children = NULL; 
   return new; 
   
}
/* see node.h for specification */
Node_T Node_create(const char* dir, Node_T parent){
   Node_T new;
   
   assert(dir != NULL);

   new = (Node_T) malloc(sizeof(struct node));
   if(new == NULL) {
      return NULL;
   }

   new->path = Node_buildPath(parent, dir);
   new->status = FALSE; 
   new->contents = NULL;
   new->length = 0; 
   if(new->path == NULL) {
      free(new);
      return NULL;
   }

   new->parent = parent;
   new->children = DynArray_new(0);
   if(new->children == NULL) {
      free(new->path);
      free(new);
      return NULL;
   } 
   return new;
}

/* see node.h for specification */
size_t Node_destroy(Node_T n) {
   size_t i;
   size_t count = 0;
   Node_T c;

   assert(n != NULL);
   if (Node_getStatus(n) != TRUE){
      for(i = 0; i < DynArray_getLength(n->children); i++) {
         if (n->status == FALSE) {
            c = DynArray_get(n->children, i);
            count += Node_destroy(c);
         }
      }
      if (n->children != NULL)
      DynArray_free(n->children);
   }
   free(n->path);
   free(n);
   count++;

   return count;
}

/* see node.h for specification */


const char* Node_getPath(Node_T n) {
   assert(n != NULL);

   return n->path;
}



/* see node.h for specification */
size_t Node_getNumChildren(Node_T n) {
   assert(n != NULL);
   if (n->status == TRUE) return 0;
   assert (n->children != NULL); 

   return DynArray_getLength(n->children);
}



/* see node.h for specification */
Node_T Node_getChild(Node_T n, size_t childID) {
   assert(n != NULL);

   if(DynArray_getLength(n->children) > childID) {
      return DynArray_get(n->children, childID);
   }
   else {
      return NULL;
   }
}

/* see node.h for specification */
Node_T Node_getParent(Node_T n) {
   assert(n != NULL);

   return n->parent;
}





/* see node.h for specification */
int Node_linkChild(Node_T parent, Node_T child) {
   size_t i;
   char* rest;

   assert(parent != NULL);
   assert(child != NULL);
  
   /* Checks if already in tree */ 
   if(Node_hasChild(parent, child->path, NULL)) {
      return ALREADY_IN_TREE;
   }
   i = strlen(parent->path);
   
   /* Checks for parent-child errors */ 
   if(strncmp(child->path, parent->path, i)) {
      return PARENT_CHILD_ERROR;
   }
   rest = child->path + i;
   if(strlen(child->path) >= i && rest[0] != '/') {
      return PARENT_CHILD_ERROR;
   }
   rest++;
   if(strstr(rest, "/") != NULL) {
      return PARENT_CHILD_ERROR;
   }
   child->parent = parent;
   /* Checks if already in tree */ 
   if(DynArray_bsearch(parent->children, child, &i,
                       (int (*)(const void*, const void*)) Node_compare) == 1) {
      return ALREADY_IN_TREE;
   }

   if(DynArray_addAt(parent->children, i, child) == TRUE) {
      return SUCCESS;
   }
   else {
      return PARENT_CHILD_ERROR;
   }
}

/* see node.h for specification */
int  Node_unlinkChild(Node_T parent, Node_T child) {
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);
   /* To avoid using a variable before its definition */
   i = 0; 
   if(DynArray_bsearch(parent->children, child, &i,
                       (int (*)(const void*, const void*)) Node_compare) == 0) {
      return PARENT_CHILD_ERROR;
   }

   (void) DynArray_removeAt(parent->children, i);

   return SUCCESS;
}


/* see node.h for specification */
int Node_addChild(Node_T parent, const char* dir) {
   Node_T new;
   int result;

   assert(parent != NULL);
   assert(dir != NULL);

   new = Node_create(dir, parent);
   if(new == NULL) {
      return PARENT_CHILD_ERROR;
   }
   result = Node_linkChild(parent, new);
   if(result != SUCCESS)
      (void) Node_destroy(new);

   return result;
}


/* see node.h for specification */
char* Node_toString(Node_T n) {
   char* copyPath;

   assert(n != NULL);

   copyPath = malloc(strlen(n->path)+1);
   if(copyPath == NULL) {
      return NULL;
   }
   else {
      return strcpy(copyPath, n->path);
   }
}
