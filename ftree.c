#include <stdio.h>
// Add your system includes here.
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "ftree.h"


/*
 * Returns the FTree rooted at the path fname.
 *
 * A helper for generate_ftree 
 *
 */
struct TreeNode *helper_ftree(const char *fname, char * path){
    struct stat stat_buf;

    //new path 
    int len = strlen(path) + strlen(fname) + 2;
    char new_path[len];

    //creating the new path
    strcpy(new_path, path);
    strcat(new_path, "/");
    strcat(new_path, fname);

    //path check
    if (lstat(new_path, &stat_buf) == -1) {
        fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
        return NULL;
    }

    //contructing a Root TreeNode
    struct TreeNode * t_node = malloc(sizeof(struct TreeNode));
        
    // malloc check
    if (t_node == NULL){
        fprintf(stderr, "TreeNode - memory allocation\n");
        exit(1);
    }

    t_node->fname = malloc(strlen(fname) + 1);

    // malloc check
    if (t_node->fname == NULL){
        fprintf(stderr, "TreeNode Name - memory allocation\n");
        exit(1);
    }

    strcpy(t_node->fname, fname);
    t_node->permissions = stat_buf.st_mode & 0777;
    t_node->contents = NULL;
    t_node->next = NULL;

    //base case - reg file
    if (S_ISREG(stat_buf.st_mode)){
        t_node->type = '-';
        return t_node;
    }
    else if (S_ISLNK(stat_buf.st_mode)){         //base case - link file
        t_node->type = 'l';
        return t_node;
    }
    else{           //recursive case - directory
        DIR *d_ptr = opendir(new_path);

        //error check for opendir system call
        if (d_ptr == NULL){
            exit(1);
        }

        struct dirent *entry_ptr;
        entry_ptr = readdir(d_ptr);

        t_node->type = 'd';

        int inital_member = 1; // to identify whether we are at the first member of the directory (1) or not (0)

        struct TreeNode * curr_node;
        while (entry_ptr != NULL) {
            if ((entry_ptr->d_name)[0] == '.'){
                entry_ptr = readdir(d_ptr);
            }
            else if (inital_member){
                inital_member = 0;
                t_node->contents = helper_ftree(entry_ptr->d_name, new_path);
                curr_node = t_node->contents; 
                entry_ptr = readdir(d_ptr);
            }
            else{
                curr_node->next = helper_ftree(entry_ptr->d_name, new_path);
                curr_node = curr_node->next;
                entry_ptr = readdir(d_ptr);
            }
        }

        //error check for closedir system call
        if (closedir(d_ptr) == -1){
            exit(1);
        }

        return t_node;
    }
}

/*
 * Returns the FTree rooted at the path fname.
 *
 * Use the following if the file fname doesn't exist and return NULL:
 * fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
 *
 */
struct TreeNode *generate_ftree(const char *fname) {

    struct stat stat_buf;

    //path check
    if (lstat(fname, &stat_buf) == -1) {
        fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
        return NULL;
    }

    return helper_ftree(fname, ".");

}



/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 *
 * The only print statements that you may use in this function are:
 * printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions)
 * printf("%s (%c%o)\n", root->fname, root->type, root->permissions)
 *
 */
void print_ftree(struct TreeNode *root) {
	
    // Here's a trick for remembering what depth (in the tree) you're at
    // and printing 2 * that many spaces at the beginning of the line.

    static int depth = 0;
    printf("%*s", depth * 2, "");

    if (root->type == '-' || root->type == 'l'){
        printf("%s (%c%o)\n", root->fname, root->type, root->permissions);
    }
    else{
        printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions);
        struct TreeNode * curr_node = root->contents;       
        depth++;
        while (curr_node != NULL){
            print_ftree(curr_node);
            curr_node = curr_node->next;
        }
        depth--;
    }
}


/* 
 * Deallocate all dynamically-allocated memory in the FTree rooted at node.
 * 
 */
void deallocate_ftree (struct TreeNode *node) {
   if (node->type == '-' || node->type == 'l'){
       free(node->fname);   
       free(node);
   }
   else{
       struct TreeNode * curr_node = node->contents;     // current traversing node
       struct TreeNode * del = curr_node;                // freeing memory node
       while (curr_node != NULL){
           curr_node = curr_node->next;
           deallocate_ftree(del);
           del = curr_node;
       }
       free(node->fname);
       free(node);
   }
}
