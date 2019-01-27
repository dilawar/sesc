/*------------------------------------------------------------
 *                              CACTI 4.0
 *         Copyright 2005 Hewlett-Packard Development Corporation
 *                         All Rights Reserved
 *
 * Permission to use, copy, and modify this software and its documentation is
 * hereby granted only under the following terms and conditions.  Both the
 * above copyright notice and this permission notice must appear in all copies
 * of the software, derivative works or modified versions, and any portions
 * thereof, and both notices must appear in supporting documentation.
 *
 * Users of this software agree to the terms and conditions set forth herein, and
 * hereby grant back to Hewlett-Packard Company and its affiliated companies ("HP")
 * a non-exclusive, unrestricted, royalty-free right and license under any changes,
 * enhancements or extensions  made to the core functions of the software, including
 * but not limited to those affording compatibility with other hardware or software
 * environments, but excluding applications which incorporate this software.
 * Users further agree to use their best efforts to return to HP any such changes,
 * enhancements or extensions that they make and inform HP of noteworthy uses of
 * this software.  Correspondence should be provided to HP at:
 *
 *                       Director of Intellectual Property Licensing
 *                       Office of Strategy and Technology
 *                       Hewlett-Packard Company
 *                       1501 Page Mill Road
 *                       Palo Alto, California  94304
 *
 * This software may be distributed (but not offered for sale or transferred
 * for compensation) to third parties, provided such third parties agree to
 * abide by the terms and conditions of this notice.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND HP DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL HP
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "cacti42_areadef.h"
#include "cacti42_basic_circuit.h"
#include "cacti42_def.h"
#include "cacti_interface.h"
#include "cacti42_leakage.h"
#include "cacti42_time.h"

#define NEXTINT(a) skip(); scanf("%d",&(a));
#define NEXTFLOAT(a) skip(); scanf("%lf",&(a));

/*---------------------------------------------------------------*/

//v4.1: No longer using calculate_area function as area has already been
//computed for the given tech node
extern double calculate_area(area_type,double);


int input_data(int argc,char *argv[])
{
   int C,B,A,ERP,EWP,RWP,NSER, NSubbanks, fully_assoc;
   double tech;
   double logbanks, assoc;
   double logbanksfloor, assocfloor;
   int bits_output = 64;

   if ((argc!=6) && (argc!=9)&& (argc!=15)) {
      printf("Cmd-line parameters: C B A TECH NSubbanks\n");
      printf("                 OR: C B A TECH RWP ERP EWP NSubbanks\n");
      exit(1);
   }

   B = atoi(argv[2]);
   if ((B < 1)) {
       printf("Block size must >=1\n");
       exit(1);
   }

    if (argc==9)
     {
		if ((B*8 < bits_output)) {
			printf("Block size must be at least %d\n", bits_output/8);
			exit(1);
		}

		tech = atof(argv[4]);
		if ((tech <= 0)) {
			printf("Feature size must be > 0\n");
			exit(1);
		 }
		if ((tech > 0.8)) {
			printf("Feature size must be <= 0.80 (um)\n");
			 exit(1);
		 }

       RWP = atoi(argv[5]);
       ERP = atoi(argv[6]);
       EWP = atoi(argv[7]);
       NSER = 0;

       if ((RWP < 0) || (EWP < 0) || (ERP < 0)) {
		 printf("Ports must >=0\n");
		 exit(1);
       }
       if (RWP > 2) {
		 printf("Maximum of 2 read/write ports\n");
		 exit(1);
       }
       if ((RWP+ERP+EWP) < 1) {
       	 printf("Must have at least one port\n");
       	 exit(1);
       }

       NSubbanks = atoi(argv[8]);

       if (NSubbanks < 1 ) {
         printf("Number of subbanks should be greater than or equal to 1 and should be a power of 2\n");
         exit(1);
       }

       logbanks = logtwo((double)(NSubbanks));
       logbanksfloor = floor(logbanks);

       if(logbanks > logbanksfloor){
         printf("Number of subbanks should be greater than or equal to 1 and should be a power of 2\n");
         exit(1);
       }

     }

   else if(argc==6)
     {

		if ((B*8 < bits_output)) {
			printf("Block size must be at least %d\n", bits_output/8);
			exit(1);
		}

		tech = atof(argv[4]);
		if ((tech <= 0)) {
			printf("Feature size must be > 0\n");
			exit(1);
		 }

		if ((tech > 0.8)) {
			printf("Feature size must be <= 0.80 (um)\n");
			exit(1);
		 }

       RWP=1;
       ERP=0;
       EWP=0;
       NSER=0;

       NSubbanks = atoi(argv[5]);
       if (NSubbanks < 1 ) {
         printf("Number of subbanks should be greater than or equal to 1 and should be a power of 2\n");
         exit(1);
       }
       logbanks = logtwo((double)(NSubbanks));
       logbanksfloor = floor(logbanks);

       if(logbanks > logbanksfloor){
         printf("Number of subbanks should be greater than or equal to 1 and should be a power of 2\n");
         exit(1);
       }

     }
	 else
     {
       tech = atof(argv[9]);
	   NSubbanks = atoi(argv[8]);
	   if ((tech <= 0)) {
			printf("Feature size must be > 0\n");
			exit(1);
		 }

	   if ((tech > 0.8)) {
			printf("Feature size must be <= 0.80 (um)\n");
			exit(1);
		}
     }

   C = atoi(argv[1])/((int) (NSubbanks));
   if (atoi(argv[1]) < 64) {
       printf("Cache size must be greater than 32!\n");
       exit(1);
   }

   if ((strcmp(argv[3],"FA") == 0) || (argv[3][0] == '0'))
     {
       A=C/B;
       fully_assoc = 1;
     }
   else
     {
       if (strcmp(argv[3],"DM") == 0)
         {
           A=1;
           fully_assoc = 0;
         }
       else
         {
           fully_assoc = 0;
           A = atoi(argv[3]);
		   if ((A < 0)||(A > 16)) {
             printf("Associativity must be  1,2,4,8,16 or 0(fully associative)\n");
             exit(1);
           }
           assoc = logtwo((double)(A));
           assocfloor = floor(assoc);

           if(assoc > assocfloor){
             printf("Associativity should be a power of 2\n");
             exit(1);
           }

           }
     }

   if (!fully_assoc && C/(B*A) < 1) {
     printf("Number of sets is less than 1:\n  Need to either increase cache size, or decrease associativity or block size\n  (or use fully associative cache)\n");
       exit(1);
   }

   return(OK);
}

