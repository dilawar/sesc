/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Wei Liu

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

#include "Profile.h"
#include "ReportGen.h"
#include "ProcessId.h"
#include "SescConf.h"
#include "OSSim.h"
#include "TaskContext.h"
#include "ExecutionFlow.h"

//
// cache behavior profiling
//
ProfCache::ProfCache()
{
  const char *CacheName = SescConf->getCharPtr("Profiling", "CacheName");
  pcache = PCacheType::create(CacheName, "", "ProfL2");
  nRdHit = 0;
  nWrHit = 0;
  nRdMiss = 0;
  nWrMiss = 0;
}

ProfCache::~ProfCache()
{
  if (pcache)
    pcache->destroy();
}

bool ProfCache::read(PAddr addr)
{
  PCacheType::CacheLine *cl = pcache->readLine(addr);
  if (cl) {
    nRdHit++;
    return true;
  } else { 
    nRdMiss++;

    cl = pcache->fillLine(addr);
    I(cl);
    cl->s = CLEAN;
    return false;
  }  
}

//XXX: only WB cache is modeled
bool ProfCache::write(PAddr addr)
{
  PCacheType::CacheLine *cl = pcache->writeLine(addr);
  if (cl) {
    nWrHit++;
    cl->s = DIRTY;
    return true;
  } else { 
    nWrMiss++;

    cl = pcache->fillLine(addr);
    I(cl);
    cl->s = DIRTY;
    return false;
  }  
}

//
// task hoisting distance, size and #squash profiling
//
Profile::TaskInfoType  Profile::tasks;
Profile::WriteInfoType Profile::writes;
#ifdef LOCAL_WRITE
Profile::AddrSet       Profile::localWriteAddrs;
#endif
#ifdef WRITE_REVISE
Profile::WriteList     Profile::localWrites;
#endif

Profile::Profile()
{  
  startInst = 0;
  stopInst  = 0;
  currTaskID = 0;
  currTime   = 0;
  maxCurrTime = 0;
  lastRecInst = 0;
  started   = false;

  //push the first thread into spawns
  SpawnInfo sInfo;
  sInfo.taskID = 0;
  sInfo.instPos= 0;
  sInfo.vTime  = 0;
  spawns.push_front(sInfo);

  readParameters(osSim->getProfSectionName());

  subscribe();
}

Profile::~Profile()
{
  unsubscribe();
}

void Profile::readParameters(const char *section)
{
  const char *defaultSection = "Profiling";

  if (section == NULL)
    section = "Profiling";

  if (SescConf->checkInt(section, "taskSizeThrd"))
    taskSizeThrd = SescConf->getInt(section, "taskSizeThrd");
  else  
    taskSizeThrd = SescConf->getInt(defaultSection, "taskSizeThrd");

  if (SescConf->checkInt(section, "staticHoistThrd"))
    staticHoistThrd = SescConf->getInt(section, "staticHoistThrd");
  else  
    staticHoistThrd = SescConf->getInt(defaultSection, "staticHoistThrd");

  if (SescConf->checkInt(section, "dynamicHoistThrd"))
    dynamicHoistThrd = SescConf->getInt(section, "dynamicHoistThrd");
  else  
    dynamicHoistThrd = SescConf->getInt(defaultSection, "dynamicHoistThrd");

  if (SescConf->checkInt(section, "maxHoistThrd"))
    maxHoistThrd = SescConf->getInt(section, "maxHoistThrd");
  else  
    maxHoistThrd = SescConf->getInt(defaultSection, "maxHoistThrd");

  if (SescConf->checkInt(section, "spawnOverhead"))
    spawnOverhead = SescConf->getInt(section, "spawnOverhead");
  else  
    spawnOverhead = SescConf->getInt(defaultSection, "spawnOverhead");

  if (SescConf->checkDouble(section, "violationThrd"))
    violationThrd = SescConf->getDouble(section, "violationThrd");
  else  
    violationThrd = SescConf->getDouble(defaultSection, "violationThrd");

  if (SescConf->checkDouble(section, "l2MissOccThrd"))
    l2MissOccThrd = SescConf->getDouble(section, "l2MissOccThrd");
  else  
    l2MissOccThrd = SescConf->getDouble(defaultSection, "l2MissOccThrd");

  if (SescConf->checkDouble(section, "sHoistOccThrd"))
    sHoistOccThrd = SescConf->getDouble(section, "sHoistOccThrd");
  else  
    sHoistOccThrd = SescConf->getDouble(defaultSection, "sHoistOccThrd");

  if (SescConf->checkDouble(section, "dHoistOccThrd"))
    dHoistOccThrd = SescConf->getDouble(section, "dHoistOccThrd");
  else  
    dHoistOccThrd = SescConf->getDouble(defaultSection, "dHoistOccThrd");

  if (SescConf->checkDouble(section, "extraWorkRate"))
    extraWorkRate = SescConf->getDouble(section, "extraWorkRate");
  else  
    extraWorkRate = SescConf->getDouble(defaultSection, "extraWorkRate");

  if (SescConf->checkInt(section, "latReadHit"))
    latReadHit = SescConf->getInt(section, "latReadHit");
  else  
    latReadHit = SescConf->getInt(defaultSection, "latReadHit");

  if (SescConf->checkInt(section, "latReadMiss"))
    latReadMiss = SescConf->getInt(section, "latReadMiss");
  else  
    latReadMiss = SescConf->getInt(defaultSection, "latReadMiss");

  if (SescConf->checkInt(section, "latWriteHit"))
    latWriteHit = SescConf->getInt(section, "latWriteHit");
  else  
    latWriteHit = SescConf->getInt(defaultSection, "latWriteHit");

  if (SescConf->checkInt(section, "latWriteMiss"))
    latWriteMiss = SescConf->getInt(section, "latWriteMiss");
  else  
    latWriteMiss = SescConf->getInt(defaultSection, "latWriteMiss");
}

