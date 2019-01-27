#!/usr/bin/env perl

BEGIN {
  my $tmp = $0;
  my $tmp2;

  $tmp = readlink($tmp) if( -l $tmp );
  $tmp =~ s/report.pl//;
  unshift(@INC, $tmp);

  $tmp2 = $0;
  $tmp2 =~ s/report.pl//;
  $tmp2 .= $tmp;
  unshift(@INC, $tmp2);
}

use sesc;
use strict;
use Time::localtime;
use Getopt::Long;

###########################################
# Parameters section:
# All the parameters start with op_

my $op_all;
my $op_last;
my $op_bpred=1; # Show by default
my $op_cpu=1;   # Show by default
my $op_sim=1;   # Show by default
my $op_inst=1;  # Show by default
my $op_help;
my $op_versionmem;
my $op_tls;
my $op_table;
my $op_lock;
my $op_atomic;
my $op_cc;
my $op_baad;
my $op_cg;
my $op_dead;

my $result = GetOptions("a",\$op_all,
                        "last",\$op_last,
                        "bpred!",\$op_bpred,
                        "cpu!",\$op_cpu,
                        "inst!",\$op_inst,
                        "sim!",\$op_sim,
                        "versionmem",\$op_versionmem,
                        "tls",\$op_tls,
                        "atomic",\$op_atomic,
                        "lock",\$op_lock,
                        "table",\$op_table,
                        "cc",\$op_cc,
                        "baad",\$op_baad,
                        "cg",\$op_cg,
                        "dead",\$op_dead,
                        "help",\$op_help
                       );


exit &main();

sub processParams {
  my $badparams=0;

  if( @ARGV == 0 && !( $op_all or $op_last) ) {
    print "Specify the trace to process\n";
    $badparams =1;
  }

  if( $op_last and $op_all ) {
    print "-last and -a are mutually exclusive options\n";
    $badparams =1;
  }

  if( $op_help or $badparams ) {
    usage();
    exit 0;
  }
}

sub usage {
  print "./report.pl [options] <sescDump>\n";
  print "\t-a            : Reports for all the stat files in current directory\n";
  print "\t-last         : Reports the newest stat file in current directory\n";
  print "\t-long         : Reports much more information (verbose)\n";
  print "\t-[no]bpred    : Deactivae/Activate branch predictor statistics\n";
  print "\t-[no]cpu      : Deactivae/Activate CPU statistics\n";
  print "\t-[no]inst     : Deactivae/Activate instruction statistics\n";
  print "\t-[no]sim      : Show results without comments\n";
  print "\t-table        : Statistics table sumarry (god for scripts)\n";
  print "\t-baad         : BAAD statistics\n";
  print "\t-cg           : CriticalityGraph statistics\n";
  print "\t-dead         : List report files whose simulations didn't complete\n";
  print "\t-help         : Show this help\n";
}

###########################################
# Global variables.
my $nCPUs;
my $cpuType;
my $nCycles;
my $slowdown;
my $nInstTotal;
my $nLoadTotal;
my $nStoreTotal;
my $freq;
my $cf;    # current conf File

###########################################
# main section:


sub main {

  processParams();

  my @flist;
  if( $op_all ) {
    opendir(DIR,".");
    # short by modification date
    @flist = sort {(stat($a))[9] cmp (stat($b))[9]} (grep (/sesc\_.+\..{6}$/, readdir (DIR)));
    closedir(DIR);
  }elsif( $op_last ) {
    opendir(DIR,".");
    # short by modification date
    my @tmp = sort {(stat($a))[9] cmp (stat($b))[9]} (grep (/sesc\_.+\..{6}$/, readdir (DIR)));
    @flist = ($tmp[@tmp-1]);
    closedir(DIR);
  }else{
    @flist = @ARGV;
  }

  if( @flist == 0 ) {
    print "No stat files to process\n";
    exit 0;
  }

  if( $op_dead ) {
    my %dead ;
    foreach my $f (@flist) {
      #Catches problems accessing files without permission
      open(FILE,"<$f") or next;
      close(FILE);

      my $c = sesc->new($f);

      my $bench = $c->getCkResultField("OSSim","bench");
      if( ($bench eq 0) ) {         
          print "$f\n";
      }
    }
    exit;
  }

  foreach my $file (@flist) {

    $cf = sesc->new($file);

    my $bench = $cf->getResultField("OSSim","bench");

    $nCycles = $cf->getResultField("OSSim","nCycles");
    next unless ($nCycles);

    print "# Bench : $bench\n";
    printf "# File  : %-20s : %30s\n", $file,ctime((stat($file))[9]);

    print "# THESE ARE PARTIAL STATISTICS\n" if( $cf->getResultField("OSSim","reportName") =~ /Signal/ );

    simStats($file) if( $op_sim );
    branchStats($file) if( $op_bpred );
    instStats($file)  if( $op_inst );
    tradCPUStats($file) if( $op_cpu );

    tradMemStats($file,"dataSource");
    tradMemStats($file,"instrSource");

    breakMemPowerStats($file);

    tradPowerStats($file);

    showStatReport($file) if( $op_table );

    showVersionMem($file) if( $op_versionmem );

    showCCStats($file)    if( $op_cc );

    showTLSReport($file)  if( $op_tls );

    showBaadStats($file)  if( $op_baad );

    atomicStats($file)    if( $op_atomic );

    lockStats($file)      if( $op_lock );

    showCGStats($file)    if( $op_cg );
  }
}

sub partialBusStat {
  my $section = shift;
  my $device  = shift;
  my $name    = shift;

  if ( $device =~ /BUS/i or $name =~ /BUS/i ) {
    my $size = $cf->getConfigEntry(key=>"bsize",section=>$section);
    my $nPackets = $cf->getResultField("Data${name}(0)_occ","n")
      + $cf->getResultField("DataP(0)_${name}_occ","n")
      + $cf->getResultField("Data${name}_occ","n");
    printf " ${name} %g MB/s : ", $freq*$nPackets*$size/($nCycles);
  }
}

sub partialMemStat {
  my $field   = shift;
  my $section = shift;
  my $cpuID   = shift;

  my $dataSource = $cf->getConfigEntry(key=>$field,section=>$section);

  my @name = split(/ +/,$dataSource);
  # $name[0] has the next level in hierarchy
  # $name[1] has the ID for stats

  # if the Cache has no name, statistics are not reported
  return unless (defined $name[1]);

  my $orig = $name[1];

  if ($cf->getResultField("${name[1]}","readHit")==0 && $cf->getResultField("${name[1]}","readMiss")==0) {
    partialBusStat($section, $name[0], $name[1]);

    $name[1] = "P(${cpuID})_${name[1]}"
  }

  if ($cf->getResultField("${name[1]}","readHit")==0 && $cf->getResultField("${name[1]}","readMiss")==0) {
      partialMemStat("lowerLevel", $name[0]);
      return;
  }

  print $orig;
  if ($cf->getResultField("${name[1]}","readMiss")) {
    printf " %3.1f ",$cf->getResultField("${name[1]}_occ","v");

    my $total =
      $cf->getResultField("${name[1]}","readHit")
        + $cf->getResultField("${name[1]}","readHalfHit")
        + $cf->getResultField("${name[1]}","readHalfMiss")
        + $cf->getResultField("${name[1]}","readMiss")
        + $cf->getResultField("${name[1]}","writeHit")
        + $cf->getResultField("${name[1]}","writeHalfHit")
        + $cf->getResultField("${name[1]}","writeHalfMiss")
        + $cf->getResultField("${name[1]}","writeMiss");

    my $miss = $cf->getResultField("${name[1]}","readMiss")
        + $cf->getResultField("${name[1]}","writeMiss");

    printf " %5.2f%", 100*($miss)/$total;

    my $tmp = $cf->getResultField("${name[1]}","readMiss");
    printf " (%4.1f%"  , (100*$tmp/$total);

    my $tmp = $cf->getResultField("${name[1]}","writeMiss");
    printf ",%4.1f%)", (100*$tmp/$total);

    printf " %5.2f%",100*$total/($nLoadTotal+$nStoreTotal+1);

    # Miss BW
    my $size = $cf->getConfigEntry(key=>"bsize",section=>$name[0]);
    printf " %5.2fGB/s",$freq*$miss*$size/(1000*$nCycles);
    
    if ($cf->getResultField("${name[1]}_LVIDTable_nLinesOnRestart","v")) {
      printf " nLines(%4.1f", $cf->getResultField("${name[1]}_LVIDTable_nLinesOnKill","v");
      printf ",%4.1f", $cf->getResultField("${name[1]}_LVIDTable_nLinesOnRestart","v");
      printf ",%4.1f", $cf->getResultField("${name[1]}_LVIDTable_nLinesOnSetSafe","v");
      printf ",%4.1f)", $cf->getResultField("${name[1]}_LVIDTable_nLinesOnSetFinished","v");
    }

  }
  print " : ";

  partialMemStat("lowerLevel", $name[0]);
}

