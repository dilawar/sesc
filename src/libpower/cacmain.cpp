
#include <stdlib.h>
#include <unistd.h>

#include "SescConf.h"
#include "ReportGen.h"

#include "wattch/wattch_setup.h"
#include "orion/orion_setup.h"

void cacti_setup();

int32_t main(int32_t argc, char **argv)
{
  // Note: This is not integrated with powermain.cpp in a single pass because
  // cacti and wattch have same function names. Therefore, they do not link
  // together.
  if (argc<3) {
    fprintf(stderr,"Usage:\n\t%s <tmp.conf> <power.conf>\n",argv[0]);
    exit(-1);
  }

  unlink(argv[2]);
  Report::openFile(argv[2]);

  SescConf = new SConfig(argv[1]);

  fprintf(stderr,"++++++++++++++BEGIN CACTI\n");
  cacti_setup();
  fprintf(stderr,"++++++++++++++END   CACTI\n");

  // dump the cactify configuration
  SescConf->dump(true);

  Report::close();
}
