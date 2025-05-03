#!/bin/bash

# List of arguments
configfile=$1
usecfgrunlist=$2
dosplitwise=$3
scriptsdir=$4

analyzer -b -q $scriptsdir'/parse_gep_rootfiles.C('\"$configfile\"', '$usecfgrunlist', '$dosplitwise')'
