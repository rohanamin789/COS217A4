/* ft.c
   Authors: Rohan Amin and Alex Luo */

#include "ft.h"
#include <assert.h>
#include <stddef.h>
#include "a4def.h"
#include "node.h"
#include "dynarray.h"
#include <string.h>
#include <stdlib.h>


/* A File Tree is an AO with 3 state variables: */
/* a flag for if it is in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;

/* a pointer to the root node in the hierarchy */
static Node_T root;

/* a counter of the number of nodes in the hierarchy */
static size_t count;


/*
   Performs a pre-order traversal of the tree rooted at n,
   inserting each payload to DynArray_T d beginning at index i.
   Returns the next unused index in d after the insertion(s).
*/
static size_t FT_preOrderTraversal(Node_T n, DynArray_T d, size_t i) {
   size_t c;

   assert(d != NULL);

   if(n != NULL) {
      (void) DynArray_set(d, i, Node_getPath(n));
      i++;
      for(c = 0; c < Node_getNumChildren(n); c++)
            i = FT_preOrderTraversal(Node_getChild(n, c), d, i);
   }
   return i;
}

/*
   Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length.
*/
static void FT_strlenAccumulate(char* str, size_t* pAcc) {
   assert(pAcc != NULL);

   if(str != NULL)
      *pAcc += (strlen(str) + 1);
}

/*
   Alternate version of strcat that inverts the typical argument
   order, appending str onto acc, and also always adds a newline at
   the end of the concatenated string.
*/
static void FT_strcatAccumulate(char* str, char* acc) {
   assert(acc != NULL);

   if(str != NULL)
      strcat(acc, str); strcat(acc, "\n");
}

/*
   Given a prospective parent and child node,
   adds child to parent's children list, if possible
   If not possible, destroys the hierarchy rooted at child
   and returns PARENT_CHILD_ERROR, otherwise, returns SUCCESS.
*/

