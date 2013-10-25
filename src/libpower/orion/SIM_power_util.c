#include <stdio.h>
#include <string.h>
#include <math.h>
#include "LIB_defines.h"
#include "SIM_power.h"


/* Hamming distance table */
static u_char h_tab[256] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};


static u_int SIM_power_Hamming_slow( LIB_Type_max_uint old_val, LIB_Type_max_uint new_val, LIB_Type_max_uint mask )
{
  /* old slow code, I don't understand the new fast code though */
  /* LIB_Type_max_uint dist;
  u_int Hamming = 0;
  
  dist = ( old_val ^ new_val ) & mask;
  mask = (mask >> 1) + 1;
  
  while ( mask ) {
    if ( mask & dist ) Hamming ++;
    mask = mask >> 1;
  }

  return Hamming; */

#define TWO(k) (BIGONE << (k)) 
#define CYCL(k) (BIGNONE/(1 + (TWO(TWO(k))))) 
#define BSUM(x,k) ((x)+=(x) >> TWO(k), (x) &= CYCL(k)) 
  LIB_Type_max_uint x;
  
  x = (old_val ^ new_val) & mask;
  x = (x & CYCL(0)) + ((x>>TWO(0)) & CYCL(0)); 
  x = (x & CYCL(1)) + ((x>>TWO(1)) & CYCL(1)); 
  BSUM(x,2); 
  BSUM(x,3); 
  BSUM(x,4); 
  BSUM(x,5); 

  return x; 
}


int SIM_power_init(void)
{
  u_int i;
  
  /* initialize Hamming distance table */
  for (i = 0; i < 256; i++)
    h_tab[i] = SIM_power_Hamming_slow(i, 0, 0xFF);

  return 0;
}


/* assume LIB_Type_max_uint is u_int64_t */
u_int SIM_power_Hamming(LIB_Type_max_uint old_val, LIB_Type_max_uint new_val, LIB_Type_max_uint mask)
{
  union {
	  LIB_Type_max_uint x;
	  u_char id[8];
	} u;
  u_int rval;

  u.x = (old_val ^ new_val) & mask;
  
  rval = h_tab[u.id[0]];
  rval += h_tab[u.id[1]];
  rval += h_tab[u.id[2]];
  rval += h_tab[u.id[3]];
  rval += h_tab[u.id[4]];
  rval += h_tab[u.id[5]];
  rval += h_tab[u.id[6]];
  rval += h_tab[u.id[7]];

  return rval;
}


u_int SIM_power_Hamming_group(LIB_Type_max_uint d1_new, LIB_Type_max_uint d1_old, LIB_Type_max_uint d2_new, LIB_Type_max_uint d2_old, u_int width, u_int n_grp)
{
  u_int rval = 0;
  LIB_Type_max_uint g1_new, g1_old, g2_new, g2_old, mask;

  mask = HAMM_MASK(width);
  
  while (n_grp--) {
    g1_new = d1_new & mask;
    g1_old = d1_old & mask;
    g2_new = d2_new & mask;
    g2_old = d2_old & mask;

    if (g1_new != g1_old || g2_new != g2_old)
      rval ++;

    d1_new >>= width;
    d1_old >>= width;
    d2_new >>= width;
    d2_old >>= width;
  }

  return rval;
}


inline u_int SIM_power_logtwo(LIB_Type_max_uint x)
{
  u_int rval = 0;
  
  while (x >> rval) rval++;
  if (x == (BIGONE << (rval - 1))) rval--;

  return rval;
}


/*#
 * this routine takes the number of rows and cols of an array structure and
 * attemps to make it more of a reasonable circuit structure by trying to
 * make the number of rows and cols as close as possible.  (scaling both by
 * factors of 2 in opposite directions)
 *
 * Parameters:
 *   rows -- # of array rows
 *   cols -- # of array cols
 *
 * Return value: (if positive) scale factor which is the amount that the rows
 *               should be divided by and the cols should be multiplied by.
 *               (if negative) scale factor which is the amount that the cols
 *               should be divided by and the rows should be multiplied by.
 */
