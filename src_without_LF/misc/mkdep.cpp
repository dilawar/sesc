/*
 * mkdep extracted from the linux kernel to speed up the dependence
 * check for the IACOMA projects: MINT, SESC, SMT...
 *
 * It has been modified so that recursive dependence works. Also, some
 * Linux specific options have been modified.
 *
 * This program rocks! We have gone from 2.62 secs to 0.01 secs.
 *
 * Note by Milos: The speed comes at a price. It only deals with included .h
 * files. Including .c files or extension-less files (like most STL includes)
 * results in those files being omitted from dependence lists. I added support
 * for including .hpp files (C++ header files)
 *
 * Originally by Linus Torvalds.
 *
 *
 * Smart CONFIG_* processing by Werner Almesberger, Michael Chastain.
 *
 * Usage: mkdep cflags -- file ...
 * 
 * Read source files and output makefile dependency lines for them.
 * I make simple dependency lines for #include <*.h> and #include "*.h".
 * I also find instances of CONFIG_FOO and generate dependencies
 *    like include/config/foo.h.
 *
 * 1 August 1999, Michael Elizabeth Chastain, <mec@shout.net>
 * - Keith Owens reported a bug in smart config processing.  There used
 *   to be an optimization for "#define CONFIG_FOO ... #ifdef CONFIG_FOO",
 *   so that the file would not depend on CONFIG_FOO because the file defines
 *   this symbol itself.  But this optimization is bogus!  Consider this code:
 *   "#if 0 \n #define CONFIG_FOO \n #endif ... #ifdef CONFIG_FOO".  Here
 *   the definition is inactivated, but I still used it.  It turns out this
 *   actually happens a few times in the kernel source.  The simple way to
 *   fix this problem is to remove this particular optimization.
 *
 * 2.3.99-pre1, Andrew Morton <andrewm@uow.edu.au>
 * - Changed so that 'filename.o' depends upon 'filename.[cS]'.  This is so that
 *   missing source files are noticed, rather than silently ignored.
 *
 * 2.4.2-pre3, Keith Owens <kaos@ocs.com.au>
 * - Accept cflags followed by '--' followed by filenames.  mkdep extracts -I
 *   options from cflags and looks in the specified directories as well as the
 *   defaults.   Only -I is supported, no attempt is made to handle -idirafter,
 *   -isystem, -I- etc.
 */

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>



char __depname[512] = "";
#define depname (__depname+9)
int32_t hasdep;

struct path_struct {
	int32_t len;
	char *buffer;
};
struct path_struct *path_array;
int32_t paths;


/* Current input file */
static const char *g_filename;

/*
 * This records all the configuration options seen.
 * In perl this would be a hash, but here it's a long string
 * of values separated by newlines.  This is simple and
 * extremely fast.
 */
char * str_config  = NULL;
int32_t    size_config = 0;
int32_t    len_config  = 0;


char **pendingDeps;
int32_t nPendingDeps=0;
int32_t pendingDepsSize=0;

void addPendingDeps(const char *dep)
{
  int32_t i;

  for(i=0;i<nPendingDeps;i++) {
    if(strcmp(pendingDeps[i],dep) == 0 )
      return;
  }
  
  if( pendingDepsSize == 0 ) {
    pendingDepsSize = 2048;
    pendingDeps = (char **)malloc(sizeof(char *)*pendingDepsSize);
  }else{
    if((nPendingDeps+1) >= pendingDepsSize ){
      pendingDepsSize *= 2;
      pendingDeps = (char **)realloc(pendingDeps, sizeof(char *)*pendingDepsSize);
    }
    
    if (pendingDeps == NULL) { 
      perror("malloc config"); 
      exit(1); 
    }
  }
  
  pendingDeps[nPendingDeps] = strdup(dep);
  if (pendingDeps[nPendingDeps] == NULL) { 
    perror("malloc config"); 
    exit(1); 
  }

  nPendingDeps++;
}


char **allDeps;
char ***foundDeps;
int32_t nFoundDeps=0;
int32_t foundDepsSize=0;

#define MAXDEPS  1024