sub tradMemStats {
  my $file = shift;
  my $src = shift;

  my $dataSource = $cf->getConfigEntry(key=>$src,section=>$cpuType);

  my @name = split(/ +/,$dataSource);
  next unless (defined $name[1]);

  my $total =
    $cf->getResultField("P(0)_${name[1]}","readHit")
      + $cf->getResultField("P(0)_${name[1]}","readHalfHit")
      + $cf->getResultField("P(0)_${name[1]}","readHalfMiss")
      + $cf->getResultField("P(0)_${name[1]}","readMiss")
      + $cf->getResultField("P(0)_${name[1]}","writeHit")
      + $cf->getResultField("P(0)_${name[1]}","writeHalfHit")
      + $cf->getResultField("P(0)_${name[1]}","writeHalfMiss")
      + $cf->getResultField("P(0)_${name[1]}","writeMiss");

  return unless ($total);

  printf "################################################################################\n";
  printf "Proc  Cache Occ MissRate (RD, WR) %%DMemAcc MB/s : ... \n";
  for(my $i=0;$i<$nCPUs;$i++) {
    my $cpuType = $cf->getConfigEntry(key=>"cpucore",index=>$i);

    my $dataSource = $cf->getConfigEntry(key=>"dataSource",section=>$cpuType);

    my @name = split(/ +/,$dataSource);
    next unless (defined $name[1]);

    my $total =
      $cf->getResultField("P(${i})_${name[1]}","readHit")
      + $cf->getResultField("P(${i})_${name[1]}","readHalfHit")
      + $cf->getResultField("P(${i})_${name[1]}","readHalfMiss")
      + $cf->getResultField("P(${i})_${name[1]}","readMiss")
      + $cf->getResultField("P(${i})_${name[1]}","writeHit")
      + $cf->getResultField("P(${i})_${name[1]}","writeHalfHit")
      + $cf->getResultField("P(${i})_${name[1]}","writeHalfMiss")
      + $cf->getResultField("P(${i})_${name[1]}","writeMiss");

    next if ($total == 0);

    printf " %3d  ",$i;
    partialMemStat($src, $cpuType, $i);
    print "\n";
  }
}


sub tradPowerStats {
  my $file = shift;

  return unless($cf->getResultField("PowerMgr","totPower"));

  printf "################################################################################\n";

  print "Proc        Fetch     Issue      Mem       Exec     Clock     Total (watts)\n";
  for(my $i=0;$i<$nCPUs;$i++) {
    WritePowerLine("Proc(${i})","${i}", 1);
  }
  WritePowerLine("PowerMgr","Total", 1);
}

sub WritePowerLine {
  my $section = shift;
  my $header  = shift;
  my $delay   = shift;

  print "$header";

  my $allPower    = 0;

  my $tmp = $cf->getResultField($section,"fetchPower") *$slowdown;
  $allPower += $tmp;
  printf " %9.3f ",($tmp*$delay);

  $tmp = $cf->getResultField($section,"issuePower") *$slowdown;
  $allPower += $tmp;
  printf " %9.3f ",($tmp*$delay);

  $tmp = $cf->getResultField($section,"memPower") *$slowdown;
  $allPower += $tmp;
  printf " %9.3f ",($tmp*$delay);

  $tmp = $cf->getResultField($section,"execPower") *$slowdown;
  $allPower += $tmp;
  printf " %9.3f ",($tmp*$delay);

  $tmp = $cf->getResultField($section,"clockPower") *$slowdown;
  $allPower += $tmp;
  printf " %9.3f ",($tmp*$delay);

  $tmp = $cf->getResultField($section,"totPower") *$slowdown;
  printf " %9.3f ",($tmp*$delay);

  print " WRONG ENERGY" unless ($allPower > $tmp *0.95 && $allPower < $tmp *1.05 || $tmp == 0);

  print "\n";
}

sub breakMemPowerStats {
  return unless($cf->getResultField("PowerMgr","totPower"));

  my $energy  = $cf->getResultField("EnergyMgr","memEnergy");

  my $file = shift;

  printf "################################################################################\n";

  print "Proc CacheName LVID revLVID Energy : ... \n";

  for(my $i=0;$i<$nCPUs;$i++) {
    my $cpuType = $cf->getConfigEntry(key=>"cpucore",index=>$i);

    my $dataSource = $cf->getConfigEntry(key=>"dataSource",section=>$cpuType);

    my @name = split(/ +/,$dataSource);
    next unless (defined $name[1]);

    my $total =
      $cf->getResultField("P(${i})_${name[1]}","rdHitEnergy")
      + $cf->getResultField("P(${i})_${name[1]}","rdMissEnergy")
      + $cf->getResultField("P(${i})_${name[1]}","wrHitEnergy")
      + $cf->getResultField("P(${i})_${name[1]}","wrMissEnergy")
      + $cf->getResultField("P(${i})_${name[1]}","lineFillEnergy");

    next if ($total == 0);

    printf " %3d  ",$i;
    partialBreakMemPowerStat("dataSource", $cpuType, $energy, $i);

    printf " MVC ";
    printBreakMemPowerStats("MVVCache", $energy);
    print "\n";
  }
}

sub printBreakMemPowerStats {
    my $entry = shift;
    my $totEnergy = shift;

    my $cacheE =
        $cf->getResultField("$entry","rdHitEnergy")
        + $cf->getResultField("$entry","rdMissEnergy")
        + $cf->getResultField("$entry","wrHitEnergy")
        + $cf->getResultField("$entry","wrMissEnergy")
        + $cf->getResultField("$entry","lineFillEnergy");

    printf " %5.2f% ", 100*$cacheE/$totEnergy;

    my $lvidE =
        $cf->getResultField("$entry","rdLVIDEnergy")
        + $cf->getResultField("$entry","wrLVIDEnergy");
    
    if ($lvidE) {
        printf " %5.2f% ", 100*$lvidE/$totEnergy;

        my $revE =
            $cf->getResultField("$entry","rdRevLVIDEnergy")
            + $cf->getResultField("$entry","wrRevLVIDEnergy");

        printf " %5.2f% ", 100*$revE/$totEnergy;
    }

    print " : ";
}

sub partialBreakMemPowerStat {
  my $field     = shift;
  my $section   = shift;
  my $totEnergy = shift;
  my $cpuID     = shift;

  my $dataSource = $cf->getConfigEntry(key=>$field,section=>$section);

  my @name = split(/ +/,$dataSource);
  # $name[0] has the next level in hierarchy
  # $name[1] has the ID for stats

  # if the Cache has no name, statistics are not reported
  return unless (defined $name[1]);

  my $orig = $name[1];

  $name[1] = "P(${cpuID})_${name[1]}"
    if ($cf->getResultField("${name[1]}","readHit")==0 && $cf->getResultField("${name[1]}","readMiss")==0);

  if ($cf->getResultField("${name[1]}","readHit")==0 && $cf->getResultField("${name[1]}","readMiss")==0) {
    partialBreakMemPowerStat("lowerLevel", $name[0], $totEnergy);
    return;
  }

  print " " . $orig . " ";

  printBreakMemPowerStats($name[1], $totEnergy);

  partialBreakMemPowerStat("lowerLevel", $name[0], $totEnergy);
}

sub showVersionMem {
  my $file = shift;

  return unless( $op_versionmem );

  print "      Task Squashes  Kills Commits    Sq.Inst   UsedInstr Avg.Size  PerUsed  PerExe   SP1   KP1\n";

  my $entries_no = $cf->getResultField("VMemStats","Entries");

  my $totUseful = $cf->getResultField("VersionMem","UsefulInstr");
  my $totSquashed = $cf->getResultField("VersionMem","SquashInstr");

  my $totDynTasks = 0;
  my $totComTasks = 0;

  for(my $i=0;$i<$entries_no;$i++) {
    my $taskAddr = $cf->getResultField("VMemStats(${i})Instr","TaskAddr");

    print "0x";
    print   $taskAddr;

    my $squashCnt = $cf->getResultField("VMemStats(${i})Cnts","Squash");
    printf " %8d",$squashCnt;

    my $killCnt = $cf->getResultField("VMemStats(${i})Cnts","Kill");
    printf " %6d",$killCnt;

    $totDynTasks += $killCnt;

    my $commitCnt = $cf->getResultField("VMemStats(${i})Cnts","Commit");
    printf " %7d",$commitCnt;

    $totDynTasks += $commitCnt;
    $totComTasks += $commitCnt;

    my $squashInstrCnt = 
        $cf->getResultField("VMemStats(${i})Instr","SquashInstr");

    printf " %10d",$squashInstrCnt;

    my $usefulInstrCnt = $cf->getResultField("VMemStats(${i})Instr","UsefulInstr");

    printf "  %10d",$usefulInstrCnt;

    printf " %8.2f ",$usefulInstrCnt/$commitCnt;

    printf " %6.2f%",100*$usefulInstrCnt/($usefulInstrCnt+$squashInstrCnt);

    printf " %6.2f%",100*$usefulInstrCnt/$totUseful;

    my $addr_no = $cf->getResultField("VMemStats(${i})Squashes","Entries");

    my $max_cnt=-1;
    my $tot_cnt=0;

    for(my $j=0; $j<$addr_no; $j++) {
      my $squashAddr = 
          $cf->getResultField("VMemStats(${i})Squashes(${j})","SAddr");
      my $squashCnt = 
          $cf->getResultField("VMemStats(${i})Squashes(${j})","SCnt");
      $tot_cnt += $squashCnt;
      if( $squashCnt > $max_cnt ) {
          $max_cnt = $squashCnt;
      }
    }

    printf " %5.2f",$max_cnt/$tot_cnt if($tot_cnt!=0);

    $addr_no = $cf->getResultField("VMemStats(${i})Kills","Entries");

    $max_cnt=-1;
    $tot_cnt=0;

    for(my $j=0; $j<$addr_no; $j++) {
      my $killAddr = 
          $cf->getResultField("VMemStats(${i})Kills(${j})","KAddr");
      my $killCnt = 
          $cf->getResultField("VMemStats(${i})Kills(${j})","KCnt");
      $tot_cnt += $killCnt;
      if( $killCnt > $max_cnt ) {
          $max_cnt = $killCnt;
      }
    }

    printf " %5.2f",$max_cnt/$tot_cnt if ($tot_cnt != 0);

    print "\n";

  }
  printf "---Versioned Memory Statistics Summary---\n";
  printf "Total Dynamic Tasks: %d\n",$totDynTasks;
  printf "Avg. Task Size     : %d\n",$totUseful/$totComTasks 
      if($totComTasks!=0);
  printf "Useful/Total(U+S)  : %f\n",$totUseful/($totSquashed+$totUseful)
      if(($totSquashed+$totUseful) != 0);
}