int SIM_power_squarify( int rows, int cols )
{
  int scale_factor = 0;

  if ( rows == cols )
    return 1;

  while ( rows > cols ) {
    rows = rows / 2;
    cols = cols * 2;
    if ( rows <= cols )
      return ( 1 << scale_factor );

    scale_factor ++;
  }

  while ( cols > rows ) {
    rows = rows * 2;
    cols = cols / 2;
    if ( cols <= rows )
      return -( 1 << scale_factor );

    scale_factor ++;
  }

  // added to remove warning
  return 0;
}


double SIM_power_driver_size(double driving_cap, double desiredrisetime)
{
  double nsize, psize;
  double Rpdrive; 

  Rpdrive = desiredrisetime/(driving_cap*log(PARM(VSINV))*-1.0);
  psize = SIM_power_restowidth(Rpdrive,PCH);
  nsize = SIM_power_restowidth(Rpdrive,NCH);
  if (psize > Wworddrivemax) {
    psize = Wworddrivemax;
  }
  if (psize < 4.0 * LSCALE)
    psize = 4.0 * LSCALE;

  return (psize);
}


inline int SIM_print_stat_energy(char *path, double Energy, int print_flag)
{
  u_int path_len;

  if (print_flag) {
    if (path) {
      path_len = strlen(path);
      strcat(path, ": %g\n");
      fprintf(stderr, path, Energy);
      SIM_power_res_path(path, path_len);
    }
    else
      fprintf(stderr, "%g\n", Energy);
  }

  return 0;
}


/*
 * SIDE EFFECT: change string dest
 *
 * NOTES: use SIM_power_res_path to restore dest
 */
char *SIM_power_strcat(char *dest, char *src)
{
  if (dest) {
    strcat(dest, "/");
    strcat(dest, src);
  }

  return dest;
}


int SIM_power_res_path(char *path, u_int id)
{
  if (path) path[id] = 0;

  return 0;
}


u_int SIM_power_strlen(char *s)
{
  if (s) return strlen(s);
  else return 0;
}


int SIM_power_dump_tech_para(void)
{
  fprintf(stderr, "technology: 0.%dum\n", PARM(TECH_POINT));
  fprintf(stderr, "Vdd: %gV\n", Vdd);
  fprintf(stderr, "frequency: %gHz\n", PARM(Freq));
  fprintf(stderr, "minimal spacing metal capacitance: %gF/um\n", CCmetal);
  fprintf(stderr, "2x minimal spacing metal capacitance: %gF/um\n", CC2metal);
  fprintf(stderr, "distant metal capacitance: %gF/um\n", Cmetal);
  fprintf(stderr, "gate capacitance: %gF/um^2\n", PARM(Cgate));
  fprintf(stderr, "N type diffusion area capacitance: %gF/um^2\n", PARM(Cndiffarea));
  fprintf(stderr, "N type diffusion side capacitance: %gF/um\n", PARM(Cndiffside));
  fprintf(stderr, "N type diffusion overlap capacitance: %gF/um\n", PARM(Cndiffovlp) + PARM(Cnoxideovlp));
  fprintf(stderr, "P type diffusion area capacitance: %gF/um^2\n", PARM(Cpdiffarea));
  fprintf(stderr, "P type diffusion side capacitance: %gF/um\n", PARM(Cpdiffside));
  fprintf(stderr, "P type diffusion overlap capacitance: %gF/um\n", PARM(Cpdiffovlp) + PARM(Cpoxideovlp));
  fprintf(stderr, "sense factor: 0.5\n");
  fprintf(stderr, "memory cell read access transistor width: %gum\n", Wmemcellr);
  fprintf(stderr, "memory cell write access transistor width: %gum\n", Wmemcellw);
  /* FIXME: error in code */
  fprintf(stderr, "memory cell inverter PMOS transistor width: %gum\n", Wmemcella);
  fprintf(stderr, "memory cell inverter NMOS transistor width: %gum\n", Wmemcella * Wmemcellbscale);
  fprintf(stderr, "memory cell height: %gum\n", RegCellHeight);
  fprintf(stderr, "memory cell width: %gum\n", RegCellWidth);
  fprintf(stderr, "bitline space: %gum\n", BitlineSpacing);
  fprintf(stderr, "wordline space: %gum\n", WordlineSpacing);

  return 0;
}
