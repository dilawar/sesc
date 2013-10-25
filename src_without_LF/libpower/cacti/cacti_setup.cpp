/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2005 University of California, Santa Cruz

   Contributed by Jose Renau

This file is part of SESC.

SESC is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2, or (at your option) any later version.

SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should  have received a copy of  the GNU General  Public License along with
SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <ctype.h>
#include <math.h>

//#include "nanassert.h"
//#include "Snippets.h"
//#include "SescConf.h"
#include "nanassert.h"
#include "SescConf.h"
#include "Snippets.h"

extern "C" {
#include "cacti42_areadef.h"
#include "cacti_interface.h"
#include "cacti42_def.h"
#include "cacti42_io.h"
}

#ifdef SESC_SESCTHERM
#include "ThermTrace.h"

ThermTrace  *sescTherm=0;
#endif


/*------------------------------------------------------------------------------*/
#include <vector>

static double tech;
static int res_memport;
static double wattch2cactiFactor = 1;

double getEnergy(int size
                 ,int bsize
                 ,int assoc
                 ,int rdPorts
                 ,int wrPorts
                 ,int subBanks
                 ,int useTag
                 ,int bits
                 ,xcacti_flp *xflp
                 );
double getEnergy(const char *,xcacti_flp *);


//extern "C" void output_data(result_type *result, arearesult_type *arearesult,
//                            area_type *arearesult_subbanked, 
//                            parameter_type *parameters, double *NSubbanks);
/*
extern "C" void output_data(result_type *result, 
							arearesult_type *arearesult, 
							parameter_type *parameters);
 

extern "C" total_result_type cacti_interface(
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
		int pure_sram);

extern "C" void xcacti_power_flp(const result_type *result,
                      const arearesult_type *arearesult,
                      const area_type *arearesult_subbanked,
                      const parameter_type *parameters,
                      xcacti_flp *xflp);
*/

void iterate();

int getInstQueueSize(const char* proc)
{
   // get the clusters
  int min = SescConf->getRecordMin(proc,"cluster") ;
  int max = SescConf->getRecordMax(proc,"cluster") ;
  int total = 0;
  int num = 0;
  
  for(int i = min ; i <= max ; i++){
    const char* cc = SescConf->getCharPtr(proc,"cluster",i) ;
    if(SescConf->checkInt(cc,"winSize")){
      int sz = SescConf->getInt(cc,"winSize") ;
      total += sz;
      num++;
    }
  }

  // check
  if(!num){
    fprintf(stderr,"no clusters\n");
    exit(-1);
  }

  return total/num;
}

