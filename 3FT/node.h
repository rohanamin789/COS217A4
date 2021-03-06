/*--------------------------------------------------------------------*/
/* node.h                                                             */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include <stddef.h>
#include "a4def.h"

/*
   a Node_T is an object that contains a path payload and references to
   the node's parent (if it exists) and children (if they exist).
*/
typedef struct node* Node_T;


/*
   Given a parent node and a directory string dir, returns a new
   Node_T or NULL if any allocation error occurs in creating
   the node or its fields.
   The new structure is initialized to have its path as the parent's
   path (if it exists) prefixed to the directory string parameter,
   separated by a slash. It is also initialized with its parent link
   as the parent parameter value, but the parent itself is not changed
   to link to the new node.  The children links are initialized but
   do not point to any children.
*/


Node_T Node_create(const char* dir, Node_T parent);

/*
   Given a parent node and a file path dir, returns a new
   Node_T or NULL if any allocation error occurs in creating
   the node or its fields.
   The new structure is initialized to have its path as the parent's
   path (if it exists) prefixed to the directory string parameter,
   separated by a slash. It is also initialized with its parent link
   as the parent parameter value, but the parent itself is not changed
   to link to the new node.  The file represented by Node_T has 
   contents (contents) and length (length) */

Node_T Node_addFile(const char* dir, Node_T parent,
                    void* contents, size_t length); 


/* Destroys the entire hierarchy of nodes rooted at n,
  including n itself.
  Returns the number of nodes destroyed.*/
size_t Node_destroy(Node_T n);

/* Changes the file contents of Node_T n by replacing it with newContents
   and changing the file's length to newLength */ 

void Node_changeFileContents(Node_T n,
                             void* newContents, size_t newLength);

/* Returns TRUE (1) if the Node_T n is a File and FALSE (0)  if not */ 
boolean Node_getStatus(Node_T n);

/* Returns a void* with the contents of file stored in Node_T n */ 
void *Node_getFileContents(Node_T n);

/* Returns a size_t of the length of the file stored in Node_T n */
size_t Node_getFileLength(Node_T n);


/* Returns n's path.*/
const char* Node_getPath(Node_T n);

/* Returns the number of child directories n has. */
size_t Node_getNumChildren(Node_T n);

/*
   Returns the child node of n with identifier childID, if one exists,
   otherwise returns NULL.
*/
Node_T Node_getChild(Node_T n, size_t childID);

/*
   Returns the parent node of n, if it exists, otherwise returns NULL
*/
Node_T Node_getParent(Node_T n);

/*
  Makes child a child of parent, if possible, and returns SUCCESS.
  This is not possible in the following cases:
  * child's path is not parent's path + / + directory,
    in which case returns PARENT_CHILD_ERROR
    * parent already has a child with child's path,
    in which case returns ALREADY_IN_TREE
    * parent is unable to allocate memory to store new child link,
    in which case returns MEMORY_ERROR
*/
int Node_linkChild(Node_T parent, Node_T child);

/*
  Unlinks node parent from its child node child. child is unchanged.
  Returns PARENT_CHILD_ERROR if child is not a child of parent,
  and SUCCESS otherwise.
*/
int Node_unlinkChild(Node_T parent, Node_T child);

/*
  Creates a new node such that the new node's path is dir appended to
  n's path, separated by a slash, and that the new node has no
  children of its own. The new node's parent is n, and the new node is
  added as a child of n.
  (Reiterating for clarity: unlike with Node_create, parent *is*
  changed so that the link is bidirectional.)
  Returns SUCCESS upon completion, or:
  MEMORY_ERROR if the new node cannot be created,
  ALREADY_IN_TREE if parent already has a child with that path
*/
int Node_addChild(Node_T parent, const char* dir);

/*
  Returns a string representation for n, 
  or NULL if there is an allocation error.
  Allocates memory for the returned string,
  which is then owned by client!
*/
char* Node_toString(Node_T n);

#endif