void output_time_components(result_type *result,parameter_type *parameters)
{
int A;
       printf(" address routing delay (ns): %g\n",result->subbank_address_routing_delay/1e-9);
       //printf(" address routing power (nJ): %g\n",result->subbank_address_routing_power*1e9);

  A=parameters->tag_associativity;
   if (!parameters->fully_assoc)
     {
       printf(" decode_data (ns): %g\n",result->decoder_delay_data/1e-9);
       printf("     dyn. energy (nJ): %g\n",result->decoder_power_data.readOp.dynamic*1e9);
	   printf("     leak. power (mW): %g\n",result->decoder_power_data.readOp.leakage*1e3);
     }
   else
     {
       printf(" tag_comparison (ns): %g\n",result->decoder_delay_data/1e-9);
       printf("     dyn. energy (nJ): %g\n",result->decoder_power_data.readOp.dynamic*1e9);
	   printf("     leak. power (mW): %g\n",result->decoder_power_data.readOp.leakage*1e3);
     }
   printf(" wordline and bitline data (ns): %g\n",(result->wordline_delay_data+result->bitline_delay_data)/1e-9);
   printf("  dyn. wordline energy (nJ): %g\n",(result->wordline_power_data.readOp.dynamic)*1e9);
   printf("  leak. wordline power (mW): %g\n",(result->wordline_power_data.readOp.leakage)*1e3);
   printf("  dyn. read data bitline energy (nJ): %g\n",(result->bitline_power_data.readOp.dynamic)*1e9);
   printf("  dyn. write data bitline energy (nJ): %g\n",(result->bitline_power_data.writeOp.dynamic)*1e9);
   printf("  leak. data bitline power (mW): %g\n",(result->bitline_power_data.writeOp.leakage)*1e3);

   printf(" sense_amp_data (ns): %g\n",result->sense_amp_delay_data/1e-9);
   printf("     dyn. energy (nJ): %g\n",result->sense_amp_power_data.readOp.dynamic*1e9);
   printf("     leak. power (mW): %g\n",result->sense_amp_power_data.readOp.leakage*1e3);
   if (!parameters->fully_assoc)
     {
       printf(" decode_tag (ns): %g\n",result->decoder_delay_tag/1e-9);
       printf("     dyn. energy (nJ): %g\n",result->decoder_power_tag.readOp.dynamic*1e9);
	   printf("     leak. power (mW): %g\n",result->decoder_power_tag.readOp.leakage*1e3);

        printf(" wordline and bitline tag (ns): %g\n",(result->wordline_delay_tag+result->bitline_delay_tag)/1e-9);
		printf("  dyn. wordline energy (nJ): %g\n",(result->wordline_power_tag.readOp.dynamic)*1e9);
		printf("  leak. wordline power (mW): %g\n",(result->wordline_power_tag.readOp.leakage)*1e3);
		printf("  dyn. read data bitline energy (nJ): %g\n",(result->bitline_power_tag.readOp.dynamic)*1e9);
		printf("  dyn. write data bitline energy (nJ): %g\n",(result->bitline_power_tag.writeOp.dynamic)*1e9);
		printf("  leak. data bitline power (mW): %g\n",(result->bitline_power_tag.writeOp.leakage)*1e3);

       printf(" sense_amp_tag (ns): %g\n",result->sense_amp_delay_tag/1e-9);
       printf("  dyn. read energy (nJ): %g\n",result->sense_amp_power_tag.readOp.dynamic*1e9);
	   printf("  leak. read power (mW): %g\n",result->sense_amp_power_tag.readOp.leakage*1e3);
       printf(" compare (ns): %g\n",result->compare_part_delay/1e-9);
       printf("  dyn. read energy (nJ): %g\n",result->compare_part_power.readOp.dynamic*1e9);
	   printf("  leak. read power (mW): %g\n",result->compare_part_power.readOp.leakage*1e3);
       if (A == 1)
         {
           printf(" valid signal driver (ns): %g\n",result->drive_valid_delay/1e-9);
           printf("     dyn. read energy (nJ): %g\n",result->drive_valid_power.readOp.dynamic*1e9);
		   printf("    leak. read power (mW): %g\n",result->drive_valid_power.readOp.leakage*1e3);
         }
       else {
         printf(" mux driver (ns): %g\n",result->drive_mux_delay/1e-9);
         printf("  dyn. read energy (nJ): %g\n",result->drive_mux_power.readOp.dynamic*1e9);
		 printf("  leak. read power (mW): %g\n",result->drive_mux_power.readOp.leakage*1e3);
         printf(" sel inverter (ns): %g\n",result->selb_delay/1e-9);
         printf("  dyn. read energy (nJ): %g\n",result->selb_power.readOp.dynamic*1e9);
		 printf("  leak. read power (mW): %g\n",result->selb_power.readOp.leakage*1e3);
       }
     }
   printf(" data output driver (ns): %g\n",result->data_output_delay/1e-9);
   printf("	   dyn. read energy (nJ): %g\n",result->data_output_power.readOp.dynamic*1e9);
   printf("	   leak. read power (mW): %g\n",result->data_output_power.readOp.leakage*1e3);
   printf(" total_out_driver (ns): %g\n", result->total_out_driver_delay_data/1e-9);
   printf("	 dyn. read energy (nJ): %g\n", result->total_out_driver_power_data.readOp.dynamic*1e9);
   printf("	 leak. read power (mW): %g\n", result->total_out_driver_power_data.readOp.leakage*1e3);

   printf(" total data path (without output driver) (ns): %g\n",result->subbank_address_routing_delay/1e-9+result->decoder_delay_data/1e-9+result->wordline_delay_data/1e-9+result->bitline_delay_data/1e-9+result->sense_amp_delay_data/1e-9);
   if (!parameters->fully_assoc)
     {
       if (A==1)
         printf(" total tag path is dm (ns): %g\n", result->subbank_address_routing_delay/1e-9+result->decoder_delay_tag/1e-9+result->wordline_delay_tag/1e-9+result->bitline_delay_tag/1e-9+result->sense_amp_delay_tag/1e-9+result->compare_part_delay/1e-9);
       else
         printf(" total tag path is set assoc (ns): %g\n", result->subbank_address_routing_delay/1e-9+result->decoder_delay_tag/1e-9+result->wordline_delay_tag/1e-9+result->bitline_delay_tag/1e-9+result->sense_amp_delay_tag/1e-9+result->compare_part_delay/1e-9+result->drive_mux_delay/1e-9+result->selb_delay/1e-9);
     }
}

