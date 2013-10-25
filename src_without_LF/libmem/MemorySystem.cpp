#include <stdio.h>
#include <math.h>

#include "SescConf.h"

#include "Cache.h"
#include "Bus.h"
#include "PriorityBus.h"
#include "MemCtrl.h"
#include "Bank.h"
#include "StridePrefetcher.h"
#include "AddressPrefetcher.h"
#include "MemoryOS.h"
#include "MemorySystem.h"

#define k_std          "std"
#define k_write_policy "writePolicy"
#define k_cache        "cache"
#define k_icache       "icache"
#define k_mvcache      "mvcache"
#define k_dsmcache     "dsmcache"
#define k_dirmvcache   "dirmvcache"
#define k_memvpred     "memvpred"
#define k_prefbuff     "prefbuff"
#define k_addrpref     "addrpref"
#define k_bus          "bus"
#define k_priobus      "prioritybus"
#define k_memctrl      "memctrl"
#define k_bank         "bank"
#define k_niceCache    "niceCache"
#define k_WB           "WB"
#define k_WT           "WT"
#define k_SV           "SV"
#define k_procsPerNode "procsPerNode"


/* MemorySystem CLASS */

MemorySystem::MemorySystem(int32_t processorId) 
  : GMemorySystem(processorId)
  ,pID(processorId)
{
  if (SescConf->isInt("",k_procsPerNode))
    procsPerNode = SescConf->getInt("",k_procsPerNode);
  else
    procsPerNode = 1;
}

GMemoryOS *MemorySystem::buildMemoryOS(const char *section)
{ 
  const char *osType = SescConf->getCharPtr(section, "OSType");

  if (strcasecmp(osType, "std") == 0)
    return new StdMemoryOS(this, section);

  return GMemorySystem::buildMemoryOS(section);
}

/* Returns new created MemoryObj or NULL in known-error mode */
MemObj *MemorySystem::buildMemoryObj(const char *device_type, 
				     const char *device_descr_section, 
				     const char *device_name)
{ 
  MemObj *new_memory_device;

  // You may insert here the further specializations you may need 
  if (!strcasecmp(device_type, k_cache) || !strcasecmp(device_type,k_icache)) {
    SescConf->isCharPtr(device_descr_section, k_write_policy);
    const char *write_policy = SescConf->getCharPtr(device_descr_section, 
						    k_write_policy);

    if(!strcasecmp(write_policy, k_WB)) {         // if it is write-back
      new_memory_device = new WBCache(this, 
				      device_descr_section, 
				      device_name);

    
    } else if (!strcasecmp(write_policy, k_WT)) { // if it is write-through
      new_memory_device = new WTCache(this, 
				      device_descr_section, 
				      device_name);

    } else if (!strcasecmp(write_policy, k_SV)) { // if it is spec vers
      new_memory_device = new SVCache(this, 
				      device_descr_section, 
				      device_name);

    } else {                                      // if it is not WB or WT
      MSG("The write policy you have specified is not valid. "\
	  "Assuming write-back.");
      new_memory_device = new WBCache(this, 
				      device_descr_section, 
				      device_name);       
    }

  } else if (!strcasecmp(device_type, k_niceCache)) { // nice cache always hits

    new_memory_device = new NICECache(this, device_descr_section, device_name);

  } else if (!strcasecmp(device_type, k_prefbuff)) {

    new_memory_device = new StridePrefetcher(this, 
					     device_descr_section, 
					     device_name);

  } else if (!strcasecmp(device_type, k_addrpref)) {

    new_memory_device = new AddressPrefetcher(this, 
					      device_descr_section, 
					      device_name);

  } else if (!strcasecmp(device_type, k_bus)) {

    new_memory_device = new Bus(this, device_descr_section, device_name);

  } else if (!strcasecmp(device_type, k_priobus)) {

    new_memory_device = new PriorityBus(this, device_descr_section, device_name);

  } else if (!strcasecmp(device_type, k_bank)) {

    new_memory_device = new Bank(this, device_descr_section, device_name);

  } else if (!strcasecmp(device_type, k_memctrl)) {

    new_memory_device = new MemCtrl(this, device_descr_section, device_name);

  } else if (!strcasecmp(device_type, k_void)) {      // For testing purposes

    return NULL; 

  } else {

    // Check the lower level because it may have it
    return GMemorySystem::buildMemoryObj(device_type, 
					 device_descr_section, 
					 device_name);
  }

  I(!new_memory_device->isHighestLevel());

  return new_memory_device;
}

#ifdef TASKSCALAR
GLVID *MemorySystem::findCreateLVID(HVersion *ver) 
{
  return &glvid;
}
#endif

/* END MemorySystem CLASS */
