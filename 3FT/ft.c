/* ft.c
   Authors: Rohan Amin and Alex Luo */

#include "ft.h"
#include <assert.h>
#include <stddef.h>
#include "a4def.h"
#include "node.h"
#include "dynarray.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>


static boolean isInitialized;
static Node_T root;
static size_t count;

enum Status {DIRECTORY, FILE}; 

struct node {
   /* the absolute path of the directory */
   char* path;

   /* links to child directories, each NULL if there is no such child.
      invariant: if child1 == NULL, child2 == NULL. */
   DynArray_T children;
   DynArray_T files;
   size_t length;
   boolean file; 
   void* contents; 

   /* link to this directory's parent directory, or NULL if the root */
   Node_T parent;
};

/* struct fileNode{
   char* path;
   void* contents; 
   size_t length;
   Node_T parent; 
   }; */
   
   

int FT_insertDir(char *path){
   Node_T curr;
   int result;
   assert(path != NULL);
   if(!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   result = FT_insertRestOfPath(path,curr);
   assert( CHECKERDT_ISVALID);
   return result;
}


static int FT_appendFiles(char* path, Node_T parent,
                          void* contents, size_t length){
   Node_T curr = parent;
   Node_T firstNew = NULL;
   Node_T new;
   char* copyPath;
   char* restPath = path;
   char* dirToken;
   int result;
   size_t newCount = 0;

   assert (path != NULL);
   if (curr == NULL) {
      if (root != NULL) {
            return CONFLICTING_PATH;
         }
   }
   else{
      if(!strcmp(path,Node_getPath(curr)))
         return ALREADY_IN_TREE;
         restPath += (strlen(Node_getPath(curr)) + 1);
   }
   if (curr->status == TRUE) return PARENT_CHILD_ERROR; 
   copyPath = malloc(strlen(restPath) + 1);
   if (copyPath == NULL)
      return MEMORY_ERROR;
   strcpy(copyPath, restPath);
   dirToken = strtok(copyPath, "/");

   while (dirToken != NULL){
      new = Node_addFile(dirToken, curr, contents, length);
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

      if (new == NULL) {
         (void) Node_destroy(firstNew); 
         free(copyPath);
         return MEMORY_ERROR;
      }
      curr = new;
      dirToken = strtok(NULL, "/");
   }
   free(copyPath);
   if(parent == NULL){
      return INITIALIZATION_ERROR;
   }
   else{
      result = FT_linkParentToChild(parent, firstNew);
      if (result == SUCCESS)
         count += newCount;
      else
         (void) Node_destroy(firstNew);
      return result;
   }
   
         
         
}

static int FT_insertRestOfPath(char* path, Node_T parent){
   Node_T curr = parent;
   Node_T firstNew = NULL;
   Node_T new;
   char* copyPath;
   char* restPath = path;
   char* dirToken;
   int result;
   size_t newCount = 0;

   assert (path != NULL);
   if (curr == NULL) {
      if(root != NULL) {
         return CONFLICTING_PATH;
      }
   }
   else{
      if(!strcmp(path,Node_getPath(curr)))
         return ALREADY_IN_TREE;
      restPath += (strlen(Node_getPath(curr)) + 1);
   }
   if (curr->status == TRUE) return PARENT_CHILD_ERROR; 
   copyPath = malloc(strlen(restPath) + 1);
   if (copyPath == NULL)
      return MEMORY_ERROR;
   strcpy(copyPath, restPath);
   dirToken = strtok(copyPath, "/");
   while (dirToken != NULL) {
      new = Node_create(dirToken, curr);
      newCount++;

      if(firstNew == NULL)
         firstnew = new;
      else {
         result = FT_linkParentToChild(curr, new);
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
   free(copyPath);

   if(parent == NULL){
      root = firstNew;
      count = newCount;
      return SUCCESS:
   }
   else {
      result = FT_linkParentToChild(parent, firstNew);
      if(result == SUCESS)
         count += newCount;
      else
         (void) Node_destroy(firstNew);
      return result;
   }
}


static int FT_linkParentToChild(Node_T parent, Node_T child){
   assert (parent != NULL);
   assert(parent->status != FILE); 

   if(Node_linkChild(parent,child) != SUCCESS) {
      (void) Node_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}




/* static File_T FT_traverseFilePathFrom(char* path, Node_T curr){
   File_T found;
   Node_T curr;
   size_t i,j;
   assert(path != NULL);
   if (curr == NULL)
      return NULL;
   else if (!strcmp(path, Node_getFilePath(found)))
      return found;
   else if(!strncmp(path, Node_getFilePath(found)), strlen(Node_getFilePath(curr))){
            for (i = 0; i < Node_getNumChildren(curr); i++){
               curr = FT_traverseFilePathFrom(path, Node_getChild(curr,i));
               if (curr->files != NULL)
                  for (j = 0; j < DynArray_getLength(curr->files){
                        if (strcmp(path, Node_getFilePath(curr->files[j]->path))) 
                            found = curr->files[j]; 
                     }
                  if (found != NULL) return found; 
            }
         }
   }
            
} 
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


static Node_T FT_traversePath(char* path){
   assert(path != NULL);
   return FT_traversePathFrom(path, root);
}



boolean FT_containsDir(char *path){ 
   Node_T curr;
   boolean result;
   assert(path != NULL);

   if(!isInitialized)
      return FALSE;

   curr = FT_traversePath(path);

   if(curr == NULL)
      return = FALSE;
   else if(strcmp(path,Node_getPath(curr)))
      restult = FALSE;
   else
      result = TRUE;

   assert(CHECKERFT_ISVALID);
   
}


static int FT_rmPathAt(char* path, Node_T curr){
   Node_T parent;
   assert(path != NULL);
   assert(curr != NULL);
   parent = Node_getParent(Curr);

   if (!strcmp(path, Node_getPath(curr))){
      if(parent == NULL)
         root = NULL;
      else
         Node_unlinkChild(parent, curr);
      FT_removePathFrom(curr);

      return SUCCESS:

   }
   else
      return NO_SUCH_PATH;
}


int FT_remDir(char *path){
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
   assert(CHECKER FT);

   return result; 
}

static void FT_removePathFrom(Node_T curr) {
   if(curr != NULL) {
      count -= Node_destroy(curr);
   }
}


int FT_insertFile(char *path, void *contents, size_t length){
   Node_T curr;
   int result;
   assert (path != NULL);

   if (!isInitiailized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   result = FT_appendFiles(path, curr, contents, length);
   return result; 
}

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
   else
      result = TRUE;
   return result; 
}
               


int FT_rmFile(char *path){
   Node_T parent;
   Node_T curr;
   curr = FT_traversePath(path);
   if (strcmp(path,Node_getPath(curr))) return NO_SUCH_PATH; 
   else
      Node_unlinkChild(parent,curr);
   FT_removePathFrom(curr);
   return SUCCESS; 
                     
}


void *FT_getFileContents(char *path){
   File_T curr;
   void* contents;
   assert (path != NULL);
   if (!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);

   if (curr == NULL)
      contents = NULL;
   else if(strcmp(path, Node_getFile));
   else
      contents = curr->contents;
   return contents; 
   
}
void *FT_replaceFileContents(char *path, void *newContents,
                             size_t newLength){
   Node_T curr;
   void* oldContents; 
   assert (path != NULL);
   if (!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   if (curr == NULL)
      return NOT_IN_TREE;
   else if (curr->status != TRUE) return NULL; 
   else if(strcmp(path,Node_getPath(curr))) return NULL; 
   else{
      oldContents = curr->contents; 
      curr->contents = newContents;
      curr->length = newLength;
   }
   return oldContents; 
}

int FT_stat(char *path, boolean *type, size_t *length){
   Node_T curr; 
   assert (path != NULL);
   if (!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   if (strcmp(path, Node_getPath(curr)))
      return NO_SUCH_PATH; 
   if (curr->status == FALSE)
      type = FALSE;
   else{
      type = TRUE;
      length = curr->length; 
   }
   return SUCCESS; 
   
   
}


int FT_init(void){
   if (isInitialized)
      return INITIALIZATION_ERROR;
   isInitialized = 1;
   root = NULL;
   count = 0;
   return SUCCESS; 
}

int FT_destroy(void){
   if(!isInitialized)
      return INITIALIZATION_ERROR;
   FT_removePathFrom(root);
   root = NULL;
   isInitialized = 0;
   return SUCCESS; 
}

char *FT_toString(void){
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char* result = NULL;

   assert(CheckerDT_isValid(isInitialized,root,count));

   if(!isInitialized)
      return NULL;

   nodes = DynArray_new(count);
   (void) DT_preOrderTraversal(root, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void*)) DT_strlenAccumulate, (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      assert(CheckerDT_isValid(isInitialized,root,count));
      return NULL;
   }
   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void*)) DT_strcatAccumulate, (void *) result);

   DynArray_free(nodes);
   assert(CheckerDT_isValid(isInitialized,root,count));
   return result;
}
   