sub showCCStats {
  my $file = shift;

  printf "###############################################################################\n";

  my $tot_rdSh  = 0;
  my $tot_wrSh  = 0;
  my $tot_nrdSh = 0;
  my $tot_nwrSh = 0;
  my $tot_rdHdl = 0;
  my $tot_wrHdl = 0;

  for(my $i=0; $i<$nCPUs; $i++) {
    my $readSharers    = 
      $cf->getResultField("(M${i})","readSharers");
    my $writeSharers   = 
      $cf->getResultField("(M${i})","writeSharers");
    my $readNoSharers  =
      $cf->getResultField("(M${i})","readNoSharers");
    my $writeNoSharers = 
      $cf->getResultField("(M${i})","writeNoSharers");
    my $readHandler = 
      $cf->getResultField("(M${i})","readHandlerCnt");
    my $writeHandler = 
      $cf->getResultField("(M${i})","writeReqCnt");

    $tot_rdSh  += $readSharers;
    $tot_wrSh  += $writeSharers;
    $tot_nrdSh += $readNoSharers;
    $tot_nwrSh += $writeNoSharers;
    $tot_rdHdl += $readHandler;
    $tot_wrHdl += $writeHandler;
  }

  my $readSharersPerReq  = $tot_rdSh / $tot_rdHdl;
  my $writeSharersPerReq = $tot_wrSh / $tot_wrHdl;

  my $readSharersNoZero = 0;
  if (($tot_rdHdl - $tot_nrdSh) gt 0) {
    $readSharersNoZero   = $tot_rdSh / ($tot_rdHdl - $tot_nrdSh);
  }

  my $writeSharersNoZero = 0;
  if (($tot_wrHdl - $tot_nwrSh) gt 0) {
    $writeSharersNoZero = $tot_wrSh / ($tot_wrHdl - $tot_nwrSh);
  }

  my $readFromMem  = $tot_nrdSh / $tot_rdHdl;
  my $writeFromMem = $tot_nwrSh / $tot_wrHdl;

  my $readPerc  = $tot_rdHdl / ($tot_rdHdl + $tot_wrHdl);
  my $writePerc = $tot_wrHdl / ($tot_rdHdl + $tot_wrHdl);

  printf "# Cache Coherence\n";

  printf "read_sharers:\t\t %9.2f\n", $tot_rdSh;
  printf "write_sharers:\t\t %9.2f\n", $tot_wrSh;
  printf "read_no_sharers:\t %9.2f\n", $tot_nrdSh;
  printf "write_no_sharers:\t %9.2f\n", $tot_nwrSh;
  printf "total_reads:\t\t %9.2f\n", $tot_rdHdl;
  printf "total_writes:\t\t %9.2f\n", $tot_wrHdl;
  printf "reads/total:\t\t %9.2f\n", $readPerc;
  printf "writes/total:\t\t %9.2f\n", $writePerc;
  printf "read_sharers/read:\t %9.2f\n", $readSharersPerReq;
  printf "write_sharers/write:\t %9.2f\n\n", $writeSharersPerReq;
  printf "read_sharers_nz/read:\t %9.2f\n", $readSharersNoZero;
  printf "write_sharers_nz/write:\t %9.2f\n", $writeSharersNoZero;
  printf "read_from_mem/reads:\t %9.2f\n", $readFromMem;
  printf "write_from_mem/writes:\t %9.2f\n\n", $writeFromMem;

  my $bench = $cf->getResultField("OSSim","bench");

  printf "res: $bench: %9.2f %9.2f %9.2f %9.2f\n", $readSharersNoZero, 
$writeSharersNoZero, $readFromMem, $writeFromMem;

}


sub showBaadStats {
    my $file = shift;
    
    printf "###############################################################################\n";

    printf "#baad0       BBSize   FetchSize  FetchBlock\n";
    printf "baad0  ";

    # BBSize
    printf " %9.2f ", $cf->getResultField("FetchEngine(0)","szBB_Avg");
    # FetchSize
    printf " %9.2f ", $cf->getResultField("FetchEngine(0)","szFS_Avg");
    # FetchBlock
    printf " %9.2f ", $cf->getResultField("FetchEngine(0)","szFB_Avg");
    printf "\n";
}