#ifdef SESC_SESCTHERM
static void update_layout_bank(const char *blockName, xcacti_flp *xflp, const ThermTrace::FLPUnit *flp) {

  double dx;
  double dy;
  double a;
  //    +------------------------------+
  //    |         bank ctrl            |
  //    +------------------------------+
  //    |  tag   | de |  data          |
  //    | array  | co |  array         |
  //    |        | de |                |
  //    +------------------------------+
  //    | tag_ctrl | data_ctrl         |
  //    +------------------------------+

  const char *flpSec = SescConf->getCharPtr("","floorplan");
  size_t max = SescConf->getRecordMax(flpSec,"blockDescr");
  max++;
  
  char cadena[1024];

  //--------------------------------
  // Find top block bank_ctrl
  dy = flp->delta_y*(xflp->bank_ctrl_a/xflp->total_a);

  if (xflp->bank_ctrl_e) {
    // Only if bankCtrl consumes energy
    sprintf(cadena, "%sBankCtrl %g %g %g %g", blockName, flp->delta_x, dy, flp->x, flp->y);
    SescConf->updateRecord(flpSec, "blockDescr", strdup(cadena) , max);
    sprintf(cadena,"%sBankCtrlEnergy", blockName);
    SescConf->updateRecord(flpSec, "blockMatch", strdup(cadena) , max);
    max++;
  }

  double tag_start_y = dy + flp->y;
  
  //--------------------------------
  // Find lower blocks tag_ctrl and data_ctrl
  dy = flp->delta_y*((3*xflp->tag_ctrl_a+3*xflp->data_ctrl_a)/xflp->total_a);

  a  = xflp->tag_ctrl_a+xflp->data_ctrl_a;
  dx = flp->delta_x*(xflp->tag_ctrl_a/a);

  double tag_end_y = flp->y+flp->delta_y-dy;
  if (xflp->tag_array_e) {
    // Only if tag consumes energy
    sprintf(cadena,"%sTagCtrl   %g %g %g %g", blockName, dx/3, dy, flp->x+dx/3, tag_end_y);
    SescConf->updateRecord(flpSec, "blockDescr", strdup(cadena) , max);
    sprintf(cadena,"%sTagCtrlEnergy", blockName);
    SescConf->updateRecord(flpSec, "blockMatch", strdup(cadena) , max);
    max++;
  }

  sprintf(cadena,"%sDataCtrl  %g %g %g %g", blockName, (flp->delta_x-dx)/3, dy, flp->x+dx+(flp->delta_x-dx)/3, tag_end_y);
  SescConf->updateRecord(flpSec, "blockDescr", strdup(cadena) , max);
  sprintf(cadena,"%sDataCtrlEnergy", blockName);
  SescConf->updateRecord(flpSec, "blockMatch", strdup(cadena) , max);
  max++;

  //--------------------------------
  // Find middle blocks tag array, decode, and data array
  a  = xflp->tag_array_a+xflp->data_array_a+xflp->decode_a;
  dy = tag_end_y - tag_start_y;

  if (xflp->tag_array_e) {
    dx = flp->delta_x*(xflp->tag_array_a/a);
    sprintf(cadena, "%sTagArray  %g %g %g %g", blockName, dx, dy, flp->x, tag_start_y);
    SescConf->updateRecord(flpSec, "blockDescr", strdup(cadena) , max);
    sprintf(cadena,"%sTagArrayEnergy", blockName);
    SescConf->updateRecord(flpSec, "blockMatch", strdup(cadena) , max);
    max++;
  }

  double x = flp->x + dx;
  dx = flp->delta_x*((xflp->decode_a)/a);
  sprintf(cadena, "%sDecode    %g %g %g %g", blockName, dx, dy, x, tag_start_y);
  SescConf->updateRecord(flpSec, "blockDescr", strdup(cadena) , max);
  sprintf(cadena,"%sDecodeEnergy", blockName);
  SescConf->updateRecord(flpSec, "blockMatch", strdup(cadena) , max);
  max++;

  dx = flp->delta_x*((xflp->data_array_a)/a);
  sprintf(cadena, "%sDataArray %g %g %g %g", blockName, dx, dy, flp->x+flp->delta_x-dx, tag_start_y);
  SescConf->updateRecord(flpSec, "blockDescr", strdup(cadena) , max);
  sprintf(cadena,"%sDataArrayEnergy", blockName);
  SescConf->updateRecord(flpSec, "blockMatch", strdup(cadena) , max);
  max++;

}

static void update_sublayout(const char *blockName, xcacti_flp *xflp, const ThermTrace::FLPUnit *flp, int id) {

  if(id==1) {
    update_layout_bank(blockName,xflp,flp);
  }else if ((id % 2) == 0) {
    // even number
    ThermTrace::FLPUnit flp1 = *flp;
    ThermTrace::FLPUnit flp2 = *flp;
    if (flp->delta_x > flp->delta_y) {
      // x-axe is bigger
      flp1.delta_x = flp->delta_x/2;

      flp2.delta_x = flp->delta_x/2;
      flp2.x       = flp->x+flp->delta_x/2;
    }else{
      // y-axe is bigger
      flp1.delta_y = flp->delta_x/2;

      flp2.delta_y = flp->delta_y/2;
      flp2.y       = flp->y + flp->delta_y/2;
    }
    update_sublayout(blockName, xflp, &flp1, id/2);
    update_sublayout(blockName, xflp, &flp2, id/2);
  }else{
    MSG("Invalid number of banks to partition. Please use power of two");
    exit(-1);
    I(0); // In
  }
}

