/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Karin Strauss

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

#include "TMSystemBus.h"
#include "TMMemorySystem.h"
#include "TMCache.h"
#include "TMDebug.h"
#include <vector>
#include "Epoch.h"

using namespace std;
SMPSystemBus::SMPSystemBus(SMemorySystem *dms, const char *section, const char *name)
  : MemObj(section, name)
  ,l2DataProv("%s:Number of line providers",name)
#ifdef TLS
,VCRMaxCount("%s:Max VCR used",name)
#endif
{
  MemObj *lowerLevel = NULL;

  I(dms);
  lowerLevel = dms->declareMemoryObj(section, "lowerLevel");

  if (lowerLevel != NULL)
    addLowerLevel(lowerLevel);

  SescConf->isInt(section, "numPorts");
  SescConf->isInt(section, "portOccp");
  SescConf->isInt(section, "delay");
  
  delay = SescConf->getInt(section, "delay");


#ifdef TLS
  busDelay= SescConf->getInt("TLS","busaccessdelay");
  VCRCount=0;
#endif
  char portName[100];
  sprintf(portName, "%s_bus", name);

  busPort = PortGeneric::create(portName, 
				SescConf->getInt(section, "numPorts"), 
				SescConf->getInt(section, "portOccp"));
}

SMPSystemBus::~SMPSystemBus() 
{
  // do nothing
}

Time_t SMPSystemBus::getNextFreeCycle() const
{
  return busPort->nextSlot();
}

Time_t SMPSystemBus::nextSlot(MemRequest *mreq)
{
  return getNextFreeCycle();
}

bool SMPSystemBus::canAcceptStore(PAddr addr) const
{
  return true;
}

void SMPSystemBus::access(MemRequest *mreq)
{
  GMSG(mreq->getPAddr() < 1024,
       "mreq dinst=0x%p paddr=0x%x vaddr=0x%x memOp=%d",
       mreq->getDInst(),
       (uint32_t) mreq->getPAddr(),
       (uint32_t) mreq->getVaddr(),
       mreq->getMemOperation());
  
  I(mreq->getPAddr() > 1024); 

  switch(mreq->getMemOperation()){
  case MemRead:     read(mreq);      break;
  case MemReadW:    
  case MemWrite:    write(mreq);     break;
  case MemPush:     push(mreq);      break;
  default:          specialOp(mreq); break;
  }

  // for reqs coming from upper level:
  // MemRead means I need to read the data, but I don't have it
  // MemReadW means I need to write the data, but I don't have it
  // MemWrite means I need to write the data, but I don't have permission
  // MemPush means I don't have space to keep the data, send it to memory
}

void SMPSystemBus::read(MemRequest *mreq)
{
  if(pendReqsTable.find(mreq) == pendReqsTable.end()) {
    doReadCB::scheduleAbs(nextSlot(mreq)+delay, this, mreq);
  } else {
    doRead(mreq);
  }
}

void SMPSystemBus::write(MemRequest *mreq)
{
  SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);
  if(pendReqsTable.find(mreq) == pendReqsTable.end()) {
    doWriteCB::scheduleAbs(nextSlot(mreq)+delay, this, mreq);
  } else {
    doWrite(mreq);
  }
}

void SMPSystemBus::push(MemRequest *mreq)
{
  doPushCB::scheduleAbs(nextSlot(mreq)+delay, this, mreq);  
}

void SMPSystemBus::specialOp(MemRequest *mreq)
{
  I(0);
}

void SMPSystemBus::doRead(MemRequest *mreq)
{
  SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);
    // no need to snoop, go straight to memory
  if(!sreq->needsSnoop()) {
    goToMem(mreq);
    return;
  }
  if(pendReqsTable.find(mreq) == pendReqsTable.end()) {

    unsigned numSnoops = getNumSnoopCaches(sreq);

    // operation is starting now, add it to the pending requests buffer
    pendReqsTable[mreq] = getNumSnoopCaches(sreq);

    if(!numSnoops) { 
      // nothing to snoop on this chip
      finalizeRead(mreq);
      return;
      // TODO: even if there is only one processor on each chip, 
      // request is doing two rounds: snoop and memory
    }
	// VERSION COMBINING REGISTER IS ALLOCATED HERE
	#ifdef TLS
		if (mreq->isDataReq() && sreq->needsData())VCRCount++;
	#endif
    // distribute requests to other caches, wait for responses
	 for(uint32_t i = 0; i < upperLevel.size(); i++) {
      if(upperLevel[i] != static_cast<SMPMemRequest *>(mreq)->getRequestor()) {
	upperLevel[i]->returnAccess(mreq);
      }
    }
  } 
  else {
    // operation has already been sent to other caches, receive responses

    I(pendReqsTable[mreq] > 0);
    I(pendReqsTable[mreq] <= (int) upperLevel.size());
	
	//printf("Mreq count %d\n",pendReqsTable[mreq]);
    pendReqsTable[mreq]--;
    if(pendReqsTable[mreq] != 0) {
      // this is an intermediate response, request is not serviced yet
      return;
    }

    // this is the final response, request can go up now
    finalizeRead(mreq);
  }
}

