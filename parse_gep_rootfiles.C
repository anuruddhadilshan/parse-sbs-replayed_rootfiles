#include <iostream>
#include <filesystem>
#include <vector>
#include <string> 
#include <cstring>
#include <dirent.h>
#include "TStopwatch.h"

#include "read_parsescript_config.h"


std::vector<int> makeRunnumVecFromDirRootFiles(const TString& dirPath);
std::vector<TString> makeFileNameVecFromRunNum(const TString& dirPath, const int input_runNum);
bool makeAParsedROOTfile(TString input_ROOTfile_dirpath, TString rootfile_name, TString output_dir_path, TCut globalcut);
void makeTheParsedROOTfile(std::vector<int> runnum_vec, TString input_ROOTfile_dirpath, TString output_dir_path, TString outrootfilename, TCut globalcut);

int parse_gep_rootfiles(const char* configfilename, const bool useCfgFileRunList = true, const int splitwise = 1) //Enter splitwise = 1 for split-wise ROOT file parsing and any other number to make a single parsed ROOT file.
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

	TString input_ROOTfile_dirpath = configfile.return_inputdir();
	TString output_dir_path = configfile.return_outputdir();
	TCut globalcut = configfile.return_globalcut();

	//// Make the run number vector ////
	std::vector<int> runnum_vec;

	if ( useCfgFileRunList )
	{
		runnum_vec = configfile.return_RunListVector();

		if (runnum_vec.size() == 0)
		{
			std::cerr << "Error: cfg file run-list is EMPTY!\n";
			return 1;
		}
	}
	else
	{
		runnum_vec = makeRunnumVecFromDirRootFiles( input_ROOTfile_dirpath ); // Vector to hold all the good run numbers for the input parameters.

		if (runnum_vec.size() == 0)
		{
			std::cerr << "Error: No matching replayed root files found in the input directory! \n";
			return 1;
		}
	}
	
	
	if ( splitwise == 1 ) //Make seperate ROOT files for each individual ROOT file.
	{
		std::cout << "\nMaking split-wise parsed ROOT files...\n";
		//// Loop over each and every run number in the runnum_vec, and copy the ROOT file names into a another vector, run_segset_names_vec. ////
	 	for (const auto& runnum : runnum_vec)
	 	{
			std::vector<TString> run_segset_names_vec = makeFileNameVecFromRunNum(input_ROOTfile_dirpath, runnum); // The vector to hold the names of ROOT files for the given runnum.

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
		TString outrootfilename = configfile.return_outputfilename();
		if ( outrootfilename == "") outrootfilename = "gep_parsed_default.root";
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


// Function to extract unique run numbers from ROOT file names in a given directory
std::vector<int> makeRunnumVecFromDirRootFiles(const TString& dirPath) 
{
    std::set<int> runNumbers;  // Set to automatically filter duplicates

    // Open the directory
    TSystemDirectory dir("dir", dirPath);
    TList* files = dir.GetListOfFiles();

    if (!files) {
        std::cerr << "Directory not found or could not be read: " << dirPath << std::endl;
        return {};
    }

    // Iterator for the list of files
    TIter next(files);
    TSystemFile* file;


    while ((file = (TSystemFile*)next()))
    {
        // Skip directories
        if (file->IsDirectory()) continue;

        TString fileName = file->GetName();
                               
        if (fileName.BeginsWith("gep5_fullreplay_") && fileName.EndsWith(".root")) 
        {
            TObjArray* tokens = fileName.Tokenize("_");
            if (tokens->GetEntries() > 3) 
            {
            	TString runStr = ((TObjString*)tokens->At(2))->GetString();  // Third token: run number
                int runNum = runStr.Atoi();
                std::cout << "Extracted run: " << runNum << std::endl;
                runNumbers.insert(runNum);
            }

            delete tokens;
        }
    }

    // Convert set to vector before returning
    return std::vector<int>(runNumbers.begin(), runNumbers.end());
}

// Function to make a list of file names for a given CODA run number (different segments & streams).
std::vector<TString> makeFileNameVecFromRunNum(const TString& dirPath, const int input_runNum)
{
	std::vector<TString> fileListForRunNum;

	// Open the directory
    TSystemDirectory dir("dir", dirPath);
    TList* files = dir.GetListOfFiles();

    // Iterator for the list of files
    TIter next(files);
    TSystemFile* file;

     while ((file = (TSystemFile*)next()))
    {
        // Skip directories
        if (file->IsDirectory()) continue;

        TString fileName = file->GetName();
                               
        if (fileName.BeginsWith("gep5_fullreplay_") && fileName.EndsWith(".root")) 
        {
            TObjArray* tokens = fileName.Tokenize("_");
            if (tokens->GetEntries() > 3) 
            {
            	TString runStr = ((TObjString*)tokens->At(2))->GetString();  // Third token: run number
                
                int runNum = runStr.Atoi();
                
                if ( runNum == input_runNum )
                {
                	fileListForRunNum.push_back(fileName);
                }
            }

            delete tokens;
        }
    }

    return fileListForRunNum;
}

bool makeAParsedROOTfile(TString input_ROOTfile_dirpath, TString rootfile_name, TString output_dir_path, TCut globalcut)
{
	// Open input root file. Copy the Trees.
	TFile* inputrootfile = new TFile(Form("%s/%s", input_ROOTfile_dirpath.Data(), rootfile_name.Data()), "OPEN"); 
	TTree* in_T = (TTree*)inputrootfile->Get("T");
	
	if (in_T == nullptr) 
	{
		std::cout << "Problem with accessing the T tree. Skipping parsing the ROOT file: " << rootfile_name << '\n'; 
		return false; // We don't need to try continue and parse the root file if there is a problem with accessing the maing T event tree.
	}

	TTree* in_E = (TTree*)inputrootfile->Get("E");
	TTree* in_TShel = (TTree*)inputrootfile->Get("TShel");
	TTree* in_TSLeft = (TTree*)inputrootfile->Get("TSLeft");
	TTree* in_TSsbs = (TTree*)inputrootfile->Get("TSsbs");
	
	TString outrootfilename = Form("%s", rootfile_name.Data());

	TFile* outputrootfile = new TFile(Form("%s/%s", output_dir_path.Data(), outrootfilename.Data()),"RECREATE");

	std::cout << "**Making the parsed ROOT file for file: " << rootfile_name << '\n';

	//Cloning the E, TShel, TSLeft, and TSsbs trees.
	TTree* E; 
	if(in_E != nullptr) E = in_E->CloneTree();

	TTree* TShel;
	if(in_TShel != nullptr) TShel = in_TShel->CloneTree();

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
	//TChain* C_TShel = new TChain("TShel");
	TChain* C_TSLeft = new TChain("TSLeft");
	TChain* C_TSsbs = new TChain("TSsbs");

	for (const auto& runnum : runnum_vec)
	{
		TString in_rootfile = Form("%s/*%i*.root", input_ROOTfile_dirpath.Data(), runnum);	
		C_T->Add(in_rootfile.Data());
		C_E->Add(in_rootfile.Data());
		C_TSLeft->Add(in_rootfile.Data());
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
	// TTree* TShel;
	// TShel = C_TShel->CloneTree();
	TTree* TSLeft;
	TSLeft = C_TSLeft->CloneTree();
	TTree* TSsbs;
	TSsbs = C_TSsbs->CloneTree();
		
	outputrootfile->Write();
	outputrootfile->Close();
	delete outputrootfile;
}