#ifndef UGLYMEMREQUEST_H
#define UGLYMEMREQUEST_H

// As the name of the file indicates. This is UGLY
// programming. Pointers with voids!!!
//
// NOONE SHOULD USE THIS CLASS
//
// This class is kept until all the code that uses it has been
// rewriten using the CBMemRequest (callback base).
//
// Volunteers?

#include "MemRequest.h"

class IntlMemRequest : public MemRequest {
private:
  // Internal Memory request. Call your favourite call back
  static pool<IntlMemRequest, true> actPool;
  friend class pool<IntlMemRequest, true>;

  void destroy();

public:
  typedef void (*WorkFuncType)(IntlMemRequest *);

protected:
  MemObj       *baseMemObj;
  WorkFuncType  workFunc;
  void         *workData;
  void         *baseData;

  IntlMemRequest();
  virtual ~IntlMemRequest();

  void kernelOp();

  StaticCallbackMember0<IntlMemRequest, &IntlMemRequest::kernelOp> kernelOpCB;

public:

  static IntlMemRequest *create(MemObj * imp
				,int32_t iPhAddr
				,MemOperation mop
				,TimeDelta_t lat
				,WorkFuncType wF = 0
				,void *idata = 0
                                ,void *bdata = 0);

  void setWorkData(void *data) { workData = data; }
  void *getWorkData() const { return workData; }

  void setBaseData(void *data) {baseData = data; }
  void *getBaseData() const { return baseData; }

  void reLaunch(int32_t iPhAddr, MemOperation mop,
		TimeDelta_t lat = 0, WorkFuncType wF = 0);

  void schedule(TimeDelta_t lat);
  void scheduleAbs(Time_t tim);

  VAddr getVaddr() const;
  void ack(TimeDelta_t lat);
};

#endif // UGLYMEMREQUEST_H