void Profile::mergeProfFile(const char *dstFile, char *srcFile1, char *srcFile2) const
{
  FILE *fpDst = fopen(dstFile, "w");
  I(fpDst);
  fprintf(fpDst, "( eliminated ");

  // cat file1
  FILE *fpSrc1 = fopen(srcFile1, "r");
  if (fpSrc1 == NULL) {
    MSG("Warning: no merge on profiling files");
    fclose(fpDst);
    return;
  }

  char s[1024*1024];
  fgets(s, 1024*1024-1, fpSrc1);
  fputs(s, fpDst);
  fclose(fpSrc1);
  
  fprintf(fpDst, " ");
  
  // cat file2
  if (srcFile2 != NULL) {
    FILE *fpSrc2 = fopen(srcFile2, "r");
    if (fpSrc2 != NULL) {
      char s[1024*1024];
      fgets(s, 1024*1024-1, fpSrc2);
      fputs(s, fpDst);
      fclose(fpSrc2);
    }
  }

  fprintf(fpDst, " )\n");
  fclose(fpDst);
}

void Profile::reportValue() const
{
  int32_t sumExec = 0;
  long long sumInst = 0;
  long long sumRdMiss = 0;
  long long sumWrMiss = 0;
  int32_t numTask = 0;
  for (TaskInfoType::const_iterator it = tasks.begin();
       it != tasks.end(); it++) {
    int32_t taskID = (*it).first;
    const TaskInfo &tInfo = (*it).second;
   
    sumExec += tInfo.nExec;
    sumInst += tInfo.nInst;
    sumRdMiss += tInfo.nRdMiss;
    sumWrMiss += tInfo.nWrMiss;

    numTask++;
  }

  FILE *takeFp = fopen("take.tscc.prof", "w");
  I(takeFp);
  fprintf(takeFp, "( taken ");

  if (osSim->getProfPhase() == 1) {
    //TODO: need nicer profiling output, to be compatible to compiler/profiling
    //
    //Profiling Phase 1: 
    //  -- eliminate tasks which have small hoisting distance/small size without spawn
    //  -- select tasks which have significant hoisting occupancy
    //
    char fname1[80];
    sprintf(fname1, "%s.prof", OSSim::getBenchName());
    FILE *fp = fopen(fname1, "w");
    I(fp);
    for (TaskInfoType::const_iterator it = tasks.begin();
         it != tasks.end(); it++) {
      int32_t taskID = (*it).first;
      TaskInfo &tInfo = (TaskInfo &)((*it).second);
      bool bElim = false;

      I(tInfo.nExec);
      if (tInfo.nSpawn == 0 && tInfo.nInst/tInfo.nExec <= taskSizeThrd) {
        bElim = true;
        tInfo.note = DPHASE1_SMALL;
      } else if (tInfo.nStaticHoist/tInfo.nExec <= staticHoistThrd) {
        if ((float)tInfo.nStaticHoist/sumInst >= sHoistOccThrd) 
          bElim = false;
        else { 
          bElim = true;
          tInfo.note = DPHASE1_SHORTHOIST;
        }  
      } else if (tInfo.nStaticHoist/tInfo.nExec >= maxHoistThrd) {
        bElim = true;
        tInfo.note = DPHASE1_HUGEHOIST;
      }

      if (bElim)
        fprintf(fp, "%d ", taskID);
      else
        fprintf(takeFp, "%d ", taskID);
    }
    fclose(fp);

    mergeProfFile("elim.tscc.prof", fname1, NULL);
  } else if (osSim->getProfPhase() == 2) {  
    //
    //Profiling Phase 2: 
    //  -- eliminate tasks which have high squash number/small hoisting distance
    //  -- eliminate tasks which have too many extra work (for energy reason)
    //  -- select tasks which have significant L2 read/write miss occupancy
    //
    char fname1[80];
    sprintf(fname1, "%s.prof", OSSim::getBenchName());
    char fname2[80];
    sprintf(fname2, "%s.prof2", OSSim::getBenchName());
    FILE *fp = fopen(fname2, "w");
    I(fp);
    for (TaskInfoType::const_iterator it = tasks.begin();
         it != tasks.end(); it++) {
      int32_t taskID = (*it).first;
      TaskInfo &tInfo = (TaskInfo &)((*it).second);
      bool bElim = false;

      I(tInfo.nExec);
      if (tInfo.eliminated
          || (float)tInfo.nDynHoist/tInfo.nExec <= (float)dynamicHoistThrd
          || tInfo.nDynHoist == 0 
          || ((float)tInfo.nStaticHoist/tInfo.nDynHoist)-1 > extraWorkRate
          || (float)tInfo.nViolations/tInfo.nExec >= (float)violationThrd
         ) {
        
        if ((float)(tInfo.nRdMiss+tInfo.nWrMiss)/sumInst >= l2MissOccThrd) {
          bElim = false;
          tInfo.note = PREFETCH;
        } else {
          bElim = true;
          tInfo.note = DPHASE2;
        }
      }

      if (bElim)
        fprintf(fp, "%d ", taskID);
      else
        fprintf(takeFp, "%d ", taskID);
    }
    fclose(fp);
    
    mergeProfFile("elim.tscc.prof", fname1, fname2);
  } else {
    MSG("Unsupported Phase %d", osSim->getProfPhase());
  }

  fprintf(takeFp, ") \n");
  fclose(takeFp);

  // Report
  int32_t ii = 0;
  for (TaskInfoType::const_iterator it = tasks.begin();
       it != tasks.end(); it++) {
    int32_t taskID = (*it).first;
    const TaskInfo &tInfo = (*it).second;
   
    I(tInfo.nExec);
    Report::field("Profile_(%d):taskID=%d:startAddr=0x%lx:nExec=%d:nSpawn=%d:nInst=%lld:occInst=%.2f:nStaticHoist=%.2f:nDynHoist=%.2f:nViolations=%.2f:eliminated=%d:nRdHit=%ld:nRdMiss=%ld:nWrHit=%ld:nWrMiss=%ld:occStaticHoist=%.2f:occL2Miss=%.2f:note=%d",
                  ii++, taskID, tInfo.startAddr, tInfo.nExec, tInfo.nSpawn,
                  tInfo.nInst/tInfo.nExec, (float)tInfo.nInst/sumInst,
                  (float)tInfo.nStaticHoist/tInfo.nExec, (float)tInfo.nDynHoist/tInfo.nExec,
                  (float)tInfo.nViolations/tInfo.nExec, tInfo.eliminated,
                  tInfo.nRdHit, tInfo.nRdMiss, tInfo.nWrHit, tInfo.nWrMiss,
                  (float)tInfo.nStaticHoist/sumInst, (float)(tInfo.nRdMiss+tInfo.nWrMiss)/sumInst*1000,
                  tInfo.note);
  }

  Report::field("Profile:taskSizeThrd=%d", taskSizeThrd);
  Report::field("Profile:staticHoistThrd=%d", staticHoistThrd);
  Report::field("Profile:maxHoistThrd=%d", maxHoistThrd);
  Report::field("Profile:dynamicHoistThrd=%d", dynamicHoistThrd);
  Report::field("Profile:violationThrd=%.2f", violationThrd);
  Report::field("Profile:spawnOverhead=%d", spawnOverhead);
  Report::field("Profile:l2MissOccThrd(1E-3)=%.2f", l2MissOccThrd*1000);
  Report::field("Profile:sHoistOccThrd=%.2f", sHoistOccThrd);
  Report::field("Profile:dHoistOccThrd=%.2f(Not used)", dHoistOccThrd);
  Report::field("Profile:extraWorkRate=%.2f", extraWorkRate);

  Report::field("Profile:numTask=%d:averageTaskSize=%lld:totalInst=%lld",
                numTask, sumInst/sumExec, sumInst);
}