sub showStatReport {
  my $file = shift;

  printf "################################################################################\n";

  my $name = $file;
  $name =~ /.*sesc\_([^ ]*)......./;
  $name = $1;

  my $totCycles = 0;
  for(my $i=0;$i<$nCPUs;$i++) {
      $totCycles += $cf->getResultField("Proc(${i})","clockTicks");
  }

  my $nGradInsts  = $cf->getResultField("ProcessId","nGradInsts");
  my $nWPathInsts = $cf->getResultField("ProcessId","nWPathInsts");
  if ($nGradInsts == 0) {
    my $nInst;
    my $tmp;
    for(my $i=0;$i<$nCPUs;$i++) {
        $tmp += $cf->getResultField("FetchEngine(${i})","nDelayInst1");
        
        $nInst += $cf->getResultField("PendingWindow(${i})_iBJ","n")
	    + $cf->getResultField("PendingWindow(${i})_iLoad","n")
	    + $cf->getResultField("PendingWindow(${i})_iStore","n")
            + $cf->getResultField("PendingWindow(${i})_iALU","n")
            + $cf->getResultField("PendingWindow(${i})_iComplex","n")
            + $cf->getResultField("PendingWindow(${i})_fpALU","n")
            + $cf->getResultField("PendingWindow(${i})_fpComplex","n")
            + $cf->getResultField("PendingWindow(${i})_other","n");
    }
    $nGradInsts  = $nInst;
    $nWPathInsts = $tmp;
  }
  my $niKillGradInsts = $cf->getResultField("ProcessId","niKillGradInsts");
  my $nrKillGradInsts = $cf->getResultField("ProcessId","nrKillGradInsts");
  my $nKillWPathInsts    = $cf->getResultField("ProcessId","nrKillWPathInsts");
  my $nRestartGradInsts  = $cf->getResultField("ProcessId","nRestartGradInsts");
  my $nRestartWPathInsts = $cf->getResultField("ProcessId","nRestartWPathInsts");

  my $nTLSGradInsts  = $niKillGradInsts + $nrKillGradInsts + $nRestartGradInsts;
  my $nTLSWPathInsts = $nKillWPathInsts + $nRestartWPathInsts;

  #############################################################################
  printf "#table0                            BusyCPU %: CommitInst: nCycles : nWPathInsts : IPC\n";

  printf "table0  %25s ", $name;
  # BusyCPU
  printf " %9.2f ", 100*$totCycles/$nCycles;
  # CommitInst
  printf " %9.0f ", $nGradInsts;
  # nCycles
  printf " %9.0f ", $nCycles;
  # of wrong path instructions
  printf " %9.0f ", $nWPathInsts;
  # IPC
  printf " %9.3f ", $nGradInsts/$nCycles;
  printf "\n";

  #############################################################################
  print  "#table2a                                 Fetch    Issue      Mem     Exec    Clock    Total\n";
  my $txt = sprintf "table2a  %25s ", $name;
  WritePowerLine("PowerMgr", $txt, 1);

  #############################################################################
  my $totEnergy     = $cf->getResultField("EnergyMgr","totEnergy");
  if ($totEnergy) {
      my $coreRenEnergy = 0;
      my $coreWinEnergy = 0;
      my $coreROBEnergy = 0;
      my $coreBusEnergy = 0;
      my $coreRegEnergy = 0;
      my $lsqEnergy = 0;
      
      for(my $i=0;$i<$nCPUs;$i++) {
          last unless( $cf->getResultField("Proc(${i})","clockTicks") );
          
          my $cpuType    = $cf->getConfigEntry(key=>"cpucore",index=>$i);
          for (my $j=0; ; $j++) {
              my $clusterType = $cf->getConfigEntry(key=>"cluster", section=>$cpuType, index=>$j);
              last unless (defined $clusterType);
              
              $coreRenEnergy += $cf->getResultField("Proc(${i})","renameEnergy");

              $coreWinEnergy += $cf->getResultField("Proc(${i})_$clusterType","depTableEnergy"); # DDIS
              $coreWinEnergy += $cf->getResultField("Proc(${i})_$clusterType","winDepsEnergy");  # DDIS
              
              $coreWinEnergy += $cf->getResultField("Proc(${i})_$clusterType","windowRdWrEnergy");
              $coreWinEnergy += $cf->getResultField("Proc(${i})_$clusterType","windowCheckEnergy");
              $coreWinEnergy += $cf->getResultField("Proc(${i})_$clusterType","windowSelEnergy");
              
              $coreBusEnergy += $cf->getResultField("Proc(${i})_$clusterType","forwardBusEnergy");
              $coreBusEnergy += $cf->getResultField("Proc(${i})_$clusterType","resultBusEnergy");

          }
              
          $coreROBEnergy += $cf->getResultField("Proc(${i})","ROBEnergy");

          $coreRegEnergy += $cf->getResultField("Proc(${i})","wrIRegEnergy");
          $coreRegEnergy += $cf->getResultField("Proc(${i})","rdFPRegEnergy");
          $coreRegEnergy += $cf->getResultField("Proc(${i})","wrFPRegEnergy");
          $coreRegEnergy += $cf->getResultField("Proc(${i})","rdFPRegEnergy");

          foreach my $tag ("FULoad", "FUStore", "FUMemory") {
              $lsqEnergy += $cf->getResultField("${tag}(${i})","ldqCheckEnergy");
              $lsqEnergy += $cf->getResultField("${tag}(${i})","stqCheckEnergy");
              $lsqEnergy += $cf->getResultField("${tag}(${i})","ldqRdWrEnergy");
              $lsqEnergy += $cf->getResultField("${tag}(${i})","stqRdWrEnergy");
              $lsqEnergy += $cf->getResultField("${tag}(${i})","iALUEnergy");
          }
      }
      
      my $coreEnergy  =  $coreRenEnergy + $coreWinEnergy + $coreROBEnergy +$coreBusEnergy + $coreRegEnergy + $lsqEnergy;
      my $totPower    = $cf->getResultField("PowerMgr","totPower") *$slowdown;
  
      print  "#table2b                            Core (w) : Rename  : Window  : ROB  : Buses  : Regs  : LDSQ\n";
      
      printf "table2b %25s ", $name;
      # % Core of total
      printf " %9.3f ", $totPower*$coreEnergy/$totEnergy;
      # % Ren of core
      printf " %9.3f ", $totPower*$coreRenEnergy/$totEnergy;
      # % Win of core
      printf " %9.3f ", $totPower*$coreWinEnergy/$totEnergy;
      # % ROB of core
      printf " %9.3f ", $totPower*$coreROBEnergy/$totEnergy;
      # % Bus of core
      printf " %9.3f ", $totPower*$coreBusEnergy/$totEnergy;
      # % Reg of core
      printf " %9.3f ", $totPower*$coreRegEnergy/$totEnergy;
      # % LSQ of core
      printf " %9.3f ", $totPower*$lsqEnergy/$totEnergy;
      printf "\n";

      #############################################################################
      print  "#table2c                            TotEnergy : CoreEnergy\n";
      
      printf "table2c %25s ", $name;
      printf " %5.4g ", $totEnergy*1e-9;
      printf " %5.4g ", $coreEnergy*1e-9;
      printf "\n";
  }

  #############################################################################
  if ($cf->getResultField("Proc(0)_robUsed","n")) {
      printf "#table8                              ProcId : ROBuse %: LDQ Use %: STQ use %: c1 winUse % : c2 winUse % :...\n";

      for(my $i=0;$i<$nCPUs;$i++) {
          my $cpuType = $cf->getConfigEntry(key=>"cpucore",index=>$i);
          last unless (defined $cf->getConfigEntry(key=>"robSize",section=>$cpuType));
          
          printf "table8  %25s  %9d ", $name, $i;
          
          my $max;
          my $tmp;
          # ROBUse
          $max = $cf->getConfigEntry(key=>"robSize",section=>$cpuType);
          $tmp = $cf->getResultField("Proc(${i})_robUsed","v");
          printf " %9.2f ", 100*$tmp/$max;
          # LDQ Use
          $max = $cf->getConfigEntry(key=>"maxLoads",section=>$cpuType);
          $tmp = $cf->getResultField("FULoad(${i})_ldqNotUsed","v");
          printf " %9.2f ", 100*($max-$tmp)/$max;
          # STQ Use
          $max = $cf->getConfigEntry(key=>"maxStores",section=>$cpuType);
          $tmp = $cf->getResultField("FUStore(${i})_stqNotUsed","v");
          printf " %9.2f ", 100*($max-$tmp)/$max;
          
          for (my $j=0; ; $j++) {
              my $clusterType = $cf->getConfigEntry(key=>"cluster", section=>$cpuType, index=>$j);
              last unless (defined $clusterType);
              
              # Window Use
              $max = $cf->getConfigEntry(key=>"winSize",section=>$clusterType);
              $tmp = $cf->getResultField("Proc(${i})_${clusterType}_winNotUsed","v");
              $tmp = $max if ($cf->getResultField("Proc(${i})_${clusterType}_winNotUsed","n") == 0); # Not used
              printf " %9.2f ", 100*($max-$tmp)/$max;
          }
          ##########
          printf "\n";
      }

  #############################################################################
      for(my $i=0;$i<$nCPUs;$i++) {
          my $clockTicks= $cf->getResultField("Proc(${i})","clockTicks");
          next unless( $clockTicks > 1 );

          printf "#table9a                              IPC : szFB : szBB : brMiss : brMissTime : iMissRate \n";
          printf "table9a  %26s ", $name;

          my $nInst   = $cf->getResultField("PendingWindow(${i})_iBJ","n")
              + $cf->getResultField("PendingWindow(${i})_iLoad","n")
 	      + $cf->getResultField("PendingWindow(${i})_iStore","n")
              + $cf->getResultField("PendingWindow(${i})_iALU","n")
              + $cf->getResultField("PendingWindow(${i})_iComplex","n")
              + $cf->getResultField("PendingWindow(${i})_fpALU","n")
              + $cf->getResultField("PendingWindow(${i})_fpComplex","n")
              + $cf->getResultField("PendingWindow(${i})_other","n");

          # IPC
          printf " %9.3f ", $nInst/$clockTicks;

          # szFB
          my $nTaken    = $cf->getResultField("BPred(${i})","nTaken");
          printf " %9.2f ", $nInst/$nTaken;

          # szBB
          my $nBranches = $cf->getResultField("BPred(${i})","nBranches");
          printf " %9.2f ", $nInst/$nBranches;

          # branchMissRate
          my $nMiss = $cf->getResultField("BPred(${i})","nMiss");
          printf " %9.2f ", 100*$nMiss/$nBranches;

          # brMissTime
          printf " %9.2f ", $cf->getResultField("FetchEngine(${i})_avgBranchTime","v");

          # icache miss
          my $iaccess = $cf->getResultField("P(${i})_IL1","readHalfMiss") + 
              $cf->getResultField("P(${i})_IL1","readMiss") + 
              $cf->getResultField("P(${i})_IL1","readHit") + 
              $cf->getResultField("P(${i})_IL1","writeHalfMiss") + 
              $cf->getResultField("P(${i})_IL1","writeMiss") + 
              $cf->getResultField("P(${i})_IL1","writeHit");

         $iaccess = 1 unless ($iaccess);

          printf " %9.2f ", 100*($cf->getResultField("P(${i})_IL1","readMiss")+$cf->getResultField("P(${i})_IL1","writeMiss")) /$iaccess;
          
          printf "\n";

          my $nDepsOverflow = 1;
          my $nDeps_0;
          my $nDeps_1;
          my $nDeps_2;
          my $nDepsMiss;
              
          my $cpuType = $cf->getConfigEntry(key=>"cpucore",index=>$i);
          for (my $j=0; ; $j++) {
              my $clusterType = $cf->getConfigEntry(key=>"cluster", section=>$cpuType, index=>$j);
              last unless (defined $clusterType);
              
              $nDepsOverflow += $cf->getResultField("Proc(${i})_${clusterType}_depTable","nDepsOverflow");

              $nDeps_0   += $cf->getResultField("Proc(${i})_${clusterType}_depTable","nDeps_0");
              $nDeps_1   += $cf->getResultField("Proc(${i})_${clusterType}_depTable","nDeps_1");
              $nDeps_2   += $cf->getResultField("Proc(${i})_${clusterType}_depTable","nDeps_2");
              $nDepsMiss += $cf->getResultField("Proc(${i})_${clusterType}_depTable","nDepsMiss");
          }

          next unless ($nDeps_0 > 1);

          printf "#table9                              ProcID: IPC : 0 deps : 1 deps : 2 deps : %re-dispatcch :#Overflows :L1 hit Predictor Accuracy\n";
          # ProcID
          printf "table9  %26s  %9d : ", $name, $i;


          my $bench = $name;
          $bench =~ /([^ _]*).mips$/;
          $bench = $1;

          printf " %-10s ", $bench;

          # IPC
          printf " & %9.3f ", $nInst/$clockTicks;

          # nDeps
          printf " & %9.1f ", 100*$nDeps_0/($nDeps_0 + $nDeps_1 + $nDeps_2);
          printf " & %9.1f ", 100*$nDeps_1/($nDeps_0 + $nDeps_1 + $nDeps_2);
          printf " & %9.1f ", 100*$nDeps_2/($nDeps_0 + $nDeps_1 + $nDeps_2);

          # re-dispatch
          printf " & %9.1f ", 100*$nDepsMiss/($nDeps_0 + $nDeps_1 + $nDeps_2);

          # cycles between overflows
          if ($clockTicks/$nDepsOverflow > 50e3) {
              printf " & \$>\$50k ";
          }else{
              printf " & %9.0f ", $clockTicks/$nDepsOverflow;
          }

          print "\\\\ :";

          printf " %-10s ", $bench;

          my $daccess = $cf->getResultField("P(${i})_DL1","readHalfMiss") + 
              $cf->getResultField("P(${i})_DL1","readMiss") + 
              $cf->getResultField("P(${i})_DL1","readHit") + 
              $cf->getResultField("P(${i})_DL1","writeHalfMiss") + 
              $cf->getResultField("P(${i})_DL1","writeMiss") + 
              $cf->getResultField("P(${i})_DL1","writeHit");

          printf " & %9.2f ", 100*($cf->getResultField("P(${i})_DL1","readMiss")
                                  +$cf->getResultField("P(${i})_DL1","writeMiss")
                                  )/$daccess;

          my $nL1Hit_pHit   = $cf->getResultField("L1Pred","nL1Hit_pHit");
          my $nL1Hit_pMiss  = $cf->getResultField("L1Pred","nL1Hit_pMiss");
          my $nL1Miss_pHit  = $cf->getResultField("L1Pred","nL1Miss_pHit");
          my $nL1Miss_pMiss = $cf->getResultField("L1Pred","nL1Miss_pMiss");

          my $total = $nL1Miss_pMiss+$nL1Miss_pHit+$nL1Hit_pMiss+$nL1Hit_pHit+1;

          printf " & %9.2f ", 100*($nL1Miss_pMiss+$nL1Hit_pHit)/$total;
          printf " & %9.2f ", 100*$nL1Hit_pMiss /$total;
          printf " & %9.2f ", 100*$nL1Miss_pHit /$total;

          print "\\\\";

          print "\n";
      }
  }

}

