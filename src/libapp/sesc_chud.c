
#ifdef SESC_OSX_CHUD

#include <stdlib.h>

#include <CHUD/CHUD.h>
#include <ppc_intrinsics.h>

static int chud_pmc1SPR;
static int chud_pmc2SPR;
static int chud_pmc3SPR;
static int chud_pmc4SPR;
static int chud_pmc5SPR;
static int chud_pmc6SPR;
static int chud_pmc7SPR;
static unsigned int chud_pmc1_begin;
static unsigned int chud_pmc1_end;
static unsigned int chud_pmc2_begin;
static unsigned int chud_pmc2_end;
static unsigned int chud_pmc3_begin;
static unsigned int chud_pmc3_end;
static unsigned int chud_pmc4_begin;
static unsigned int chud_pmc4_end;
static unsigned int chud_pmc5_begin;
static unsigned int chud_pmc5_end;
static unsigned int chud_pmc6_begin;
static unsigned int chud_pmc6_end;
static unsigned int chud_pmc7_begin;
static unsigned int chud_pmc7_end;

void chud_boot()
{
  chudInitialize();
  chudSetErrorLogFile(stderr);

  if(chudAcquireSamplingFacility(0)!=chudSuccess) { /* 0 = don't wait for facility */
    printf("ERROR: sampling facility in use by process #%d\n", chudGetSamplingFacilityOwnerPID());
    exit(-1);
  }

  chudSetMarkFilter(chudAllCPUDev, chudFmodeOnlyMarked);
  //chudUmarkPID(getpid(), 1);
  chudUmarkCurrentThread(1);

  switch(chudProcessorClass(chudCPU1Dev)) {
  case PPC_750:
  case PPC_7400:
  case PPC_7450:
    chudSetPMCEvent(chudAllCPUDev, PMC_1, 34); // BR Instr
    chudSetPMCEvent(chudAllCPUDev, PMC_2,  1); // cycles
    chudSetPMCEvent(chudAllCPUDev, PMC_3, 26); // BR Flush
    chudSetPMCEvent(chudAllCPUDev, PMC_4,  2); // Instructions
    chudSetPMCEvent(chudAllCPUDev, PMC_5,  2); // L2 hit
    chudSetPMCEvent(chudAllCPUDev, PMC_6, 29); // L2 miss
    chud_pmc1SPR = chud_7450_pmc1;
    chud_pmc2SPR = chud_7450_pmc2;
    chud_pmc3SPR = chud_7450_pmc3;
    chud_pmc4SPR = chud_7450_pmc4;
    chud_pmc5SPR = chud_7450_pmc5;
    chud_pmc6SPR = chud_7450_pmc6;
    chud_pmc7SPR = chud_7450_pmc6;
    break;
  case PPC_970:
    chudSetPMCEvent(chudAllCPUDev, PMC_1,  1); // Inst Comp
    chudSetPMCEvent(chudAllCPUDev, PMC_2, 15); // cycles
    chudSetPMCEvent(chudAllCPUDev, PMC_3, 22); // Br flush
    chudSetPMCEvent(chudAllCPUDev, PMC_4, 25); // Br correct
    chudSetPMCEvent(chudAllCPUDev, PMC_5,  0); // SRQ Flush
    chudSetPMCEvent(chudAllCPUDev, PMC_6,  0); // LRQ Flush
    chudSetPMCEvent(chudAllCPUDev, PMC_7, 18); // LRQ Full
    chud_pmc1SPR = chud_970_pmc1;
    chud_pmc2SPR = chud_970_pmc2;
    chud_pmc3SPR = chud_970_pmc3;
    chud_pmc4SPR = chud_970_pmc4;
    chud_pmc5SPR = chud_970_pmc5;
    chud_pmc6SPR = chud_970_pmc6;
    chud_pmc7SPR = chud_970_pmc7;

    chudSetPMCMuxPosition(chudAllCPUDev, PMC_MUX_TTM0SEL     , PMC_TTM0_FUNC_IFU);
    chudSetPMCMuxPosition(chudAllCPUDev, PMC_MUX_TTM1SEL     , PMC_TTM1_FUNC_ISU);
    // TTM2 does not exit?
    chudSetPMCMuxPosition(chudAllCPUDev, PMC_MUX_TTM3SEL     , 3);
    // SPEC is not reported
    chudSetPMCMuxPosition(chudAllCPUDev, PMC_MUX_BYTELANE0SEL, PMC_BYTELANE_LSU0);
    chudSetPMCMuxPosition(chudAllCPUDev, PMC_MUX_BYTELANE1SEL, PMC_BYTELANE_TTM1);
    // LANE2 is not used
    chudSetPMCMuxPosition(chudAllCPUDev, PMC_MUX_BYTELANE3SEL, PMC_BYTELANE_TTM0);

    break;
  default:
    fprintf(stderr, "ERROR: unrecognized CPU class\n");
    exit(-1);
    break;
  }
}

