/**
 * This file implements parallel mergesort.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include "mergesort.h"

/* this function will be called by mergesort() and also by parallel_mergesort(). 
   It merges two sorted subarrays A[leftstart..leftend] and A[rightstart..rightend]
   into a single sorted subarray A[leftstart..rightend]. We use the temp array B[] to help with merging. 
*/
void merge(int leftstart, int leftend, int rightstart, int rightend){
    int i, j, k;
    
    // Copying data to temp array B
    for(i = leftstart; i <= rightend; i++) {
        B[i] = A[i];
    }
    // using 2 pointer to merge the two subarrays
    i = leftstart;    // Left subarray
    j = rightstart;   // Right subarray
    k = leftstart;    // Merged array

    // Merge the temp arrays back into A[leftstart..rightend]
    while(i <= leftend && j <= rightend) {
        if(B[i] <= B[j]) {
            A[k] = B[i];
            i++;
        } else {
            A[k] = B[j];
            j++;
        }
        k++;
    }
    
    // Copy remaining elements of left subarray
    while(i <= leftend) {
        A[k] = B[i];
        i++;
        k++;
    }
    
    // Copy remaining elements of right subarray
    while(j <= rightend) {
        A[k] = B[j];
        j++;
        k++;
    }
}


/* this function will be called by parallel_mergesort() as its base case.
   Classic recursive merge sort implementation. Divide, sort then merge.
*/
void my_mergesort(int left, int right){
    if(left < right) {
        int mid = left + (right - left) / 2;
        
        // Sort first and second halves
        my_mergesort(left, mid);
        my_mergesort(mid + 1, right);
        
        // Merge the sorted halves
        merge(left, mid, mid + 1, right);
    }
}

/* this function will be called by the testing program. 
   It implements parallel mergesort using pthreads.
   The argument is a pointer to struct argument which contains
   left index, right index and level of recursion. 
*/
void * parallel_mergesort(void *arg){
    struct argument *args = (struct argument *)arg;
    int left = args->left;
    int right = args->right;
    int level = args->level;
    
    // Base case: if we've reached the cutoff level, use single-threaded sort
    if(level >= cutoff || left >= right) {
        if(left < right) {
            my_mergesort(left, right);
        }
        return NULL;
    }
    
    // If we haven't reached cutoff, create threads for parallel processing
    int mid = left + (right - left) / 2;
    
    // Create arguments for left and right halves
    struct argument *left_arg = buildArgs(left, mid, level + 1);
    struct argument *right_arg = buildArgs(mid + 1, right, level + 1);
    
    // Check if memory allocation succeeded
    if(left_arg == NULL || right_arg == NULL) {
        // Fallback to single-threaded if allocation fails
        if(left_arg) free(left_arg);
        if(right_arg) free(right_arg);
        my_mergesort(left, right);
        return NULL;
    }
    
    pthread_t left_thread, right_thread;
    
    // Create threads for left and right halves
    pthread_create(&left_thread, NULL, parallel_mergesort, left_arg);
    pthread_create(&right_thread, NULL, parallel_mergesort, right_arg);
    
    // Wait for both threads to complete
    pthread_join(left_thread, NULL);
    pthread_join(right_thread, NULL);
    
    // Free the argument structures we created
    free(left_arg);
    free(right_arg);
    
    // Merge the sorted halves
    merge(left, mid, mid + 1, right);
    
    return NULL;
}

/* we build the argument for the parallel_mergesort function.
   Allocating memory for struct argument and initialising its 3 values
*/
struct argument * buildArgs(int left, int right, int level){
    struct argument *arg = (struct argument *)malloc(sizeof(struct argument));
    if(arg == NULL) {
        return NULL;
    }
    
    arg->left = left;
    arg->right = right;
    arg->level = level;
    
    return arg;
}
