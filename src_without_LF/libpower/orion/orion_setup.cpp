using namespace std ;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <iostream>

#include "SIM_power.h"
#include "SIM_power_router.h"
#include "SIM_router_power.h"
#include "SIM_reg_power.h"
#include "orresult.h"
#include "orion_setup.h"

void orion_setup(SConfig* SescConf)
{
  void iterate(SConfig* SescConf);

  /* router parameters */
  FUNC(SIM_router_power_init, &GLOB(router_info), &GLOB(router_power));

  char path[2048] ;
  strcpy(path,"myrouter") ;
  orResult orr =  SIM_router_stat_energy(&GLOB(router_info), &GLOB(router_power), 0, path, MAX_ENERGY, 1.0, 1, PARM(Freq));

  SescConf->updateRecord("netEnergy","routerEnergy",orr.totEnergy);
 
  /* bus parameters */
  iterate(SescConf);

  /* buffer parameters */
  double e_cbuf_rw = 1.0 * ((&GLOB(router_info))->n_total_in) * ((&GLOB(router_info))->flit_width) / ((&GLOB(router_info))->central_buf_info.blk_bits);
  double val = SIM_reg_stat_energy(&((&GLOB(router_info))->central_buf_info),&((&GLOB(router_power))->central_buf),e_cbuf_rw,e_cbuf_rw,2,path,MAX_ENERGY);
  
  val = val * 1e9;
  SescConf->updateRecord("netEnergy","bufferEnergy",val);
}

void sttol(const char *src, char*dest){
  dest[0] = '\0' ;
  for(unsigned int i = 0 ; i <= strlen(src); i++) dest[i] = tolower(src[i]) ;
}

void iterate(SConfig* SescConf)
{
  double getEnergy(SConfig*, const char*) ;
  void sttol(const char* src, char* dest);

  vector<char *> vv ;
  SescConf->getAllSections(vv) ;

  vector<char *>::iterator it; 
  char line[100] ;
  for(it = vv.begin();it != vv.end();it++){
    char *block = *it ;

    if (!SescConf->checkCharPtr(block,"deviceType")) 
      continue;

    const char *name = SescConf->getCharPtr(block,"deviceType") ;
    
    sttol(name, line);
    if(!strcmp(line,"vbus")){ 
      double eng = getEnergy(SescConf,block) ;

      // write it
      SescConf->updateRecord(block,"busEnergy",eng) ;
    }
  }
}

double getEnergy(SConfig *SescConf, const char* section)
{
  int numPorts = SescConf->getInt(section,"numPorts");
  double busLength = (double) SescConf->getInt(section,"busLength");
  int busWidth = SescConf->getInt(section,"busWidth");
  double delay = (double) SescConf->getInt(section,"delay");
  double fac = 1.0;

  SIM_power_bus_t bt ;
  if(busWidth > 64) {
    fac = ((double) busWidth)/64.0;
    busWidth = 64;
  }
  SIM_bus_init(&bt,GENERIC_BUS, IDENT_ENC,busWidth,0,1,1,busLength,0) ;
  double benergy = numPorts * numPorts * (bt.e_switch*bt.bit_width*1e9)/2.0 * fac;
  return benergy/delay;
}
