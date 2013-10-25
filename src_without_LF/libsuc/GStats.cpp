/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  Smruti Sarangi
		  Luis Ceze
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

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#include "GStats.h"
#include "ReportGen.h"

GStats::Container *GStats::store=0;

GStats::~GStats()
{
  unsubscribe();
  free(name);
}

/*********************** GStatsCntr */

GStatsCntr::GStatsCntr(const char *format,...)
{
  I(format!=0);      // Mandatory to pass a description
  I(format[0] != 0); // Empty string not valid

  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  data = 0;

  name = str;
  subscribe();
}

double GStatsCntr::getDouble() const
{
  return (double)data;
}

void GStatsCntr::reportValue() const
{
  Report::field("%s=%lld", name, data);
}


/*********************** GStatsAvg */

GStatsAvg::GStatsAvg(const char *format,...)
{
  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  data = 0;
  nData = 0;

  name = str;
  subscribe();
}

double GStatsAvg::getDouble() const
{
  double result = 0;

  if(nData)
    result = (double)data / nData;

  return result;
}

void GStatsAvg::reportValue() const
{
  Report::field("%s:v=%g:n=%lld", name, getDouble(), nData);
}


/*********************** GStatsPDF */

GStatsPDF::GStatsPDF(const char *format,...)
{
  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  data = 0;
  nData = 0;

  name = str;
  subscribe();
}

void GStatsPDF::sample(const int32_t v)
{
  data += v;
  nData++;

  if(density.find(v) == density.end()) {
    density[v] = 1;
  } else {
    density[v]++;
  }
}

// Merge two GStatsPDF together
void GStatsPDF::sample(GStatsPDF &g) 
{
  data += g.data;
  nData += g.nData;

  HASH_MAP<int32_t,int>::iterator it;
  for(it=g.density.begin(); it!=g.density.end(); it++) {
    if( density.find( (*it).first ) == density.end() ) {
      density[ (*it).first ] = (*it).second;
    } else {
      density[ (*it).first ] += (*it).second;
    }
  }
}

void GStatsPDF::msamples(const long long v, long long n) 
{
  data  += v;
  nData += n;

  if(density.find(v) == density.end()) {
    density[v] = n;
  } else {
    density[v] += n;
  }
}

double GStatsPDF::getStdDev() const
{
  double avg = getDouble();

  double sum = 0.0;

  HASH_MAP<int32_t,int>::const_iterator it;

  for(it=density.begin(); it!=density.end(); it++) {
    double value = (*it).first;
    double occurrence = (*it).second;
    sum += occurrence * (value - avg) * (value - avg);
  }

  return sqrt(sum/double(nData));
}

double GStatsPDF::getSpread(double p) const
{
  double avg = getDouble();
  double perAvg = p*avg;
  int32_t cnt=0;

  HASH_MAP<int32_t,int>::const_iterator it;
    
  for(it=density.begin(); it!=density.end(); it++) {
    if( double((*it).first) > perAvg ) {
      cnt += (*it).second;
    }
  }
  
  return (double)cnt / (double)nData;
}

void GStatsPDF::reportValue() const
{
  Report::field("%s:v=%g:sdev=%g:s=%g:n=%lld", name, getDouble(), getStdDev(),
		getSpread(0.90),nData);
}

/*********************** GStats */

char *GStats::getText(const char *format,
                      va_list ap)
{
  char strid[1024];

  vsprintf(strid, format, ap);

  return strdup(strid);
}



void GStats::subscribe()
{
  if( store == 0 )
    store = new Container;
  cpos=store->insert(store->end(),this);
}

void GStats::unsubscribe()
{
  I(store);
  I(*cpos==this);
  store->erase(cpos);
//   bool found = false;
//   I(store);
  
//   for(ContainerIter i = store->begin(); i != store->end(); i++) {
//     if(*i == this) {
//       found = true;
//       store->erase(i);
//       break;
//     }
//   }

//   GMSG(!found, "GStats::unsubscribe It should be in the list");
}

void GStats::report(const char *str)
{
  Report::field("BEGIN GStats::report %s", str);
  if (store) {
    for(ContainerIter i = store->begin(); i != store->end(); i++) {
      (*i)->prepareReport(); //give class a chance to do any calculations
      (*i)->reportValue();
    }
  }

  Report::field("END GStats::report %s", str);
}