static int FT_linkParentToChild(Node_T parent, Node_T child){
   assert (parent != NULL);
   if (Node_getStatus(parent) == TRUE) return NOT_A_DIRECTORY; 

   if(Node_linkChild(parent,child) != SUCCESS) {
      (void) Node_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}

/*
   Destroys the entire hierarchy of nodes rooted at curr,
   including curr itself.
*/

static void FT_removePathFrom(Node_T curr) {
   if(curr != NULL) {
      count -= Node_destroy(curr);
   }
}

/*
   Inserts a new file with contents (contents) and a length (length) 
   into the tree rooted at parent.
   If a node representing path already exists, returns ALREADY_IN_TREE
   If there is an allocation error in creating any of the new nodes or
   their fields, returns MEMORY_ERROR
   If there is an error linking any of the new nodes,
   returns PARENT_CHILD_ERROR
   If there is a path issue, returns CONFLICTING_PATH
   Otherwise, returns SUCCESS
*/
static int FT_appendFiles(char* path, Node_T parent,
                          void* contents, size_t length){
   Node_T curr; 
   Node_T firstNew = NULL;
   Node_T new;
   char* copyPath;
   char* restPath = path;
   char* dirToken;
   int result;
   size_t newCount = 0;
   char* temp; 

   assert (path != NULL);
   curr = parent; 
   if (curr == NULL) {
      /* File cannot be root */ 
      return CONFLICTING_PATH;
   }
   else{
      /* Checks if path is already in Tree */ 
      if(!strcmp(path,Node_getPath(curr)))
         return ALREADY_IN_TREE;
         restPath += (strlen(Node_getPath(curr)) + 1);
   }

   /* Checks if Node is a File */ 
   if (Node_getStatus(curr) == TRUE) return NOT_A_DIRECTORY; 
   copyPath = (char*) malloc(strlen(restPath) + 1);

   /* Returns memory error if memory allocation fails */ 
   if (copyPath == NULL)
      return MEMORY_ERROR;
   strcpy(copyPath, restPath);
   dirToken = strtok(copyPath, "/");

   while (dirToken != NULL){
      temp = dirToken;
      dirToken = strtok(NULL, "/");
      /* When adding a file, only the last token
         in the path is the file */
      if (dirToken == NULL)
         new = Node_addFile(temp, curr, contents, length); 
      else{
         /* Creates directories rather than files
            leading up to the final token */
         new = Node_create(temp, curr);
      }
      newCount++;
      if(firstNew == NULL)
         firstNew = new;
      else{
         result = FT_linkParentToChild(curr, new);
         if (result != SUCCESS) {
            (void) Node_destroy(new); 
            (void) Node_destroy(firstNew);
            free(copyPath);
            return result;
         }
      }
      /* Returns memory error if memory allocation fails */
      if (new == NULL) {
         (void) Node_destroy(firstNew); 
         free(copyPath);
         return MEMORY_ERROR;
      }
      curr = new;
   }
   free(copyPath);
   if(parent == NULL){
      return INITIALIZATION_ERROR;
   }
   else{
      /* Adds new Node to file tree hierarchy */ 
      result = FT_linkParentToChild(parent, firstNew);
      if (result == SUCCESS)
         count += newCount;
      else
         (void) Node_destroy(firstNew);
      return result;
   }        
}
/*
   Inserts a new path into the tree rooted at parent, or, if
   parent is NULL, as the root of the data structure.
   If a node representing path already exists, returns ALREADY_IN_TREE
   If there is an allocation error in creating any of the new nodes or
   their fields, returns MEMORY_ERROR
   If there is an error linking any of the new nodes,
   returns PARENT_CHILD_ERROR
   Otherwise, returns SUCCESS
*/
static int FT_insertRestOfPath(char* path, Node_T parent){
   Node_T curr; 
   Node_T firstNew = NULL;
   Node_T new;
   char* copyPath;
   char* restPath; 
   char* dirToken;
   int result;
   size_t newCount = 0;

   assert (path != NULL);
   restPath = path; 
   curr = parent;
   /* Returns error if there is a conflicting path */
   if (curr == NULL) {
      if(root != NULL) {
         return CONFLICTING_PATH;
      }
   }
   else{
      /* Checks if path is already in Tree */ 
      if(!strcmp(path,Node_getPath(curr)))
         return ALREADY_IN_TREE;
      restPath += (strlen(Node_getPath(curr)) + 1);
   }
   copyPath = malloc(strlen(restPath) + 1);

   /* Returns MEMORY_ERROR if memory allocation fails */ 
   if (copyPath == NULL)
      return MEMORY_ERROR;
   strcpy(copyPath, restPath);
   dirToken = strtok(copyPath, "/");
   while (dirToken != NULL) {
      new = Node_create(dirToken, curr);
      newCount++;

      if(firstNew == NULL)
         firstNew = new;
      else {
         result = FT_linkParentToChild(curr, new);
         /* Reverts changes if the node isn't successfully linked */
         if(result != SUCCESS) {
            (void) Node_destroy(new);
            (void) Node_destroy(firstNew);
            free(copyPath);
            return result;
         }
      }
      if(new == NULL){
         (void) Node_destroy(firstNew);
         free(copyPath);
         return MEMORY_ERROR;
      }
      curr = new;
      dirToken = strtok(NULL, "/");
   }
   /* Frees the memory allocated for local variable */ 
   free(copyPath);

   if(parent == NULL){
      /* If the structure is initialized but the new path 
         is the first element, puts the Node at the root */ 
      root = firstNew;
      count = newCount;
      return SUCCESS;
   }
   else {
      /* Links new node to parent if the Node is not the root */
      result = FT_linkParentToChild(parent, firstNew);
      if(result == SUCCESS)
         count += newCount;
      else
         (void) Node_destroy(firstNew);
      return result;
   }
}
/*
  Removes the directory hierarchy rooted at path starting from
  curr. If curr is the data structure's root, root becomes NULL.
  Returns NO_SUCH_PATH if curr is not the node for path,
  and SUCCESS otherwise.
*/

static int FT_rmPathAt(char* path, Node_T curr){
   Node_T parent;
   assert(path != NULL);
   assert(curr != NULL);
   parent = Node_getParent(curr);
   
   if (Node_getStatus(curr) != FALSE)
      return NOT_A_DIRECTORY; 
   if (!strcmp(path, Node_getPath(curr))){
      if(parent == NULL)
         root = NULL;
      else
         Node_unlinkChild(parent, curr);
      FT_removePathFrom(curr);

      return SUCCESS;

   }
   else
      return NO_SUCH_PATH; 
}



/*
   Starting at the parameter curr, traverses as far down
   the hierarchy as possible while still matching the path
   parameter.
   Returns a pointer to the farthest matching node down that path,
   or NULL if there is no node in curr's hierarchy that matches
   a prefix of the path
*/
static Node_T FT_traversePathFrom(char* path, Node_T curr) {
   Node_T found;
   size_t i;

   assert (path != NULL);
   if(curr == NULL)
      return NULL;
   else if(!strcmp(path, Node_getPath(curr)))
      return curr;
   else if(!strncmp(path, Node_getPath(curr), strlen(Node_getPath(curr)))){
      for(i = 0; i < Node_getNumChildren(curr); i++){
         found = FT_traversePathFrom(path, Node_getChild(curr, i));
         if(found != NULL) 
            return found;
      }
      return curr;
   }
   return NULL; 
}
/*
   Returns the farthest node reachable from the root following a given
   path, or NULL if there is no node in the hierarchy that matches a
   prefix of the path.
*/

static Node_T FT_traversePath(char* path){
   assert(path != NULL);
   return FT_traversePathFrom(path, root);
}





/*
   Inserts a new directory into the tree at path, if possible.
   Returns SUCCESS if the new directory is inserted.
   Returns INITIALIZATION_ERROR if not in an initialized state.
   Returns CONFLICTING_PATH if path is not underneath existing root.
   Returns NOT_A_DIRECTORY if a proper prefix of path exists as a file.
   Returns ALREADY_IN_TREE if the path already exists (as dir or file).
   Returns PARENT_CHILD_ERROR if a new child cannot be added in path.
   Returns MEMORY_ERROR if unable to allocate sufficient memory.
*/

int FT_insertDir(char *path){
   Node_T curr;
   int result;
   assert(path != NULL);
   if(!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   result = FT_insertRestOfPath(path,curr);
   return result;
}


/*
  Returns TRUE if the tree contains the full path parameter as a
  directory and FALSE otherwise.
*/

boolean FT_containsDir(char *path){ 
   Node_T curr;
   boolean result;
   assert(path != NULL);

   if(!isInitialized){
      result = FALSE; 
      return result;
   }

   curr = FT_traversePath(path);

   if(curr == NULL){
      result = FALSE; 
      return result;
   }
   else if(strcmp(path,Node_getPath(curr)))
      result = FALSE;
   else if (Node_getStatus(curr) != FALSE) result = FALSE; 
   else
      result = TRUE;
   return result;   
   
}

/*
  Removes the FT hierarchy rooted at the directory path.
  Returns SUCCESS if found and removed.
  Returns INITIALIZATION_ERROR if not in an initialized state.
  Returns NOT_A_DIRECTORY if path exists but is a file not a directory.
  Returns NO_SUCH_PATH if the path does not exist in the hierarchy.
*/


int FT_rmDir(char *path){
   Node_T curr;
   int result;
   assert(path != NULL);

   if(!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   if(curr == NULL)
      result = NO_SUCH_PATH;
   else
      result = FT_rmPathAt(path, curr);
   return result; 
}



/*
   Inserts a new file into the hierarchy at the given path, with the
   given contents of size length bytes.
   Returns SUCCESS if the new file is inserted.
   Returns INITIALIZATION_ERROR if not in an initialized state.
   Returns CONFLICTING_PATH if path is not underneath existing root, 
                            or if path would be the FT root.
   Returns NOT_A_DIRECTORY if a proper prefix of path exists as a file.
   Returns ALREADY_IN_TREE if the path already exists (as dir or file).
   Returns PARENT_CHILD_ERROR if a new child cannot be added in path.
   Returns MEMORY_ERROR if unable to allocate sufficient memory.
*/

int FT_insertFile(char *path, void *contents, size_t length){
   Node_T curr;
   int result;
   assert (path != NULL);

   if (!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   result = FT_appendFiles(path, curr, contents, length);
   return result; 
}

/*
  Returns TRUE if the tree contains the full path parameter as a
  file and FALSE otherwise.
*/

boolean FT_containsFile(char *path){
   Node_T curr;
   boolean result;
   assert (path != NULL);
   if (!isInitialized)
      return FALSE;
   curr = FT_traversePath(path);

   if(curr == NULL)
      result = FALSE;
   else if(strcmp(path, Node_getPath(curr)))
      result = FALSE;
   else if(Node_getStatus(curr) != TRUE){
      result = FALSE;
      return result; 
   }
   else
      result = TRUE;
   return result; 
}
               
/*
  Removes the FT file at path.
  Returns SUCCESS if found and removed.
  Returns INITIALIZATION_ERROR if not in an initialized state.
  Returns NOT_A_FILE if path exists but is a directory not a file.
  Returns NO_SUCH_PATH if the path does not exist in the hierarchy.
*/

int FT_rmFile(char *path){
   Node_T parent;
   Node_T curr;
   assert (path != NULL); 
   curr = FT_traversePath(path);
   
   if (strcmp(path,Node_getPath(curr))) return NO_SUCH_PATH; 
   else if (Node_getStatus(curr) != TRUE) return NOT_A_FILE; 
   else{
      parent = Node_getParent(curr);
      Node_unlinkChild(parent ,curr);
   }
   FT_removePathFrom(curr);
   return SUCCESS; 
                     
}

/*
  Returns the contents of the file at the full path parameter.
  Returns NULL if the path does not exist or is a directory.
  Note: checking for a non-NULL return is not an appropriate
  contains check -- the contents of a file may be NULL.
*/

void *FT_getFileContents(char *path){
   Node_T curr;
   void* contents;
   assert (path != NULL);
   if (!isInitialized)
      return (void*) INITIALIZATION_ERROR;
   curr = FT_traversePath(path);

   if (curr == NULL)
      contents = NULL;
   else if (Node_getStatus(curr) != TRUE)
      contents = NULL;  
   else
      contents = Node_getFileContents(curr);
   return contents; 
   
}

/*
  Replaces current contents of the file at the full path parameter with
  the parameter newContents of size newLength.
  Returns the old contents if successful. (Note: contents may be NULL.)
  Returns NULL if the path does not already exist or is a directory.
*/

void *FT_replaceFileContents(char *path, void *newContents,
                             size_t newLength){
   Node_T curr;
   void* oldContents; 
   assert (path != NULL);
   if (!isInitialized)
      return NULL;
   curr = FT_traversePath(path);
   if (curr == NULL)
      oldContents = NULL; 
   else if (Node_getStatus(curr) != TRUE) oldContents = NULL; 
   else if(strcmp(path,Node_getPath(curr))) oldContents = NULL; 
   else{
      oldContents = Node_getFileContents(curr); 
      Node_changeFileContents(curr, newContents, newLength); 
   }
   return oldContents; 
}

/*
  Returns SUCCESS if path exists in the hierarchy,
  returns NO_SUCH_PATH if it does not, and
  returns INITIALIZATION_ERROR if the structure is not initialized.
  When returning SUCCESSS,
  if path is a directory: *type is set to FALSE, *length is unchanged
  if path is a file: *type is set to TRUE, and
  *length is set to the length of file's contents.
  When returning a non-SUCCESS status, *type and *length are unchanged.
*/

int FT_stat(char *path, boolean *type, size_t *length){
   Node_T curr; 
   assert (path != NULL);
   if (!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   if (strcmp(path, Node_getPath(curr)))
      return NO_SUCH_PATH; 
   if (Node_getStatus(curr) == FALSE)
      *type = FALSE;
   else{
      *type = TRUE;
      *length = Node_getFileLength(curr); 
   }
   return SUCCESS; 
}


/*
  Sets the data structure to initialized status.
  The data structure is initially empty.
  Returns INITIALIZATION_ERROR if already initialized,
  and SUCCESS otherwise.
*/

int FT_init(void){
   if (isInitialized)
      return INITIALIZATION_ERROR;
   isInitialized = 1;
   root = NULL;
   count = 0;
   return SUCCESS; 
}

/*
  Removes all contents of the data structure and
  returns it to uninitialized status.
  Returns INITIALIZATION_ERROR if not already initialized,
  and SUCCESS otherwise.
*/

int FT_destroy(void){
   if(!isInitialized)
      return INITIALIZATION_ERROR;
   FT_removePathFrom(root);
   root = NULL;
   isInitialized = 0;
   return SUCCESS; 
}

/*
  Returns a string representation of the
  data structure, or NULL if the structure is
  not initialized or there is an allocation error.
  Allocates memory for the returned string,
  which is then owned by client!
*/

char *FT_toString(void){
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char* result = NULL;

   if(!isInitialized)
      return NULL;

   nodes = DynArray_new(count);
   (void) FT_preOrderTraversal(root, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate, (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      return NULL;
   }
   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate, (void *) result);

   DynArray_free(nodes);
   return result;
}
   

