//----------------------------------------------------------------------------
// File: sescspot.cpp
//
// Description: Processes therm files created by SESC to report 
//              temperature data
// Authors    : David van der Bokke and Andrew Hill 
//
// Patch #1 (Javi): Read values from sesc.conf file


#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>

#include "SescConf.h"
#include "ThermTrace.h"

#include "flp.h"
#include "temperature.h"
#include "temperature_block.h"
#include "temperature_grid.h"
#include "util.h"
using namespace std;


// simulator options
static char *flp_cfg = "amd.flp";		/* has the floorplan configuration	*/
RC_model_t *model = 0;
static double *temp;
static double *power;
static flp_t *flp;

/* file defs */

char *infile;
char *outfile;
char *hotfile;
char *conffile = "power.conf";

FILE *p_outfile;

float cycles;  // cycle divider(10000)
float frequency;
float curcycle;	// cycle we are on

//----------------------------------------------------------------------------
// dump_tempAppend1 : Dumps values to the output file
//

void dump_tempAppend1(flp_t *flp, double *temp, char *file) {

  FILE *fp = fopen (file, "a");
  if (fp==0) {
    MSG("Error: %s could not be opened for writing\n", file);
    exit(4);
  }

  fprintf(fp, "\n");    
  fprintf(fp, "%-12d", (int)curcycle);

  // on chip temperatures	
  for (int32_t i=0; i < flp->n_units; i++)
    fprintf(fp, "%-12f", temp[i]-273.15);

  fclose(fp);   
}


//----------------------------------------------------------------------------
// dump_tempTitles : Dumps column names to file
//

void dump_tempTitles(flp_t *flp, char *file) {

  char str[STR_SIZE];
  FILE *fp = fopen(file, "w");

  fprintf(fp, "%-12s", "Cycle");

  for (int32_t i = 0; i < flp->n_units; i++) {
    fprintf(fp, "%-12s", flp->units[i].name);
  }

  fclose(fp);
}


//Parser the configuration file
static thermal_config_t thermal_config;
void parseConfigFile() {

  // Read initialization parameters
  const char *modelSec = SescConf->getCharPtr("thermal","model");

  // SESCSpot specific parameters
  thermal_config = default_thermal_config();

  thermal_config.thermal_threshold = 111.8 + 273.15;	/* temperature threshold for DTM (Kelvin)*/

  thermal_config.init_temp = SescConf->getDouble(modelSec,"InitialTemp") + 273.15;
  thermal_config.ambient   = SescConf->getDouble(modelSec,"ambientTemp") + 273.15;

  thermal_config.t_chip    = SescConf->getDouble("thermSpot","ChipThickness");

  thermal_config.c_convec  = SescConf->getDouble("thermSpot","ConvectionCapacitance");
  thermal_config.r_convec  = SescConf->getDouble("thermSpot","ConvectionResistance");

  thermal_config.s_sink    = SescConf->getDouble("thermSpot","HeatsinkLength");
  thermal_config.t_sink    = SescConf->getDouble("thermSpot","HeatsinkThinkness");

  thermal_config.s_spreader= SescConf->getDouble("thermSpot","SpreaderLength");
  thermal_config.t_spreader= SescConf->getDouble("thermSpot","SpreaderThickness");

  thermal_config.t_interface = SescConf->getDouble("thermSpot","InterfaceMaterialThickness");

  thermal_config.block_omit_lateral = 0;			/* omit lateral chip resistances?	*/
  
#if 1
  /* set grid model as default	*/
  thermal_config.grid_rows = 20;				/* grid resolution - no. of rows	*/
  thermal_config.grid_cols = 20;				/* grid resolution - no. of cols	*/
  strcpy(thermal_config.model_type, GRID_MODEL_STR);
#endif

  SescConf->lock();
}

int32_t process_trace(const char *input_file
		  ,double power_timestep
		  ,int32_t power_samples_per_therm_sample
		  ,bool rabbit
		  ,int32_t cyclesPerSample
		  ) {

  int32_t num_computations = 0;
  // ----------------------------------------------------------------
  // READ trace
  ThermTrace trace(input_file);
  
  static int32_t pending_power_samples_from_prev_trace = 0;
  static int32_t samples=0;

  while(trace.read_energy()) {
    //-------------------------------------
    // Advance Power
    if (pending_power_samples_from_prev_trace<=0) {
      for(size_t j=0;j<trace.get_energy_size();j++) {
	power[j] = trace.get_energy(j);
      }
      
      samples = 1;
      pending_power_samples_from_prev_trace = power_samples_per_therm_sample-1;
    }
    
    while(pending_power_samples_from_prev_trace > 0 || rabbit) {
      if (!trace.read_energy())
	break;
      
      for(size_t j=0;j<trace.get_energy_size();j++) {
	power[j] += trace.get_energy(j);
      }
      
      samples++;
      pending_power_samples_from_prev_trace--;
      
      if (rabbit && samples > 20*power_samples_per_therm_sample) {
	pending_power_samples_from_prev_trace = 0;
	break;
      }
    }
    
    for(size_t j=0;j<trace.get_energy_size();j++) {
      power[j] = power[j]/samples;
    }
    
    //-------------------------------------
    // Advance Thermal
    
    static double cur_time = 0;
    double ts = power_timestep*samples;
    compute_temp(model, power, temp, ts);
    cur_time += ts;
    
    num_computations++;
    
    for(size_t j=0;j<trace.get_energy_size();j++) {
      power[j] = 0;
    }
    samples = 0;
    
    //-------------
    static double print_at = 0;
    if (cur_time > print_at ) {
      print_at = cur_time + 0.001; // every 1ms
      
      if (rabbit)
	cout << "R " << cur_time << " ";
      else
	cout << "T " << cur_time << " ";
      
      for (int32_t i=0; i < flp->n_units; i++)
	cout << temp[i] - 273.15 << "\t";
    
      cout << endl;
    }
  }
    
  return num_computations;
}

