#include "stdio.h"
#include "stdlib.h"

// Recursive function to calculate fibonacci sequence
// term - nth term to calculate
int fib_rec(int term){
  if (term < 3)
    return term;
  else
    return(fib_rec(term - 1) + fib_rec(term - 2));
}

// Wrapper for tail-recursive function to calculate fibonacci sequence
// term - nth term to calculate
int fib_twrap(int term){
  return(fib_trec(term, 2, 1));
}

// Tail-recursive function to calculate fibonacci sequence
// term - nth term to calculate
// val - current fibonacci term
// prev - previous fibonacci term
int fib_trec(int term, int val, int prev){
  if (term == 1)
    return prev;
  if (term == 2)
    return val;

  return(fib_trec((term - 1), (val + prev), val));
}

// Iterative loop function to calculate fibonacci sequence
// term - nth term to calculate
int fib_loop(int term){
  int a = 1;
  int b = 2;
  int i = 0; 
 
  if (term < 3)
    return term;

  for (i = 3; i <= term; i++){
    b += a;
    a = b - a;
  }

  return b;
}

// Struct to represent a fib term cache element
struct fib_elem{
  int term;
  int val;
  struct fib_elem *next;
  struct fib_elem *prev;
} fibc;

typedef struct fib_elem* pfibc;


// Function to add a fib term cache element to a cache
// cache - the cache to insert into
// term - the term being inserted
// val - value of the term being inserted
int cache_insert(pfibc cache, int term, int val){
  pfibc tail = NULL;
  pfibc curr = NULL;
  pfibc elem = NULL;

  if (!cache){
    printf("cache_insert: can't insert into this cache\n");
    return -1;
  }
  
  elem = malloc(sizeof(fibc));
  if (!elem){
    printf("cache_insert: couldn't allocate new cache element\n");
    return -1;
  }

  tail = cache->prev;
  elem->next = cache;
  elem->prev = tail;

  if (tail)
    tail->next = elem;
  else{
    printf("cache_insert: linked list has a broken reference\n");
    return -1;
  }
  
  elem->term = term;
  elem->val = val;
}  

// Function to destroy(free) a cache
// cache - the cache to be destroyed
int cache_destroy(pfibc cache){
  pfibc curr = NULL;
  pfibc next = NULL;
  
  if (!cache){
    printf("cache_destroy: invalid cache\n");
    return -1;
  }
  
  curr = cache->next;
  while ((curr) && (curr != cache)){
    next = curr->next;
    free(curr);
    curr = next;
  }
  
  free(cache);
}
 
// Function to find a term within the fib cache
// cache - cache to search inside for term
// term - the term to search for inside the cache
int cache_find(pfibc cache, int term){
  pfibc curr = NULL;
  pfibc prev = NULL;
    
  if (!cache)
    return -1;
	
  prev = cache->prev;
  while ((prev) && (prev != cache)){
    curr = prev;
    prev = curr->prev;
    if (curr->term == term)
      return curr->val;
  }
 
  return -1;
}

// Function to recursively calculate fibonacci sequence using a cache
// cache - cache to store previously calculated terms in
// term - nth term to calculate
int fib_cached(pfibc cache, int term){
  int val = 0;
  val = cache_find(cache, term);
  if (val > 0)
    return val;

  if (term < 3){
    cache_insert(cache, term, term);
    return term;
  }
  
  val = fib_cached(cache, (term - 1)) + fib_cached(cache, (term - 2));
  cache_insert(cache, term, val);
  return val;
}
 
// Wrapper for caching recursive fibonacci sequence calculation function
// term - nth term to calculate
int fib_cwrap(int term){
  int result = 0;
  pfibc cache = malloc(sizeof(fibc));
  if (cache){
    cache->next = cache;
    cache->prev = cache;
    result = fib_cached(cache, term);
    cache_destroy(cache);
    return result;
  }
  
  printf("fib_cwrap: couldn't create new cache\n");
  return -1;
}
    
void main(){
  int x = 25;
  printf("Recursive: %d\n", fib_rec(x));
  printf("Tail Recursive: %d\n", fib_twrap(x));
  printf("Iterative Loop: %d\n", fib_loop(x));
  printf("Recursive cached: %d\n", fib_cwrap(x));

}