sub showTLSReport {
  my $file = shift;

  printf "################################################################################\n";

  my $name = $file;
  $name =~ /.*sesc\_([^ ]*)......./;
  $name = $1;

  my $totCycles = 0;
  for(my $i=0;$i<$nCPUs;$i++) {
      $totCycles += $cf->getResultField("Proc(${i})","clockTicks");
  }

  my $nGradInsts  = $cf->getResultField("ProcessId","nGradInsts");
  my $nWPathInsts = $cf->getResultField("ProcessId","nWPathInsts");
  my $niKillGradInsts = $cf->getResultField("ProcessId","niKillGradInsts");
  my $nrKillGradInsts = $cf->getResultField("ProcessId","nrKillGradInsts");
  my $nKillWPathInsts    = $cf->getResultField("ProcessId","nrKillWPathInsts");
  my $nRestartGradInsts  = $cf->getResultField("ProcessId","nRestartGradInsts");
  my $nRestartWPathInsts = $cf->getResultField("ProcessId","nRestartWPathInsts");

  my $nTLSGradInsts  = $niKillGradInsts + $nrKillGradInsts + $nRestartGradInsts;
  my $nTLSWPathInsts = $nKillWPathInsts + $nRestartWPathInsts;

  printf "#table1                       BusyCPU %: UsefulCPU%: iUseful % : iDataKill% : iResKill% : iRestart %: CommitInst: nCycles : nWPathInsts\n";

  printf "table1  %25s ", $name;
  # BusyCPU
  printf " %9.2f ", 100*$totCycles/$nCycles;
  my $iUseful = $nGradInsts/($nGradInsts+$nTLSGradInsts);
  # usefulCPU (% of useful cycles)
  printf " %9.2f ", 100*$iUseful*$totCycles/$nCycles;
  # iUseful
  printf " %9.2f ", 100*$iUseful;
  # iDataKill
  printf " %9.2f ", 100*$niKillGradInsts/($nGradInsts+$nTLSGradInsts);
  # iResKill
  printf " %9.2f ", 100*$nrKillGradInsts/($nGradInsts+$nTLSGradInsts);
  # iRestart
  printf " %9.2f ", 100*$nRestartGradInsts/($nGradInsts+$nTLSGradInsts);
  # CommitInst
  printf " %9.0f ", $nGradInsts;
  # nCycles
  printf " %9.0f ", $nCycles;
  # of wrong path instructions
  printf " %d ", $nWPathInsts;
  printf "\n";

#  return if( $cf->getResultField("HVersion","nCreate") < 2);

  printf "#table3                    Tot-ooSpawns%%:Co-ooSpawns%%:  mNext/C  :  mLast/C  :  kills/C  : restarts/C : nTot-ooSpawn : squash/spawn\n";
  printf "table3  %25s ", $name;

  my $totSpawn = $cf->getResultField("TC","nInOrderSpawn") 
      + $cf->getResultField("TC","nOutOrderSpawn");
  $totSpawn = 1 unless ($totSpawn);
  # Tot-ooSpawns (Total number of out-of-order spawns)
  printf " %9.2f ", 100*$cf->getResultField("TC","nOutOrderSpawn")/ $totSpawn;

  my $nCommited = $cf->getResultField("TC","nCorrectInOrderSpawn")
      + $cf->getResultField("TC","nCorrectOutOrderSpawn");
  $nCommited = 1 unless ($nCommited);
  # CooSpawns (Total number of correct, commited,out-of-order spawns)
  printf " %9.2f ", 100 *$cf->getResultField("TC","nCorrectOutOrderSpawn")/$nCommited;

  my $nKills    = $cf->getResultField("ProcessId","niKills") 
      + $cf->getResultField("ProcessId","nrKills");
  my $nRestarts = $cf->getResultField("ProcessId","nRestarts");

  # MergeNext per task commit (TODO: Remove nMergeFirst. Now it is kept
  # until all the simulations are converted to nMergeNext)
  printf " %9.4f ", $cf->getResultField("TC","nMergeFirst") + $cf->getResultField("TC","nMergeNext") / $nCommited;
  # MergeLast per task commit
  printf " %9.4f ", $cf->getResultField("TC","nMergeLast")  / $nCommited;
  # kills per task commit
  printf " %9.4f ", $nKills / $nCommited;
  # restarts per task commit
  printf " %9.4f ",  $nRestarts / $nCommited;
  # total number of OO spawns
  printf " %9.4f ",  $cf->getResultField("TC","nOutOrderSpawn");
  # restarts+Kills per task 
  printf " %9.4f ",  ($nRestarts+$nKills) / ($nCommited+$nRestarts+$nKills);

  print "\n";

  print  "#table4                      nTasksAhd :TotSpawns/C  : Sp in Lft \n";
  printf "table4  %25s ", $name;
  # number of tasks ahead TC_nTasksAhead
  my $nTasksAhead = $cf->getResultField("TC_nTasksAhead","v");
  printf " %9.5f ", $nTasksAhead;
  printf " %9.7f ", $totSpawn / $nCommited;

  my $nCSMax = $cf->getResultField("HVersion","nChildrenStatsMax");
  my $totNZeroChildren;
  for(my $i=1;$i<$nCSMax-1;$i++) {
    $totNZeroChildren += $cf->getResultField("HVersion(${i})","nChildren");
  }

  $totNZeroChildren =1 if ($totNZeroChildren == 0);
  printf " %9.7f ", $totSpawn / $totNZeroChildren;

  print "\n";

  print  "#table5                     TaskSizeClk   ClaimDist ReleaseDist MergeSuccDist %MissFetch %%EnInMV\n";
  printf "table5  %25s ", $name;
  printf " %10.1f ", $nGradInsts/$nCommited;
  my $tmp = $cf->getResultField("HVersion","nClaim");
  if ($tmp == 0) {
    printf "       NA ";
  }else{
    printf " %9.1f ", $totCycles/$tmp;
  }
  $tmp = $cf->getResultField("HVersion","nRelease");
  if ($tmp == 0) {
    printf "       NA ";
  }else{
    printf " %9.1f ", $totCycles/$tmp;
  }
  $tmp = $cf->getResultField("TC","nMergeSuccessors");
  if ($tmp == 0) {
    printf "       NA ";
  }else{
    printf " %9.1f ", $totCycles/$tmp;
  }

  # MissFetch insts
  my $tmp;
  my $nInst;
  for(my $i=0;$i<$nCPUs;$i++) {
      $tmp += $cf->getResultField("FetchEngine(${i})","nDelayInst1");

      $nInst += $cf->getResultField("PendingWindow(${i})_iBJ","n")
          + $cf->getResultField("PendingWindow(${i})_iLoad","n")
	  + $cf->getResultField("PendingWindow(${i})_iStore","n")
          + $cf->getResultField("PendingWindow(${i})_iALU","n")
          + $cf->getResultField("PendingWindow(${i})_iComplex","n")
          + $cf->getResultField("PendingWindow(${i})_fpALU","n")
          + $cf->getResultField("PendingWindow(${i})_fpComplex","n")
          + $cf->getResultField("PendingWindow(${i})_other","n");
  }
  printf " %8.2f%% ", 100*$tmp/($nInst+$tmp);
  printf " %9.1f ", calcPMVStructEnergy();
  print "\n";

  my $nChildrenStatsMax = $cf->getResultField("HVersion","nChildrenStatsMax");
  my $total;

  print "#table6 nChildren        :";
  for(my $i=0;$i<$nChildrenStatsMax-1;$i++) {
    $total += $cf->getResultField("HVersion(${i})","nChildren");
    printf " %9.0f " , $i;
  }
  $total =1 if ($total == 0);
  print "   More  \n";

  printf "table6  %25s ", $name;
  for(my $i=0;$i<$nChildrenStatsMax;$i++) {
    $tmp = $cf->getResultField("HVersion(${i})","nChildren");
    printf " %9.1f ", 100*$tmp/$total;
  }
  print "\n";

  print "#table7         cavaCyc:vpacc:cavaIPC:avgCkpDist(inst):ckpSize(inst):ckpSize(pred):\%wastedInst:ckpFailed:nCommitInsts:nCkps:L2missrate\n";
  my $totIPC = ($nGradInsts+$nTLSGradInsts)/$nCycles;
  my $cavaCyc = $nCycles; 
  my $nPreds = $cf->getResultField("VP","actualHit") + $cf->getResultField("VP","actualMiss");
  $nPreds = 1 if ($nPreds == 0);
  my $vpacc = $cf->getResultField("VP","actualHit")/$nPreds;
  my $nPredsFetch = $cf->getResultField("VPSEL_GLVaBHLV","hit") + 
      $cf->getResultField("VPSEL_GLVaBHLV","miss");
  $nPredsFetch = 1 if ($nPredsFetch == 0);
  my $vpaccFetch = $cf->getResultField("VPSEL_GLVaBHLV","hit")/$nPredsFetch;
  printf "table7  %25s ", $name;
  printf " %9.0f  %9.3f ", $cavaCyc, $vpaccFetch;

  my $cavaIPC = $nGradInsts/$cavaCyc;
  printf " %9.2f ", $cavaIPC;
  
  my $nCkps = $cf->getResultField("TC", "nDiscards") + 
              $cf->getResultField("TC", "nRestores");
  $nCkps = 1 if ($nCkps == 0);

  printf "     %5.0f ", ($nGradInsts+$nTLSGradInsts)/$nCkps;

  printf " %9.0f ", $cf->getResultField("VP_avgSizeInsts", "v");
  printf " %9.1f ", $cf->getResultField("VP_avgOutsPreds", "v");
  printf " %9.2f ", $nTLSGradInsts/($nGradInsts+$nTLSGradInsts);
  printf " %9.2f ", $cf->getResultField("TC", "nRestores")/$nCkps;
  printf " %9.0f ", $nGradInsts;
  printf " %9.0f ", $nCkps;

  # ugly code. it will be removed soon
  my $daccess = $cf->getResultField("P(0)_DL1","readHalfMiss") + 
      $cf->getResultField("P(0)_DL1","readMiss") + 
      $cf->getResultField("P(0)_DL1","readHit") + 
      $cf->getResultField("P(0)_DL1","writeHalfMiss") + 
      $cf->getResultField("P(0)_DL1","writeMiss") + 
      $cf->getResultField("P(0)_DL1","writeHit");

  my $l2miss = $cf->getResultField("L2","readMiss") 
      + $cf->getResultField("L2","writeMiss") ;
  printf " %9.4f\n", $l2miss/$daccess;
  
}