void SMPSystemBus::finalizeRead(MemRequest *mreq)
{
  finalizeAccess(mreq);
}

void SMPSystemBus::doWrite(MemRequest *mreq)
{
  SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);

  // no need to snoop, go straight to memory
  if(!sreq->needsSnoop()) {
    goToMem(mreq);
    return;
  }

  if(pendReqsTable.find(mreq) == pendReqsTable.end()) {

    unsigned numSnoops = getNumSnoopCaches(sreq);

    // operation is starting now, add it to the pending requests buffer
    pendReqsTable[mreq] = getNumSnoopCaches(sreq);

    if(!numSnoops) { 
      // nothing to snoop on this chip
      finalizeWrite(mreq);
      return;
      // TODO: even if there is only one processor on each chip, 
      // request is doing two rounds: snoop and memory
    }

	//VERSION COMBINING REGISTER ALLOCATED HERE
	#ifdef TLS
		if (mreq->isDataReq()&& sreq->needsData())VCRCount++;
	#endif
    // distribute requests to other caches, wait for responses
	 for(uint32_t i = 0; i < upperLevel.size(); i++) {
      if(upperLevel[i] != static_cast<SMPMemRequest *>(mreq)->getRequestor()) {
	upperLevel[i]->returnAccess(mreq);
      }
    }
  } 
  else {
    // operation has already been sent to other caches, receive responses

    I(pendReqsTable[mreq] > 0);
    I(pendReqsTable[mreq] <= (int) upperLevel.size());

    pendReqsTable[mreq]--;
    if(pendReqsTable[mreq] != 0) {
      // this is an intermediate response, request is not serviced yet
      return;
    }

    // this is the final response, request can go up now
    finalizeWrite(mreq);
  }
}

void SMPSystemBus::finalizeWrite(MemRequest *mreq)
{
  finalizeAccess(mreq);
}

void SMPSystemBus::finalizeAccess(MemRequest *mreq)
{
  PAddr addr  = mreq->getPAddr();
  SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);
  
  pendReqsTable.erase(mreq);
  #ifdef TLS
	  if (mreq->isDataReq())
	  {
	  	
	  	int32_t nLesserEpochs=processResponses(mreq);
	 	//Version combining has concluded
    	//doVCRCB::scheduleAbs(busDelay*wrSize+nextSlot(mreq)+delay, this, mreq);
	 	if (VCRMaxCount.getValue()<VCRCount)
  			VCRMaxCount.inc();
  		if (sreq->needsData()|| sreq->getMemOperation()==MemPush)VCRCount--;
  		
		 l2DataProv.sample(nLesserEpochs);
		 mreq->getVcacheFlags()->clear();

 		
 		 //Account for all bus transfers for version combination
		 sreq->goUpAbs(busDelay*nLesserEpochs+nextSlot(mreq)+delay); 
	  	}
	  	else
	  	  	 sreq->goUpAbs(nextSlot(mreq)+delay);
	#else
	  sreq->goUpAbs(nextSlot(mreq)+delay);  
   #endif
 
  // request completed, respond to requestor 
  // (may have to come back later to go to memory)

}

void SMPSystemBus::goToMem(MemRequest *mreq)
{
  mreq->goDown(delay, lowerLevel[0]);
}

void SMPSystemBus::doPush(MemRequest *mreq)
{
#ifdef TLS
   SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);
   TimeDelta_t VCRDelay;
   PAddr addr= mreq->getPAddr();
  // no need to snoop, go straight to memory
  if(!sreq->needsSnoop()) {
    goToMem(mreq);
    return;
  }

  if(pendReqsTable.find(mreq) == pendReqsTable.end()) {

    unsigned numSnoops = getNumSnoopCaches(sreq);

    // operation is starting now, add it to the pending requests buffer
    pendReqsTable[mreq] = getNumSnoopCaches(sreq);

   	VCRCount++;
    // distribute requests to other caches, wait for responses
    for(uint32_t i = 0; i < upperLevel.size(); i++) {
      if(upperLevel[i] != static_cast<SMPMemRequest *>(mreq)->getRequestor()) {
	upperLevel[i]->returnAccess(mreq);
      }
    }
  return;
  } 
  else {
    // operation has already been sent to other caches, receive responses

    I(pendReqsTable[mreq] > 0);
    I(pendReqsTable[mreq] <= (int) upperLevel.size());

    pendReqsTable[mreq]--;
    if(pendReqsTable[mreq] != 0) {
      // this is an intermediate response, request is not serviced yet
      return;
    }

    // this is the final response, Request will now be sent down
    VCRDelay=finalizePush(mreq);
    mreq->goDown(VCRDelay+delay, lowerLevel[0]);
  }
 #else 
  	mreq->goDown(delay, lowerLevel[0]);
 #endif
}