//----------------------------------------------------------------------------
// sim_init : Initialise temperature/power statistics
//    


void sim_init() {
  parseConfigFile();

  flp = read_flp(flp_cfg, 0);
  I(model==0);

  model = alloc_RC_model(&thermal_config, flp);
  populate_R_model(model, flp);
  populate_C_model(model, flp);

  // allocate the temp and power arrays	
  temp  = hotspot_vector(model);
  power = hotspot_vector(model);

  set_temp(model, temp, thermal_config.init_temp);
}

void showUsage() 
{
  printf("SESCSpot 3.1 Usage Summary:\n");
  printf("Arguments:\n");
  printf("\t-h <--hotfile>   HotSpot Temp Trace  - file that can be used by HotSpot3\n");
  printf("\t-i <--infile>    Input file          - The therm file for which you want temperature data\n");
  printf("\t-o <--outfile>   Output file         - Output file name for either steady state or transient temperature\n");
  printf("\t-c <--conffile>  Configuration file) - This is the mappings from floorplan variables to sesc variables\n");
  printf("\t-f <--floorplan> Floorplan file      - Set to the floorplan you wish to use.  Default is 'ev6.flp'\n");
}

int32_t main(int32_t argc, char **argv)
{
  int32_t i, n, num,x, longindex, lines = 0;
  int32_t opt;
  //FILE *pin, *tout;	/* trace file pointers	*/
  char names[MAX_UNITS][STR_SIZE];
  double vals[MAX_UNITS];
  char linen[4000], *src;
  char name[100];
  char c;

  curcycle = 0;

  std::string tmpstring;

  static struct option long_options[] =
    { 
      {"infile", required_argument, 0, 'i'},
      {"outfile", required_argument, 0, 'o'},
      {"configfile", required_argument, 0, 'c'},
      {"floorplan", required_argument, 0, 'f'},
      {0,0,0,0}
    };
  // process parameters
  while ((opt = getopt_long(argc, argv, "h:c:f:i:o:", long_options, &longindex)) != -1) {
    switch (opt) {
    case 'h':
      hotfile = optarg;
      break;
    case 'c':
      conffile = optarg;
      break;
    case 'f':
      flp_cfg = optarg;
      break;
    case 'i':
      infile = optarg;
      break;
    case 'o':
      outfile = optarg;
      break;
    default:
      showUsage();
      exit(1);
      break;
    }
  }

  cerr << "1.Booting sescspot" << endl;

  if ((conffile==NULL || infile=="")) {
    showUsage();
    exit(1);
  }

  if (outfile)
    if (!(p_outfile = fopen(outfile, "w")))
      fatal("Unable to open output file\n");

  cerr << "1.Booting sescspot" << endl;

  SescConf = new SConfig(conffile);

  const char *modelSec= SescConf->getCharPtr("thermal","model");

  int32_t cyclesPerSample = SescConf->getInt(modelSec,"CyclesPerSample");
  double frequency    = SescConf->getDouble("technology","Frequency");

  int32_t power_samples_per_therm_sample = SescConf->getInt(modelSec,"PowerSamplesPerThermSample");
  double power_timestep = cyclesPerSample/frequency;
  double time_step = power_timestep * power_samples_per_therm_sample;

  cout << "USED MODEL TIMESTEP: " << time_step << endl;

  clock_t start, end;
	
  cout << "STARTING SESCSPOT" << endl;
  start = clock();
  sim_init();

  printf("Chip area %gx%gmm^2\n", 1e3*get_total_width(flp), 1e3*get_total_height(flp));
  printf("AmbientTemp %gC\n",thermal_config.ambient-273.15);

#if 0
  printf("%-20s\t%-15s\n","Unit","Steady-State Temperatures Without Warmup");

  for (int32_t i=0; i < flp->n_units; i++)
    printf("%-20s\t%.2f\n", flp->units[i].name, temp[i]-273.15);
#endif

  FILE *p_hotfile=0;
  if (hotfile) {
    if (!(p_hotfile = fopen(hotfile, "w")))
      fatal("Unable to open hotfile file\n");

    for (int32_t i=0; i < flp->n_units; i++)
      fprintf(p_hotfile,"%-20s ", flp->units[i].name);
    fprintf(p_hotfile,"\n");
    // FIXME: read hot file
  }else{
    dump_tempTitles(flp, outfile);
  }

  end = clock();

  cout << "MODEL INITIALIZED" << endl;
  double elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
  cout << "MODEL INITIALIZATION TIME IS " << elapsed << "SECONDS" << endl;

  double total;
  int32_t num_computations = 0;

  if (argc>7) {
    num_computations = 0;
    
    for(int32_t i=7;i<argc;i++) {
      cout << "Processing trace " << argv[i] << endl;
      start = clock();
      
      num_computations += process_trace(argv[i]
					,power_timestep
					,power_samples_per_therm_sample
					,false
					,cyclesPerSample
					);
      
      end = clock();
      elapsed+= ((double) (end - start)) / CLOCKS_PER_SEC;
    }
  }

  elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
  cout << "COMPUTATION TIME FOR " << num_computations << " ITERATIONS IS " << elapsed << "  (" << elapsed/num_computations << "SECONDS PER ITERATION )" << endl;
  
  return 0;
}

