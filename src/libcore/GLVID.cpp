
#include "GLVID.h"
#include "TaskContext.h"


GLVIDDummy::GLVIDDummy() 
{
}

SubLVIDType GLVIDDummy::getSubLVID() const 
{
  return 0;
}

void GLVIDDummy::garbageCollect() 
{
  HVersionDomain::tryPropagateSafeTokenAll();
}

bool GLVIDDummy::isKilled() const
{ 
  return false; 
}