void update_layout(const char *blockName, xcacti_flp *xflp) {

  const ThermTrace::FLPUnit *flp = sescTherm->findBlock(blockName);
  I(flp);
  if (flp == 0) {
    MSG("Error: blockName[%s] not found in blockDescr",blockName);
    exit(-1);
    return; // no match found
  }
  
  update_sublayout(blockName, xflp, flp, xflp->NSubbanks*xflp->assoc);
}
#endif

void iterate()
{
  std::vector<char *> sections;
  std::vector<char *>::iterator it; 

  SescConf->getAllSections(sections) ;

  char line[100] ;
  for(it = sections.begin();it != sections.end(); it++) {
    const char *block = *it;

    if (!SescConf->checkCharPtr(block,"deviceType")) 
      continue;

    const char *name = SescConf->getCharPtr(block,"deviceType") ;

    if(strcasecmp(name,"vbus")==0){
      SescConf->updateRecord(block,"busEnergy",0.0) ; // FIXME: compute BUS energy
    }else if (strcasecmp(name,"niceCache") == 0) {
      // No energy for ideal caches (DRAM bank)
      SescConf->updateRecord(block, "RdHitEnergy"   ,0.0);
      SescConf->updateRecord(block, "RdMissEnergy"  ,0.0);
      SescConf->updateRecord(block, "WrHitEnergy"   ,0.0);
      SescConf->updateRecord(block, "WrMissEnergy"  ,0.0);
      
    }else if(strstr(name,"cache") 
             || strstr(name,"tlb")
             || strstr(name,"mem")
             || strstr(name,"dir") 
             || !strcmp(name,"revLVIDTable") ) {

      xcacti_flp xflp;
      double eng = getEnergy(block, &xflp);

#ifdef SESC_SESCTHERM2
      if (SescConf->checkCharPtr(block,"blockName")) {
        const char *blockName = SescConf->getCharPtr(block,"blockName");
        MSG("%s (block=%s) has blockName %s",name, block, blockName);
        update_layout(blockName, &xflp);
      }
#else
      // write it
      SescConf->updateRecord(block, "RdHitEnergy"   ,eng);
      SescConf->updateRecord(block, "RdMissEnergy"  ,eng * 2); // Rd miss + lineFill
      SescConf->updateRecord(block, "WrHitEnergy"   ,eng);
      SescConf->updateRecord(block, "WrMissEnergy"  ,eng * 2); // Wr miss + lineFill
#endif
    }
  }
}

char * strfy(int v){
  char *t = new char[10] ;
  sprintf(t,"%d",v);
  return t ;
}
char *strfy(double v){
  char *t = new char[10] ;
  sprintf(t,"%lf",v);
  return t ;
}


double getEnergy(int size
                 ,int bsize
                 ,int assoc
                 ,int rdPorts
                 ,int wrPorts
                 ,int subBanks
                 ,int useTag
                 ,int bits
                 ,xcacti_flp *xflp
                 ) {
  int nsets = size/(bsize*assoc);
  int fully_assoc, associativity;
  int rwPorts = 0;
  double ret;

  if (nsets == 0) {
    printf("Invalid cache parameters size[%d], bsize[%d], assoc[%d]\n", size, bsize, assoc);
    exit(0);
  }
  if (subBanks == 0) {
    printf("Invalid cache subbanks parameters\n");
    exit(0);
  }

  if ((size/subBanks)<64) {
    printf("size %d: subBanks %d: assoc %d : nset %d\n",size,subBanks,assoc,nsets);
    size =64*subBanks;
  }

  if (rdPorts>1) {
    wrPorts = rdPorts-2;
    rdPorts = 2;
  }
  if ((rdPorts+wrPorts+rwPorts) < 1) {
    rdPorts = 0;
    wrPorts = 0;
    rwPorts = 1;
  }
  
  if (bsize*8 < bits)
    bsize = bits/8;

  BITOUT = bits;

  if (size == bsize * assoc) {
    fully_assoc = 1;
  }else{
    fully_assoc = 0;
  }

  size = roundUpPower2(size);

  if (fully_assoc) {
    associativity = size/bsize;
  }else{
    associativity = assoc;
  }

  if (associativity >= 32)
    associativity = size/bsize;

  nsets = size/(bsize*associativity);

  total_result_type result2;

  printf("\n\n\n########################################################");
  printf("\nInput to Cacti_Interface...");
  printf("\n size = %d, bsize = %d, assoc = %d, rports = %d, wports = %d", 
	 size, bsize, associativity, rdPorts, wrPorts);
  printf("\n subBanks = %d, tech = %f, bits = %d", 
	 subBanks, tech, bits);
  
  result2 = cacti_interface(size, bsize, associativity, 
			    rwPorts, rdPorts, wrPorts, 
			    0, subBanks, tech, bits, 
			    0, 0,  // custom tag
			    0, 
			    useTag);

#ifdef DEBUG
  //output_data(&result,&arearesult,&arearesult_subbanked,&parameters);
  output_data(&result2.result,&result2.area,&result2.params);
#endif

  //xcacti_power_flp(&result,&arearesult,&arearesult_subbanked,&parameters, xflp);
  xcacti_power_flp(&result2.result, &result2.area, &result2.arearesult_subbanked,
	               &result2.params, xflp);

  //return wattch2cactiFactor * 1e9*(result.total_power_without_routing/subBanks + result.total_routing_power);
  return wattch2cactiFactor * 1e9*(result2.result.total_power_without_routing.readOp.dynamic / subBanks + 
	  result2.result.total_routing_power.readOp.dynamic);
}


