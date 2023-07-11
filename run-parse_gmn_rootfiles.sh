#!/bin/bash

# List of arguments
configfile=$1
dosplitwise=$2
scriptsdir=$3

root $scriptsdir'/parse_gmn_rootfiles.C('\"$configfile\"', '$dosplitwise')'
