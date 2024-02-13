#include <iostream>
#include <filesystem>
#include <vector>
#include <string> 
#include <cstring>
#include <dirent.h>
#include "TStopwatch.h"

#include "read_parsescript_config.h"
#include "beam_variables.h"

namespace fs = std::filesystem;
 
std::vector<int> makeRunnumvec(TString target, int kin_num, int sbsfieldscale);
std::vector<std::string> getFileNamesWithSubstring(TString input_dirpath, int runnum);
bool makeAParsedROOTfile(TString input_ROOTfile_dirpath, std::string rootfile_name, TString output_dir_path, TCut globalcut);
void makeTheParsedROOTfile(std::vector<int> runnum_vec, TString input_ROOTfile_dirpath, TString output_dir_path, TString outrootfilename, TCut globalcut);

int parse_gmn_rootfiles(const char* configfilename, int splitwise = 1) //Enter splitwise = 1 for split-wise ROOT file parsing and any other number to make a single parsed ROOT file.
{

	auto total_time_start = std::chrono::high_resolution_clock::now();
	TStopwatch *StopWatch = new TStopwatch();

	//// Read config file and copy the input parameres to local variable. ////
	Configfile configfile;
	int configfile_error = configfile.readin_parsescript_configfile(configfilename); // 0: Yes, -1: No.
	
	if ( configfile_error == -1) // Stop the program if the configuration file is incomplete/ has issues.
	{
		std::cerr << "Program stopping.\n";
		return 1;
	}

	int pass_num = configfile.return_pass_num();
	int kin_num = configfile.return_kin_num();
	int sbsfieldscale = configfile.return_sbsfieldscale();
	TString target = configfile.return_target();
	////////////////////////////////////////

	//// Make the run number vector as per the input parameters. ////
	std::vector<int> runnum_vec = makeRunnumvec(target, kin_num, sbsfieldscale); // Vector to hold all the good run numbers for the input parameters.

	if (runnum_vec.size() == 0)
	{
		std::cerr << "Error: There are no replayed ROOT files for the given input parameters! Stopping the program.\n";
		return 1;
	};
	////////////////////////////////////////

	TString input_ROOTfile_dirpath = configfile.return_inputdir();
	TString output_dir_path = configfile.return_outputdir();
	TCut globalcut = configfile.return_globalcut();
	
	if ( splitwise == 1 ) //Make seperate ROOT files for each individual ROOT file.
	{
		std::cout << "\nMaking split-wise parsed ROOT files...\n";
		//// Loop over each and every run number in the runnum_vec, and copy the ROOT file names into a another vector, run_segset_names_vec. ////
	 	for (const auto& runnum : runnum_vec)
	 	{
			std::vector<std::string> run_segset_names_vec = getFileNamesWithSubstring(input_ROOTfile_dirpath, runnum); // The vector to hold the names of ROOT files for the given runnum.

			if( run_segset_names_vec.size() == 0)
			{
				std::cerr << "No replayed ROOT files for run number " << runnum << " in the directory: " << input_ROOTfile_dirpath << '\n';
				continue;
			}

			std::cout << "*Run number: " << runnum << '\n';

			//// Loop over each and every ROOT file in the run_segset_names_vec and create a parsed ROOT file for each and every ROOT file in the vector ////
			for(const auto& rootfile_name : run_segset_names_vec)
			{
				//Making the parsed ROOT files.
				bool is_success = makeAParsedROOTfile(input_ROOTfile_dirpath, rootfile_name, output_dir_path, globalcut);

				if(!is_success) continue; // Skip to the next file if there is an eroor with parsing the rootfile.
			}

			std::cout << '\n';
		}
	
	}

	else //Make a single parsed ROOT file for all the runs with the given run parameters.
	{
		std::cout << "\nMaking a single parsed ROOT file...\n";
		TString outrootfilename = Form("parsed_gmn_SBS%i_targ%s_sbsmagscale%i.root", kin_num, target.Data(), sbsfieldscale);
		makeTheParsedROOTfile(runnum_vec, input_ROOTfile_dirpath, output_dir_path, outrootfilename, globalcut);
	}		

	auto total_time_end = std::chrono::high_resolution_clock::now();
	auto total_time_duration = std::chrono::duration_cast<std::chrono::minutes>(total_time_end - total_time_start);
	std::cout << endl << "---------------------------------------------------" << endl;
	std::cout << "Finished parsing rootfiles" << endl;
	std::cout << "---------------------------------------------------" << endl;
	std::cout << "Total time: " << total_time_duration.count() << " minutes. " << endl;
			
	return 0;
}

std::vector<int> makeRunnumvec(TString target, int kin_num, int sbsfieldscale)
{
	std::cout << "\n--------------------------------------\n";
	std::cout << "Building runnum vector...\n"; 
	int nruns = lookup_parsed_runs_cnt(target, kin_num, sbsfieldscale);
	std::cout << "Number of good runs as per the input parameters: " << nruns << '\n';
	std::cout << "--------------------------------------\n" << '\n';


	std::vector<int> runnum_vec;

	for( int i = 0; i < nruns; i++ )
	{
		runnum_vec.push_back(lookup_parsed_runnums(target, kin_num, sbsfieldscale, i));
	}

	return runnum_vec;
}