GStats *GStats::getRef(const char *str)
{
  for(ContainerIter i = store->begin(); i != store->end(); i++) {

    if(strcasecmp((*i)->name, str) == 0)
      return *i;
  }

  return 0;
}

/*********************** GStatsProfiler */

GStatsProfiler::GStatsProfiler(const char *format, ...)
{
  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);


  name = str;
  subscribe();
}

void GStatsProfiler::sample(uint32_t key)
{
  ProfHash::iterator it = p.find(key);
  if(it != p.end()) {
    (*it).second++;
  }else{
    p[key] = 1;
  }
}

void GStatsProfiler::reportValue() const 
{
  ProfHash::const_iterator it;
  for( it = p.begin(); it != p.end(); it++ ) {
    Report::field("%s(%d)=%d",name,(*it).first,(*it).second);
  }
}



/*********************** GStatsMax */

GStatsMax::GStatsMax(const char *format,...)
{
  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  maxValue = 0;
  nData = 0;

  name = str;
  subscribe();
}

void GStatsMax::reportValue() const
{
  Report::field("%s:max=%ld:n=%lld", name, maxValue, nData);
}

/*********************** GStatsHist */

GStatsHist::GStatsHist(const char *format,...) : numSample(0), cumulative(0)
{
  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  name = str;
  subscribe();
}

void GStatsHist::reportValue() const
{
  Histogram::const_iterator it;
    
  uint32_t maxKey = 0;

  for(it=H.begin();it!=H.end();it++) {
    Report::field("%s(%lu)=%llu",name,(*it).first,(*it).second);
    if((*it).first > maxKey)
      maxKey = (*it).first;
  }
  long double div = cumulative; // cummulative has 64bits (double has 54bits mantisa)
  div /= numSample;

  Report::field("%s_MaxKey=%lu",name,maxKey);
  Report::field("%s_Avg=%f",name,(float)div);
  Report::field("%s_Samples=%lu",name,numSample);
}

void GStatsHist::sample(uint32_t key, unsigned long long weight)
{
  if(H.find(key)==H.end())
    H[key]=0;

  H[key]+=weight;

  numSample += weight;
  cumulative += weight * key;
}

/*********************** GStatsTimingAvg */

GStatsTimingAvg::GStatsTimingAvg(const char *format,...)
{
  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  data = 0;
  nData = 0;
  lastUpdate = 0;
  lastValue = 0;

  name = str;
  subscribe();
}

void GStatsTimingAvg::sample(const int32_t v)
{
  if(lastUpdate != globalClock && lastUpdate != 0) {
    data += lastValue;
    nData++;
  }

  lastValue = v;
  lastUpdate = globalClock;
}

/*********************** GStatsTimingHist */

GStatsTimingHist::GStatsTimingHist(const char *format,...)
{
  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  name = str;
  subscribe();
  
  lastUpdate = 0;
  lastKey = 0;

  reportWholeHist = true;
}

void GStatsTimingHist::reportValue() const
{
  Histogram::const_iterator it;
  unsigned long long w = 0;
  double wavg = 0;

  for(it=H.begin();it!=H.end();it++) {

    if((*it).first == lastKey)
      w = globalClock-lastUpdate;
    else
      w = 0;

    wavg += ((*it).first * ((*it).second + w)) / globalClock;

    if(reportWholeHist) 
      Report::field("%s(%lu)=%llu",name,(*it).first,(*it).second+w);
  }

  Report::field("%s_AutoAvg=%f", name, wavg);
}

void GStatsTimingHist::sample(uint32_t key)
{
  GStatsHist::sample(lastKey, (unsigned long long) (globalClock-lastUpdate));
  lastUpdate = globalClock;
  lastKey = key;
}

/* GStatsEventTimingHist */

GStatsEventTimingHist::GStatsEventTimingHist(const char *format,...)
{
  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  name = str;
  subscribe();

  currentSum = 0;
  lastSample = 0;
  lastHistEvent = 0;
}

void GStatsEventTimingHist::buildHistogram(bool limit) 
{
  EventTimes::iterator it, begin_it;

  begin_it = beginT.begin();
  
  while( (it=evT.begin()) != evT.end() ) {

     if( limit && ( begin_it != beginT.end() && (*it) >= (*begin_it)) )
       break;
    
     I( begin_it==beginT.end() || *it < *begin_it );
     GStatsHist::sample(currentSum,(*it)-lastHistEvent);
     currentSum += evH[ (*it) ];
     I(currentSum >= 0);
     I((*it)>=lastHistEvent);      
     lastHistEvent = (*it);

     evH.erase( (*it) );     
     evT.erase( (*it) );
  }  
}