void output_area_components(arearesult_type *arearesult, parameter_type *parameters)
{
    printf("\nArea Components:\n\n");

	//v4.1: No longer using calculate_area function as area has already been
	//computed for the given tech node.
/*
    printf("Aspect Ratio Data height/width: %f\n", aspect_ratio_data);
    printf("Aspect Ratio Tag height/width: %f\n", aspect_ratio_tag);
    printf("Aspect Ratio Subbank height/width: %f\n", aspect_ratio_subbank);
    printf("Aspect Ratio Total height/width: %f\n\n", aspect_ratio_total);
*/
    printf("Aspect Ratio Total height/width: %f\n\n", arearesult->aspect_ratio_total);

	printf("Data array (mm^2): %f\n",arearesult->dataarray_area.scaled_area);
	printf("Data predecode (mm^2): %f\n",arearesult->datapredecode_area.scaled_area);
	printf("Data colmux predecode (mm^2): %f\n",arearesult->datacolmuxpredecode_area.scaled_area);
	printf("Data colmux post decode (mm^2): %f\n",arearesult->datacolmuxpostdecode_area.scaled_area);
	printf("Data write signal (mm^2): %f\n",arearesult->datawritesig_area.scaled_area);

	printf("\nTag array (mm^2): %f\n",arearesult->tagarray_area.scaled_area);
	printf("Tag predecode (mm^2): %f\n",arearesult->tagpredecode_area.scaled_area);
	printf("Tag colmux predecode (mm^2): %f\n",arearesult->tagcolmuxpredecode_area.scaled_area);
	printf("Tag colmux post decode (mm^2): %f\n",arearesult->tagcolmuxpostdecode_area.scaled_area);
	printf("Tag output driver decode (mm^2): %f\n",arearesult->tagoutdrvdecode_area.scaled_area);
	printf("Tag output driver enable signals (mm^2): %f\n",arearesult->tagoutdrvsig_area.scaled_area);

	printf("\nPercentage of data ramcells alone of total area: %f %%\n", arearesult->perc_data);
    printf("Percentage of tag ramcells alone of total area: %f %%\n",arearesult->perc_tag);
    printf("Percentage of total control/routing alone of total area: %f %%\n",arearesult->perc_cont);
	printf("\nSubbank Efficiency : %f\n", arearesult->sub_eff);
	printf("Total Efficiency : %f\n",arearesult->total_eff);
	printf("\nTotal area One bank (mm^2): %f\n",arearesult->totalarea);
	printf("Total area subbanked (mm^2): %f\n",arearesult->subbankarea);

}