void growFoundDeps(int32_t xtra)
{
  int32_t i;
  
  if( foundDepsSize > nFoundDeps + xtra )
    return;
    
  if( foundDepsSize == 0 ) {
    foundDepsSize = 256;
    foundDeps = (char ***)malloc(sizeof(char **)*foundDepsSize);
    for(i=0;i<foundDepsSize;i++) {
      foundDeps[i] = (char **)calloc(sizeof(char *),MAXDEPS);
      if (foundDeps[i] == NULL) { 
	perror("malloc config"); 
	exit(1); 
      }
    }
    allDeps = (char **)calloc(sizeof(char *),MAXDEPS);
  }else{
    if((nFoundDeps+1) >= foundDepsSize ){
      int32_t val = foundDepsSize;
      
      foundDepsSize *= 2;
      foundDeps = (char ***)realloc(foundDeps, sizeof(char **)*foundDepsSize);

      for(i=val;i<foundDepsSize;i++) {
	foundDeps[i] = (char **)calloc(sizeof(char *),MAXDEPS);
	if (foundDeps[i] == NULL) { 
	  perror("malloc config"); 
	  exit(1); 
	}
      }
    }
  }
  if (foundDeps == NULL) { 
    perror("malloc config"); 
    exit(1); 
  }
}

#if 0
char *simplifyPath(const char *str)
{
  /* Simplify dep
   *
   * /projs/build/../esesc/
   *
   * becomes
   *
   * /projs/esesc/
   *
   */
  char *dep = strdup(str);
  char *depCopy = strdup(dep);
  char *pos = strstr(depCopy,"/..");

  if (pos) {
    char *path=pos-1;
    while(path>dep && *path != '/' ) {
      path--;
    }
    *path=0;
    pos +=3;
    sprintf(dep,"%s%s",depCopy,pos);
  }

  return dep;
}
#endif

void addFoundDeps(const char *target, const char *dep)
{
  int32_t i;
  int32_t targetPos=-1;

  for(i=0;i<nFoundDeps;i++) {
    if(strcmp(foundDeps[i][0],target) == 0 ) {
      targetPos = i;
      break;
    }
  }
  if( targetPos < 0 ) {
    growFoundDeps(1);
    targetPos = nFoundDeps;
    foundDeps[nFoundDeps++][0] = strdup(target);
  }

  for(i=1;i<MAXDEPS;i++) {
    if( foundDeps[targetPos][i] == 0 )
      break;
    if( strcmp(foundDeps[targetPos][i],dep) == 0 ) {
      return;
    }
  }

  if( i>= MAXDEPS ) {
    fprintf(stderr,"addFoundDeps: MAXDEPS(%d) is too small\n",MAXDEPS);
    exit(-1);
  }

  foundDeps[targetPos][i] = strdup(dep);

  for(i=0;i<MAXDEPS;i++) {
    if( allDeps[i] == 0 )
      break;
    if( strcmp(allDeps[i],dep) == 0 ) {
      return;
    }
  }
  if( i>= MAXDEPS ) {
    fprintf(stderr,"addFoundDeps: MAXDEPS(%d) is too small\n",MAXDEPS);
    exit(-1);
  }

  allDeps[i] = strdup(dep);
}

void expandDeps(const char *target)
{
  int32_t i;
  int32_t j;
  int32_t targetPos = -1;
  
  for(i=0;i<nFoundDeps;i++) {
    if( strcmp(foundDeps[i][0],target) == 0 ) {
      targetPos = i;
      break;
    }
  }
  if( targetPos < 0 )
    return;
      
  for(i=0;i<nFoundDeps;i++) {
    if( i == targetPos )
      continue;
    
    j=1;
    while( foundDeps[i][j] ) {
      if( strcmp(foundDeps[i][j],target) == 0 ) {
	int32_t k=1;
	if( foundDeps[targetPos][1] == 0 )
	  continue;
	while(foundDeps[targetPos][k]){
	  char *curStr=foundDeps[targetPos][k];
	  int32_t   curLen=strlen(curStr);
	  if((curLen>2)&&(curStr[curLen-2]=='.')&&(curStr[curLen-1]=='h')){
	    addFoundDeps(foundDeps[i][0],foundDeps[targetPos][k]);
	  }else if((curLen>4)&&(curStr[curLen-4]=='.')&&(curStr[curLen-3]=='h')&&
		   (curStr[curLen-2]=='p')&&(curStr[curLen-1]=='p')){
	    addFoundDeps(foundDeps[i][0],foundDeps[targetPos][k]);
	  }else if((curLen>4)&&(curStr[curLen-4]=='.')&&(curStr[curLen-3]=='c')&&
		   (curStr[curLen-2]=='p')&&(curStr[curLen-1]=='p')){
	    addFoundDeps(foundDeps[i][0],foundDeps[targetPos][k]);
	  }
	  k++;
	}
      }
      j++;
    }
  }
}

