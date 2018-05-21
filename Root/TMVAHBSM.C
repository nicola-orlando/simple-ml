#include <iostream> // Stream declarations
#include <vector>
#include <limits>
#include <string> 

#include "TSystem.h"
#include "TChain.h"
#include "TCut.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TH1.h"
#include "TMath.h"
#include "TFile.h"
#include "TStopwatch.h"
#include "TROOT.h"

#include "TMVA/GeneticAlgorithm.h"
#include "TMVA/GeneticFitter.h"
#include "TMVA/IFitterTarget.h"
#include "TMVA/Factory.h"
#include "TMVA/Reader.h"

//Delay screen output
#include <chrono>
#include <thread>

using namespace std;

//The functions in this namespace are not being used yet, might be necessary to speed up the fit with TRExFitter 

namespace HistoManager {

  std::vector< std::pair< TString,TString> > Regions()
  {
    std::vector< std::pair< TString,TString> > regions_and_cuts;

    std::cout<<"Define the regions"<<std::endl;
    std::vector< TString > region_name;
    std::vector< TString > region_M_name; region_M_name.push_back("0M");  region_M_name.push_back("1M");   region_M_name.push_back("2M"); 
    std::vector< TString > region_j_name; region_j_name.push_back("5j");  region_j_name.push_back("6_8j"); region_j_name.push_back("9j");
    std::vector< TString > region_b_name; region_b_name.push_back("3b");  region_b_name.push_back("4b");  
  
    TString region="";
    for( auto M_name : region_M_name )
      for( auto j_name : region_j_name )
	for( auto b_name : region_b_name )
	  {
	    region="reg1l"+M_name+j_name+b_name;
	    region_name.push_back(region);
	    region="";
	  }

    std::cout<<"Define the cuts"<<std::endl;
    std::vector< TString > region_cut;
    std::vector< TString > region_M_cut; region_M_cut.push_back("mtjets_n==0");  region_M_cut.push_back("mtjets_n==1");          region_M_cut.push_back("mtjets_n>=2"); 
    std::vector< TString > region_j_cut; region_j_cut.push_back("jets_n==5");    region_j_cut.push_back("jets_n>=6&&jets_n<=8"); region_j_cut.push_back("jets_n>=9");
    std::vector< TString > region_b_cut; region_b_cut.push_back("bjets_n==3");   region_b_cut.push_back("bjets_n>=4");  
    
    TString cut="";
    for( auto M_cut : region_M_cut )
      for( auto j_cut : region_j_cut )
	for( auto b_cut : region_b_cut )
	  {
	    cut="("+M_cut+"&&"+j_cut+"&&"+b_cut+")";
	    region_cut.push_back(cut);
	    cut="";
	  }
  
    unsigned int size=region_name.size();
    if( region_name.size() != region_cut.size() ) {
      std::cout<<"region_name(size="<<region_name.size()<<") and region_cut (size="<<region_cut.size()<<") have a different sizes"<<std::endl;
      exit(0);
    }
    for(int in=0; in<size; in++)  regions_and_cuts.push_back( std::make_pair( region_name[in],region_cut[in]) );    

    return regions_and_cuts;
  } 


  void DebugRegions( std::vector< std::pair< TString,TString> > regions_and_cuts)
  {
    std::cout<<"Printing regions and cuts"<<std::endl;
    for(auto region_and_cut : regions_and_cuts) std::cout<<"Region "<<region_and_cut.first <<" and cut "<<region_and_cut.second <<std::endl;
    std::cout<<"Done with printing regions and cuts"<<std::endl;
  }

  std::vector< TH1F > BookHistograms(std::vector< std::pair< TString, TString > >  regions_and_cuts, int nbins, float min, float max, TString variable)
  {
    std::vector< TH1F > histo_list;
    for(auto region_and_cut : regions_and_cuts)
      {
	TString h_title=region_and_cut.first + variable;
	TH1F h(h_title,h_title,nbins,min,max);
	histo_list.push_back(h);
      }
    return histo_list;
  }
  
  //Under development..
  /*
  std::vector< TTreeFormula > DefineTTreeFormula(TTree *tree, std::vector< std::pair< TString, TString> > regions_and_cuts)
  {
    std::vector<TTreeFormula > cuts_list;
    for(auto region_and_cut : regions_and_cuts) 
      {
	//cuts_list.push_back( TTreeFormula cut(region_and_cut.first,region_and_cut.second,tree) );
	//cut.GetNdata();
      }
    return cuts_list;
  }						
  */						  
						
}


namespace TMVA {

  enum class P_COL{RED,GREEN,BLUE,MAGENTA};
  enum class P_STY{NORMAL,BOLD,UNDERLINE};

  //Based on https://en.wikipedia.org/wiki/ANSI_escape_code
  void PrintUtil(TString to_be_printed,TMVA::P_COL col,TMVA::P_STY sty)
  {
    TString color="";
    TString style="";
    
    if(col==TMVA::P_COL::RED)color="31";
    else if(col==TMVA::P_COL::GREEN)color="32";
    else if(col==TMVA::P_COL::BLUE)color="34";
    else if(col==TMVA::P_COL::MAGENTA)color="35";
    else {std::cout<<"Color not found"<<std::endl; exit(0);} 
    
    if(sty==TMVA::P_STY::NORMAL)style="3";
    else if(sty==TMVA::P_STY::BOLD)style="1";
    else if(sty==TMVA::P_STY::UNDERLINE)style="4";
    else {std::cout<<"Style not found"<<std::endl; exit(0);} 

    TString print="\033["+style+";"+color+"m"+to_be_printed+"\033[0m";
    std::cout<< print <<std::endl;
    return;
  }