void output_data(result_type *result,arearesult_type *arearesult, parameter_type *parameters)
{
   double datapath,tagpath;

   FILE *stream;

   stream=fopen("cache_params.aux", "w");

   datapath = result->subbank_address_routing_delay+result->decoder_delay_data+result->wordline_delay_data+result->bitline_delay_data+result->sense_amp_delay_data+result->total_out_driver_delay_data+result->data_output_delay;
   if (parameters->tag_associativity == 1) {
         tagpath = result->subbank_address_routing_delay+result->decoder_delay_tag+result->wordline_delay_tag+result->bitline_delay_tag+result->sense_amp_delay_tag+result->compare_part_delay+result->drive_valid_delay;
   } else {
         tagpath = result->subbank_address_routing_delay+result->decoder_delay_tag+result->wordline_delay_tag+result->bitline_delay_tag+result->sense_amp_delay_tag+result->compare_part_delay+
			 //result->drive_mux_delay+
			 result->selb_delay+result->data_output_delay+result->total_out_driver_delay_data;
   }

   if (stream){
	   fprintf(stream, "#define PARAMNDWL %d\n#define PARAMNDBL %d\n#define PARAMNSPD %f\n#define PARAMNTWL %d\n#define PARAMNTBL %d\n#define PARAMNTSPD %d\n#define PARAMSENSESCALE %f\n#define PARAMGS %d\n#define PARAMDNOR %d\n#define PARAMTNOR %d\n#define PARAMRPORTS %d\n#define PARAMWPORTS %d\n#define PARAMRWPORTS %d\n#define PARAMMUXOVER %d\n", result->best_Ndwl, result->best_Ndbl, result->best_Nspd, result->best_Ntwl, result->best_Ntbl, result->best_Ntspd, result->senseext_scale, (result->senseext_scale==1.0), result->data_nor_inputs, result->tag_nor_inputs, parameters->num_read_ports, parameters->num_write_ports, parameters->num_readwrite_ports, result->best_muxover);
   }

    fclose(stream);
#  if OUTPUTTYPE == LONG
      printf("\n---------- CACTI version 4.1 ----------\n");
      printf("\nCache Parameters:\n");
	  printf("  Number of banks: %d\n",(int)result->subbanks);
      printf("  Total Cache Size: %d\n",(int) (parameters->cache_size));
      printf("  Size in bytes of a bank: %d\n",parameters->cache_size / (int)result->subbanks);
      printf("  Number of sets per bank: %d\n",parameters->number_of_sets);
      if (parameters->fully_assoc)
        printf("  Associativity: fully associative\n");
      else
        {
          if (parameters->tag_associativity==1)
            printf("  Associativity: direct mapped\n");
          else
            printf("  Associativity: %d\n",parameters->tag_associativity);
        }
      printf("  Block Size (bytes): %d\n",parameters->block_size);
      printf("  Read/Write Ports: %d\n",parameters->num_readwrite_ports);
      printf("  Read Ports: %d\n",parameters->num_read_ports);
      printf("  Write Ports: %d\n",parameters->num_write_ports);
      printf("  Technology Size: %2.2fum\n", parameters->tech_size);
      printf("  Vdd: %2.1fV\n", parameters->VddPow);

      printf("\nAccess Time (ns): %g\n",result->access_time*1e9);
      printf("Cycle Time (ns):  %g\n",result->cycle_time*1e9);
      //if (parameters->fully_assoc)
        //{
			printf("Total dynamic Read Power at max. freq. (W): %g\n",result->total_power_allbanks.readOp.dynamic/result->cycle_time);
			printf("Total leakage Read/Write Power all Banks (mW): %g\n",result->total_power_allbanks.readOp.leakage*1e3);
			printf("Total dynamic Read Energy all Banks (nJ): %g\n",result->total_power_allbanks.readOp.dynamic*1e9);
			printf("Total dynamic Write Energy all Banks (nJ): %g\n",result->total_power_allbanks.writeOp.dynamic*1e9);
			printf("Total dynamic Read Energy Without Routing (nJ): %g\n",result->total_power_without_routing.readOp.dynamic*1e9);
			printf("Total dynamic Write Energy Without Routing (nJ): %g\n",result->total_power_without_routing.writeOp.dynamic*1e9);
			printf("Total dynamic Routing Energy (nJ): %g\n",result->total_routing_power.readOp.dynamic*1e9);
			printf("Total leakage Read/Write Power Without Routing (mW): %g\n",result->total_power_without_routing.readOp.leakage*1e3);
			//printf("Total leakage Write Power all Banks (mW): %g\n",result->total_power_allbanks.writeOp.leakage*1e3);
			//printf("Total leakage Write Power Without Routing (mW): %g\n",result->total_power_without_routing.writeOp.leakage*1e3);
			printf("Total leakage Read/Write Routing Power (mW): %g\n",result->total_routing_power.readOp.leakage*1e3);

          //printf("Maximum Bank Power (nJ):  %g\n",(result->subbank_address_routing_power+result->decoder_power_data+result->wordline_power_data+result->bitline_power_data+result->sense_amp_power_data+result->data_output_power+result->total_out_driver_power_data)*1e9);
          //      printf("Power (W) - 500MHz:  %g\n",(result->decoder_power_data+result->wordline_power_data+result->bitline_power_data+result->sense_amp_power_data+result->data_output_power)*500*1e6);
        //}
      /*else
        {
	  printf("Total dynamic Read Power at max. freq. (W): %g\n",result->total_power_allbanks.readOp.dynamic/result->cycle_time);
	  printf("Total dynamic Read Energy all Banks (nJ): %g\n",result->total_power_allbanks.readOp.dynamic*1e9);
	  printf("Total leakage Read Power all Banks (mW): %g\n",result->total_power_allbanks.readOp.leakage*1e3);
	  printf("Total dynamic Write Energy all Banks (nJ): %g\n",result->total_power_allbanks.writeOp.dynamic*1e9);
	  printf("Total leakage Write Power all Banks (mW): %g\n",result->total_power_allbanks.writeOp.leakage*1e3);
	  printf("Total dynamic Read Energy Without Routing (nJ): %g\n",result->total_power_without_routing.readOp.dynamic*1e9);
	  printf("Total leakage Read Power Without Routing (mW): %g\n",result->total_power_without_routing.readOp.leakage*1e3);
	  printf("Total dynamic Write Energy Without Routing (nJ): %g\n",result->total_power_without_routing.writeOp.dynamic*1e9);
	  printf("Total leakage Write Power Without Routing (mW): %g\n",result->total_power_without_routing.writeOp.leakage*1e3);
	  printf("Total dynamic Routing Energy (nJ): %g\n",result->total_routing_power.readOp.dynamic*1e9);
	  printf("Total leakage Routing Power (mW): %g\n",result->total_routing_power.readOp.leakage*1e3);*/
          //printf("Maximum Bank Power (nJ):  %g\n",(result->subbank_address_routing_power+result->decoder_power_data+result->wordline_power_data+result->bitline_power_data+result->sense_amp_power_data+result->total_out_driver_power_data+result->decoder_power_tag+result->wordline_power_tag+result->bitline_power_tag+result->sense_amp_power_tag+result->compare_part_power+result->drive_valid_power+result->drive_mux_power+result->selb_power+result->data_output_power)*1e9);
          //      printf("Power (W) - 500MHz:  %g\n",(result->decoder_power_data+result->wordline_power_data+result->bitline_power_data+result->sense_amp_power_data+result->total_out_driver_power_data+result->decoder_power_tag+result->wordline_power_tag+result->bitline_power_tag+result->sense_amp_power_tag+result->compare_part_power+result->drive_valid_power+result->drive_mux_power+result->selb_power+result->data_output_power)*500*1e6);
        //}
      printf("\nBest Ndwl (L1): %d\n",result->best_Ndwl);
      printf("Best Ndbl (L1): %d\n",result->best_Ndbl);
      printf("Best Nspd (L1): %f\n",result->best_Nspd);
      printf("Best Ntwl (L1): %d\n",result->best_Ntwl);
      printf("Best Ntbl (L1): %d\n",result->best_Ntbl);
      printf("Best Ntspd (L1): %d\n",result->best_Ntspd);
      //printf("Nor inputs (data): %d\n",result->data_nor_inputs);
      //printf("Nor inputs (tag): %d\n",result->tag_nor_inputs);

	  output_area_components(arearesult,parameters);
      printf("\nTime Components:\n");

      printf(" data side (with Output driver) (ns): %g\n",datapath/1e-9);
      if (!parameters->fully_assoc)
        printf(" tag side (with Output driver) (ns): %g\n",(tagpath)/1e-9);
      output_time_components(result,parameters);

#  else
      printf("%d %d %d  %d %d %d %d %d %d  %e %e %e %e  %e %e %e %e  %e %e %e %e  %e %e %e %e  %e %e\n",
               parameters->cache_size,
               parameters->block_size,
               parameters->associativity,
               result->best_Ndwl,
               result->best_Ndbl,
               result->best_Nspd,
               result->best_Ntwl,
               result->best_Ntbl,
               result->best_Ntspd,
               result->access_time,
               result->cycle_time,
               datapath,
               tagpath,
               result->decoder_delay_data,
               result->wordline_delay_data,
               result->bitline_delay_data,
               result->sense_amp_delay_data,
               result->decoder_delay_tag,
               result->wordline_delay_tag,
               result->bitline_delay_tag,
               result->sense_amp_delay_tag,
               result->compare_part_delay,
               result->drive_mux_delay,
               result->selb_delay,
               result->drive_valid_delay,
               result->data_output_delay,
               result->precharge_delay);



# endif

}

void output_params(parameter_type *parameters)
{
      printf("\n***** Cache Parameters:\n");
      printf("  Cache Size: %d\n",(int) (parameters->cache_size));
      printf("  Number of sets per bank: %d\n",parameters->number_of_sets);
      if (parameters->fully_assoc)
        printf("  Associativity: fully associative\n");
      else        {
          if (parameters->tag_associativity==1)
            printf("  Associativity: direct mapped\n");
          else
            printf("  Associativity: %d\n",parameters->tag_associativity);
      }
      printf("  tag_associativity: %d\n",parameters->tag_associativity);
      printf("  data_associativity: %d\n",parameters->data_associativity);
      printf("  Block Size (bytes): %d\n",parameters->block_size);
      printf("  Read/Write Ports: %d\n",parameters->num_readwrite_ports);
      printf("  Read Ports: %d\n",parameters->num_read_ports);
      printf("  Write Ports: %d\n",parameters->num_write_ports);
      printf("  num_single_ended_read_ports: %d\n", parameters->num_single_ended_read_ports);
      printf("  NSubbanks: %d\n",parameters->NSubbanks);
      printf("  fully_assoc: %d\n", parameters->fully_assoc);
      printf("  fudgefactor: %2.1fV\n", parameters->fudgefactor);
      printf("  Technology Size: %2.2fum\n", parameters->tech_size);
      printf("  Vdd: %2.1fV\n", parameters->VddPow);
      printf("  sequential_access: %dV\n", parameters->sequential_access);
      printf("  fast_access: %d\n", parameters->fast_access);
      printf("  force_tag: %d\n", parameters->force_tag);
      printf("  tag_size: %d\n", parameters->tag_size);
      printf("  nr_bits_out: %d\n", parameters->nr_bits_out);
      printf("  pure_sram: %d\n", parameters->pure_sram);
      printf("  **************************\n");

}