static void do_depname(void)
{
  if (!hasdep) {
    hasdep = 1;
    if (g_filename) {
      addFoundDeps(depname,g_filename);
    }
  }
}

/*
 * Grow the configuration string to a desired length.
 * Usually the first growth is plenty.
 */
void grow_config(int32_t len)
{
  while (len_config + len > size_config) {
    if (size_config == 0)
      size_config = 2048;
    str_config = (char *)realloc(str_config, size_config *= 2);
    if (str_config == NULL) { 
      perror("malloc config"); 
      exit(1); 
    }
  }
}



/*
 * Lookup a value in the configuration string.
 */
int32_t is_defined_config(const char * name, int32_t len)
{
  const char * pconfig;
  const char * plast = str_config + len_config - len;
  for ( pconfig = str_config + 1; pconfig < plast; pconfig++ ) {
    if (pconfig[ -1] == '\n'
	&&  pconfig[len] == '\n'
	&&  !memcmp(pconfig, name, len))
      return 1;
  }
  return 0;
}



/*
 * Add a new value to the configuration string.
 */
void define_config(const char * name, int32_t len)
{
  grow_config(len + 1);

  memcpy(str_config+len_config, name, len);
  len_config += len;
  str_config[len_config++] = '\n';
}



/*
 * Clear the set of configuration strings.
 */
void clear_config(void)
{
  len_config = 0;
  define_config("", 0);
}


/*
 * Handle an #include line.
 */
void handle_include(int32_t start, const char * name, int32_t len)
{
  struct path_struct *path;
  int32_t i;

  if (len >= 7 && !memcmp(name, "config/", 7))
    define_config(name+7, len-7-2);

  for (i = start, path = path_array+start; i < paths; ++i, ++path) {
    memcpy(path->buffer+path->len, name, len);
    path->buffer[path->len+len] = '\0';
    if (access(path->buffer, F_OK) == 0) {
      do_depname();

      addFoundDeps(depname,path->buffer);

      if((len>2)&&(name[len-2]=='.')&&(name[len-1]=='h')){
	/* This is a .h fie */
	/*	fprintf(stderr,"addPending for .h %s\n",path->buffer);*/
	addPendingDeps(path->buffer);
      }else if((len>4)&&(name[len-4]=='.')&&(name[len-3]=='h')&&
	       (name[len-2]=='p')&&(name[len-1]=='p')){
	/* This is a .hpp file */
	/*	fprintf(stderr,"addPending for .hpp %s\n",path->buffer);*/
	addPendingDeps(path->buffer);
      }else if((len>4)&&(name[len-4]=='.')&&(name[len-3]=='c')&&
	       (name[len-2]=='p')&&(name[len-1]=='p')){
	/* This is a .hpp file */
	/*	fprintf(stderr,"addPending for .hpp %s\n",path->buffer);*/
	addPendingDeps(path->buffer);
      }
      return;
    }
  }
}



/*
 * Add a path to the list of include paths.
 */