void Profile::updateTime()
{
  // update current time

  currTime += ExecutionFlow::getnExecRabbit() - lastRecInst;
  lastRecInst = ExecutionFlow::getnExecRabbit();
}

void Profile::recStartInst() 
{
  startInst = ExecutionFlow::getnExecRabbit();
}

void Profile::recStopInst() 
{
  stopInst = ExecutionFlow::getnExecRabbit();
}
  
void Profile::recSpawn(int32_t pid)
{
  updateTime();
  
  SpawnInfo sInfo;
  sInfo.taskID = currTaskID;
  sInfo.instPos = ExecutionFlow::getnExecRabbit();
  sInfo.vTime = currTime;
  spawns.push_front(sInfo);

  TaskInfoType::iterator tInfoIt = tasks.find(currTaskID);
  I(tInfoIt != tasks.end());
  TaskInfo &tInfo = (*tInfoIt).second;
  tInfo.nSpawn ++;
}

void Profile::recCommit(int32_t pid, int32_t tid)
{
  if (!osSim->enoughMarks1())
    return;

  updateTime();

  //LOG("%lld: Commit (%d)", currTime, tid);
  //statistic current task
  recStopInst();

  TaskInfo &tInfo = tasks[currTaskID];
  
  long long exeInst = stopInst - startInst;
  tInfo.nInst += exeInst;

  // We have negative dynamic hoisting, how can we live with that?
  if (tInfo.spawnPos == 0)
    tInfo.currHoist = 0;
  else if (tInfo.currHoist == 0 || tInfo.currHoist > (long long)tInfo.seqBeginTime - (long long)tInfo.beginTime) {
    tInfo.currHoist = tInfo.seqBeginTime - tInfo.beginTime;
  }  
  tInfo.nDynHoist += tInfo.currHoist;

#ifdef LOCAL_WRITE
  // clear local writes
  localWriteAddrs.clear();
#endif  
#ifdef WRITE_REVISE
  while (!localWrites.empty()) {
    WriteInfo *w = localWrites.front();
    localWrites.pop_front();
    // write back the correct virtual time anyway
    // TODO: revising becomes very hard, so I appromix it
    //w->vTime = w->instPos - tInfo.startPos + tInfo.beginTime;
  }
  I(localWrites.empty());
#endif

  //reset some task information
  tInfo.nUncountRdMiss = 0;
  tInfo.nUncountWrMiss = 0;


  //prepare for next task
  recStartInst();

  int32_t preTaskID = currTaskID;
  currTaskID = tid;

  TaskInfo &childTaskInfo = tasks[currTaskID];
  childTaskInfo.nExec ++;
  childTaskInfo.currHoist = 0;

  if (preTaskID != currTaskID) //if same, means it is a loop
    childTaskInfo.predecessor = preTaskID;

  //set the start address of child task
  childTaskInfo.startAddr = osSim->getContextRegister(pid, 31);

  SpawnInfo sInfo;
  //get the spawn info for the child task
  if (spawns.empty()) {
    //this happens because the mark may between the spawn() and commit()
    MSG("WARNING: spawned before mark");
    sInfo.taskID = 0;
    sInfo.instPos = tasks[0].startPos;
    sInfo.vTime = tasks[0].beginTime;
  } else {
    int32_t oldID = sInfo.taskID; 
    sInfo = spawns.front();
    // printf("oldID=%d, newID=%d\n", oldID, sInfo.taskID);
    spawns.pop_front();
  }

  childTaskInfo.beginTime = sInfo.vTime;
  childTaskInfo.spawnedBy = sInfo.taskID;
  childTaskInfo.spawnPos = sInfo.instPos;
  childTaskInfo.startPos = startInst;
  childTaskInfo.seqBeginTime = currTime;

  LOG("%lld:taskID=%d:beginTime=%lld:seqBeginTime=%lld\n",
      currTime, currTaskID, 
      childTaskInfo.beginTime, childTaskInfo.seqBeginTime);

  if (childTaskInfo.spawnPos != 0)
    childTaskInfo.nStaticHoist += currTime - childTaskInfo.beginTime;
  
  
  // reset currTime to this task's beginTime
  I (currTime >= childTaskInfo.beginTime);

  if (currTime > maxCurrTime)
    maxCurrTime = currTime;
  currTime = childTaskInfo.beginTime;
}