// std::vector<std::string> getFileNamesWithSubstring(TString input_dirpath, int runnum) // Note: 9/2/2023: This function does not seems to behave as intended anymore. Maybe it is a result of using a newer ROOT version.
// {
//     const char* directoryPath = input_dirpath.Data();
//     const char* substring = std::to_string(runnum).c_str();
    
//     std::vector<std::string> fileNames;

//     DIR* dir = opendir(directoryPath);
//     if (dir == nullptr) 
//     {
//         std::cerr << "Error opening directory: " << directoryPath << std::endl;
//         return fileNames;
//     }

//     dirent* entry;
//     while ((entry = readdir(dir)) != nullptr) 
//     {
//         if (entry->d_type == DT_REG) 
//         {  // Only consider regular files
//             std::string fileName = entry->d_name;
//             if (fileName.find(substring) != std::string::npos) 
//             {
//                 fileNames.push_back(fileName);
//             }
//         }
//     }

//     closedir(dir);

//     return fileNames;
// }

std::vector<std::string> getFileNamesWithSubstring(TString input_dirpath, int runnum) 
{
	const char* directoryPath_cst = input_dirpath.Data();
	std::string directoryPath = directoryPath_cst;
	const char* searchLiteral_cst = std::to_string(runnum).c_str();
	std::string searchLiteral = searchLiteral_cst;
	//const char* searchLiteral = "e1209019_fullreplay_13597_stream0";

    std::vector<std::string> matchingFiles;

    try 
    {
        for (const auto& entry : fs::directory_iterator(directoryPath)) 
        {
            if (entry.is_regular_file()) 
            {
                std::string filename = entry.path().filename().string();
                if (filename.find(searchLiteral) != std::string::npos) 
                {
                    matchingFiles.push_back(filename);
                }
            }
        }
    }
    catch (const fs::filesystem_error& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return matchingFiles;
}

bool makeAParsedROOTfile(TString input_ROOTfile_dirpath, std::string rootfile_name, TString output_dir_path, TCut globalcut)
{
	// Open input root file. Copy the Trees.
	TFile* inputrootfile = new TFile(Form("%s/%s", input_ROOTfile_dirpath.Data(), rootfile_name.c_str()), "OPEN"); 
	TTree* in_T = (TTree*)inputrootfile->Get("T");
	
	if (in_T == nullptr) 
	{
		std::cout << "Problem with accessing the T tree. Skipping parsing the ROOT file: " << rootfile_name << '\n'; 
		return false; // We don't need to try continue and parse the root file if there is a problem with accessing the maing T event tree.
	}

	TTree* in_E = (TTree*)inputrootfile->Get("E");
	TTree* in_TSLeft = (TTree*)inputrootfile->Get("TSLeft");
	TTree* in_TSsbs = (TTree*)inputrootfile->Get("TSsbs");
	
	TString outrootfilename = Form("%s", rootfile_name.c_str());

	TFile* outputrootfile = new TFile(Form("%s/%s", output_dir_path.Data(), outrootfilename.Data()),"RECREATE");

	std::cout << "**Making the parsed ROOT file for file: " << rootfile_name << '\n';

	//Cloning the E, TSLeft, and TSsbs trees.
	TTree* E; 
	if(in_E != nullptr) E = in_E->CloneTree();

	TTree* TSLeft; 
	if(in_TSLeft != nullptr) TSLeft = in_TSLeft->CloneTree();

	TTree* TSsbs; 
	if(in_TSsbs != nullptr) TSsbs = in_TSsbs->CloneTree();

	//Making a copy of the main "T" tree with the provided global cuts applied.
	TTree* T; 
	T = in_T->CopyTree(globalcut);
		
	outputrootfile->Write();
	outputrootfile->Close();
	delete outputrootfile;

	inputrootfile->Close();
	delete inputrootfile;

	return true;
}

void makeTheParsedROOTfile(std::vector<int> runnum_vec, TString input_ROOTfile_dirpath, TString output_dir_path, TString outrootfilename, TCut globalcut)
{
	TChain* C_T = new TChain("T");
	TChain* C_E = new TChain("E");
	TChain* C_TSLeft = new TChain("TSLeft");
	TChain* C_TSsbs = new TChain("TSsbs");

	for (const auto& runnum : runnum_vec)
	{
		TString in_rootfile = Form("%s/*%i*.root", input_ROOTfile_dirpath.Data(), runnum);
		C_T->Add(in_rootfile.Data());
		C_E->Add(in_rootfile.Data());
		C_TSLeft->Add(in_rootfile.Data());
		C_TSsbs->Add(in_rootfile.Data());
	}

	TFile* outputrootfile = new TFile(Form("%s/%s", output_dir_path.Data(), outrootfilename.Data()),"RECREATE");
		
	//Making a copy of the main "T" tree with the provided global cuts applied.
	TTree* T; 
	T = C_T->CopyTree(globalcut);
	TTree* E;
	//Cloning the other TTrees.
	E = C_E->CloneTree();
	TTree* TSLeft;
	TSLeft = C_TSLeft->CloneTree();
	TTree* TSsbs;
	TSsbs = C_TSsbs->CloneTree();
		
	outputrootfile->Write();
	outputrootfile->Close();
	delete outputrootfile;
}