total_result_type cacti_interface(
		int cache_size,
		int line_size,
		int associativity,
		int rw_ports,
		int excl_read_ports,
		int excl_write_ports,
		int single_ended_read_ports,
		int banks,
		double tech_node,
		int output_width,
		int specific_tag,
		int tag_width,
		int access_mode,
		int pure_sram)
{
   int C,B,A,ERP,EWP,RWP,NSER;
   double tech;
   double logbanks, assoc;
   double logbanksfloor, assocfloor;
   int seq_access = 0;
   int fast_access = 0;
   int bits_output = output_width;
   int nr_args = 9;
   double NSubbanks = (double)banks;

   double ratioofbankstoports;

   extern int force_tag, force_tag_size;



   total_result_type endresult;

   result_type result;
   arearesult_type arearesult;
   area_type arearesult_subbanked;
   parameter_type parameters;

   /* input parameters:
         C B A ERP EWP */

   /*dt: make sure we're using some simple leakage reduction */
   dualVt = FALSE;

//#ifdef XCACTI
  //parameters.latchsa    = 0;
  //parameters.ignore_tag = 0;
//#endif
   force_tag = 0;
   parameters.force_tag = 0;

   if(specific_tag) {
      force_tag = 1;
      force_tag_size = tag_width;
      parameters.force_tag = 1;
      parameters.tag_size = tag_width;
      //parameters.ignore_tag = 1;
   }
   

   switch (access_mode){
	   case 0:
		   seq_access = fast_access = FALSE;
		   break;
	   case 1:
		   seq_access = TRUE;
		   fast_access = FALSE;
		   break;
	   case 2:
		   seq_access = FALSE;
		   fast_access = TRUE;
		   break;
   }

   C = cache_size;
   A = associativity;
   B = line_size;
   if ((B < 1)) {
       printf("Block size must >=1\n");
       return endresult;
	   //exit(1);
   }

   if ((B*8 < bits_output)) {
       printf("Block size must be at least %d\n", bits_output/8);
       return endresult;
	   //exit(1);
   }


   tech = tech_node;
   if ((tech <= 0)) {
       printf("Feature size must be > 0\n");
       return endresult;
	   //exit(1);
   }
   if ((tech > 0.8)) {
       printf("Feature size must be <= 0.80 (um)\n");
       return endresult;
	   //exit(1);
   }

   if(nr_args==6) {
     RWP=1;
     ERP=0;
     EWP=0;
     NSER=0;
   } else if(nr_args==8) {
     RWP=1;
     ERP=0;
     EWP=0;
     NSER=0;
     bits_output = output_width;
     seq_access = 1;
   } else if (nr_args ==9) {
     RWP = rw_ports;
     ERP = excl_read_ports;
     EWP = excl_write_ports;
     NSER = single_ended_read_ports;
   } else if (nr_args>=10) {
     RWP = rw_ports;
     ERP = excl_read_ports;
     EWP = excl_write_ports;
     NSER = single_ended_read_ports;
     seq_access = 1;
   }
   if ((RWP < 0) || (EWP < 0) || (ERP < 0)) {
		printf("Ports must >=0\n");
		return endresult;
		//exit(1);
   }
   if (RWP > 2) {
		printf("Maximum of 2 read/write ports\n");
		return endresult;
		//exit(1);
   }
   if ((RWP+ERP+EWP) < 1) {
   	 printf("Must have at least one port\n");
   	 return endresult;
	 //exit(1);
   }

   if (NSubbanks < 1 ) {
     printf("Number of subbanks should be greater than or equal to 1 and should be a power of 2\n");
     return endresult;
	 //exit(1);
   }

   logbanks = logtwo((double)(NSubbanks));
   logbanksfloor = floor(logbanks);

   if(logbanks > logbanksfloor){
     printf("Number of subbanks should be greater than or equal to 1 and should be a power of 2\n");
     return endresult;
	 //exit(1);
   }

  if (C == B * A) {
       parameters.fully_assoc = 1;
       A = C/B;
  }else{
       parameters.fully_assoc = 0;
  }


   C = cache_size/((int) (NSubbanks));
   if ((C < 64)) {
       printf("Cache size must >=64\n");
       return endresult;
	   //exit(1);
   }

   //A = C/B;
   if (A > 16) {
       parameters.fully_assoc = 1;
       A = 16;
  }

   /*if ((associativity == 0) || (A == C/B)) {
       A = C/B;
       parameters.fully_assoc = 1;
   } else {
       if (associativity == 1)
         {
           A=1;
           parameters.fully_assoc = 0;
         }
       else
         {
           parameters.fully_assoc = 0;
           A = associativity;
           if ((A < 1)) {
             printf("Associativity must >= 1\n");
             return endresult;
			 //exit(1);
           }
           assoc = logtwo((double)(A));
           assocfloor = floor(assoc);

           if(assoc > assocfloor){
             printf("Associativity should be a power of 2\n");
             return endresult;
			 //exit(1);
           }

           if ((A > 32)) {
             printf("Associativity must <= 32\n or try FA (fully associative)\n");
             return endresult;
			 //exit(1);
           }
         }
     }

   if (C/(B*A)<=1 && !parameters.fully_assoc) {
     //printf("Number of sets is too small:\n  Need to either increase cache size, or decrease associativity or block size\n  (or use fully associative cache)\n");
     //return endresult;
       A = C/B;
       parameters.fully_assoc = 1;
	//exit(1);
   }*/

   printf("\n########### Printing input for params for testing...###");
   printf("\n C = %d, B = %d, A = %d", C, B, A);
   printf("\n RWP = %d, ERP = %d, EWP = %d, NSER = %d", RWP, ERP, EWP, NSER);
   printf("\n banks = %d, tech = %f, bits_output = %d, fast_access = %d, pure_sram = %d", 
        banks, tech, bits_output, fast_access, pure_sram);
   printf("\n force_tag = %d, force_tag_size = %d", force_tag, force_tag_size);
   printf("\n #################\n");

   parameters.cache_size = C;
   parameters.block_size = B;

   parameters.nr_bits_out = bits_output;
   /*dt: testing sequential access mode*/
   if(seq_access) {
	parameters.tag_associativity = A;
	parameters.data_associativity = 1;
	parameters.sequential_access = 1;
   }
   else {
	parameters.tag_associativity = parameters.data_associativity = A;
	parameters.sequential_access = 0;
   }
   if(fast_access) {
	   parameters.fast_access = 1;
   }
   else {
	   parameters.fast_access = 0;
   }
   parameters.num_readwrite_ports = RWP;
   parameters.num_read_ports = ERP;
   parameters.num_write_ports = EWP;
   parameters.NSubbanks = banks;
   parameters.num_single_ended_read_ports =NSER;
   parameters.number_of_sets = C/(B*A);
   parameters.fudgefactor = .8/tech;
   parameters.tech_size=(double) tech;
   parameters.pure_sram = pure_sram;
   //If multiple banks and multiple ports are specified, then if number of banks/total number
   //of ports > 1 then assume that the multiple ports are implemented via the multiple banks.
   //Also assume that each bank has only 1 RWP port. There are some problems with this logic that
   //will be fixed in v5.0
   ratioofbankstoports = NSubbanks/(RWP + ERP + EWP);
   if(ratioofbankstoports >= 1.0){
	   //We assume that each bank has 1 RWP port.
	   parameters.num_readwrite_ports = 1;
       parameters.num_read_ports = 0;
       parameters.num_write_ports = 0;
       parameters.num_single_ended_read_ports = 0;
	}

   if (parameters.number_of_sets < 1) {
      printf("Less than one set...\n");
      return endresult;
	  //exit(1);
   }
 
  parameters.VddPow=4.5/(pow(parameters.fudgefactor,(2.0/3.0)));
  if (parameters.VddPow < 0.7)
    parameters.VddPow=0.7;
  if (parameters.VddPow > 5.0)
    parameters.VddPow=5.0;

   printf("\n##### Printing parameters for testing...#####\n");
   output_params(&parameters);

   init_tech_params_default_process();//v4.1: First initialize all tech variables
   //to 0.8 micron values. init_tech_params function below then reinitializes tech variables to
   //given process values
   init_tech_params(parameters.tech_size);
   calculate_time(&result,&arearesult,&arearesult_subbanked,&parameters,&NSubbanks);

   //v4.1: No longer using calculate_area function as area has already been
   //computed for the given tech node

   /*arearesult.dataarray_area.scaled_area = calculate_area(arearesult.dataarray_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;
   arearesult.datapredecode_area.scaled_area = calculate_area(arearesult.datapredecode_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;
   arearesult.datacolmuxpredecode_area.scaled_area = calculate_area(arearesult.datacolmuxpredecode_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;
   arearesult.datacolmuxpostdecode_area.scaled_area = calculate_area(arearesult.datacolmuxpostdecode_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;
   arearesult.datawritesig_area.scaled_area = (parameters.num_readwrite_ports+parameters.num_read_ports+parameters.num_write_ports)*calculate_area(arearesult.datawritesig_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;

   arearesult.tagarray_area.scaled_area = calculate_area(arearesult.tagarray_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;
   arearesult.tagpredecode_area.scaled_area = calculate_area(arearesult.tagpredecode_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;
   arearesult.tagcolmuxpredecode_area.scaled_area = calculate_area(arearesult.tagcolmuxpredecode_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;
   arearesult.tagcolmuxpostdecode_area.scaled_area = calculate_area(arearesult.tagcolmuxpostdecode_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;
   arearesult.tagoutdrvdecode_area.scaled_area = calculate_area(arearesult.tagoutdrvdecode_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;
   arearesult.tagoutdrvsig_area.scaled_area = (parameters.num_readwrite_ports+parameters.num_read_ports+parameters.num_write_ports)*
											   calculate_area(arearesult.tagoutdrvsig_area,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;

   arearesult.perc_data = 100*area_all_dataramcells/(arearesult.totalarea*CONVERT_TO_MMSQUARE);
   arearesult.perc_tag  = 100*area_all_tagramcells/(arearesult.totalarea*CONVERT_TO_MMSQUARE);
   arearesult.perc_cont = 100*(arearesult.totalarea*CONVERT_TO_MMSQUARE-area_all_dataramcells-area_all_tagramcells)/(arearesult.totalarea*CONVERT_TO_MMSQUARE);
   arearesult.sub_eff   = (area_all_dataramcells+area_all_tagramcells)*100/(arearesult.totalarea/100000000.0);
   arearesult.total_eff = (NSubbanks)*(area_all_dataramcells+area_all_tagramcells)*100/
							(calculate_area(arearesult_subbanked,parameters.fudgefactor)*CONVERT_TO_MMSQUARE);
   arearesult.totalarea *= CONVERT_TO_MMSQUARE;
   arearesult.subbankarea = calculate_area(arearesult_subbanked,parameters.fudgefactor)*CONVERT_TO_MMSQUARE;*/

   arearesult.dataarray_area.scaled_area = arearesult.dataarray_area.height * arearesult.dataarray_area.width * CONVERT_TO_MMSQUARE;
   arearesult.datapredecode_area.scaled_area = arearesult.datapredecode_area.height * arearesult.datapredecode_area.width * CONVERT_TO_MMSQUARE;
   arearesult.datacolmuxpredecode_area.scaled_area = arearesult.datacolmuxpredecode_area.height * arearesult.datacolmuxpredecode_area.width * CONVERT_TO_MMSQUARE;
   arearesult.datacolmuxpostdecode_area.scaled_area = arearesult.datacolmuxpostdecode_area.height * arearesult.datacolmuxpostdecode_area.width * CONVERT_TO_MMSQUARE;
   arearesult.datawritesig_area.scaled_area = (parameters.num_readwrite_ports+parameters.num_read_ports+parameters.num_write_ports)* arearesult.datawritesig_area.height * arearesult.datawritesig_area.width * CONVERT_TO_MMSQUARE;

   arearesult.tagarray_area.scaled_area = arearesult.tagarray_area.height * arearesult.tagarray_area.width * CONVERT_TO_MMSQUARE;
   arearesult.tagpredecode_area.scaled_area = arearesult.tagpredecode_area.height * arearesult.tagpredecode_area.width * CONVERT_TO_MMSQUARE;
   arearesult.tagcolmuxpredecode_area.scaled_area = arearesult.tagcolmuxpredecode_area.height * arearesult.tagcolmuxpredecode_area.width * CONVERT_TO_MMSQUARE;
   arearesult.tagcolmuxpostdecode_area.scaled_area = arearesult.tagcolmuxpostdecode_area.height* arearesult.tagcolmuxpostdecode_area.width * CONVERT_TO_MMSQUARE;
   arearesult.tagoutdrvdecode_area.scaled_area = arearesult.tagoutdrvdecode_area.height * arearesult.tagoutdrvdecode_area.width * CONVERT_TO_MMSQUARE;
   arearesult.tagoutdrvsig_area.scaled_area = (parameters.num_readwrite_ports+parameters.num_read_ports+parameters.num_write_ports)*
											 arearesult.tagoutdrvsig_area.height * arearesult.tagoutdrvsig_area.width * CONVERT_TO_MMSQUARE;

   arearesult.perc_data = 100*area_all_dataramcells/(arearesult.totalarea*CONVERT_TO_MMSQUARE);
   arearesult.perc_tag  = 100*area_all_tagramcells/(arearesult.totalarea*CONVERT_TO_MMSQUARE);
   arearesult.perc_cont = 100*(arearesult.totalarea*CONVERT_TO_MMSQUARE-area_all_dataramcells-area_all_tagramcells)/(arearesult.totalarea*CONVERT_TO_MMSQUARE);
   arearesult.sub_eff   = (area_all_dataramcells+area_all_tagramcells)*100/(arearesult.totalarea/100000000.0);
   arearesult.total_eff = (NSubbanks)*(area_all_dataramcells+area_all_tagramcells)*100/
							(arearesult_subbanked.height * arearesult_subbanked.width * CONVERT_TO_MMSQUARE);
   arearesult.totalarea *= CONVERT_TO_MMSQUARE;
   arearesult.subbankarea = arearesult_subbanked.height * arearesult_subbanked.width * CONVERT_TO_MMSQUARE;


   if(result.bitline_delay_data < 0.0) {
	   result.bitline_delay_data = 10^-12;
   }
   if(result.bitline_delay_tag < 0.0) {
	   result.bitline_delay_tag = 10^-13;
   }
   endresult.result = result;
   endresult.result.subbanks = banks;
   endresult.area = arearesult;
   endresult.params = parameters;

   return endresult;
}

