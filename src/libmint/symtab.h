#ifndef __symtab_h
#define __symtab_h

#ifndef FUNC_HASH_SIZE
#define FUNC_HASH_SIZE 128
#endif

typedef struct func_name_t {
  uint32_t addr;
  struct func_name_t *next;
  const char *fname;
} func_name_t, *func_name_ptr;

#define FUNC_ENTRY_SIZE (sizeof(struct func_name_t))

typedef struct namelist {
  const char *n_name;
  int32_t n_type;
  uint32_t n_value;
} namelist_t, *namelist_ptr;


int32_t namelist(char *objname, namelist_ptr pnlist);
void read_hdrs(char *objfile);
void close_object();

struct file_info {
  int32_t addr;
  char *fname;
  int32_t linelow;
  uint8_t *lptr;
};

#endif /* __symtab_h */
