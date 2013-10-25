
#include "UglyMemRequest.h"

pool<IntlMemRequest, true>  IntlMemRequest::actPool(32, "IntlMemRequest");

IntlMemRequest::IntlMemRequest()
  :MemRequest()
  ,workFunc(0)
  ,kernelOpCB(this)
{ 
}

IntlMemRequest::~IntlMemRequest() 
{
  // To avoid warnings
}

IntlMemRequest *IntlMemRequest::create(MemObj * imp
						 ,int32_t iPhAddr
				       ,MemOperation mop
				       ,TimeDelta_t lat
				       ,WorkFuncType wF
				       ,void *idata
				       ,void *bdata)
{
  IntlMemRequest *r = IntlMemRequest::actPool.out();

  r->setFields(0, mop, 0);
  r->dataReq    = true;

  r->baseMemObj = imp;
  r->workData   = idata;
  r->baseData   = bdata;

  r->reLaunch(iPhAddr, mop, lat, wF);
  return r;
}

void IntlMemRequest::reLaunch(int32_t iPhAddr,
			      MemOperation mop,
			      TimeDelta_t lat,
			      WorkFuncType wF) 
{
  I(memStack.empty());
  I(workFunc==0);

  pAddr     = iPhAddr;
  memOp     = mop;
  workFunc  = wF;

  currentMemObj = baseMemObj;
  accessCB.schedule(lat);
}


void IntlMemRequest::kernelOp()
{
  if (workFunc) {
    WorkFuncType wf = workFunc;
    workFunc = 0;
    (*wf)(this);
  }

  destroy(); // workFunc can be reset, then it is not destroyed
}

void IntlMemRequest::schedule(TimeDelta_t lat)
{ 
  kernelOpCB.schedule(lat);
}

void IntlMemRequest::scheduleAbs(Time_t tim)
{ 
  kernelOpCB.scheduleAbs(tim); 
}

// Overwrite MemRequest::destroy
void IntlMemRequest::destroy() 
{
  I(dinst == 0);
  if (workFunc)
    return; // Can't destroy if workFunc is still pending (POJ)

  actPool.in(this);
}

VAddr IntlMemRequest::getVaddr() const 
{
  I(0); // virtual address is unknown. Working with physical only
  return 0;
}

void IntlMemRequest::ack(TimeDelta_t lat) 
{
  if (workFunc == 0)
    return;

  if (lat ==0)
    lat++;  // THIS IS UGLY AS HELL (like the name of the file
	    // suggests). Once all the IntlMemRequest are gone, this
	    // would not be a problem

  schedule(lat);
}