double getEnergy(const char *section, xcacti_flp *xflp) {
  // set the input
  int cache_size = SescConf->getInt(section,"size") ;
  int block_size = SescConf->getInt(section,"bsize") ;
  int assoc = SescConf->getInt(section,"assoc") ;
  int write_ports = 0 ;
  int read_ports = SescConf->getInt(section,"numPorts");
  int readwrite_ports = 1;
  int subbanks = 1;
  int bits = 32;

  if(SescConf->checkInt(section,"subBanks"))
    subbanks = SescConf->getInt(section,"subBanks");

  if(SescConf->checkInt(section,"bits"))
    bits = SescConf->getInt(section,"bits");

  printf("Module [%s]...\n", section);

  return getEnergy(cache_size
                   ,block_size
                   ,assoc
                   ,read_ports
                   ,readwrite_ports
                   ,subbanks
                   ,1
                   ,bits
                   ,xflp);

}

void processBranch(const char *proc)
{
  // FIXME: add thermal block to branch predictor

  // get the branch
  const char* bpred = SescConf->getCharPtr(proc,"bpred") ;

  // get the type
  const char* type = SescConf->getCharPtr(bpred,"type") ;

  xcacti_flp xflp;

  double bpred_power=0;
  // switch based on the type
  if(!strcmp(type,"Taken") || 
     !strcmp(type,"Oracle") || 
     !strcmp(type,"NotTaken") || 
     !strcmp(type,"Static")) {
    // No tables
    bpred_power= 0;
  }else if(!strcmp(type,"2bit")) {
	 int size = SescConf->getInt(bpred,"size") ;

    // 32 = 8bytes line * 4 predictions per byte (assume 2bit counter)
    bpred_power = getEnergy(size/32, 8, 1, 1, 0, 1, 0, 8, &xflp);
    bpred_power= 0;

  }else if(!strcmp(type,"2level")) {
	 int size = SescConf->getInt(bpred,"l2size") ;

    // 32 = 8bytes line * 4 predictions per byte (assume 2bit counter)
    bpred_power = getEnergy(size/32, 8, 1, 1, 0, 1, 0, 8, &xflp);

  }else if(!strcmp(type,"ogehl")) {
    int mTables = SescConf->getInt(bpred,"mtables") ;
    int size    = SescConf->getInt(bpred,"tsize") ;

    I(0);
    // 32 = 8bytes line * 4 predictions per byte (assume 2bit counter)
    bpred_power = getEnergy(size/32, 8, 1, 1, 0, 1, 0, 8, &xflp) * mTables;

  }else if(!strcmp(type,"hybrid")) {
	 int size = SescConf->getInt(bpred,"localSize") ;

    // 32 = 8bytes line * 4 predictions per byte (assume 2bit counter)
    bpred_power = getEnergy(size/32, 8, 1, 1, 0, 1, 0, 8, &xflp);
#ifdef SESC_SESCTHERM2
    // FIXME: update layout
#endif
    size = SescConf->getInt(bpred,"metaSize");

    // 32 = 8bytes line * 4 predictions per byte (assume 2bit counter)
    bpred_power += getEnergy(size/32, 8, 1, 1, 0, 1, 0, 8, &xflp);

  }else{
    MSG("Unknown energy for branch predictor type [%s]", type);
    exit(-1);
  }

#ifdef SESC_SESCTHERM2
  // FIXME: partition energies per structure
  const char *bpredBlockName = SescConf->getCharPtr(bpred, "blockName");
  update_layout(bpredBlockName, &xflp);
#else
  SescConf->updateRecord(proc,"bpredEnergy",bpred_power) ;
#endif

  int btbSize  = SescConf->getInt(bpred,"btbSize");
  int btbAssoc = SescConf->getInt(bpred,"btbAssoc");
  double btb_power = 0;

  if (btbSize) {
    btb_power = getEnergy(btbSize*8, 8, btbAssoc, 1, 0, 1, 1, 64, &xflp);
#ifdef SESC_SESCTHERM2
    // FIXME: partition energies per structure
    update_layout(bpredBlockName, &xflp);
#else
    SescConf->updateRecord(proc,"btbEnergy",btb_power) ;
#endif
  }else{
    SescConf->updateRecord(proc,"btbEnergy",0.0) ;
  }

  double ras_power =0;
  int ras_size = SescConf->getInt(bpred,"rasSize");
  if (ras_size) {
    ras_power = getEnergy(ras_size*8, 8, 1, 1, 0, 1, 0, 64, &xflp);
#ifdef SESC_SESCTHERM2
    // FIXME: partition energies per structure (all bpred may share a block)
    update_layout(bpredBlockName, &xflp);
#else
    SescConf->updateRecord(proc,"rasEnergy",ras_power) ;
#endif
  }else{
    SescConf->updateRecord(proc,"rasEnergy",0.0) ;
  }

}

