#ifndef __NPE_H_
#define __NPE_H_

#include "flp.h"

/* normalized polish expression	*/
typedef struct NPE_t_st
{
	int32_t *elements;
	int32_t size;

	/* positions of the units	*/
	int32_t *unit_pos;
	int32_t n_units;

	/* 
	 * flipping positions  - where 
	 * a unit is immediately adjacent 
	 * to a cut_type and vice-versa
	 */
	int32_t *flip_pos;
	int32_t n_flips;

	/* positions of the chains	*/
	int32_t *chain_pos;
	int32_t n_chains;

	/* no. of units till this position	*/
	int32_t *ballot_count;
}NPE_t;

/* NPE routines	*/

/* the starting solution for simulated annealing	*/
NPE_t *NPE_get_initial(flp_desc_t *flp_desc);
/* uninitialization	*/
void free_NPE(NPE_t *expr);
/* debug print	*/
void print_NPE(NPE_t *expr, flp_desc_t *flp_desc);
/* 
 * move M1 of the floorplan paper 
 * swap two units adjacent in the NPE	
 */
void NPE_swap_units(NPE_t *expr, int32_t pos);
/* move M2 - invert a chain of cut_types in the NPE	*/
void NPE_invert_chain(NPE_t *expr, int32_t pos);
/* move M3 - swap adjacent cut_type and unit in the NPE	*/
int32_t NPE_swap_cut_unit(NPE_t *expr, int32_t pos);
/* make a random move out of the above	*/
NPE_t *make_random_move(NPE_t *expr);
/* make a copy of this NPE	*/
NPE_t *NPE_duplicate(NPE_t *expr);

#endif