void SMPSystemBus::invalidate(PAddr addr, ushort size, MemObj *oc)
{
  invUpperLevel(addr, size, oc);
}

void SMPSystemBus::doInvalidate(PAddr addr, ushort size)
{
  I(0);
}

void SMPSystemBus::returnAccess(MemRequest *mreq)
{
  mreq->goUpAbs(nextSlot(mreq)+delay);
}
#ifdef TLS
int32_t SMPSystemBus::processResponses(MemRequest *mreq)
{
		SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);
		typedef std::vector<tls::CacheFlags>::iterator ItVcacheFlags;
		int32_t nLesserEpochs=0;
		int32_t wrSize=(sreq->getVcacheFlags())->size();
		//I(wrSize<=nProcs);

		//MEM ACCESS IN NO LONGER NECESSARY
		//Epoch checks are now done in the MESI protocol
		//So if line with proper epoch is not found the tag is set to 0
		//Consequently standard MESI causes request to go down to memory
		//Still has mem access code to prevent breaking further code
		if (sreq->getMemOperation()==MemReadW)
		{
			if (!wrSize)
			{
				//No other cache has line
				sreq->setClearOthersExposed();
				if (sreq->needsData())
					sreq->setMemAccess();
				return 0;
			}
			else
			{
				 sreq->clearSetOthersExposed(); 
				 sreq->clearClearOthersExposed()	;
				 ItVcacheFlags itVcacheFlags;
				 for (itVcacheFlags=sreq->getVcacheFlags()->begin();itVcacheFlags!=sreq->getVcacheFlags()->end();itVcacheFlags++)
				 {
				 	//If there are lines with epoch number greater than mine then set Others exposed
				 	if((*itVcacheFlags).getEpoch()->getClock()<(sreq->getOriginalRequest()->getEpoch())->getClock())
				 	{
				 		nLesserEpochs++;
				 	}
				 	else
				  		sreq->setSetOthersExposed();
				 }		  
				 if (!nLesserEpochs) 
				 {
				 	sreq->setMemAccess();
				 	sreq->setClearOthersExposed();
				 }
 				//If write miss then Lesser epochs makes sense
				 return sreq->needsData()? nLesserEpochs:0;

			}
		}
		if  (sreq->getMemOperation()==MemRead)
		{
			if (!wrSize)
			{
				//No other cache has line
				if (sreq->needsData())
					sreq->setMemAccess();
				return 0;
			}	
			else
			{
		    	 sreq->clearMemAccess();
				 sreq->clearSetOthersExposed(); 	
				 ItVcacheFlags itVcacheFlags;
				 for (itVcacheFlags=sreq->getVcacheFlags()->begin();itVcacheFlags!=sreq->getVcacheFlags()->end();itVcacheFlags++)
				 {
				 	//If there are lines with epoch number greater than mine then set exposed read
				 	if((*itVcacheFlags).getEpoch()->getClock()<(sreq->getOriginalRequest()->getEpoch())->getClock())
				 	{
				 		nLesserEpochs++;
				 	}
				 	else
				  		sreq->setSetOthersExposed();
				 }		  
				 if (!nLesserEpochs) sreq->setMemAccess();
				 return nLesserEpochs;
			}
		}
		if (sreq->getMemOperation()==MemPush)
		{
 			ItVcacheFlags itVcacheFlags;
			for (itVcacheFlags=sreq->getVcacheFlags()->begin();itVcacheFlags!=sreq->getVcacheFlags()->end();itVcacheFlags++)
				 {
					//Actual VCR
				 	if((*itVcacheFlags).getEpoch()->getClock()<(sreq->getOriginalRequest()->getEpoch())->getClock())
				 	{
				 		nLesserEpochs++;
				 	}
				 }		  
			return nLesserEpochs;
		}
		return 0;//All other cases
}
 TimeDelta_t SMPSystemBus::finalizePush(MemRequest *mreq)
{
  PAddr addr  = mreq->getPAddr();
  SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);
  
  pendReqsTable.erase(mreq);
	  if (mreq->isDataReq())
	  {
	  	
	  	int32_t nLesserEpochs=processResponses(mreq);
	 	//Version combining has concluded
    	//doVCRCB::scheduleAbs(busDelay*wrSize+nextSlot(mreq)+delay, this, mreq);
	 	if (VCRMaxCount.getValue()<VCRCount)
  			VCRMaxCount.inc();
  		if (sreq->needsData()|| sreq->getMemOperation()==MemPush)VCRCount--;
  		
		 l2DataProv.sample(nLesserEpochs);
		 mreq->getVcacheFlags()->clear();

 		 //Account for all bus transfers for version combination
	  	 return busDelay*nLesserEpochs;
	  	}
	  	else
			I(0);//Should not be called for any other reason
		return 0;
}

#endif

