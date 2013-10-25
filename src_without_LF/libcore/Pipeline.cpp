/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
		  Milos Prvulovic

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

#include "SescConf.h"
#include "Pipeline.h"

IBucket::IBucket(size_t size, Pipeline *p, bool clean)
  : FastQueue<DInst *>(size)
  ,cleanItem(clean)
  ,pipeLine(p)
  ,markFetchedCB(this) 
{
}

void IBucket::markFetched() 
{
  I(fetched == false);
  IS(fetched = true); // Only called once

#ifdef SESC_BAAD
  for(size_t i =0 ; i<size() ; i++) {
    getData(getIdFromTop(i))->setFetch2Time();
  }
#endif
  
  pipeLine->readyItem(this);
}

bool PipeIBucketLess::operator()(const IBucket *x, const IBucket *y) const
{
  return x->getPipelineId() > y->getPipelineId();
}

Pipeline::Pipeline(size_t s, size_t fetch, int32_t maxReqs)
  : PipeLength(s)
  ,bucketPoolMaxSize(s+1+maxReqs)
  ,MaxIRequests(maxReqs)
  ,nIRequests(maxReqs)
  ,buffer(2*s+1+maxReqs)  // double s for the cleanMarks
{
  maxItemCntr = 0;
  minItemCntr = 0;

  nCleanMarks = 0;

  bucketPool.reserve(bucketPoolMaxSize);
  I(bucketPool.empty());
  
  for(size_t i=0;i<bucketPoolMaxSize;i++) {
    IBucket *ib = new IBucket(fetch+1, this); // +1 fake instructions
    bucketPool.push_back(ib);

    ib = new IBucket(4, this, true);
    cleanBucketPool.push_back(ib);
  }

  I(bucketPool.size() == bucketPoolMaxSize);
}

Pipeline::~Pipeline() 
{
  while(!bucketPool.empty()) {
    delete bucketPool.back();
    bucketPool.pop_back();
  }
  while(!cleanBucketPool.empty()){
    delete cleanBucketPool.back();
    cleanBucketPool.pop_back();
  }
  while(!buffer.empty()) {
    delete buffer.top();
    buffer.pop();
  }
  while(!received.empty()) {
    delete received.top();
    received.pop();
  }
}

void Pipeline::cleanMark()
{
  nCleanMarks++;

  I(!cleanBucketPool.empty());

  IBucket *b = cleanBucketPool.back();
  cleanBucketPool.pop_back();

  b->setPipelineId(maxItemCntr);
  maxItemCntr++;

  nIRequests--;

  b->push(0);
  readyItem(b);
}

void Pipeline::readyItem(IBucket *b) 
{
  b->setClock();
  nIRequests++;

  if( b->getPipelineId() != minItemCntr ) {
    received.push(b);
    return;
  }

  // If the message is received in-order. Do not use the sorting
  // receive structure (remember that a cache can respond
  // out-of-order the memory requests)
  minItemCntr++;
  if( b->empty() )
    doneItem(b);
  else
    buffer.push(b);

  clearItems(); // Try to insert on minItem reveiced (OoO) buckets
}

void Pipeline::clearItems()
{
  while( !received.empty() ) {
    IBucket *b = received.top(); 

    if(b->getPipelineId() != minItemCntr)
      break;
   
    received.pop();

    minItemCntr++;
    if( b->empty() )
      doneItem(b);
    else
      buffer.push(b);
  }
}

IBucket *Pipeline::nextItem() 
{
  while(1) {
    if (buffer.empty()) {
#ifdef DEBUG
      // It should not be possible to propagate more buckets
      clearItems();
      I(buffer.empty());
#endif
      return 0;
    }

    if( ((buffer.top())->getClock() + PipeLength) > globalClock )
      return 0;

    IBucket *b = buffer.top();
    buffer.pop();
    I(!b->empty());
    if (!b->cleanItem) {
      I(!b->empty());
      I(b->top() != 0);

      if (nCleanMarks) {
        // Swallow fakes if clean mark set
        do {
          if (!b->top()->isFake())
            return b;

          b->top()->killSilently();
          b->pop();
        } while(!b->empty());
        I(b->empty());
        
        bucketPool.push_back(b);
        continue;
      }

      return b;
    }

    I(b->cleanItem);
    I(!b->empty());
    I(b->top() == 0);
    b->pop();
    I(b->empty());
    cleanBucketPool.push_back(b);
    nCleanMarks--;
  }

  I(0);
}

PipeQueue::PipeQueue(CPU_t i)
  :pipeLine(
            SescConf->getInt("cpucore", "decodeDelay",i)
            +SescConf->getInt("cpucore", "renameDelay",i)
            ,SescConf->getInt("cpucore", "fetchWidth",i)
            ,SescConf->getInt("cpucore", "maxIRequests",i))
  ,instQueue(SescConf->getInt("cpucore", "instQueueSize",i))
{
  SescConf->isInt("cpucore", "decodeDelay", i);
  SescConf->isBetween("cpucore", "decodeDelay", 1, 64,i);

  SescConf->isInt("cpucore", "renameDelay", i);
  SescConf->isBetween("cpucore", "renameDelay", 1, 64, i);

  SescConf->isInt("cpucore", "maxIRequests",i);
  SescConf->isBetween("cpucore", "maxIRequests", 0, 32000,i);
    
  SescConf->isInt("cpucore", "instQueueSize",i);
  SescConf->isBetween("cpucore", "instQueueSize"
                      ,SescConf->getInt("cpucore","fetchWidth",i)
                      ,32768,i);

}

PipeQueue::~PipeQueue()
{
  // do nothing
}