void GStatsEventTimingHist::begin_sample(unsigned long long id)
{
  EventHistory::iterator it_now;
  
  it_now = beginH.find(globalClock);
  if( it_now == beginH.end() ) {
    beginH[globalClock] = 0;
    it_now = beginH.find(globalClock); 
  }   

  // Add another request pending on this begin point
  (*it_now).second = (*it_now).second + 1;

  // Make sure begin point is included in begin-timeline
  beginT.insert(globalClock);

  evPending[id].start = globalClock;
}

void GStatsEventTimingHist::commit_sample(unsigned long long id)
{
  EventHistory::iterator it_begin;
  Time_t startTime;
  bool build = false;
  
  I( evPending.find(id) != evPending.end() );
  
  startTime = evPending[id].start;
  evPending.erase(id);  
    
  it_begin = beginH.find( startTime ); 
  I( it_begin != beginH.end() );

  (*it_begin).second = (*it_begin).second - 1;   
  if( (*it_begin).second == 0 ) {
    beginT.erase(startTime);    
    beginH.erase(startTime);
    build = true;
  }

  EventHistory::iterator it_now, it_before;
  
  it_now = evH.find(globalClock);
  if( it_now == evH.end() ) {
    evH[globalClock] = 0;
    it_now = evH.find(globalClock); 
  }   
  it_before = evH.find(startTime);
  if(it_before == evH.end()) {
    evH[startTime] = 0; 
    it_before = evH.find(startTime);  
  }

  evT.insert(globalClock);
  evT.insert(startTime);
  
  (*it_now).second = (*it_now).second - 1;
  (*it_before).second = (*it_before).second + 1;

  if(build)
    buildHistogram(true);
}

void GStatsEventTimingHist::remove_sample(unsigned long long id)
{
  EventHistory::iterator it_begin;
  Time_t startTime;
  
  I( evPending.find(id) != evPending.end() );
  
  startTime = evPending[id].start;
  evPending.erase(id);  
    
  it_begin = beginH.find( startTime ); 
  I( it_begin != beginH.end() );

  (*it_begin).second = (*it_begin).second - 1;   
  if( (*it_begin).second == 0 ) {
    beginT.erase(startTime);
    beginH.erase(startTime);    

    buildHistogram(true);
  }   
}

void GStatsEventTimingHist::reportValue() const
{
  GStatsHist::reportValue(); 
}

/* GStatsPeriodicHist */

GStatsPeriodicHist::GStatsPeriodicHist(int32_t p, const char *format,...)
{
  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  name = str;
  subscribe();
  
  data = 0;
  period = p;
  lastUpdate = 0;
}

void GStatsPeriodicHist::inc()
{
  if(globalClock > (lastUpdate + period)) {
    GStatsHist::sample(data, 1);

    // figuring out if there were periods with 0 activity
    Time_t delta = globalClock - lastUpdate;
    long long nZeroPeriods = (delta / period) - 1;

    // sampling the zero activity
    for(long long i = 0; i < nZeroPeriods; i++) 
      GStatsHist::sample(0, 1);

    // starting a new period 
    lastUpdate = globalClock - (globalClock % period);
    data = 0; 
  }
  data++;
}

void GStatsPeriodicHist::reportValue() const
{
  Report::field("%s_period=%llu",name,period);
  GStatsHist::reportValue();
}

/*********************** GStatsChangeHist */

GStatsChangeHist::GStatsChangeHist(const char *format,...)
{
  char *str;
  va_list ap;

  va_start(ap, format);
  str = getText(format, ap);
  va_end(ap);

  name = str;
  subscribe();
  
  lastUpdate = 0;
}

void GStatsChangeHist::reportValue() const
{
  Histogram::const_iterator it;

  for(it=H.begin();it!=H.end();it++)
    Report::field("%s(%lu)=%llu",name,(*it).first,(*it).second);

}

void GStatsChangeHist::sample(uint32_t key)
{
  if (lastUpdate != globalClock)
    GStatsHist::sample(key, 1);

  lastUpdate = globalClock;
}
