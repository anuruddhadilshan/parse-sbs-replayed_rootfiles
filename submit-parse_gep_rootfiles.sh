#!/bin/bash

# Setting necessary envioronments / workflow names (ONLY user specific part)
# Simply set the abosolute path to the directory where all the shell scripts, parse script, and config files are located. These shell script assumes they are all in the same directory.
export SCRIPT_DIR=/w/halla-scshelf2102/sbs/adr/parse-GMn_rootfiles
workflowname='parse-gep-rootfiles'

configfilename=$1
usecfgrunlist=$2
dosplitwise=$3 #input 1 to make split-wise parsed ROOT files and any other number to make a single parsed ROOT file.
run_on_ifarm=$4 #input 1 to run on ifarm and any other number to run on batch mode.

if [[ "$#" -ne 4 ]]; then
	echo -e "\n--!--\n Invalid number of arguments!!"
	echo -e "This script expects 3 arguments: <configfilename> <use-configfile-runlis?> <split-wise?> <run_on_ifarm?>\n"
	exit;
fi

configfile=$SCRIPT_DIR'/'$1

script=$SCRIPT_DIR'/run-parse_gep_rootfiles.sh'

if [[ $run_on_ifarm -ne 1 ]]; then
	swif2 create $workflowname
	echo "Adding requested parsing job to swif2 "
	swif2 add-job -workflow $workflowname -partition production -cores 1 -disk 100GB -ram 5GB $script $configfile $usecfgrunlist $dosplitwise $SCRIPT_DIR
	swif2 run $workflowname
	echo -e "\n Getting workflow status. May take a few minutes...\n"
	swif2 status $workflowname
else 
	echo 'Running the parsing script locally ...'
	$script $configfile $usecfgrunlist $dosplitwise $SCRIPT_DIR
fi



 