my $pmvEnergy;
my $ntrEnergy;

sub calcPMVStructEnergy {
    my $memEnergy  = $cf->getResultField("EnergyMgr","memEnergy") + 
        $cf->getResultField("EnergyMgr","fetchEnergy");

    return if ($memEnergy == 0);

    my $mvEnergy = 0;
    
#L1: extra tag energy, LVID, revLVID    
    for(my $i=0;$i<$nCPUs;$i++) {
        my $cpuType = $cf->getConfigEntry(key=>"cpucore",index=>$i);
        
        my $dataSource = $cf->getConfigEntry(key=>"dataSource",section=>$cpuType);
        
        my @name = split(/ +/,$dataSource);
        next unless (defined $name[1]);
        
        my $total =
            $cf->getResultField("P(${i})_${name[1]}","rdHitEnergy")
            + $cf->getResultField("P(${i})_${name[1]}","rdMissEnergy")
            + $cf->getResultField("P(${i})_${name[1]}","wrHitEnergy")
            + $cf->getResultField("P(${i})_${name[1]}","wrMissEnergy")
            + $cf->getResultField("P(${i})_${name[1]}","lineFillEnergy");
        
        $mvEnergy += $total * 0.03; # extra tag energy (guessing 3% of total cache energy)
        $mvEnergy += $cf->getResultField("P(${i})_${name[1]}","rdLVIDEnergy");
        $mvEnergy += $cf->getResultField("P(${i})_${name[1]}","wrLVIDEnergy");
        $mvEnergy += $cf->getResultField("P(${i})_${name[1]}","rdRevLVIDEnergy");
        $mvEnergy += $cf->getResultField("P(${i})_${name[1]}","wrRevLVIDEnergy");
    }

#MVC all of it
    $mvEnergy += $cf->getResultField("MVVCache","rdHitEnergy");
    $mvEnergy += $cf->getResultField("MVVCache","rdMissEnergy");
    $mvEnergy += $cf->getResultField("MVVCache","wrHitEnergy");
    $mvEnergy += $cf->getResultField("MVVCache","wrMissEnergy");
    $mvEnergy += $cf->getResultField("MVVCache","lineFillEnergy");
    $mvEnergy += $cf->getResultField("MVVCache","rdLVIDEnergy");
    $mvEnergy += $cf->getResultField("MVVCache","wrLVIDEnergy");
    $mvEnergy += $cf->getResultField("MVVCache","rdRevLVIDEnergy");
    $mvEnergy += $cf->getResultField("MVVCache","wrRevLVIDEnergy");

    $memEnergy = 1 if ($memEnergy == 0);
    $pmvEnergy = ($mvEnergy/$memEnergy) * 100;
    return $pmvEnergy;
}

