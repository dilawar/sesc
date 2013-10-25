#include <stdio.h>
#include <stdlib.h>
#include "sescapi.h"

/* void sesc_begin_epochs(void){ */
/*   fprintf(stderr,"TLS simulator needed for sesc_begin_epochs\n"); */
/*   exit(-1); */
/* } */

int  sesc_future_epoch(void){
  fprintf(stderr,"TLS simulator needed for sesc_future_epoch\n");
  exit(-1);
  return 0;
}

volatile void  sesc_future_epoch_jump(void *codeAddr){
  fprintf(stderr,"TLS simulator needed for sesc_future_epoch_jump\n");
  exit(-1);  
}

void sesc_commit_epoch(void){
  fprintf(stderr,"TLS simulator needed for sesc_commit_epoch\n");
  exit(-1);
}
void sesc_change_epoch(void){
  fprintf(stderr,"TLS simulator needed for sesc_change_epoch\n");
  exit(-1);
}
/* void sesc_end_epochs(void){ */
/*   fprintf(stderr,"TLS simulator needed for sesc_end_epochs\n"); */
/*   exit(-1); */
/* } */