void Profile::recInitial(int32_t pid)
{
  if (!osSim->enoughMarks1())
    return;

  MSG ("Begin profiling...");

  started = true;

  updateTime();

  //statistic current task
  recStopInst();

  TaskInfo &tInfo = tasks[currTaskID];
  
  long long exeInst = stopInst - startInst;
  tInfo.nExec ++;
  tInfo.nInst += exeInst;

  tInfo.beginTime = currTime;
  tInfo.seqBeginTime = maxCurrTime;
}

void Profile::recTermination(int32_t pid)
{
  if (!osSim->enoughMarks1())
    return;

  updateTime();

  //statistic current task
  recStopInst();

  TaskInfo &tInfo = tasks[currTaskID];
  
  long long exeInst = stopInst - startInst;
  tInfo.nInst += exeInst;

  // TODO: maxCurrTime is the wavefront!
  MSG("Finished Time: %lld", maxCurrTime);
}

void Profile::recWrite(VAddr vaddr, icode_ptr picode, bool silent)
{
#ifdef LOCAL_WRITE
  localWriteAddrs.insert(vaddr);
#endif  

  TaskInfo &tInfo = tasks[currTaskID];

  updateTime();

  // model cache behavior first
  // XXX: do I need to use paddr, as long as we have one applicatin they are same
  if (profCache.write(vaddr)) {
    currTime += latWriteHit;
    tInfo.nWrHit++;
  } else {
    currTime += latWriteMiss;
    tInfo.nWrMiss++;
    tInfo.nUncountWrMiss++;
  }

  WriteInfo &wInfo = writes[vaddr];
  // Time_t curVTime = ExecutionFlow::getnExecRabbit()-tInfo.startPos
                    // + tInfo.beginTime;

  if (silent) {
#ifndef LOCAL_WRITE
    if (wInfo.vTime != 0 && wInfo.vTime < currTime)
#endif
      return; //just return, do not record silent stores
  }  

  wInfo.instPos = ExecutionFlow::getnExecRabbit();
  wInfo.picode = picode;
  wInfo.taskID = currTaskID;

  wInfo.vTime = currTime;

#ifdef WRITE_REVISE
  localWrites.push_front(&wInfo);
#endif  
}