void chud_begin()
{
  chudFlushProcessorCaches();
  chudStartPMCs(); /* just let 'em roll */
  chudClearPMCs();

  chudReadSPR(chudCPU1Dev, chud_pmc1SPR, &chud_pmc1_begin);
  chudReadSPR(chudCPU1Dev, chud_pmc2SPR, &chud_pmc2_begin);
  chudReadSPR(chudCPU1Dev, chud_pmc3SPR, &chud_pmc3_begin);
  chudReadSPR(chudCPU1Dev, chud_pmc4SPR, &chud_pmc4_begin);
  chudReadSPR(chudCPU1Dev, chud_pmc5SPR, &chud_pmc5_begin);
  chudReadSPR(chudCPU1Dev, chud_pmc6SPR, &chud_pmc6_begin);
  chudReadSPR(chudCPU1Dev, chud_pmc7SPR, &chud_pmc7_begin);
}

void chud_end()
{
  chudStopPMCs();

  chudReadSPR(chudCPU1Dev, chud_pmc1SPR, &chud_pmc1_end);
  chudReadSPR(chudCPU1Dev, chud_pmc2SPR, &chud_pmc2_end);
  chudReadSPR(chudCPU1Dev, chud_pmc3SPR, &chud_pmc3_end);
  chudReadSPR(chudCPU1Dev, chud_pmc4SPR, &chud_pmc4_end);
  chudReadSPR(chudCPU1Dev, chud_pmc5SPR, &chud_pmc5_end);
  chudReadSPR(chudCPU1Dev, chud_pmc6SPR, &chud_pmc6_end);
  chudReadSPR(chudCPU1Dev, chud_pmc7SPR, &chud_pmc7_end);


  chudUmarkCurrentThread(0);
  chudReleaseSamplingFacility(0); /* 0 = do not keep samples around */

  switch(chudProcessorClass(chudCPU1Dev)) {
  case PPC_750:
  case PPC_7400:
  case PPC_7450:
    fprintf(stderr, "CHUD Instruc  : %d\n", chud_pmc4_end - chud_pmc4_begin);
    fprintf(stderr, "CHUD cycles   : %d\n", chud_pmc2_end - chud_pmc2_begin);
    fprintf(stderr, "CHUD BR Instr : %d\n", chud_pmc1_end - chud_pmc1_begin);
    fprintf(stderr, "CHUD BR Flush : %d\n", chud_pmc3_end - chud_pmc3_begin);
    fprintf(stderr, "CHUD L2 hit   : %d\n", chud_pmc5_end - chud_pmc5_begin);
    fprintf(stderr, "CHUD L2 miss  : %d\n", chud_pmc6_end - chud_pmc6_begin);
    fprintf(stderr, "CHUD IPC      : %3.2f\n"
	    , ((double) (chud_pmc4_end - chud_pmc4_begin))/(chud_pmc2_end - chud_pmc2_begin));
    fprintf(stderr, "CHUD BRMiss   : %7.3f%%\n"
	    , 100*((double) (chud_pmc3_end - chud_pmc3_begin))
	    /(chud_pmc3_end - chud_pmc3_begin + chud_pmc1_end - chud_pmc1_begin));
    fprintf(stderr, "CHUD L2 Miss  : %7.3f%%\n"
	    , 100*((double) (chud_pmc6_end - chud_pmc6_begin))
	    /(chud_pmc6_end - chud_pmc6_begin + chud_pmc5_end - chud_pmc5_begin));
    break;
  case PPC_970:
    fprintf(stderr, "CHUD Instruc  : %d\n", chud_pmc1_end - chud_pmc1_begin);
    fprintf(stderr, "CHUD cycles   : %d\n", chud_pmc2_end - chud_pmc2_begin);
    fprintf(stderr, "CHUD BR Flush : %d\n", chud_pmc3_end - chud_pmc3_begin);
    fprintf(stderr, "CHUD BR Instr : %d\n", chud_pmc4_end - chud_pmc4_begin);
    fprintf(stderr, "CHUD SRQ Flush: %d\n", chud_pmc5_end - chud_pmc5_begin);
    fprintf(stderr, "CHUD LRQ Flush: %d\n", chud_pmc6_end - chud_pmc6_begin);
    fprintf(stderr, "CHUD LRQ full : %d\n", chud_pmc7_end - chud_pmc7_begin);
    fprintf(stderr, "CHUD IPC      : %3.2f\n"
	    , ((double) (chud_pmc1_end - chud_pmc1_begin))/(chud_pmc2_end - chud_pmc2_begin));
    fprintf(stderr, "CHUD BRMiss   : %7.3f%%\n"
	    , 100*((double) (chud_pmc3_end - chud_pmc3_begin))
	    /(chud_pmc3_end - chud_pmc3_begin + chud_pmc4_end - chud_pmc4_begin));
    fprintf(stderr, "CHUD Mem Flush/inst: %7.4f%%\n"
	    , 100*((double) (chud_pmc5_end - chud_pmc5_begin + chud_pmc6_end - chud_pmc6_begin))
	    /(chud_pmc1_end - chud_pmc1_begin));

    break;
  default:
    fprintf(stderr, "ERROR: unrecognized CPU class\n");
    exit(-1);
    break;
  }
}

#endif
