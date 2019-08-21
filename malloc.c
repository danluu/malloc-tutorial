#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
// Don't include stdlb since the names will conflict?

#define DEBUG  // comment out when not debugging
#define MIN_SIZE sizeof(char) // minimum size of free space to be held by a heap block

// TODO: align

// sbrk some extra space every time we need it.
// This does no bookkeeping and therefore has no ability to free, realloc, etc.
void *nofree_malloc(size_t size) {
  void *p = sbrk(0);
  void *request = sbrk(size);
  if (request == (void*) -1) { 
    return NULL; // sbrk failed
  } else {
    assert(p == request); // Not thread safe.
    return p;
  }
}

struct block_meta {
  size_t size;
  struct block_meta *next;
  int free;
  #ifdef DEBUG
  int magic;    // For debugging only. 
  #endif
};

#define META_SIZE sizeof(struct block_meta)

void *global_base = NULL;

// Iterate through blocks until we find one that's large enough.
struct block_meta *find_free_block(struct block_meta **last, size_t size) {
  struct block_meta *current = global_base;
  while (current && !(current->free && current->size >= size)) {
    *last = current;
    current = current->next;
  }
  // if block is big enough split it into two blocks, return the first
  if (current->size > (META_SIZE + size + MIN_SIZE)) {
	// calculate the offset for the pointer to the second part of the block
	struct block_meta *temp = (struct block_meta *) ((char *) current + size + META_SIZE + MIN_SIZE);
	// include the new block into the linked list of blocks	
	temp->size = current->size - size - META_SIZE;
	temp->next = current->next;
	temp->free = 1;
	current->size = size;
	current->next = temp; 
  } 
  return current;
}

struct block_meta *request_space(struct block_meta* last, size_t size) {
  struct block_meta *block;
  block = sbrk(0);
  void *request = sbrk(size + META_SIZE);
  assert((void*)block == request); // Not thread safe.
  if (request == (void*) -1) {
    return NULL; // sbrk failed.
  }
  
  if (last) { // NULL on first request.
    last->next = block;
  }
  block->size = size;
  block->next = NULL;
  block->free = 0;
  #ifdef DEBUG
  block->magic = 0x12345678;
  #endif
  return block;
}

// If it's the first ever call, i.e., global_base == NULL, request_space and set global_base.
// Otherwise, if we can find a free block, use it.
// If not, request_space.
void *malloc(size_t size) {
  struct block_meta *block;
  // TODO: align size?

  if (size <= 0) {
    return NULL;
  }

  if (!global_base) { // First call.
    block = request_space(NULL, size);
    if (!block) {
      return NULL;
    }
    global_base = block;
  } else {
    struct block_meta *last = global_base;
    block = find_free_block(&last, size);
    if (!block) { // Failed to find free block.
      block = request_space(last, size);
      if (!block) {
	    return NULL;
      }
    } else {      // Found free block
      block->free = 0;
	  #ifdef DEBUG
      block->magic = 0x77777777;
	  #endif
    }
  }
 
  return(block+1);
}

void *calloc(size_t nelem, size_t elsize) {
  size_t size = nelem * elsize;
  void *ptr = malloc(size);
  memset(ptr, 0, size);
  return ptr;
}

// TODO: maybe do some validation here.
struct block_meta *get_block_ptr(void *ptr) {
  return (struct block_meta*)ptr - 1;
}

void free(void *ptr) {
  if (!ptr) {
    return;
  }

  // TODO: consider merging blocks once splitting blocks is implemented.
  struct block_meta* block_ptr = get_block_ptr(ptr);
  assert(block_ptr->free == 0);
  #ifdef DEBUG
  assert(block_ptr->magic == 0x77777777 || block_ptr->magic == 0x12345678);
  #endif
  block_ptr->free = 1;
  #ifdef DEBUG
  block_ptr->magic = 0x55555555;  
  #endif
}

void *realloc(void *ptr, size_t size) {
  if (!ptr) { 
    // NULL ptr. realloc should act like malloc.
    return malloc(size);
  }

  struct block_meta* block_ptr = get_block_ptr(ptr);
  if (block_ptr->size >= size) {
    // We have enough space. Could free some once we implement split.
    return ptr;
  }

  // Need to really realloc. Malloc new space and free old space.
  // Then copy old data to new space.
  void *new_ptr;
  new_ptr = malloc(size);
  if (!new_ptr) {
    return NULL; // TODO: set errno on failure.
  }
  memcpy(new_ptr, ptr, block_ptr->size);
  free(ptr);  
  return new_ptr;
}