  //Look for a key in the config text file and then returns the key and the option as a pair
  std::pair< std::string, std::string > MainOptionsParser(TString config, TString key)
  {
    TMVA::PrintUtil("\n>   In MainOptionsParser function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    std::pair<std::string, std::string> option_and_key;
    size_t field_option ;
    size_t field_key ;

    std::string sFileRow;
    fstream FileOpen;  FileOpen.open(config,fstream::in);
    std::string sfield, skey, svalue;
    while( getline(FileOpen,sFileRow) ) {
      istringstream ss(sFileRow);
      field_option=sFileRow.find("option ");
      field_key=sFileRow.find(key);
      if(field_option  != string::npos && field_key  != string::npos){
	  ss>>sfield>>skey>>svalue;
	  TString op_file=sfield+" "+skey+" "+svalue;
	  TMVA::PrintUtil(op_file,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	  option_and_key=std::make_pair(skey,svalue);
	  TMVA::PrintUtil("\n>   Done with MainOptionsParser function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
	  return option_and_key;
	}
      else if(FileOpen.eof()) { 
	TMVA::PrintUtil("Error, the file list is empty, badly formatted, or the option "+key+" is not found",TMVA::P_COL::RED,TMVA::P_STY::BOLD); 
	exit(0);
      }
    }
    return option_and_key;
  }  
  
  //Split the cut formulae needed for the cross validation, a bit involved now, it needs some clean up
  std::vector<TString> GetCutFormula(TString cut_formula)
  {
    TMVA::PrintUtil("\n>   In GetCutFormula function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    std::vector<TString> output_cut_list;
    std::string cut_value;
    const char *char_cut_formula=cut_formula;
    std::string local_cut_formula(char_cut_formula);
    std::istringstream ss(local_cut_formula);

    while(std::getline(ss, cut_value, ',')) {
      	TMVA::PrintUtil("INFO Splitted cut "+cut_value,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
      output_cut_list.push_back(cut_value);
    }

    TMVA::PrintUtil("\n>   Done with GetCutFormula function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);    
    return output_cut_list;
  }


  //Pick a label for the factory name in case the cross validation sampling is run, this has to be called is we run on the cross validation. 
  //Currently it has hard coded the splitting of the sample for the cross validation (it expect a 10-fold cross validation)  
  TString GetTitleExtension(TString cut_for_training)
  {
    TMVA::PrintUtil("\n>   In GetTitleExtension function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    TString cross_validation_sample_label="";
    const char *char_cut_for_training=cut_for_training;
    std::string string_cut_for_training(char_cut_for_training);
    std::size_t find_position_of_substring = string_cut_for_training.find("%2=="); 
    //Be careful the argument to pass to "substr" depends on the number of digits used to define the cross_validation samples
    //+4 works for one digit splitting, +5 for two digits 
    string_cut_for_training=string_cut_for_training.substr(string_cut_for_training.find("%2==")+4); 
    //Remove the last two characters "))"
    string_cut_for_training.pop_back();
    string_cut_for_training.pop_back();
    cross_validation_sample_label=(TString)string_cut_for_training;
    TMVA::PrintUtil("INFO Cross validation sample label "+cross_validation_sample_label,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
    TMVA::PrintUtil("\n>   Done with GetTitleExtension function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    return cross_validation_sample_label;
  }
  
  
  //Pick the total number of entries passing the sampling cut for the cross validation
  //When running on the cross validation we want to use the full stats of each sample for the training
  int GetTreeEntries(TTree *tree, TString cut)
  {
    TMVA::PrintUtil("\n>   In GetTreeEntry function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    int total_tree_entries=0;
    std::shared_ptr<TH1F> counter = std::make_shared<TH1F>("counter","",100,0,999999999999);
    tree->Draw("event_number>>counter",cut);   
    total_tree_entries=counter->Integral();
    
    TMVA::PrintUtil("\n>   Done with GetTreeEntry function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    return total_tree_entries;
  }

  
  //Get the background tree from a TChain
  //Unlike it happens for the signals we want to have the backgrounds possibly merged together 
  TChain* BackgroundChain(std::string input_files_dir, TString config_file)
  {
    TMVA::PrintUtil("\n>   In BackgroundChain function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    TChain *background_chain=new TChain();
    std::vector<TString> background_files;
    std::string background_file;
    TString tmp_background=TMVA::MainOptionsParser(config_file,"background_sample").second;
    const char *char_background_files=tmp_background;

    std::string local_background_files(char_background_files);
    std::istringstream ss(local_background_files);
    while(std::getline(ss, background_file, ',')) {
      TMVA::PrintUtil("INFO Splitted background_file "+background_file,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
      background_files.push_back(input_files_dir+background_file+"/"+TMVA::MainOptionsParser(config_file,"tree_name").second);
    }

    for(auto file : background_files ){
	TMVA::PrintUtil("INFO Adding file "+file,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	background_chain->Add(file);
      }
    TMVA::PrintUtil("\n<   Done with BackgroundChain function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    return background_chain;
  }  

  bool IsBackgroundSample(std::string backround_sample_name)
  {
    TMVA::PrintUtil("\n>  In IsBackgroundSample function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);

    bool is_background_sample=false;
    std::vector<std::string> background_list;
    size_t found_sub_name;

    background_list.push_back("Wjets22beauty");
    background_list.push_back("Wjets22charm");
    background_list.push_back("Wjets22light");
    background_list.push_back("Zjets22beauty");
    background_list.push_back("Zjets22charm");
    background_list.push_back("Zjets22light");
    background_list.push_back("ttH");
    background_list.push_back("Singletop");
    background_list.push_back("topEW");
    background_list.push_back("ttbarbb");
    background_list.push_back("ttbarcc");
    background_list.push_back("ttbarlight");
    background_list.push_back("Dibosons");

    for( auto background : background_list )
      {
	if(is_background_sample) break;
	found_sub_name=backround_sample_name.find(background);
	if( found_sub_name != string::npos ) is_background_sample=true;	  
      }  
    return is_background_sample;
    TMVA::PrintUtil("\n>  Done with IsBackgroundSample function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
  }

  //Run the training. As implemented in the main function It runs in two modes
  //1) Input files from text file list, loop over all of them
  //2) Specified value of the file input
  void Training(std::vector<std::string> variables, std::pair<std::string,std::string> signal_sample, TString config_file,std::string file_input=""){

    TMVA::PrintUtil("\n>   In Training function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    
    //Check if you are trying to train an MVA using a background file as input
    if(TMVA::IsBackgroundSample(file_input))
      {
	TMVA::PrintUtil("INFO Tried to run Training with a background sample as a signal: "+(TString)file_input,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	return;
      }

    //Getting the common path name for the files
    std::string input_files_dir=TMVA::MainOptionsParser(config_file,"input_files_dir").second;
    
    //Only the signal file is picked in this way, the background is chained
    TString fname_signal; 
    if(file_input=="")fname_signal=(TString)input_files_dir+signal_sample.second;
    else fname_signal=(TString)file_input;
    TMVA::PrintUtil("INFO Opening the following signal input for the training: "+fname_signal,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);

    TFile *input_signal;    input_signal = TFile::Open( fname_signal      );
    char tree_signal_name[1000]; sprintf(tree_signal_name,"%s",TMVA::MainOptionsParser(config_file,"tree_name").second.c_str());
    TTree *tree_signal = (TTree*)input_signal ->Get(tree_signal_name);

    //Chaining the background
    TChain *background_chain = TMVA::BackgroundChain(input_files_dir, config_file);
    TTree *tree_background = background_chain;

    // Global event weights per tree (see below for setting event-wise weights)
    Float_t signal_weight     = std::stof( TMVA::MainOptionsParser(config_file,"signal_weight").second );
    Float_t background_weight = std::stof( TMVA::MainOptionsParser(config_file,"background_weight").second );

    //The are supposed to be the same cut
    std::vector<TString> signal_cuts = TMVA::GetCutFormula(TMVA::MainOptionsParser(config_file,"cuts_signal").second.c_str());
    std::vector<TString> background_cuts = TMVA::GetCutFormula(TMVA::MainOptionsParser(config_file,"cuts_background").second.c_str());

    //Loop over the cuts, real loop only when running cross_validation with the suitable config file
    for(auto cut_for_training : signal_cuts )
      {
	TMVA::PrintUtil("INFO Training with the following cut "+cut_for_training,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	std::size_t pos_bdt_name = file_input.find("_merged/"); 
	
	// Create a new root output file.
	TString outfileName;
	if(file_input=="") outfileName=TMVA::MainOptionsParser(config_file,"output_file_name_prefix").second+signal_sample.first;
	else 
	    outfileName= TMVA::MainOptionsParser(config_file,"output_file_name_prefix").second+(TString)file_input.substr(pos_bdt_name+8,file_input.size()-5);

	std::string factoryOptions(TMVA::MainOptionsParser(config_file,"factory_options").second);
	TMVA::PrintUtil("INFO Training with factory options "+factoryOptions,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);

	TCut mycuts = (TCut)cut_for_training;
	TCut mycutb = (TCut)cut_for_training;
	
	TString factory_label;
	if(TMVA::MainOptionsParser(config_file,"training_method").second=="cross_validation")
	  factory_label="_cross_val_sample_"+TMVA::GetTitleExtension(cut_for_training);
	else if(TMVA::MainOptionsParser(config_file,"training_method").second=="default")
	  factory_label="_naive_training";
	else{
	  TMVA::PrintUtil("Error, training_method not found",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
	  exit(0);
	} 
	
	//If you put as input the input file, the output file will be added on the same directory and no plots can be done 
	//Getting also the label of the cross training
	TFile *outputFile = TFile::Open( outfileName+factory_label+".root", "RECREATE" );

	TString factory_name;
	//For training from the list
	if(file_input=="")factory_name = TMVA::MainOptionsParser(config_file,"factory_name").second+signal_sample.first+factory_label;
	//For training in batch
	if(file_input!="")factory_name = TMVA::MainOptionsParser(config_file,"factory_name").second+(TString)file_input.substr(pos_bdt_name+8,file_input.size()-5)+factory_label;
	TMVA::PrintUtil("INFO Preparing a factory with name "+factory_name+"\n",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);

	TMVA::Factory factory( factory_name , outputFile, factoryOptions );
	//assumes that the variable type is a float 
	for(auto var : variables) factory.AddVariable(var);
	
	factory.AddSignalTree    ( tree_signal    ,  signal_weight      );
	factory.AddBackgroundTree( tree_background,  background_weight  );

	factory.SetBackgroundWeightExpression(TMVA::MainOptionsParser(config_file,"background_event_weight").second);
	factory.SetSignalWeightExpression(TMVA::MainOptionsParser(config_file,"signal_event_weight").second);

	// tell the factory to use all remaining events in the trees after training for testing: if runs on cross_validatin will use the full stats of events passing the cuts
	std::string training_and_test_options(TMVA::MainOptionsParser(config_file,"training_and_test_tree_config").second);
	if(TMVA::MainOptionsParser(config_file,"training_method").second=="cross_validation")
	  {
	    training_and_test_options="nTrain_Signal="+std::to_string(TMVA::GetTreeEntries(tree_signal,cut_for_training))+":"+
	      "nTrain_Background="+std::to_string(TMVA::GetTreeEntries(tree_background,cut_for_training))+":"+
	      training_and_test_options;
	  }
	TMVA::PrintUtil("INFO Training with the following options\n",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	TMVA::PrintUtil(training_and_test_options,TMVA::P_COL::RED,TMVA::P_STY::BOLD);
	factory.PrepareTrainingAndTestTree(mycuts,mycutb,training_and_test_options);

	if( TMVA::MainOptionsParser(config_file,"mva_method").second != "BDTG" ) 
	  { TMVA::PrintUtil("MVA option not found",TMVA::P_COL::RED,TMVA::P_STY::BOLD); exit(0); }

	TMVA::PrintUtil("INFO Book the MVA method\n",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
	factory.BookMethod(TMVA::Types::kBDT,TMVA::MainOptionsParser(config_file,"mva_method").second,TMVA::MainOptionsParser(config_file,"mva_method_option").second);
	TMVA::PrintUtil("INFO Train the MVA method\n",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
	factory.TrainAllMethods();
	//For the cross validation there are no events available for testing thus those two functions below can't be used. 
	//The functions below are only used for control plots and such
	if(TMVA::MainOptionsParser(config_file,"training_method").second=="default")
	  {
	    TMVA::PrintUtil("INFO Test the MVA method\n",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
	    factory.TestAllMethods();
	    TMVA::PrintUtil("INFO Evaluate the MVA method\n",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
	    factory.EvaluateAllMethods();
	  }
	
	outputFile->Close();
	outputFile = NULL;
	delete outputFile;
	
      }//Out of the loop over the cuts

    //Clean up all pointers
    tree_signal = NULL;
    delete tree_signal;
    background_chain = NULL;
    delete background_chain;
    tree_background = NULL;
    delete tree_background;
    input_signal = NULL;
    delete input_signal;

    TMVA::PrintUtil("\n>   Done with Training function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    return;
  }


  // ------------------------------ Application ----------------------------------------------------------------
  // create a summary tree with all signal and background events and for each event the three classifier values and the true classID
  void ApplicationCreateCombinedTree(std::vector<std::string> variables, 
				     std::vector< std::pair<std::string,std::string> > signal_samples,
				     std::pair<std::string,std::string>  output_sample, TString config_file, std::string file_input=""){

    TMVA::PrintUtil("\n>   In ApplicationCreateCombinedTree function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);

    TMVA::PrintUtil("INFO Checking the how to pick the input and output files",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
    //TODO Redundant with what is done in the main, should be removed from here 
    bool use_config_file=true;
    if( file_input=="" ){
      TMVA::PrintUtil("INFO The input and output files will be defined from the configuration file",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
      use_config_file=true;
    }
    else if( file_input!="" ){
      TMVA::PrintUtil("INFO The input and output files will be defined based on arguments of the main function",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
      use_config_file=false;
    }
    
    //TTreeFormula to apply cuts in the output tree
    std::vector<TString> output_tree_cuts = TMVA::GetCutFormula(TMVA::MainOptionsParser(config_file,"output_tree_cuts").second.c_str());
    //Loop over the cuts in case of cross validation running mode, all outputs to be created accordingly

    unsigned int loop_index=0;
    for(auto output_tree_cut : output_tree_cuts)
      {
	// Create a new root output file.
	TString outfileName;
	if(TMVA::MainOptionsParser(config_file,"training_method").second=="default")
	  {
	    if(use_config_file)outfileName = TMVA::MainOptionsParser(config_file,"tree_output_file_name_prefix").second+output_sample.first+TMVA::MainOptionsParser(config_file,"tree_output_file_name_suffix").second+".root";
	    else{
	      std::size_t pos = file_input.find("user"); 
	      outfileName = (TString)file_input.substr (pos); 
	    }
	  }
	else if(TMVA::MainOptionsParser(config_file,"training_method").second=="cross_validation")
	  {
	    if(use_config_file)
	      outfileName = "cv_output_lab_"+TMVA::GetTitleExtension(output_tree_cut)+"_"+TMVA::MainOptionsParser(config_file,"tree_output_file_name_prefix").second+output_sample.first+TMVA::MainOptionsParser(config_file,"tree_output_file_name_suffix").second+".root";

	    else{
	      //Looking for a pattern in the input file for the fit input production, then passing an hard coded shift for the definition of the input file name 
	      std::size_t pos = file_input.find("_merged/"); 
	      outfileName = "cv_output_lab_"+TMVA::GetTitleExtension(output_tree_cut)+"_"+(TString)file_input.substr (pos+8); 
	    }
	  }

	//load the input file and open the tree
	TFile *input(0);
	TMVA::PrintUtil("INFO Getting input file",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	TString fname;
	if(use_config_file)fname=TMVA::MainOptionsParser(config_file,"input_files_dir").second+"/"+output_sample.second;
	else fname=(TString)file_input;
	input = TFile::Open( fname );

	TMVA::PrintUtil("INFO Preparing the output file",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	TTree* theTree = NULL;
	TMVA::PrintUtil("INFO Loading the tree",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	char tree_name[1000]; sprintf(tree_name,"%s",TMVA::MainOptionsParser(config_file,"tree_name").second.c_str());
	theTree = (TTree*)input->Get(tree_name);
	
	TMVA::PrintUtil("INFO Creating the output tree",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	TFile* outputFile = TFile::Open( outfileName, "RECREATE" );
	TMVA::PrintUtil("INFO niko 1 Creating the output tree",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	TTree* outputTree = (TTree*)theTree->CloneTree(0);
	TMVA::PrintUtil("INFO niko 2 Creating the output tree",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);

	//Adding to the output tree all the variables used for the MVA calculation
	TMVA::PrintUtil("INFO Adding the branches to the output tree",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	const unsigned int index_variables=variables.size();
	Float_t local_variables[index_variables];

	//Adding to the output all the MVA classifiers
	TMVA::PrintUtil("INFO Initialise the readers",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	const unsigned int index_classifiers=signal_samples.size();
	//Those are the output classifiers
	Float_t local_signals[index_classifiers];
	std::vector<TMVA::Reader * > readers;
	unsigned int in_s=0;
	for(auto sig : signal_samples){
	  TString sig_s=sig.first;
	  char sig_c[1000]; sprintf(sig_c,"%s",sig.first.c_str()); 
	  outputTree->Branch(sig_c,&local_signals[in_s],sig_s); 
	  TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );
	  readers.push_back(reader);
	  in_s++;
	}

	//Adding now variables to the readers
	TString method =  "BDT method";
	TMVA::PrintUtil("INFO Adding all variables to the MVA reader and loading xml files, all variables will be converted to float beforehand",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	unsigned int reader_index=0;
	for(auto reader : readers){
	  unsigned int in_v_reader=0;
	  for(auto var : variables ){
	    char var_c[1000]; sprintf(var_c,"%s",var.c_str());
	    TMVA::PrintUtil("INFO Adding a variable to the reader "+(TString)var_c,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	    reader->AddVariable(var_c , &local_variables[in_v_reader] );
	    in_v_reader++;
	  }
	  TString weight_file;
	  TMVA::PrintUtil("INFO Defining the xml file",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	  if(TMVA::MainOptionsParser(config_file,"training_method").second=="default")
	    {
	      weight_file = TMVA::MainOptionsParser(config_file,"weight_directory").second+"/"+TMVA::MainOptionsParser(config_file,"factory_name").second
		+signal_samples[reader_index].first+"_naive_training"+"_BDTG.weights.xml";
	    }
	  else if(TMVA::MainOptionsParser(config_file,"training_method").second=="cross_validation")
	    {
	      //Assuming cross training with two samples, to make sure to use orthogonal samples
	      //weight_file = TMVA::MainOptionsParser(config_file,"weight_directory").second+"/"+TMVA::MainOptionsParser(config_file,"factory_name").second
	      //+signal_samples[reader_index].first+"_cross_val_sample_"+TMVA::GetTitleExtension(output_tree_cut)+"_BDTG.weights.xml";

	      if(loop_index==0){
		weight_file = TMVA::MainOptionsParser(config_file,"weight_directory").second+"/"+TMVA::MainOptionsParser(config_file,"factory_name").second
		  +signal_samples[reader_index].first+"_cross_val_sample_"+TMVA::GetTitleExtension(output_tree_cuts[1])+"_BDTG.weights.xml";
	      }
	      else if(loop_index==1){
		weight_file = TMVA::MainOptionsParser(config_file,"weight_directory").second+"/"+TMVA::MainOptionsParser(config_file,"factory_name").second
		  +signal_samples[reader_index].first+"_cross_val_sample_"+TMVA::GetTitleExtension(output_tree_cuts[0])+"_BDTG.weights.xml";
	      }
	      else{
		TMVA::PrintUtil("Error, no viable configuration for the cross training",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
		exit(0);
	      }
	    }
	  else
	    {
	      TMVA::PrintUtil("Error, training_method not found",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
	      exit(0);
	    }
	  
	  TMVA::PrintUtil("INFO Reading weight file "+weight_file+" corresponding to the cut "+output_tree_cut,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	  reader->BookMVA( method, weight_file );
	  reader_index++;
	}

	//char cut[1000]; sprintf(cut,"%s",output_tree_cut);
	//TTreeFormula output_cuts("output_cuts",cut,theTree);
	TMVA::PrintUtil("INFO Defining the cut for the output tree",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	TTreeFormula output_cuts("output_cuts",output_tree_cut,theTree);
	output_cuts.GetNdata();
	
	TMVA::PrintUtil("INFO Loading variables from the input tree",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	TMVA::PrintUtil("INFO Assuming that all variables are Float_t",TMVA::P_COL::RED,TMVA::P_STY::BOLD);

	TMVA::PrintUtil("INFO Getting the tree branches form the header file TMVAHBSM.h",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);

#include "TMVAHBSM.h"

	/* Not ready yet
	   TMVA::PrintUtil("INFO Booking the histograms",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	   std::vector< std::pair< TString,TString> > regions_and_cuts=HistoManager::Regions();
	   HistoManager::DebugRegions(regions_and_cuts);
	   HistoManager::BookHistograms(regions_and_cuts,500,0,5000,"meff");
	   HistoManager::BookHistograms(regions_and_cuts,200,-1,1,"BDTG");
	   HistoManager::BookHistograms(regions_and_cuts,50,0,50,"jets_n");
	   HistoManager::BookHistograms(regions_and_cuts,50,0,50,"bjets_n");
	   HistoManager::BookHistograms(regions_and_cuts,10,0,10,"mtjets_n");
	*/

	TStopwatch sw;
	sw.Start();
	Int_t nEvent = theTree->GetEntries();
	for (Long64_t ievt=0; ievt<nEvent; ievt++) {
	  if (ievt%5000 == 0) { 
	    char ci[1000]; sprintf(ci,"%lli over %i",ievt,nEvent);
	    TString prog="INFO Progessing input event "+(TString)(ci);
	    TMVA::PrintUtil(prog,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL); 
	  }
      
	  theTree->GetEntry(ievt);
	  if( ! output_cuts.EvalInstance() ) continue;

	  local_variables[0]=jets_n;
	  local_variables[1]=meff;
	  local_variables[2]=bjets_n;
	  local_variables[3]=mtjets_n;
	  local_variables[4]=loosemtjets_n;
	  local_variables[5]=J_leadingb_invariant_mass;
	  local_variables[6]=J_lepton_invariant_mass;
	  local_variables[7]=centrality;
	  local_variables[8]=jets40_n;
	  local_variables[9]=mbb_leading_bjets;
	  local_variables[10]=mbb_softest_bjets;
	  local_variables[11]=jets_pt->at(0);
	  local_variables[12]=jets_pt->at(1);
	  local_variables[13]=jets_pt->at(2);
	  local_variables[14]=jets_pt->at(3);
	  local_variables[15]=jets_pt->at(4);
	  if(loosemtjets_m->size()!=0) local_variables[16]=loosemtjets_m->at(0);
	  else local_variables[16]=-100;
	  if(loosemtjets_pt->size()!=0) local_variables[17]=loosemtjets_pt->at(0);
	  else local_variables[17]=-100;
	  if(loosemtjets_nconsts->size()!=0) local_variables[18]=loosemtjets_nconsts->at(0);
	  else local_variables[18]=-100;
	  local_variables[19]=fabs(dPhi_lepmet);
	  local_variables[20]=dRaverage_jetjet;
	  local_variables[21]=dRmin_jetjet;
	  local_variables[22]=fabs(dPhi_lepjet);
	  local_variables[23]=fabs(dPhi_lepbjet);

	  unsigned int reader_index=0;
	  for(auto reader : readers) {
	    local_signals[reader_index] = reader->EvaluateMVA( method );
	    reader_index++;
	  }

	  outputTree->Fill();
	}
	// get elapsed time
	sw.Stop();
	TMVA::PrintUtil("INFO End of the event loop",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	sw.Print();
    
	input->Close();

	TMVA::PrintUtil("INFO Writing output files",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	outputFile->Write();
	outputFile->Close();

	TString file_created="INFO Created output file "+(TString)outfileName.Data()+" containing the MVA output histograms";
	TMVA::PrintUtil(file_created,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);

	for(auto reader : readers) delete reader;
	readers.clear();

	//incrementing the index
	loop_index++;
	
	//need to understand how to make sure that I don't have a memory leak.. 
	/*
	delete input;
	delete theTree;	
	delete outputFile;
	delete outputTree;
	*/	
      }

    TMVA::PrintUtil("\n>   Done with ApplicationCreateCombinedTree function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
  }

  void CleanDir()
  {
    TMVA::PrintUtil("\n>   In CleanDir function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    gROOT->ProcessLine(".!rm *.root");
    gROOT->ProcessLine(".!rm weights/*.*");
    TMVA::PrintUtil("\n>   Done with CleanDir function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
  }

  std::vector<std::string> GetMVAVariables(int configuration_type)
  {
    TMVA::PrintUtil("\n>   In GetMVAVariables function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    //default set is currently 2
    std::vector<std::string> MVAVariables;

    if(configuration_type==0)
      {
	MVAVariables.push_back("jets_n"  );
	MVAVariables.push_back("meff"    );
	MVAVariables.push_back("bjets_n" );
	MVAVariables.push_back("mtjets_n");
      }
    else if(configuration_type==1)
      {
	MVAVariables.push_back("jets_n"                           );
	MVAVariables.push_back("meff"                             );
	MVAVariables.push_back("bjets_n"                          );
	MVAVariables.push_back("mtjets_n"                         );
	MVAVariables.push_back("loosemtjets_n"                    );
	MVAVariables.push_back("J_leadingb_invariant_mass"        );
	MVAVariables.push_back("J_lepton_invariant_mass"          );
	MVAVariables.push_back("centrality"                       );
	MVAVariables.push_back("jets40_n"                         );
	MVAVariables.push_back("mbb_leading_bjets"                );
	MVAVariables.push_back("mbb_softest_bjets"                );
	MVAVariables.push_back("jets_pt[0]"                       );
	MVAVariables.push_back("jets_pt[1]"                       );
	MVAVariables.push_back("jets_pt[2]"                       );
	MVAVariables.push_back("jets_pt[3]"                       );
	MVAVariables.push_back("jets_pt[4]"                       );
	MVAVariables.push_back("Alt$(loosemtjets_m[0],-100)"      );
	MVAVariables.push_back("Alt$(loosemtjets_pt[0],-100)"     );
	MVAVariables.push_back("Alt$(loosemtjets_nconsts[0],-100)");
	MVAVariables.push_back("fabs(dPhi_lepmet)"                );
	MVAVariables.push_back("dRaverage_jetjet"                 );
	MVAVariables.push_back("dRmin_jetjet"                     );
	MVAVariables.push_back("fabs(dPhi_lepjet)"                );
	MVAVariables.push_back("fabs(dPhi_lepbjet)"               );
      }
    else{
      TMVA::PrintUtil("Error, no variables are loaded",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
      exit(0);
    }

    TMVA::PrintUtil("Using the following set of variables ",TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
    for(unsigned int i=0; i < MVAVariables.size(); i++)	TMVA::PrintUtil(MVAVariables[i],TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);

    TMVA::PrintUtil("\n>   Done with GetMVAVariables function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    return MVAVariables;
  }
  

  std::vector< std::pair<std::string, std::string> > GetSamples(TString list)
  {
    TMVA::PrintUtil("\n>   In GetSamples function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    std::vector< std::pair<std::string, std::string> > files_and_keys;
    size_t foundField ;
    std::string sFileRow;
    fstream FileOpen;  FileOpen.open(list,fstream::in);
    std::string sfield, skey, sfile;
    float weight;
    while( getline(FileOpen,sFileRow) ) {
      istringstream ss(sFileRow);
      foundField=sFileRow.find("sample");
      if(foundField  != string::npos )
	{
	  ss>>sfield>>skey>>sfile>>weight;
	  char w[1000]; sprintf(w,"%f",weight);
	  TString in_file=sfield+" "+skey+" "+sfile+" "+w;
	  TMVA::PrintUtil(in_file,TMVA::P_COL::MAGENTA,TMVA::P_STY::NORMAL);
	  files_and_keys.push_back(std::make_pair(skey,sfile));
	}
    }
    
    if(files_and_keys.size()==0){ TMVA::PrintUtil("Error, the file list is empty or badly formatted",TMVA::P_COL::RED,TMVA::P_STY::BOLD); exit(0); }
    TMVA::PrintUtil("\n>   Done with GetSamples function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    return files_and_keys;
  }


  void Plots(std::string signal_samples,std::string out_plot_dir)
  {
    TMVA::PrintUtil("\n>   In Plots function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
    char command[1000]; sprintf(command,".x TMVAPlots.C(\"%s\",\"%s\")",signal_samples.c_str(),out_plot_dir.c_str());
    gROOT->ProcessLine(command);
    TMVA::PrintUtil("\n>   Done with Plots function\n",TMVA::P_COL::BLUE,TMVA::P_STY::UNDERLINE);
  }
  

} // namespace TMVA

// ------------------------------ Run all ----------------------------------------------------------------
void TMVAHBSM(TString config_file="config/run_config.conf",std::string file_input="")
{
  TMVA::PrintUtil("\nSimple code for MVA analysis, it currenlty support only BDTG.",TMVA::P_COL::RED,TMVA::P_STY::BOLD); std::this_thread::sleep_for(std::chrono::seconds(4));

  TMVA::PrintUtil("\n==============   Start TMVAHBSM\n",TMVA::P_COL::BLUE,TMVA::P_STY::BOLD);

  TMVA::PrintUtil("\n==============   Getting all main options and configuration files\n",TMVA::P_COL::BLUE,TMVA::P_STY::BOLD);
  std::pair<std::string,std::string> option_do_clean_dir = TMVA::MainOptionsParser(config_file,"do_clean_dir");
  std::pair<std::string,std::string> option_do_training = TMVA::MainOptionsParser(config_file,"do_training");
  std::pair<std::string,std::string> option_do_plots = TMVA::MainOptionsParser(config_file,"do_plots");
  std::pair<std::string,std::string> option_do_output_tree = TMVA::MainOptionsParser(config_file,"do_output_tree");

  TString signal_list =TMVA::MainOptionsParser(config_file,"signal_list").second; 
  TString output_list =TMVA::MainOptionsParser(config_file,"output_file_list").second;

  if(option_do_clean_dir.second=="true"){
    TMVA::PrintUtil("\n==============   Start Clean previous root files and xml's\n",TMVA::P_COL::BLUE,TMVA::P_STY::BOLD);
    TMVA::CleanDir();
  }

  TMVA::PrintUtil("\n==============   Getting the MVA variables\n",TMVA::P_COL::BLUE,TMVA::P_STY::BOLD);
  std::vector<std::string> variables = TMVA::GetMVAVariables(1);

  TMVA::PrintUtil("\n==============   Getting the signal samples\n",TMVA::P_COL::BLUE,TMVA::P_STY::BOLD);
  std::vector< std::pair<std::string,std::string> > signal_samples = TMVA::GetSamples(signal_list);

  if(option_do_training.second=="true"){
    TMVA::PrintUtil("\n==============   Running the training\n\n",TMVA::P_COL::BLUE,TMVA::P_STY::BOLD);
    std::pair<std::string,std::string> dummy_signal_input; 
    if(file_input!="")TMVA::Training(variables,dummy_signal_input,config_file,file_input);
    else
      for(auto signal_input : signal_samples) TMVA::Training(variables,signal_input,config_file,file_input);
    std::string weight_directory=TMVA::MainOptionsParser(config_file,"weight_directory").second;
    char mv_command[1000]; sprintf(mv_command,".!mv weights %s",weight_directory.c_str());
    gROOT->ProcessLine(mv_command);
  }

  if(option_do_plots.second=="true"){
    TMVA::PrintUtil("\n==============   Running the plots\n\n",TMVA::P_COL::BLUE,TMVA::P_STY::BOLD);
    std::string output_file_name_prefix=TMVA::MainOptionsParser(config_file,"output_file_name_prefix").second;
    
    if(file_input=="")
      {
	for(auto signal_input : signal_samples) 
	  {
	    std::string file_to_open;
	    if(TMVA::MainOptionsParser(config_file,"training_method").second=="default") 
	      file_to_open=output_file_name_prefix+signal_input.first+"_naive_training.root";
	    else if(TMVA::MainOptionsParser(config_file,"training_method").second=="cross_validation") 
	      {
		TMVA::PrintUtil("ERROR You are trying to run the plot making when running the cross training, this is not supported",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
		exit(0);
	      }
	    TMVA::Plots(file_to_open,TMVA::MainOptionsParser(config_file,"plot_directory_prefix").second);
	  }
      }
    else
      {
	std::string file_to_open;
	if(TMVA::MainOptionsParser(config_file,"training_method").second=="default") 
	  {
	    std::size_t pos_bdt_name = file_input.find("_merged/"); 
	    file_to_open=output_file_name_prefix+(TString)file_input.substr(pos_bdt_name+8,file_input.size()-5)+"_naive_training.root";
	  }
	else if(TMVA::MainOptionsParser(config_file,"training_method").second=="cross_validation") 
	  {
	    TMVA::PrintUtil("ERROR You are trying to run the plot making when running the cross training, this is not supported",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
	    exit(0);
	  }
	TMVA::Plots(file_to_open,TMVA::MainOptionsParser(config_file,"plot_directory_prefix").second);
	
      }
  }

  //Check if you want to use the config file instead running on the file list, 
  //in this manner you avoid looping on the output list entries while processing the same sample when running in batch mode 
  bool use_config_file=true;
  if( file_input=="" ){
    TMVA::PrintUtil("INFO The input and output files will be defined from the configuration file",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
    use_config_file=true;
  }
  else if( file_input!="" ){
    TMVA::PrintUtil("INFO The input and output files will be defined based on arguments of the main function",TMVA::P_COL::RED,TMVA::P_STY::BOLD);
    use_config_file=false;
  }
  

  TMVA::PrintUtil("\n==============   Getting the output samples\n",TMVA::P_COL::BLUE,TMVA::P_STY::BOLD);
  std::vector< std::pair<std::string,std::string> > output_samples = TMVA::GetSamples(output_list);
  
  //Here we don't loop over the available signal samples as we want to have all MVA discriminant already stored in a signle file 
  if(option_do_output_tree.second=="true"){
    TMVA::PrintUtil("\n==============   Output tree production\n\n",TMVA::P_COL::BLUE,TMVA::P_STY::BOLD);
    if(use_config_file) for(auto output_sample : output_samples) TMVA::ApplicationCreateCombinedTree(variables,signal_samples,output_sample,config_file,file_input);
    //else pick a random element in the output_samples vector to make it work TODO, fix this dirty implementation
    else TMVA::ApplicationCreateCombinedTree(variables,signal_samples,output_samples[0],config_file,file_input);
  }

}
