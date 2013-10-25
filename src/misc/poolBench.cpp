
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#include <vector>

#include "pool.h"

class DummyObjTest {
  int32_t c;
  void *a;
  char x;
public:
  char get() const { return c+x;
  };
  
  void put(int32_t c_, char x_) {
    c = c_;
    x = c_;
  };
};


int32_t main()
{
  pool<DummyObjTest> pool1(16);

  std::vector<DummyObjTest *> p(64);
  p.clear();

  long long total=0;
  long long pooled=0;

  timeval stTime;
  gettimeofday(&stTime, 0);
  
  for(int32_t i=0;i<612333;i++) {
    for(char j=0;j<12;j++ ){
      DummyObjTest *o = pool1.out();
      pooled++;
      o->put(j,j);
      p.push_back(o);
    }

    for(char j=0;j<12;j++ ){
      DummyObjTest *o = p.back();
      total += o->get();
      p.pop_back();
      pool1.in(o);
    }
  }

  for(int32_t i=0;i<752333;i++) {
    for(char j=0;j<20;j++ ){
      DummyObjTest *o = pool1.out();
      pooled++;
      o->put(j-7,j);
      p.push_back(o);
    }

    for(char j=0;j<20;j++ ){
      DummyObjTest *o = p.back();
      total += o->get();
      p.pop_back();
      pool1.in(o);
    }
  }

  for(int32_t i=0;i<20552333;i++) {
    DummyObjTest *o = pool1.out();
    pooled++;
    o->put(i-1952333,2);
    total += o->get();
    pool1.in(o);
  }

  timeval endTime;
  gettimeofday(&endTime, 0);

  double msecs = (endTime.tv_sec - stTime.tv_sec) * 1000 
    + (endTime.tv_usec - stTime.tv_usec) / 1000;
  
  time_t t;
  time(&t);
  fprintf(stderr,"poolBench: %8.2f MPools/s :%s"
	  ,(double)pooled/(1000*msecs)
	  ,ctime(&t)
    );

  fprintf(stderr,"Total = %lld (135510418?)\n",total);

  return 0;
}
