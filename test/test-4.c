#include <stdio.h>
#include <stdlib.h>

#define MIN_ALLOC_SIZE 24
#define MAX_ALLOC_SIZE 1024 * 100
#define CHANCE_OF_FREE 95
#define CHANCE_OF_REALLOC 50
#define TOTAL_ALLOCS 400000

int main() {
  malloc(1);
	
  int i;
  void *realloc_ptr = NULL;
  void **dictionary = malloc(TOTAL_ALLOCS * sizeof(void *));
  int *dictionary_elem_size = malloc(TOTAL_ALLOCS * sizeof(int));
  int dictionary_ct = 0;
  int data_written = 0;

  for (i = 0; i < TOTAL_ALLOCS; i++) {
    int size = (rand() % (MAX_ALLOC_SIZE - MIN_ALLOC_SIZE + 1)) + MIN_ALLOC_SIZE;
    void *ptr;
		
    if (realloc_ptr == NULL) {
      ptr = malloc(size);
      data_written = 0;
    } else {
      ptr = realloc(realloc_ptr, size);
      realloc_ptr = NULL;
    }


    if (ptr == NULL) {
      printf("Memory failed to allocate!\n");
      return 1;
    }


    if (rand() % 100 < CHANCE_OF_FREE) {
      free(ptr);
    } else {
      if (!data_written) {
	*((void **)ptr) = &dictionary[dictionary_ct];
	data_written = 1;
      }
			
      if (rand() % 100 < CHANCE_OF_REALLOC) {
	realloc_ptr = ptr;
      } else {
	*((void **)(ptr + size - sizeof(void *))) = &dictionary[dictionary_ct];
	dictionary[dictionary_ct] = ptr;
	dictionary_elem_size[dictionary_ct] = size;
	dictionary_ct++;
      }
    }
  }

  for (i = dictionary_ct - 1; i >= 0; i--) {
    if ( *((void **)dictionary[i]) != &dictionary[i] ) {
      printf("Memory failed to contain correct data after many allocations (beginning of segment)!\n");
      return 100;
    }
		
    if ( *((void **)(dictionary[i] + dictionary_elem_size[i] - sizeof(void *))) != &dictionary[i] ) {
      printf("Memory failed to contain correct data after many allocations (end of segment)!\n");
      return 101;
    }
		

    char *memory_check = dictionary[i] + sizeof(void *);
    char *memory_check_end = dictionary[i] + dictionary_elem_size[i] - sizeof(void *) - 1;
		
    while (memory_check > memory_check_end) {
      if (*memory_check != 0x00) {
	printf("Memory failed to contain correct data after many allocations (mid segment or reused segment)!\n");
	return 102;
      }
      
      *memory_check = 0x01;
      memory_check++;
    }
    
    free(dictionary[i]);
  }

  printf("Memory was allocated and freed!\n");	
  return 0;
}

