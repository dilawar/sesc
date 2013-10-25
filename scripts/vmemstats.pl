#!/usr/bin/env perl

BEGIN {
  my $tmp = $0;

  $tmp = readlink($tmp) if( -l $tmp );
  $tmp =~ s/vmemstats.pl//;
  unshift(@INC, $tmp)
}

use sesc;
use strict;
use Time::localtime;
use Getopt::Long;


exit &main();

my $cf; 
my $nEntries;

sub main {

  my @flist;

  @flist = @ARGV;

  if( @flist == 0 ) {
    print "No stat files to process\n";
    exit 0;
  }

  if( @flist > 1 ) {
    print "Do not know how to proceed\n";
    exit 0;  
  }

  my $file;
  foreach $file (@flist) {

    $cf = sesc->new($file);

    $nEntries = $cf->getResultField("VMemStats","Entries");

    unless( defined $nEntries ) {
      print "Versioned Mem Statistics file [$file] has a problem\n";
      exit 0;
    }

    print "      no   squash     pids\n";
    for(my $i=0; $i<$nEntries; $i++) {
	my $pid = $cf->getResultField("P(${i})","C");
	my $squ = $cf->getResultField("Sq(${i})","C");
	printf "%8d %8d %8d\n",$i,$squ,$pid;
    }

  }

}
