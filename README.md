# simple-ml
Simple scripts for basic machine learning studies, project previously on Gitlab, migration and development in progress  

**Introductory readings**
    
    Boosted decision trees http://inspirehep.net/record/1245931/
    TMVA userguide https://arxiv.org/abs/physics/0703039 
    General intro and tips for machine learning https://homes.cs.washington.edu/~pedrod/papers/cacm12.pdf (including cross-validation)
    Cross validation video https://www.youtube.com/watch?v=TIgfjmp-4BA (we use 2-fold cross validation)

**Supporting notes of analyses of interest**
    
    Scalar+heavy-flavour https://cds.cern.ch/record/2258130 (analysis where we use this package) 
    Hplus https://cds.cern.ch/record/2244619

**Check here for set of initial studies**
    
    Initial study with BDT https://gitlab.cern.ch/orlando/HBSMutilsMVA/issues/17
    Initial study with NN https://gitlab.cern.ch/orlando/HBSMutilsMVA/issues/21
    Slides https://indico.cern.ch/event/600416/contributions/2485460/attachments/1415829/2167690/short.pdf
    DT vs TRF, closure test, all collected here https://indico.cern.ch/event/619040/ 
    A one more summary talk with lots of links is here https://indico.cern.ch/event/600420/contributions/2526419/attachments/1432275/2200676/stalk.pdf 
    A recent HQT talk is here https://indico.cern.ch/event/624645/contributions/2530149/attachments/1434392/2205006/hqt_2017-03-27_nlo.pdf
    First EB talk https://indico.cern.ch/event/635887/ 
    
**Setting the package up**

    git clone ssh://git@gitlab.cern.ch:7999/orlando/HBSMutilsMVA.git
    setupATLAS
    rcSetup Base,2.4.24  //just to set up a new root version
    
    Otherwise replace with: 
    export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
    source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
    rcSetup Base,2.4.24
    
**The current plotting code is based on the ROOT script**

    https://root.cern.ch/doc/v608/TMVAGui_8cxx_source.html
    
**Typical memory usage as follows**

    PID   USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND                                                                                                                                       
    25686 orlando   30  10  867m 563m 3560 R 99.1  1.9  58:58.04 root.exe        
 
**Ingredients to run**

    //Run the code as 
    root -l TMVAHBSM.C
    //Two config files are given a main one 
    Root/config/run_config.conf 
    //and a test one 
    Root/config/run_config_test.conf
    //Please make sure to have your file lists in place, signals and output files like 
    Root/data/signal_list_test.txt 
    Root/data/output_file_list_test_2.txt
    //Please make sure that the format is correct..  

**A set of tested instructions to run with screen are here** 

    https://lhcb.github.io/analysis-essentials/shell/persistent-screen.html 

**Example of config file**    
    
Please note that the code expects a full set of options, if something is not specified the code will crash. The config files under config/ are always up to date
    
    
    //Main options     
    option do_clean_dir false //clean root files and default MVA weight folder
    option do_training true //do training 
    option do_plots false //do control plots -> currently very slow
    option do_output_tree true //prepare output tree
    option training_method default //method to train the BDT alternative is cross_validation. The cross validation requires aslo different factory options (numbers of events to pick for the training)  
    
    //Secondary options
    option factory_options !V:!Silent:Transformations=I;D;P;G,D:AnalysisType=Classification //factory options, don't touch them unless you know what you are doing 
    option tree_name tree //name of the input tree
    option cuts_signal (jets_n>=5&&bjets_n>=3) //cuts on the signal
    option cuts_background (jets_n>=5&&bjets_n>=3) //cuts on the background 
    option signal_event_weight nomWeight_weight_btag*nomWeight_weight_elec*nomWeight_weight_elec_trigger*nomWeight_weight_jvt*nomWeight_weight_mc*nomWeight_weight_muon*nomWeight_weight_muon_trigger*nomWeight_weight_norm*nomWeight_weight_pu  //signal event weight
    option background_event_weight nomWeight_weight_btag*nomWeight_weight_elec*nomWeight_weight_elec_trigger*nomWeight_weight_jvt*nomWeight_weight_mc*nomWeight_weight_muon*nomWeight_weight_muon_trigger*nomWeight_weight_norm*nomWeight_weight_pu  //background event weight
    option training_and_test_tree_config nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V  //configuration for train vs testing split, don't touch them unless you know what you are doing 
    option mva_method BDTG //mva method, currently it is the only supported one
    option mva_method_option !H:!V:NTrees=1000:BoostType=Grad:Shrinkage=0.30:UseBaggedBoost:BaggedSampleFraction=0.6:SeparationType=GiniIndex:nCuts=20:MaxDepth=2  //options for the MVA, don't touch them unless you know what you are doing
    option signal_list data/signal_list.txt  //signal list for the training and testing 
    option output_file_list data/output_file_list.txt //output file list (sample where you want the MVA to be calculated)
    option plot_directory_prefix plots_v2_2017-02-20_ //directory where to store the plots
    option background_sample  ttbarlight.root,ttbarcc.root,ttbarbb.root //comma separated list of files for the background
    option signal_weight 1 //signal global weight
    option background_weight 1 //background global weight
    option output_file_name_prefix TMVASignalBackground_  //prefix for output file-name for the MVA control plots and trees
    option factory_name TMVAMultiBkg_  //factory name
    option tree_output_file_name_prefix MVA_output_tree_  //final output file name prefix
    option tree_output_file_name_suffix _v0  //final output file name suffix
    option weight_directory weights_v0_2017-02-20 //directory where to store the weights 
    option output_tree_cuts (jets_n>=5&&bjets_n>=3)  //cuts to be applied in final output tree
    option input_files_dir eos/atlas/user/o/orlando/VLQ_hbsm_2017-04-10_dt_v0_merged/ //This is the folder where all files are stored, including inputs for the training and inputs to make the fit ntuples 
    
