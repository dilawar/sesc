
 SESC can read RST traces. RST traces are generated from SPARC machines (Niagara).
To generate a RST trace, you need to install SAM (OpenSparcT1 available at www.opensparc.net)

--------------------
 To generate the trace, you must boot Solaris on SAM, then from the console:

# stop simulation to run a module
run: stop
# load rst trace generation module
stop: mod load rstrace rstracer.so
# specify output file and number of instructions
stop: rstrace -o <outputfile> -n <number_of_instructions>
stop: stepi <number_of_instructions>
# When you finish, you can go back to run mode
stop: run
run:

--------------------
 rstexample

 rstexample is a small program that reads rst traces. You can use it to count number
of instructions available on the trace. It is also the fundation for SESC to read
rst traces.

--------------------
 Reading RST traces from SESC

 cd <sesc>
 mkdir ../build
 cd ../build
 ../sesc/configure  --enable-rsttrace 
 gnumake
 gnumake sesc.conf
 ./sesc.mem ../../projs/esesc/tests/rst_trace2.rz3.gz 
 ../sesc/scripts/report.pl -a

 