void add_path(const char *name)
{
  struct path_struct *path;
  char resolved_path[PATH_MAX+1];
  const char *name2;

  if (strcmp(name, ".")) {
    /*    name2 = realpath(name, resolved_path); */
    name2= strcpy(resolved_path,name);
   
    if (!name2) {
      fprintf(stderr, "realpath(%s) failed\n", name);
      exit(1);
    }
  }else {
    name2 = "";
  }

  path_array = (struct path_struct *)realloc(path_array, (++paths)*sizeof(*path_array));
  if (!path_array) {
    fprintf(stderr, "cannot expand path_arry\n");
    exit(1);
  }

  path = path_array+paths-1;
  path->len = strlen(name2);
  path->buffer = (char *)malloc(path->len+1+256+1);
  if (!path->buffer) {
    fprintf(stderr, "cannot allocate path buffer\n");
    exit(1);
  }
  strcpy(path->buffer, name2);
  if (path->len && *(path->buffer+path->len-1) != '/') {
    *(path->buffer+path->len) = '/';
    *(path->buffer+(++(path->len))) = '\0';
  }
}



/*
 * Record the use of a CONFIG_* word.
 */
void use_config(const char * name, int32_t len)
{
  char *pc;
  int32_t i;

  pc = path_array[paths-1].buffer + path_array[paths-1].len;
  memcpy(pc, "config/", 7);
  pc += 7;

  for (i = 0; i < len; i++) {
    char c = name[i];
    if (isupper((int)c)) c = tolower(c);
    if (c == '_')   c = '/';
    pc[i] = c;
  }
  pc[len] = '\0';

  if (is_defined_config(pc, len))
    return;

  define_config(pc, len);

  do_depname();
  printf(" \\\n   $(wildcard %s.h)", path_array[paths-1].buffer);
}



/*
 * Macros for stunningly fast map-based character access.
 * __buf is a register which holds the current word of the input.
 * Thus, there is one memory access per sizeof(unsigned long) characters.
 */

#if defined(__alpha__) || defined(__i386__)  || defined(__x86_64__) \
    || defined(__ia64__) || defined(__MIPSEL__) || defined(__arm__)
#define LE_MACHINE
#endif

#ifdef LE_MACHINE
#define next_byte(x) (x >>= 8)
#define current ((unsigned char) __buf)
#else
#define next_byte(x) (x <<= 8)
#define current (__buf >> 8*(sizeof(unsigned long)-1))
#endif

#define GETNEXT { \
	next_byte(__buf); \
	if ((unsigned long) next % sizeof(unsigned long) == 0) { \
		if (next >= end) \
			break; \
		__buf = * (unsigned long *) next; \
	} \
	next++; \
}

/*
 * State machine macros.
 */
#define CASE(c,label) if (current == c) goto label
#define NOTCASE(c,label) if (current != c) goto label

/*
 * Yet another state machine speedup.
 */
#define MAX2(a,b) ((a)>(b)?(a):(b))
#define MIN2(a,b) ((a)<(b)?(a):(b))
#define MAX5(a,b,c,d,e) (MAX2(a,MAX2(b,MAX2(c,MAX2(d,e)))))
#define MIN5(a,b,c,d,e) (MIN2(a,MIN2(b,MIN2(c,MIN2(d,e)))))



/*
 * The state machine looks for (approximately) these Perl regular expressions:
 *
 *    m|\/\*.*?\*\/|
 *    m|\/\/.*|
 *    m|'.*?'|
 *    m|".*?"|
 *    m|#\s*include\s*"(.*?)"|
 *    m|#\s*include\s*<(.*?>"|
 *    m|#\s*(?define|undef)\s*CONFIG_(\w*)|
 *    m|(?!\w)CONFIG_|
 *
 * About 98% of the CPU time is spent here, and most of that is in
 * the 'start' paragraph.  Because the current characters are
 * in a register, the start loop usually eats 4 or 8 characters
 * per memory read.  The MAX5 and MIN5 tests dispose of most
 * input characters with 1 or 2 comparisons.
 */