***Relevant tags***

The list of tags relevant to preproduce studies etc is documented below 

* This tag https://gitlab.cern.ch/orlando/HBSMutilsMVA/tags/00-00-07 was used for the BDT optimisation
* The most recent tag with several new functionalities is this https://gitlab.cern.ch/orlando/HBSMutilsMVA/tags/00-00-12  

***Current limitations***

* The main current limitation appears when trying to prepare the output files for the MVA to be used for TRexFitter. The procedure, see https://gitlab.cern.ch/orlando/HBSMutilsMVA/blob/00-00-07/Root/TMVAHBSM.C#L356 , involves writing by hand all variables and ordering them properly .. **be very careful with this step**
* The do_plots option only works without cross validation

### Tutorial
This tutorial covers training and testing phases. 

***Part 1: preparation for training*** 

    As mentioned abouve, write your own set of variables here https://gitlab.cern.ch/orlando/HBSMutilsMVA/blob/00-00-07/Root/TMVAHBSM.C#L356 

    Prepare your signal file list as needed by the training. Examples are given under Root/data.  
    Format for each sample: 
    
    sample bbhtt_m_800   bbtt_800.root   1 
    
    "sample" is a mandatory keyword. 
    bbhtt_m_800 is the sample name.  
    bbtt_800.root is the root file name. 
    "1" is a normalisation correction you may want to apply (usually set to 1)
    
    From the default config file used for the training you might need to change the following lines
    The code runs as default with this config file 
    
    Root/config/run_config.conf
    
    if you want to run on a custom config file see below for instructions. 
    
    option tree_name tree //name of the input tree
    option cuts_signal (jets_n>=5&&bjets_n>=3) //cuts on the signal
    option cuts_background (jets_n>=5&&bjets_n>=3) //cuts on the background 
    option signal_event_weight nomWeight_weight_btag*nomWeight_weight_elec*nomWeight_weight_elec_trigger*nomWeight_weight_jvt*nomWeight_weight_mc*nomWeight_weight_muon*nomWeight_weight_muon_trigger*nomWeight_weight_norm*nomWeight_weight_pu  //signal event weight
    option background_event_weight nomWeight_weight_btag*nomWeight_weight_elec*nomWeight_weight_elec_trigger*nomWeight_weight_jvt*nomWeight_weight_mc*nomWeight_weight_muon*nomWeight_weight_muon_trigger*nomWeight_weight_norm*nomWeight_weight_pu  //background event weight
    option signal_list data/signal_list.txt  //signal list for the training and testing 
    option plot_directory_prefix plots_v2_2017-02-20_ //directory where to store the plots
    option background_sample  ttbarlight.root,ttbarcc.root,ttbarbb.root //comma separated list of files for the background
    option output_file_name_prefix TMVASignalBackground_  //prefix for output file-name for the MVA control plots and trees
    option factory_name TMVAMultiBkg_  //factory name
    option tree_output_file_name_prefix MVA_output_tree_  //final output file name prefix
    option tree_output_file_name_suffix _v0  //final output file name suffix
    option weight_directory weights_v0_2017-02-20 //directory where to store the weights 
    option input_files_dir eos/atlas/user/o/orlando/VLQ_hbsm_2017-04-10_dt_v0_merged/ //This is the folder where all files are stored, including inputs for the training and inputs to make the fit ntuples
    
    After that is done you are ready to run the training
    
***Part 2: run the training without cross validation***
    
    Set the main options to:
    option do_clean_dir false //clean root files and default MVA weight folder
    option do_training true //do training 
    option do_plots false //do control plots -> currently very slow
    option do_output_tree false //prepare output tree
    option training_method default //method to train the BDT alternative is cross_validation. The cross validation requires aslo different factory options (numbers of events to pick for the training)  
    
    and run with a custom file
    root -l -b -q TMVAHBSM.C
    or run with a customised configuration file
    root -l -b -q TMVAHBSM.C\(\"config/run_config.conf\"\)
    
    then prepare some plots by switching 
    option do_training false //do training 
    option do_plots true //do control plots -> currently very slow
    and rerun again as just above
    This will make a set of standard control plots  
