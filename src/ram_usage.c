#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned long size,resident,share,text,lib,data,dt;
} statm_t;

#define PAGESIZE 4096

void read_off_memory_status(statm_t *result)
{
  unsigned long dummy;
  const char* statm_path = "/proc/self/statm";

  FILE *f = fopen(statm_path,"r");
  if(!f){
    perror(statm_path);
    return;
  }
  if(7 != fscanf(f,"%ld %ld %ld %ld %ld %ld %ld",
    &result->size, &result->resident,&result->share,&result->text,&result->lib,&result->data,&result->dt))
  {
    perror(statm_path);
    return;
  }
  fclose(f);
}

unsigned long int get_ram_usage() {
    statm_t res = {0};
    read_off_memory_status(&res);
    return PAGESIZE * res.size;
}