void processorCore()
{
  const char *proc = SescConf->getCharPtr("","cpucore",0) ;
  fprintf(stderr,"proc = [%s]\n",proc);

  xcacti_flp xflp;

  //----------------------------------------------
  // Branch Predictor
  processBranch(proc);
  
  //----------------------------------------------
  // Register File
  int issueWidth= SescConf->getInt(proc,"issueWidth");
  int size    = SescConf->getInt(proc,"intRegs");
  int banks   = 1; 
  int rdPorts = 2*issueWidth;
  int wrPorts = issueWidth;
  int bits = 32;
  int bytes = 8;

  if(SescConf->checkInt(proc,"bits")) {
    bits = SescConf->getInt(proc,"bits");
    bytes = bits/8;
    if (bits*8 != bytes) {
      fprintf(stderr,"Not valid number of bits for the processor core [%d]\n",bits);
      exit(-2);
    }
  }

  if(SescConf->checkInt(proc,"intRegBanks"))
    banks = SescConf->getInt(proc,"intRegBanks");

  if(SescConf->checkInt(proc,"intRegRdPorts"))
    rdPorts = SescConf->getInt(proc,"intRegRdPorts");

  if(SescConf->checkInt(proc,"intRegWrPorts"))
    wrPorts = SescConf->getInt(proc,"intRegWrPorts");

  double regEnergy = getEnergy(size*bytes, bytes, 1, rdPorts, wrPorts, banks, 0, bits, &xflp);

  printf("\nRegister [%d bytes] banks[%d] ports[%d] Energy[%g]\n"
         ,size*bytes, banks, rdPorts+wrPorts, regEnergy);

#ifdef SESC_SESCTHERM2
  const char *blockName = SescConf->getCharPtr(proc,"IntRegBlockName");
  I(blockName);
  update_layout(blockName, &xflp);
  blockName = SescConf->getCharPtr(proc,"fpRegBlockName");
  I(blockName);
  update_layout(blockName , &xflp); // FIXME: different energy for FP register
#else
  SescConf->updateRecord(proc,"wrRegEnergy",regEnergy);
  SescConf->updateRecord(proc,"rdRegEnergy",regEnergy);
#endif


  //----------------------------------------------
  // Load/Store Queue
  size      = SescConf->getInt(proc,"maxLoads");
  banks     = 1; 
  rdPorts   = res_memport;
  wrPorts   = res_memport;

  if(SescConf->checkInt(proc,"lsqBanks"))
    banks = SescConf->getInt(proc,"lsqBanks");

  regEnergy = getEnergy(size*2*bytes,2*bytes,size,rdPorts,wrPorts,banks,1, 2*bits, &xflp);
#ifdef SESC_SESCTHERM2
  // FIXME: partition energies per structure
  blockName = SescConf->getCharPtr(proc,"LSQBlockName");
  I(blockName);
  update_layout(lsqBlockName, &xflp);
#else
  SescConf->updateRecord(proc,"ldqRdWrEnergy",regEnergy);
#endif
  printf("\nLoad Queue [%d bytes] banks[%d] ports[%d] Energy[%g]\n"
         ,size*2*bytes, banks, 2*res_memport, regEnergy);

  size      =  SescConf->getInt(proc,"maxStores");
 
  regEnergy = getEnergy(size*4*bytes,4*bytes,size,rdPorts,wrPorts,banks,1, 2*bits, &xflp);
#ifdef SESC_SESCTHERM2
  // FIXME: partition energies per structure
  blockName = SescConf->getCharPtr(proc,"LSQBlockName");
  I(blockName);
  update_layout(lsqBlockName, &xflp);
#else
  SescConf->updateRecord(proc,"stqRdWrEnergy",regEnergy);
#endif
  printf("\nStore Queue [%d bytes] banks[%d] ports[%d] Energy[%g]\n"
         ,size*4*bytes, banks, 2*res_memport, regEnergy);


#ifdef SESC_INORDER 
  size      =  size/4;
 
  regEnergy = getEnergy(size*4*bytes,4*bytes,size,rdPorts,wrPorts,banks,1, 2*bits, &xflp);

  printf("\nStore Inorder Queue [%d bytes] banks[%d] ports[%d] Energy[%g]\n"
         ,size*4*bytes, banks, 2*res_memport, regEnergy);

  SescConf->updateRecord(proc,"stqRdWrEnergyInOrder",regEnergy);

#ifdef SESC_SESCTHERM
  I(0);
  exit(-1); // Not supported
#endif

 #endif 
 
  //----------------------------------------------
  // Reorder Buffer
  size      = SescConf->getInt(proc,"robSize");
  banks     = size/64;
  if (banks == 0) {
    banks = 1;
  }else{
    banks = roundUpPower2(banks);
  }
  
  // Retirement should hit another bank
  rdPorts   = 1; // continuous possitions
  wrPorts   = 1;

  regEnergy = getEnergy(size*2,2*issueWidth,1,rdPorts,wrPorts,banks,0,16*issueWidth, &xflp);
#ifdef SESC_SESCTHERM2
  const char *blockName = SescConf->getCharPtr(proc,"robBlockName");
  I(blockName);
  update_layout(blockName, &xflp);
  // FIXME: partition energies per structure
#else
  SescConf->updateRecord(proc,"robEnergy",regEnergy);
#endif

  printf("\nROB [%d bytes] banks[%d] ports[%d] Energy[%g]\n",size*2, banks, 2*rdPorts, regEnergy);

  //----------------------------------------------
  // Rename Table
  {
    double bitsPerEntry = log(SescConf->getInt(proc,"intRegs"))/log(2);
    
    size      = roundUpPower2(static_cast<unsigned int>(32*bitsPerEntry/8));
    banks     = 1;
    rdPorts   = 2*issueWidth;
    wrPorts   = issueWidth;

    regEnergy = getEnergy(size,1,1,rdPorts,wrPorts,banks,0,1, &xflp);
#ifdef SESC_SESCTHERM2
    update_layout("IntRAT", &xflp); //FIXME: create a IntRATblockName
    // FIXME: partition energies per structure
#endif

    printf("\nrename [%d bytes] banks[%d] Energy[%g]\n",size, banks, regEnergy);

    regEnergy += getEnergy(size,1,1,rdPorts/2+1,wrPorts/2+1,banks,0,1, &xflp);
#ifdef SESC_SESCTHERM2
    update_layout("IntRAT", &xflp);
    // FIXME: partition energies per structure
#else
    // unified FP+Int RAT energy counter
    SescConf->updateRecord(proc,"renameEnergy",regEnergy);
#endif
  }

  //----------------------------------------------
  // Window Energy & Window + DDIS

  {
    int min = SescConf->getRecordMin(proc,"cluster") ;
    int max = SescConf->getRecordMax(proc,"cluster") ;
    I(min==0);

    for(int i = min ; i <= max ; i++) {
      const char *cluster = SescConf->getCharPtr(proc,"cluster",i) ;

      // TRADITIONAL COLLAPSING ISSUE LOGIC
      // Recalculate windowRdWrEnergy using CACTI (keep select and wake)
      
      size      = SescConf->getInt(cluster,"winSize");
      banks     = 1;
      rdPorts   = SescConf->getInt(cluster,"wakeUpNumPorts");
      wrPorts   = issueWidth;
      int robSize          = SescConf->getInt(proc,"robSize");
      float entryBits = 4*(log(robSize)/log(2)); // src1, src2, dest, instID
      entryBits += 7; // opcode
      entryBits += 1; // ready bit
      
      int tableBits = static_cast<int>(entryBits * size);
      int tableBytes;
      if (tableBits < 8) {
	tableBits  = 8;
	tableBytes = 1;
      }else{
	tableBytes = tableBits/8;
      }
      int assoc= roundUpPower2(static_cast<unsigned int>(entryBits/8));
      tableBytes = roundUpPower2(tableBytes);
      regEnergy = getEnergy(tableBytes,tableBytes/assoc,assoc,rdPorts,wrPorts,banks,1,static_cast<int>(entryBits), &xflp);
      
      printf("\nWindow [%d bytes] assoc[%d] banks[%d] ports[%d] Energy[%g]\n"
	     ,tableBytes, assoc, banks, rdPorts+wrPorts, regEnergy);
      
#ifdef SESC_SESCTHERM2
      const char *blockName = SescConf->getCharPtr(cluster,"blockName");
      I(blockName);
      update_layout(blockName, &xflp);
#else
      // unified FP+Int RAT energy counter
      SescConf->updateRecord(cluster,"windowRdWrEnergy" ,regEnergy,0);
#endif
    }
  }
}

