/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Luis Ceze

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


#include "TaskHandler.h"
#include "HVersion.h"
#include "TaskContext.h"

TaskHandler *taskHandler=0;

void TaskHandler::kill(const HVersion *ver, bool inv)
{
  I(!ver->isSafe());

  if (ver->getTaskContext() == 0)
    return;

  HVersion *v = ver->getTaskContext()->getVersion();
  ver->getTaskContext()->localKill(inv);

  v->setKilled();
}

bool TaskHandler::restart(const HVersion *ver)
{
  if (ver->getTaskContext() == 0)
    return true;

  ver->getTaskContext()->localRestart();

  TaskContext::tryPropagateSafeToken(ver->getVersionDomain());

  return true;
}

void TaskHandler::setSafe(const HVersion *ver)
{
  I(!ver->isSafe());
  TaskContext *tc=ver->getTaskContext();
  I(tc);

  tc->getVersion()->setSafe();


  // If there was any task waiting to become safe, this is the time to
  // awake it (it is SAFE!!)
  tc->awakeIfWaiting();  
}

void TaskHandler::setFinished(const HVersion *ver)
{
  I(ver->getTaskContext());
  TaskContext *tc=ver->getTaskContext();
  I(tc);


  tc->getVersion()->setFinished();
}