void state_machine(const char * map, const char * end)
{
	const char * next = map;
	const char * map_dot;
	unsigned long __buf = 0;

	for (;;) {
start:
	GETNEXT
__start:
	if (current > MAX5('/','\'','"','#','C')) goto start;
	if (current < MIN5('/','\'','"','#','C')) goto start;
	CASE('/',  slash);
	CASE('\'', squote);
	CASE('"',  dquote);
	CASE('#',  pound);
	CASE('C',  cee);
	goto start;

/* // */
slash_slash:
	GETNEXT
	CASE('\n', start);
	NOTCASE('\\', slash_slash);
	GETNEXT
	goto slash_slash;

/* / */
slash:
	GETNEXT
	CASE('/',  slash_slash);
	NOTCASE('*', __start);
slash_star_dot_star:
	GETNEXT
__slash_star_dot_star:
	NOTCASE('*', slash_star_dot_star);
	GETNEXT
	NOTCASE('/', __slash_star_dot_star);
	goto start;

/* '.*?' */
squote:
	GETNEXT
	CASE('\'', start);
	NOTCASE('\\', squote);
	GETNEXT
	goto squote;

/* ".*?" */
dquote:
	GETNEXT
	CASE('"', start);
	NOTCASE('\\', dquote);
	GETNEXT
	goto dquote;

/* #\s* */
pound:
	GETNEXT
	CASE(' ',  pound);
	CASE('\t', pound);
	CASE('i',  pound_i);
	CASE('d',  pound_d);
	CASE('u',  pound_u);
	goto __start;

/* #\s*i */
pound_i:
	GETNEXT NOTCASE('n', __start);
	GETNEXT NOTCASE('c', __start);
	GETNEXT NOTCASE('l', __start);
	GETNEXT NOTCASE('u', __start);
	GETNEXT NOTCASE('d', __start);
	GETNEXT NOTCASE('e', __start);
	goto pound_include;

/* #\s*include\s* */
pound_include:
	GETNEXT
	CASE(' ',  pound_include);
	CASE('\t', pound_include);
	map_dot = next;
	CASE('"',  pound_include_dquote);
/*	CASE('<',  pound_include_langle); */
	goto __start;

/* #\s*include\s*"(.*)" */
pound_include_dquote:
	GETNEXT
	CASE('\n', start);
	NOTCASE('"', pound_include_dquote);
	handle_include(0, map_dot, next - map_dot - 1);
	goto start;

/* #\s*include\s*<(.*)> */
pound_include_langle:
	GETNEXT
	CASE('\n', start);
	NOTCASE('>', pound_include_langle);
	handle_include(1, map_dot, next - map_dot - 1);
	goto start;

/* #\s*d */
pound_d:
	GETNEXT NOTCASE('e', __start);
	GETNEXT NOTCASE('f', __start);
	GETNEXT NOTCASE('i', __start);
	GETNEXT NOTCASE('n', __start);
	GETNEXT NOTCASE('e', __start);
	goto pound_define_undef;

/* #\s*u */
pound_u:
	GETNEXT NOTCASE('n', __start);
	GETNEXT NOTCASE('d', __start);
	GETNEXT NOTCASE('e', __start);
	GETNEXT NOTCASE('f', __start);
	goto pound_define_undef;

/*
 * #\s*(define|undef)\s*CONFIG_(\w*)
 *
 * this does not define the word, because it could be inside another
 * conditional (#if 0).  But I do parse the word so that this instance
 * does not count as a use.  -- mec
 */
pound_define_undef:
	GETNEXT
	CASE(' ',  pound_define_undef);
	CASE('\t', pound_define_undef);

	        NOTCASE('C', __start);
	GETNEXT NOTCASE('O', __start);
	GETNEXT NOTCASE('N', __start);
	GETNEXT NOTCASE('F', __start);
	GETNEXT NOTCASE('I', __start);
	GETNEXT NOTCASE('G', __start);
	GETNEXT NOTCASE('_', __start);

	map_dot = next;
pound_define_undef_CONFIG_word:
	GETNEXT
	if (isalnum(current) || current == '_')
		goto pound_define_undef_CONFIG_word;
	goto __start;

/* \<CONFIG_(\w*) */
cee:
	if (next >= map+2 && (isalnum((int)next[-2]) || next[-2] == '_'))
		goto start;
	GETNEXT NOTCASE('O', __start);
	GETNEXT NOTCASE('N', __start);
	GETNEXT NOTCASE('F', __start);
	GETNEXT NOTCASE('I', __start);
	GETNEXT NOTCASE('G', __start);
	GETNEXT NOTCASE('_', __start);

	map_dot = next;
cee_CONFIG_word:
	GETNEXT
	if (isalnum(current) || current == '_')
		goto cee_CONFIG_word;
	use_config(map_dot, next - map_dot - 1);
	goto __start;
    }
}



