**parse-sbs-replay_rootfiles** repository contains scripts to parse replayed ROOT files from the GMn/nTPE (should work for other SBS experiments such as GEn) to cut away obviously bad events for physics analysis from the main "T" event TTree. The motivation is to save time during the iterative physics analysis by not having to loop through all the "junk" data and loop through only a subset of events that have passed a set of relatively loose global cuts. Care should be take to choose these glbal cuts "loose enough", so that you will be not throwing away good/useful events for your analysis.


## 1.How it wokrs:
1. You will provide the program with the list of runs you want to parse, a set of global cuts, path to input full-replayed rootfiles, path to where the output parsed output root files should be stored, and output ROOT file name (if only a single root file is made), in a configuration file.
2. There are two modes of operation. One is, it takes a list of runs from the .cfg file and it will parse the replayed root files that corresponds to those runs. The other mode is, it will parse each and every replay root file within the directory. In both modes, the original root files will not be affected/changed and the results will be stored in the output directory as specified in the .cfg file.
3. For each ROOT file, the program will make a copy of the "T" TTree parsed as per the input cut conditions and the other three TTres, "E", "TShel", "TSLeft", and "TSsbs" are copied one to one, in a separate directory specified by the user. It is recommended to have this direcory in `/volatile` in ifarm or `~/Rootfiles` in a-onl. 
4. Aleternative to the above step 3, there is also an option to make a single ROOT file containing parsed data.

## 2. What each script does:
1. `parse_gep_rootfiles.C`: This is the main script that does the parsing of ROOT files. User should not have to modify anything inside this file unless they are experimenting.
2. `<user given name>.cfg`: The configuration file where you will input all the necessary information like the global cuts, run list, and input and output ROOT file directories.
3. `run-parse_gmn_rootfiles.sh`: The shell script that runs the `parse_gep_rootfiles.C`. User should not have to moddify this file.
4. `submit-parse_gep_rootfiles.sh`: The shell script that the user will have to execute to submit the parsing jobs either into the interactive ifarm/a-onl or into the batch farm.

## 3. Quick start guide:
1. Inside the directory where all the above mentioned scripts exists, create the `<user given name>.cfg` configuration file, following the provided `GEP1_test.cfg` file, and change the input fields as required.
2. Open the `submit-parse_gep_rootfiles.sh` file and set the fields "SCRIPT_DIR" and "workflowname" with the user specific ones.
3. Run the command: `$ submit-parse_gep_rootfiles.sh <configfilename> <use-configfile-runlis?> <do split-wise?> <run on ifarm?>`. If you need only parse the root files specified in the run-list in the .cfg file, input 1. Tf not, put 0. If split-wise file parsing is required, replace `<do split-wise?>` with 1, and if a single ROOT file needs to be created out of all the data, put any other integer. If the program needs to be run on ifarm, replace `<run on ifarm?>` with 1, and any other integer to run on batch farm.

## 4. Contact:
For any questions/bugs/concerns, contact,
>Anuruddha Rathnayake
><anuruddha@uconn.edu>
><adr@jlab.org>
><adr4zs@virginia.edu>  