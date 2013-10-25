#include "sescapi.h"  
#include <stdio.h>  
#define NUM_THREADS        2  

void *print_hello_world(void *threadid)  
{  
  printf("/n%d: Hello World!/n", threadid);  
  sesc_exit(0);  
}  

int main()  
{    
  int t;  
  sesc_init();  
  for(t=0;t<NUM_THREADS;t++){  
    printf("Creating thread %d/n", t);  
    sesc_spawn((void*) *print_hello_world,(void *) t,0);  
  }  
  sesc_exit(0);  
}  