/*void output_data_csv(total_result_type* result)
{
	 FILE *fptr;
	 errno_t err;

	 err = fopen_s(&fptr, "out.csv", "a");
	 if(err != 0){
		 printf("File out.csv could not be opened successfully\n");
	 }
	 else{*/
		//fprintf(fptr, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", "Tech node (nm),", "Capacity (bytes),", "Associativity,", "Access time (ns), ", "Cycle time (ns), ", "Dynamic read energy (nJ), ", "Dynamic read power (mW), ", "Leakage read power(mW), ", "Area (mm2), ", "Ndwl, ", "Ndbl, ", "Nspd, ", "Ntwl, ", "Ntbl, ", "Ntspd\n");
		/*fprintf(fptr, "%f%s%d%s%d%s%f%s%f%s%f%s%f%s%f%s%f%s%d%s%d%s%f%s%d%s%d%s%d%s", result->params.tech_size*1000, ",", result->params.cache_size, "," , result->params.tag_associativity, ",", result->result.access_time*1e+9, ",", result->result.cycle_time*1e+9, ",", result->result.total_power.readOp.dynamic*1e+9, ",", result->result.total_power.readOp.dynamic*1000/result->result.cycle_time, ",", result->result.total_power.readOp.leakage*1000, ",", result->area.totalarea, ",", result->result.best_Ndwl, ",", result->result.best_Ndbl, ",", result->result.best_Nspd, ",", result->result.best_Ntwl, ",", result->result.best_Ntbl, ",", result->result.best_Ntspd, "\n");
		fclose(fptr);
	 }
}*/


