//----------------------------------------------------------------------------
// File: sesctherm3Dinclude.h
//
// Description: Main include for 3-Dimensional Thermal Model with Micro-cooler implementation
// Authors    : Joseph Nayfach - Battilana
//

#ifndef _SESCTHERM3D_INCLUDE_H
#define _SESCTHERM3D_INCLUDE_H



#include <vector>
#include <complex>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <sstream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <map>
#include <time.h>


//#include "pdsp_defs.h"
//#include "util.h"
//#include "slu_sdefs.h"
#include "slu_ddefs.h"
#include "slu_util.h"

#include "levmar-2.1.3/lm.h"
#include "SescConf.h"
#include "ThermTrace.h"
#include "mtl/matrix.h"
#include "mtl/mtl.h"
#include "mtl/utils.h"
#include "mtl/lu.h"
#include "mtl/dense1D.h"
#include "nanassert.h"
using namespace std;
using namespace mtl;

#include "sesctherm3Ddefine.h"
#include "sesctherm3Dclassdeclare.h"

#include "sesctherm3Dchip.h"
#include "sesctherm3Dbase.h"
#include "sesctherm3Dutil.h"
#include "sesctherm3Dmaterial.h"
#include "sesctherm3Dlayerinfo.h"
#include "sesctherm3Dconfig.h"
#include "sesctherm3Ddatalibrary.h"
#include "sesctherm3Dregression.h"


#include "sesctherm3Dmodel.h"
#include "sesctherm3Ducool.h"
#include "sesctherm3Dgraphics.h"

//#include "sesctherm3Ddtm.h"
//#include "sesctherm3Dperformance.h"
//#include "sesctherm3Dreliability.h"

#endif