sub simStats {
  my $file = shift;

  # Begin Global Stats
  $cpuType = $cf->getConfigEntry(key=>"cpucore");

  my $techSec = $cf->getConfigEntry(key=>"technology");

  $freq = $cf->getConfigEntry(key=>"frequency",section=>$techSec) / 1e6;
  # Old configuration type
  $freq = $cf->getConfigEntry(key=>"frequency",section=>$cpuType) / 1e6 unless($freq);
  $freq = 1e3 unless($freq);

  $nCPUs= $cf->getResultField("OSSim","nCPUs");
  unless( defined $nCPUs ) {
      print "Configuration file [$file] has a problem\n";
      next;
  }

  $nCycles = $cf->getResultField("OSSim","nCycles");
  next unless ($nCycles);
  
  for (my $j=0; ; $j++) {
      my $clusterType = $cf->getConfigEntry(key=>"cluster", section=>$cpuType, index=>$j);
      last unless (defined $clusterType);
      next unless ($cf->getResultField("Proc(0)_${clusterType}_depTable","nDepsOverflow") > 1);
      
      my $clk = 10*$cf->getResultField("Proc(0)_${clusterType}_depTable","nDepsOverflow");
      $slowdown = $nCycles/($nCycles+$clk);
      $nCycles += $clk;
      
      die "Must compute overflow in a different way" if ($nCPUs != 1);
  }
  $slowdown = 1 unless (defined $slowdown);
  
  $nLoadTotal  = 0;
  $nStoreTotal = 0;
  $nInstTotal  = 0;
  for(my $i=0;$i<$nCPUs;$i++) {
      $nLoadTotal  += $cf->getResultField("PendingWindow(${i})_iLoad","n");
      $nStoreTotal += $cf->getResultField("PendingWindow(${i})_iStore","n");
      
      $nInstTotal   += $cf->getResultField("PendingWindow(${i})_iBJ","n")
          + $cf->getResultField("PendingWindow(${i})_iALU","n")
          + $cf->getResultField("PendingWindow(${i})_iComplex","n")
          + $cf->getResultField("PendingWindow(${i})_fpALU","n")
          + $cf->getResultField("PendingWindow(${i})_fpComplex","n")
          + $cf->getResultField("PendingWindow(${i})_other","n");
  }
  $nInstTotal += $nLoadTotal + $nStoreTotal;

  # End Global Stats

  print "      Exe Speed        Exe MHz         Exe Time         Sim Time (${freq}MHz)\n";

  my $secs    = $cf->getResultField("OSSim","msecs");

  $secs = 1 if( $secs == 0 );

  printf " %10.3f KIPS ",($nInstTotal/($secs*1000));
  printf " %10.4f MHz ",1e-6/($secs/$nCycles);

  printf " %10.3f secs ",$secs;

  printf " %10.3f msec",(1e-3/$freq)*$nCycles;
  printf " (rabbit)" if( $cf->getResultField("OSSim","rabbit") );

  print "\n";
}

sub instStats {
  my $file = shift;

  print "           nInst     BJ    Load   Store      INT      FP  : LD Forward , Replay : Worst Unit (clk)\n";


  for(my $i=0;$i<$nCPUs;$i++) {
      next unless( $cf->getResultField("Proc(${i})","clockTicks") );
      printf " %3d ",$i;

      my $iBJ    = $cf->getResultField("PendingWindow(${i})_iBJ","n");
      my $iLoad  = $cf->getResultField("PendingWindow(${i})_iLoad","n");
      my $iStore = $cf->getResultField("PendingWindow(${i})_iStore","n");
      my $INT    = $cf->getResultField("PendingWindow(${i})_iALU","n")
	+ $cf->getResultField("PendingWindow(${i})_iComplex","n");
      my $FP     = $cf->getResultField("PendingWindow(${i})_fpALU","n")
	+ $cf->getResultField("PendingWindow(${i})_fpComplex","n");
      
      my $nFor   = $cf->getResultField("FULoad(${i})","nForwarded");
      
      my $nInst = $iBJ + $iLoad + $iStore + $INT + $FP
	+ $cf->getResultField("PendingWindow(${i})_other","n");
      
      $nInst = 1 if ($nInst == 0);

      printf " %10.0f " ,$nInst;
      printf " %5.2f%% ",100*$iBJ/$nInst;
      printf " %5.2f%% ",100*$iLoad/$nInst;
      printf " %5.2f%% ",100*$iStore/$nInst;
      printf " %5.2f%% ",100*$INT/$nInst;
      printf " %5.2f%% ",100*$FP/$nInst;
      
      $iLoad = 1 if( $iLoad == 0 );
      printf " :     %5.2f%%",100*$nFor/$iLoad;
      
      my $cpuType    = $cf->getConfigEntry(key=>"cpucore",index=>$i);
      my $worstUnit;
      my $worstValue = 0;
      my $nReplay = 0;
      for (my $j=0; ; $j++) {
	my $clusterType = $cf->getConfigEntry(key=>"cluster", section=>$cpuType, index=>$j);
	last unless (defined $clusterType);
	
	$nReplay += $cf->getResultField("Proc(${i})_${clusterType}","nReplay");
	
	foreach my $unitID ("iBJUnit", "iLoadUnit", "iStoreUnit", "iALUUnit"
			    , "iMultUnit", "iDivUnit", "fpALUUnit", "fpMultUnit", "fpDivUnit") {
	  
	  my $tmp     = $cf->getConfigEntry(key=>$unitID, section=>$clusterType);
	  next unless (defined $tmp);
	  
	  my $val     = $cf->getResultField("${tmp}(${i})_occ","v");
	  
	  if ($val > $worstValue ) {
	    $worstValue = $val;
	    $worstUnit  = $tmp;
	  }
	}
      }
      
      
    if ($nReplay) {
	printf "   %5.0f inst/repl ",$nInst/$nReplay;
    }else{
	printf "   ????  inst/repl ";
    }

    printf " :  ${worstUnit} %4.2f ",$worstValue;

    print "\n";
  }
}

sub branchStats {
  my $file = shift;

  print "Proc  Avg.Time BPType       Total          RAS           BPred          BTB            BTAC";
  my $preType = $cf->getConfigEntry(key=>"preType");
  if( $preType > 0 ) {
    print "         " . $preType;
  }
  print "\n";

  for(my $i=0;$i<$nCPUs;$i++) {
    next unless( $cf->getResultField("Proc(${i})","clockTicks") );

    my $cpuType    = $cf->getConfigEntry(key=>"cpucore",index=>$i);

    my $branchSect  = $cf->getConfigEntry(key=>"bpred", section=>$cpuType);
    my $type        = $cf->getConfigEntry(key=>"type" , section=>$branchSect);
    my $smtContexts  = $cf->getConfigEntry(key=>"smtContexts",section=>$cpuType);
    $smtContexts++ if( $smtContexts == 0 );

    printf " %3d  ",$i;
    
    ################
    my $nBranches = 0;
    my $nMiss     = 0;
    my $avgBranchTime=0;
    for(my $j=0;$j<$smtContexts;$j++) {
      my $id = $i*$smtContexts+$j;

      $nBranches += $cf->getResultField("BPred(${id})","nBranches");
      $nMiss     += $cf->getResultField("BPred(${id})","nMiss");
      $avgBranchTime += $cf->getResultField("FetchEngine(${id})_avgBranchTime","v");
    }
    $avgBranchTime /= $smtContexts;
    if( $nBranches == 0 ) {
      $nBranches = 1;
    }

    printf " %5.3f ",$avgBranchTime;

    printf " %-9s ",$type;

    printf " %7.2f%% ",100*($nBranches-$nMiss)/($nBranches);

    ################
    my $rasHit  = 0;
    my $rasMiss = 0;
    for(my $j=0;$j<$smtContexts;$j++) {
      my $id = $i*$smtContexts+$j;
      $rasHit  += $cf->getResultField("BPred(${id})_RAS","nHit");
      $rasMiss += $cf->getResultField("BPred(${id})_RAS","nMiss");
    }

    my $rasRatio = ($rasMiss+$rasHit) <= 0 ? 0 : ($rasHit/($rasMiss+$rasHit));

    printf "(%6.2f%% of %6.2f%%) ",100*$rasRatio ,100*($rasHit+$rasMiss)/$nBranches;

    ################
    my $predHit  = $cf->getResultField("BPred(${i})_${type}","nHit");
    my $predMiss = $cf->getResultField("BPred(${i})_${type}","nMiss");

    my $predRatio = ($predMiss+$predHit) <= 0 ? 0 : ($predHit/($predMiss+$predHit));

    printf "%6.2f%% ",100*$predRatio;

    ################
    my $btbHit  = $cf->getResultField("BPred(${i})_BTB","nHit");
    my $btbMiss = $cf->getResultField("BPred(${i})_BTB","nMiss");

    my $btbRatio = ($btbMiss+$btbHit) <= 0 ? 0 : ($btbHit/($btbMiss+$btbHit));

    printf "(%6.2f%% of %6.2f%%) ",100*$btbRatio ,100*($btbHit+$btbMiss)/$nBranches;

    my $nBTAC = 0;
    for(my $j=0;$j<$smtContexts;$j++) {
      my $id = $i*$smtContexts+$j;

      $nBTAC += $cf->getResultField("FetchEngine(${id})","nBTAC");
    }
    printf "%6.2f%% ",100*$nBTAC/$nBranches;
    ################

    my $rapHit  = $cf->getResultField("BPred(${i})_Rap","nHit")
      + $cf->getResultField("BPred(${i})_CRap","nHit");
    if( $rapHit ) {
      my $rapMiss = $cf->getResultField("BPred(${i})_Rap","nMiss")
        + $cf->getResultField("BPred(${i})_CRap","nMiss");

      my $rapRatio = ($rapMiss+$rapHit) <= 0 ? 0 : ($rapHit/($rapMiss+$rapHit));

      printf "(%6.2f%% of %6.2f%%) ",100*$rapRatio ,100*($rapHit+$rapMiss)/$nBranches;
    }

    print "\n";
  }
}