void xcacti_power_flp(const result_type *result,
                      const arearesult_type *ar,
                      const area_type *arearesult_subbanked,
                      const parameter_type *parameters,
                      xcacti_flp *xflp) {

  //---------------------------------------
  // area calculations
  double data_array_area = calculate_area(ar->dataarray_area, parameters->fudgefactor);
  double tag_array_area  = calculate_area(ar->tagarray_area , parameters->fudgefactor);

  double decode_area     = calculate_area(ar->datapredecode_area, parameters->fudgefactor)+
    calculate_area(ar->tagpredecode_area, parameters->fudgefactor);

  double data_ctrl_area  = calculate_area(ar->datacolmuxpredecode_area, parameters->fudgefactor)+
    calculate_area(ar->datacolmuxpostdecode_area, parameters->fudgefactor)+
    calculate_area(ar->datawritesig_area, parameters->fudgefactor);


  double tag_ctrl_area = calculate_area(ar->tagcolmuxpredecode_area, parameters->fudgefactor)+
    calculate_area(ar->tagcolmuxpostdecode_area, parameters->fudgefactor)+
    calculate_area(ar->tagoutdrvdecode_area, parameters->fudgefactor)+
    calculate_area(ar->tagoutdrvsig_area, parameters->fudgefactor);

//  if (parameters->fully_assoc || parameters->ignore_tag) {
  if (parameters->fully_assoc || parameters->force_tag) {
    tag_ctrl_area  = 0;
    tag_array_area = 0;
  }

  double total_area = data_array_area+ tag_array_area + decode_area +
    data_ctrl_area + tag_ctrl_area;
#if 0
  double bank_ctrl_area = ar->totalarea-
    calculate_area(ar->dataarray_area, parameters->fudgefactor)-
    calculate_area(ar->tagarray_area, parameters->fudgefactor);
#else
  double bank_ctrl_area = calculate_area(*arearesult_subbanked,parameters->fudgefactor) -
    (parameters->NSubbanks)*(total_area);
#endif
  bank_ctrl_area /= parameters->NSubbanks;
  total_area += bank_ctrl_area;

  //---------------------------------------
  // Energy calculations
  double data_array_e = result->wordline_power_data.readOp.dynamic +
					    result->bitline_power_data.readOp.dynamic;
  
  double data_ctrl_e  = result->sense_amp_power_data.readOp.dynamic +
						result->data_output_power.readOp.dynamic +
						result->total_out_driver_power_data.readOp.dynamic;

  double decode_e;
  double tag_array_e;
  double tag_ctrl_e;
//  if (parameters->fully_assoc || parameters->ignore_tag) {
  if (parameters->fully_assoc || parameters->force_tag) {
    decode_e = result->decoder_power_data.readOp.dynamic;
    tag_ctrl_e  = 0;
    tag_array_e = 0;
  }else{
    tag_array_e = result->wordline_power_tag.readOp.dynamic +
					result->bitline_power_tag.readOp.dynamic;
	decode_e = result->decoder_power_data.readOp.dynamic +
				result->decoder_power_tag.readOp.dynamic;
	tag_ctrl_e = result->sense_amp_power_tag.readOp.dynamic +
				result->compare_part_power.readOp.dynamic +
				result->drive_valid_power.readOp.dynamic +
				result->drive_mux_power.readOp.dynamic +
				result->selb_power.readOp.dynamic;
  }

  double bank_ctrl_e = result->subbank_address_routing_power.readOp.dynamic;

#if 0
  printf("decode     %6.3g%%  %9.3fpJ\n",100*decode_area/total_area    ,decode_e    *PICOJ);
  printf("tag  array %6.3g%%  %9.3fpJ\n",100*tag_array_area/total_area ,tag_array_e *PICOJ);
  printf("tag  ctrl  %6.3g%%  %9.3fpJ\n",100*tag_ctrl_area/total_area  ,tag_ctrl_e  *PICOJ);
  printf("data array %6.3g%%  %9.3fpJ\n",100*data_array_area/total_area,data_array_e*PICOJ);
  printf("data ctrl  %6.3g%%  %9.3fpJ\n",100*data_ctrl_area/total_area ,data_ctrl_e *PICOJ);
  printf("bank ctrl  %6.3g%%  %9.3fpJ\n",100*bank_ctrl_area/total_area ,bank_ctrl_e *PICOJ);
#endif

  //---------------------------------------
  // Area Readjustments based on power densities

  double factor = 15;
  // readjust areas so that no block has a power density 20x the data array (%)
  double data_array_pd = data_array_e/data_array_area;
  double data_ctrl_pd = data_ctrl_e/data_ctrl_area;
  if (data_ctrl_pd > factor*data_array_pd) {
    data_ctrl_area = data_ctrl_e/(factor*data_array_pd);
  }

  double tag_array_pd;
  double tag_ctrl_pd;
//  if (parameters->fully_assoc || parameters->ignore_tag) {
  if (parameters->fully_assoc || parameters->force_tag) {
    tag_array_pd = 0;
    tag_ctrl_pd  = 0;
  }else{
    tag_array_pd = tag_array_e/tag_array_area;
    if (tag_array_pd > factor*data_array_pd) {
      tag_array_area = tag_array_e/(factor*data_array_pd);
    }

    tag_ctrl_pd = tag_ctrl_e/tag_array_area;
    if (tag_ctrl_pd > factor*data_array_pd) {
      tag_ctrl_area = tag_ctrl_e/(factor*data_array_pd);
    }
  }

  double decode_pd = decode_e/decode_area;
  if (decode_pd > factor*data_array_pd) {
    decode_area = decode_e/(factor*data_array_pd);
  }

  double bank_ctrl_pd = bank_ctrl_e/bank_ctrl_area;
  if (bank_ctrl_pd > factor*data_array_pd) {
    bank_ctrl_area = bank_ctrl_e/(factor*data_array_pd);
  }
  if (bank_ctrl_area<0 || bank_ctrl_e*PICOJ < 1) {
    bank_ctrl_area = 0;
    bank_ctrl_e    = 0;
  }

  total_area = data_array_area+ tag_array_area + decode_area +
    data_ctrl_area + tag_ctrl_area +
    bank_ctrl_area;

  // Layout per bank
  //
  //    +------------------------------+
  //    |         bank ctrl            |
  //    +------------------------------+
  //    |  tag   | de |  data          |
  //    | array  | co |  array         |
  //    |        | de |                |
  //    +------------------------------+
  //    | tag_ctrl | data_ctrl         |
  //    +------------------------------+


  printf("\nCache bank distributions and energy per access (%d banks)\n", parameters->NSubbanks);
  printf("\tbank ctrl   %5.2g%%  %9.3fpJ\n",100*bank_ctrl_area/total_area ,bank_ctrl_e *PICOJ);
  printf("\tdecode      %5.2g%%  %9.3fpJ\n",100*decode_area/total_area    ,decode_e    *PICOJ);
  printf("\ttag  array  %5.2g%%  %9.3fpJ\n",100*tag_array_area/total_area ,tag_array_e *PICOJ);
  printf("\tdata array  %5.2g%%  %9.3fpJ\n",100*data_array_area/total_area,data_array_e*PICOJ);
  printf("\ttag  ctrl   %5.2g%%  %9.3fpJ\n",100*tag_ctrl_area/total_area  ,tag_ctrl_e  *PICOJ);
  printf("\tdata ctrl   %5.2g%%  %9.3fpJ\n",100*data_ctrl_area/total_area ,data_ctrl_e *PICOJ);

  double total_e = data_array_e+ tag_array_e + decode_e + data_ctrl_e + tag_ctrl_e + bank_ctrl_e;

  printf("\ttotal               %9.3fpJ\n",total_e *PICOJ);

  xflp->bank_ctrl_a = ar->totalarea*bank_ctrl_area/total_area;
  xflp->decode_a    = ar->totalarea*decode_area/total_area;
  xflp->tag_array_a = ar->totalarea*tag_array_area/total_area;
  xflp->data_array_a= ar->totalarea*data_array_area/total_area;
  xflp->tag_ctrl_a  = ar->totalarea*tag_ctrl_area/total_area;
  xflp->data_ctrl_a = ar->totalarea*data_ctrl_area/total_area;
  xflp->total_a     = xflp->bank_ctrl_a + xflp->decode_a +
    xflp->tag_array_a + xflp->data_array_a +
    xflp->tag_ctrl_a  + xflp->data_ctrl_a;

  xflp->bank_ctrl_e = bank_ctrl_e;
  xflp->decode_e    = decode_e;
  xflp->tag_array_e = tag_array_e;
  xflp->data_array_e= data_array_e;
  xflp->tag_ctrl_e  = tag_ctrl_e;
  xflp->data_ctrl_e = data_ctrl_e;

  xflp->NSubbanks = parameters->NSubbanks;
  xflp->assoc     = parameters->tag_associativity;
//  if (parameters->fully_assoc || parameters->ignore_tag) {
  if (parameters->fully_assoc || parameters->force_tag) {
    xflp->assoc = 1;
  }
}
