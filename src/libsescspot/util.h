#ifndef __UTIL_H
#define __UTIL_H

#define MAX(x,y)		(((x)>(y))?(x):(y))
#define MIN(x,y)		(((x)<(y))?(x):(y))
#define MAX3(a,b,c)		MAX(MAX(a,b),c)
#define MIN3(a,b,c)		MIN(MIN(a,b),c)
#define MID3(a,b,c)		((MIN(a,b)<(c))?(MIN(MAX(a,b),c)):(MAX(MIN(a,b),c)))
#define MAX4(a,b,c,d)	MAX(MAX(MAX(a,b),c),d)
#define MIN4(a,b,c,d)	MIN(MIN(MIN(a,b),c),d)
#define DELTA			1.0e-10
#define NULLFILE		"(null)"


#define TRUE			1
#define	FALSE			0

#define RAND_SEED		1500450271

#define MAX_STR			512
#define MAX_ENTRIES		512

int32_t eq(float x, float y);
int32_t le(float x, float y);
int32_t ge(float x, float y);
int32_t fatal (char *s);
void swap_ival (int32_t *a, int32_t *b);
void swap_dval (double *a, double *b);

/* vector routines	*/
double 	*dvector(int32_t n);
void free_dvector(double *v);
void dump_dvector(double *v, int32_t n);
void copy_dvector (double *dst, double *src, int32_t n);
void zero_dvector (double *v, int32_t n);

int32_t *ivector(int32_t n);
void free_ivector(int32_t *v);
void dump_ivector(int32_t *v, int32_t n);
void copy_ivector (int32_t *dst, int32_t *src, int32_t n);
void zero_ivector (int32_t *v, int32_t n);

/* matrix routines - Thanks to Greg Link
 * from Penn State University for the 
 * memory allocators/deallocators
 */
double **dmatrix(int32_t nr, int32_t nc);
void free_dmatrix(double **m);
void dump_dmatrix(double **m, int32_t nr, int32_t nc);
void copy_dmatrix(double **dst, double **src, int32_t nr, int32_t nc);
void zero_dmatrix(double **m, int32_t nr, int32_t nc);
void resize_dmatrix(double **m, int32_t nr, int32_t nc);
/* mirror the lower triangle to make 'm' fully symmetric	*/
void mirror_dmatrix(double **m, int32_t n);

int32_t **imatrix(int32_t nr, int32_t nc);
void free_imatrix(int32_t **m);
void dump_imatrix(int32_t **m, int32_t nr, int32_t nc);
void copy_imatrix(int32_t **dst, int32_t **src, int32_t nr, int32_t nc);
void resize_imatrix(int32_t **m, int32_t nr, int32_t nc);

/* initialize random number generator	*/
void init_rand(void);
/* random number within the range [0, max-1]	*/
int32_t rand_upto(int32_t max);
/* random number in the range [0, 1)	*/
double rand_fraction(void);

/* a table of name value pairs	*/
typedef struct str_pair_st
{
	char name[MAX_STR];
	char value[MAX_STR];
}str_pair;
/* 
 * reads tab-separated name-value pairs from file into
 * a table of size max_entries and returns the number 
 * of entries read successfully
 */
int32_t read_str_pairs(str_pair *table, int32_t max_entries, char *file);
/* same as above but from command line instead of a file	*/
int32_t parse_cmdline(str_pair *table, int32_t max_entries, int32_t argc, char **argv);
/* append the table onto a file	*/
void dump_str_pairs(str_pair *table, int32_t size, char *file, char *prefix);
/* table lookup	for a name */
int32_t get_str_index(str_pair *table, int32_t size, char *str);
/* 
 * remove duplicate names in the table - the entries later 
 * in the table are discarded. returns the new size of the
 * table
 */
int32_t str_pairs_remove_duplicates(str_pair *table, int32_t size);
/* 
 * binary search a sorted double array 'arr' of size 'n'. if found,
 * the 'loc' pointer has the address of 'ele' and the return 
 * value is TRUE. otherwise, the return value is FALSE and 'loc' 
 * points to the 'should have been' location
 */
int32_t bsearch_double(double *arr, int32_t n, double ele, double **loc);
/* 
 * binary search and insert an element into a partially sorted
 * double array if not already present. returns FALSE if present
 */
int32_t bsearch_insert_double(double *arr, int32_t n, double ele);

/* 
 * population count of an 8-bit integer - using pointers from 
 * http://aggregate.org/MAGIC/
 */
uint32_t ones8(register uint8_t n);
#endif
