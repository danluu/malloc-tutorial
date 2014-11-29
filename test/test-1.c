#include <stdio.h>
#include <stdlib.h>

#define RUNS 10000

int main() {
  malloc(1);

  int i;
  int **arr = malloc(RUNS * sizeof(int *));

  if (arr == NULL) {
    printf("Memory failed to allocate!\n");
    return 1;
  }

  for (i = 0; i < RUNS; i++) {
    arr[i] = malloc(sizeof(int));
    if (arr[i] == NULL) {
      printf("Memory failed to allocate!\n");
      return 1;
    }
    
    *(arr[i]) = i+1;
  }

  for (i = 0; i < RUNS; i++) {
    if (*(arr[i]) != i+1) {
      printf("Memory failed to contain correct data after many allocations!\n");
      return 2;
    }
  }

  for (i = 0; i < RUNS; i++) {
    free(arr[i]);
  }
  
  free(arr);
  printf("Memory was allocated, used, and freed!\n");	
  return 0;
}