void cacti_setup()
{
  const char *technology = SescConf->getCharPtr("","technology");
  fprintf(stderr,"technology = [%s]\n",technology);
  tech = SescConf->getInt(technology,"tech");
  fprintf(stderr, "tech : %9.0fnm\n" , tech);
  tech /= 1000;

#ifdef SESC_SESCTHERM
  sescTherm = new ThermTrace(0); // No input trace, just read conf
#endif

  const char *proc    = SescConf->getCharPtr("","cpucore",0);
  const char *l1Cache = SescConf->getCharPtr(proc,"dataSource");

  const char *l1CacheSpace = strstr(l1Cache," ");
  char *l1Section = strdup(l1Cache);
  if (l1CacheSpace)
    l1Section[l1CacheSpace - l1Cache] = 0;

  res_memport = SescConf->getInt(l1Section,"numPorts");

  xcacti_flp xflp;
  double l1Energy = getEnergy(l1Section, &xflp);

  double WattchL1Energy = SescConf->getDouble("","wattchDataCacheEnergy");

  if (WattchL1Energy) {
    wattch2cactiFactor = WattchL1Energy/l1Energy;
    fprintf(stderr,"wattch2cacti Factor %g\n", wattch2cactiFactor);
  }else{
    fprintf(stderr,"-----WARNING: No wattch correction factor\n");
  }

  processorCore();

  iterate();
}