/*
 * Generate dependencies for one file.
 */
void do_depend(const char * filename, const char * command)
{
  int32_t mapsize;
  int32_t pagesizem1 = getpagesize()-1;
  int32_t fd;
  struct stat st;
  char * map;

  /*  fprintf(stderr,"Calling do_depend for %s\n",filename);*/

  fd = open(filename, O_RDONLY);
  if (fd < 0) {
    return;
  }

  fstat(fd, &st);
  if (st.st_size == 0) {
   fprintf(stderr,"%s is empty\n",filename);
    close(fd);
    return;
  }

  mapsize = st.st_size;
  mapsize = (mapsize+pagesizem1) & ~pagesizem1;
  map = (char *)mmap(NULL, mapsize, PROT_READ, MAP_PRIVATE, fd, 0);
  if ((long) map == -1) {
    perror("mkdep: mmap");
    close(fd);
    return;
  }
  if ((unsigned long) map % sizeof(unsigned long) != 0)
    {
      fprintf(stderr, "do_depend: map not aligned\n");
      exit(1);
    }

  hasdep = 0;
  clear_config();
  state_machine(map, map+st.st_size);

  munmap(map, mapsize);
  close(fd);
}



/*
 * Generate dependencies for all files.
 */
int32_t main(int32_t argc, char **argv)
{
  int32_t len;
  int32_t i;
  char path[4096];

  add_path(".");		/* for #include "..." */

  while (++argv, --argc > 0) {
    if (strncmp(*argv, "-I", 2) == 0) {
      if (*((*argv)+2)) {
	add_path((*argv)+2);
      } else {
	++argv;
	--argc;
	add_path(*argv);
      }
    }
    else if (strcmp(*argv, "--") == 0) {
      break;
    }
  }

  while (--argc > 0) {
    const char * filename = *++argv;
    const char * command  = __depname;
    g_filename = 0;
    len = strlen(filename);
    memcpy(depname, filename, len+1);
    if((len>2)&&(filename[len-2] =='.')&&
       (filename[len-1]=='c'||filename[len-1]=='S')){
      /* .c or .S files */
      depname[len-1] = 'o';
      g_filename = filename;
      command = "";
    }else if((len>4)&&(filename[len-4]=='.')&&(filename[len-3]=='c')&&
	     (filename[len-2]=='p')&&(filename[len-1]=='p')){
      /* .cpp files */
      depname[len-3] = 'o';
      depname[len-2] = 0;
      g_filename = filename;
      command = "";
    }
    do_depend(filename, command);
  }
  
  for(i=0;i<nPendingDeps;i++) {
    const char * filename = pendingDeps[i];
    const char * command  = __depname;
    g_filename = 0;
    len = strlen(filename);
    memcpy(depname, filename, len+1);

/*    fprintf(stderr,"[%s]\n",filename); */
    
    do_depend(filename, command);
  }

  for(i=0;i<nFoundDeps;i++) {
    int32_t j;
    
    if( foundDeps[i] == 0 )
      continue;
    
    j=1;
    while( foundDeps[i][j] ) {
      expandDeps(foundDeps[i][j]);
      j++;
    }
  }

  getcwd(path,4096);

#if 0
  {
    int32_t j=0;
    while( allDeps[j] ) {
      if (allDeps[j][0] == '/')
	printf(" %s",allDeps[j]);
      else
	printf(" %s/%s",path,allDeps[j]);
      j++;
    }
    printf("\n");
  }
#endif

  for(i=0;i<nFoundDeps;i++) {
    int32_t j;
    
    if( foundDeps[i] == 0 )
      continue;
    
    if( foundDeps[i][0][strlen(foundDeps[i][0])-1] != 'o' )
      continue;
    
    printf("$(OBJ)/%s:",foundDeps[i][0]);
    j=1;
    while( foundDeps[i][j] ) {
      if (foundDeps[i][j][0] == '/')
	printf(" %s",foundDeps[i][j]);
      else
	printf(" %s/%s",path,foundDeps[i][j]);
      j++;
    }
    printf("\n");
  }

  return 0;
}