sub tradCPUStats {
  my $file = shift;

  my $active=0;
  for(my $i=1;$i<$nCPUs;$i++) {
    $active += $cf->getResultField("Proc(${i})","clockTicks");
  }
  if( $active < $cf->getResultField("Proc(0)","clockTicks")/100 ) {
    $active = 0;
  }

  print "Proc  IPC  ";
  print "Active " if( $active > 0 );
  print "      Cycles  Busy   LDQ   STQ  IWin   ROB";
  print "  Regs Ports   TLB ";
  print " maxBr MisBr Br4Clk  Other\n";

  my $cycles= $cf->getResultField("OSSim","nCycles");
  $cycles=1 if( $cycles < 1 );

  for(my $i=0;$i<$nCPUs;$i++) {

    my $clockTicks= $cf->getResultField("Proc(${i})","clockTicks");
    next unless( $clockTicks > 1 );

    my $nInst   = $cf->getResultField("PendingWindow(${i})_iBJ","n")
        + $cf->getResultField("PendingWindow(${i})_iLoad","n")
        + $cf->getResultField("PendingWindow(${i})_iStore","n")
        + $cf->getResultField("PendingWindow(${i})_iALU","n")
        + $cf->getResultField("PendingWindow(${i})_iComplex","n")
        + $cf->getResultField("PendingWindow(${i})_fpALU","n")
        + $cf->getResultField("PendingWindow(${i})_fpComplex","n")
        + $cf->getResultField("PendingWindow(${i})_other","n");


    my $cpuType = $cf->getConfigEntry(key=>"cpucore",index=>$i);
    my $fetch   = $cf->getConfigEntry(key=>"fetchWidth",section=>$cpuType);
    my $issue = $fetch;
    my $smtContexts  = $cf->getConfigEntry(key=>"smtContexts",section=>$cpuType);
    $smtContexts++ if( $smtContexts == 0 );

    my $temp;

    $temp = $cf->getConfigEntry(key=>"issueWidth",section=>$cpuType);
    $issue = $temp if( $temp < $issue && $temp );

    ##########################
    # pid, IPC

    printf " %3d  %3.2f",$i,$nInst/$clockTicks;
    printf " %6.2f",100*$clockTicks/$cycles if( $active > 0 );
    printf " %12.0f ",$clockTicks;

    ##########################
    # Substract BUSY time

    $nInst = 1 if( $nInst < 1 );

    my $idealInst = $issue*$clockTicks;
    $temp = 100*$nInst/($idealInst);

    printf " %4.1f ",$temp;

    my $remaining = 100; # 100% of the time
    $remaining-=$temp;

    ##########################
    # Window %

    my $nOutsLoads = $cf->getResultField("ExeEngine(${i})","nOutsLoads");
    $temp = 100*$nOutsLoads/$idealInst;
    printf " %4.1f ",$temp;
    $remaining -= $temp;

    my $nOutsStores = $cf->getResultField("ExeEngine(${i})","nOutsStores");
    $temp = 100*$nOutsStores/$idealInst;
    printf " %4.1f ",$temp;
    $remaining -= $temp;

    my $nSmallWin = $cf->getResultField("ExeEngine(${i})","nSmallWin");
    $temp = 100*$nSmallWin/$idealInst;
    printf " %4.1f ",$temp;
    $remaining -= $temp;

    my $nSmallROB = $cf->getResultField("ExeEngine(${i})","nSmallROB");
    $temp = 100*$nSmallROB/$idealInst;
    printf " %4.1f ",$temp;
    $remaining -= $temp;

    my $nSmallREG = $cf->getResultField("ExeEngine(${i})","nSmallREG");
    $temp = 100*$nSmallREG/$idealInst;
    printf " %4.1f ",$temp;
    $remaining -= $temp;

    my $portConflich = $cf->getResultField("ExeEngine(${i})","PortConflict");
    $temp = 100*$portConflich/$idealInst;
    printf " %4.1f ",$temp;
    $remaining -= $temp;

    my $TLB = $cf->getResultField("P(${i})","TLBTime");
    $temp = 100*$issue*$TLB/$idealInst;
    printf " %4.1f ",$temp;
    $remaining -= $temp;

    my $nOutsBranches = $cf->getResultField("ExeEngine(${i})","nOutsBranches");
    $temp = 100*$nOutsBranches/$idealInst;
    printf " %4.1f ",$temp;
    $remaining -= $temp;

    ##########################
    # The rest is the control %

    my $nDelayInst1 = 0;
    my $nDelayInst2 = 0;
    for(my $j=0;$j<$smtContexts;$j++) {
      my $id = $i*$smtContexts+$j;

      $nDelayInst1  += $cf->getResultField("FetchEngine(${i})","nDelayInst1");
      $nDelayInst2  += $cf->getResultField("FetchEngine(${i})","nDelayInst2");
    }

    $temp = $remaining*$nDelayInst1/($nDelayInst1+$nDelayInst2+1);
    $remaining -= $temp;
    printf " %5.1f ",$temp;

    $temp = $remaining*$nDelayInst2/($nDelayInst1+$nDelayInst2+1);
    $remaining -= $temp;
    printf " %5.1f ",$temp;

    printf " %5.1f ",$remaining;

    print "\n";
  }

}

sub atomicStats {
  my $file = shift;

  printf "################################################################################\n";

  printf "Atomic stats:\n";

  my $ntrans = $cf->getResultField("AT","nTransactions");
  my $nsquashes = $cf->getResultField("AT","nSquashes");
  my $transtime = 0;

  printf "Total transactions (squashes) = %d (%d)\n", $ntrans, $nsquashes;
  printf "Atomic cycles = %d\n", $cf->getResultField("AT","transTime");
}

sub lockStats {
  my $file = shift;

  printf "################################################################################\n";

  printf "Lock stats:\n";

  my $time = $cf->getResultField("LOCK","Time");
  my $count = $cf->getResultField("LOCK","Count");

  printf "Lock time (instances) = %d (%d)\n", $time, $count/2;
  printf "Lock total time = %d\n", $cf->getResultField("LOCK","OccTime");
}

sub showCGStats {
  my $file = shift;
  my @name = split(/[_\.]/,$file);

  printf "################################################################################\n";

  printf "CriticalityGraph stats:\n";

  my $nTotCriticalTasks = $cf->getResultField("CG", "nTotCriticalTasks");
  my $nMidCriticalTasks = $cf->getResultField("CG", "nMidCriticalTasks");
  my $nNonCriticalTasks = $cf->getResultField("CG", "nNonCriticalTasks");
  my $tmpTotal = $nTotCriticalTasks+$nNonCriticalTasks+$nMidCriticalTasks;

  if ($tmpTotal) {
    printf "Crit1:    \%TotCriticalTasks \%MidCriticalTasks \%NonCriticalTask\n";
    printf "Crit1: %8s %9.2f %18.2f %18.2f\n", $name[3],
            100*$nTotCriticalTasks/$tmpTotal,
            100*$nMidCriticalTasks/$tmpTotal,
            100*$nNonCriticalTasks/$tmpTotal;
  }        
  
  my $nCorrectCauseSquash = $cf->getResultField("CG", "nCorrectCauseSquash");
  my $nIncorrectCauseSquash = $cf->getResultField("CG", "nIncorrectCauseSquash");
  my $nCorrectNoSquash = $cf->getResultField("CG", "nCorrectNoSquash");
  my $nIncorrectNoSquash = $cf->getResultField("CG", "nIncorrectNoSquash");
  $tmpTotal = $nCorrectCauseSquash + $nIncorrectCauseSquash 
            + $nCorrectNoSquash + $nIncorrectNoSquash;

  if ($tmpTotal) {
    printf "Crit2:                CorrectPredition      IncorrectPrediction\n";   
    printf "Crit2: [Pred,Real]    [1,1]      [0,0]      [0,1]      [1,0]\n";
    printf "Crit2: %8s       %5.2f      %5.2f      %5.2f      %5.2f\n", $name[3],
           100*$nCorrectCauseSquash/$tmpTotal,
           100*$nCorrectNoSquash/$tmpTotal,
           100*$nIncorrectCauseSquash/$tmpTotal,
           100*$nIncorrectNoSquash/$tmpTotal;
  }         

  my $nCorrectRestarted = $cf->getResultField("CG", "nCorrectRestarted");
  my $nIncorrectRestarted = $cf->getResultField("CG", "nIncorrectRestarted");
  my $nCorrectNoRestart = $cf->getResultField("CG", "nCorrectNoRestart");
  my $nIncorrectNoRestart = $cf->getResultField("CG", "nIncorrectNoRestart");
  $tmpTotal = $nCorrectRestarted + $nIncorrectRestarted 
            + $nCorrectNoRestart + $nIncorrectNoRestart;

  if ($tmpTotal) {
    printf "Crit3:                CorrectPredition      IncorrectPrediction\n";   
    printf "Crit3: [Pred,Real]    [1,1]      [0,0]      [0,1]      [1,0]\n";
    printf "Crit3: %8s       %5.2f      %5.2f      %5.2f      %5.2f\n", $name[3],
           100*$nCorrectRestarted/$tmpTotal,
           100*$nCorrectNoRestart/$tmpTotal,
           100*$nIncorrectRestarted/$tmpTotal,
           100*$nIncorrectNoRestart/$tmpTotal;
  }         
}