void Profile::recRead(VAddr vaddr, icode_ptr picode)
{
  TaskInfoType::iterator cTaskIt = tasks.find(currTaskID);
  I(cTaskIt != tasks.end());
  TaskInfo &cTaskInfo = (*cTaskIt).second;

  updateTime();

#ifdef LOCAL_WRITE
  // protected by local writes
  if (localWriteAddrs.find(vaddr) != localWriteAddrs.end())
    return;
#endif    
  
  WriteInfoType::iterator wIt = writes.find(vaddr);
  if (wIt == writes.end()) {
    //reads without write ahead
    return;
  }
  const WriteInfo &wInfo = (*wIt).second;

  // get the task info which write is in
  int32_t taskID = wInfo.taskID;
  TaskInfoType::iterator pTaskIt = tasks.find(taskID);
  I(pTaskIt != tasks.end());
  const TaskInfo &pTaskInfo = (*pTaskIt).second;
  Time_t writeTime = wInfo.vTime;

  // get current task info which read is in
  Time_t readTime = currTime;
  // Time_t readTime = ExecutionFlow::getnExecRabbit() - cTaskInfo.startPos
                    // + cTaskInfo.beginTime;
                    
  if (writeTime > readTime) {
    // SQUASH TASK
    LOG("%lld:Offending Read(0x%lx), taskID=%d",
        ExecutionFlow::getnExecRabbit(), vaddr, currTaskID);
    LOG("  WR@%lld inst 0x%lx, RD@%lld inst 0x%lx", 
        writeTime, wInfo.picode->addr, readTime, picode->addr);
    
    // remove the offending task
    cTaskInfo.nViolations ++;

    currTime = writeTime + (currTime - cTaskInfo.beginTime)
               - (latReadMiss-latReadHit) * cTaskInfo.nUncountRdMiss
               - (latWriteMiss-latWriteHit) * cTaskInfo.nUncountWrMiss;

    I (currTime >= writeTime);

    cTaskInfo.nUncountRdMiss = 0;   // after using them, clear it for next use
    cTaskInfo.nUncountWrMiss = 0;           
    cTaskInfo.beginTime = writeTime /*+ spawnOverhead*/;    //squash it
  }

  // finish this read no matter it causes squash or not.
  // model cache behavior first
  // XXX: do I need to use paddr, as long as we have one applicatin they are same
  if (profCache.read(vaddr)) {
    currTime += latReadHit;
    cTaskInfo.nRdHit++;
  } else {
    currTime += latReadMiss;
    cTaskInfo.nRdMiss++;
    cTaskInfo.nUncountRdMiss++;
  }  
}
