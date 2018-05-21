// Declaration of leaf types                                                                                                                               
Double_t        J_J_invariant_mass;
Double_t        J_leadingb_invariant_mass;
Double_t        J_lepton_invariant_mass;
Int_t           bjets_n;
Double_t        centrality;
Double_t        dRmin_bb;
Int_t           jets40_n;
Int_t           jets_n;
vector<double>  *jets_pt=0;
Double_t        mbb_leading_bjets;
Double_t        mbb_softest_bjets;
Double_t        meff;
Int_t           mtjets_n;
vector<double>  *loosemtjets_eta=0;
vector<double>  *loosemtjets_m=0;
Int_t           loosemtjets_n;
vector<double>  *loosemtjets_nconsts=0;
vector<double>  *loosemtjets_phi=0;
vector<double>  *loosemtjets_pt=0;
Int_t           ttjets_n;
Double_t        dPhi_lepmet;
Double_t        dPhi_lepjet;
Double_t        dPhi_lepbjet;
Double_t        dRaverage_jetjet;
Double_t        dRmin_jetjet;

theTree->SetBranchAddress("J_J_invariant_mass", &J_J_invariant_mass);
theTree->SetBranchAddress("J_leadingb_invariant_mass", &J_leadingb_invariant_mass);
theTree->SetBranchAddress("J_lepton_invariant_mass", &J_lepton_invariant_mass);
theTree->SetBranchAddress("bjets_n", &bjets_n);
theTree->SetBranchAddress("centrality", &centrality);
theTree->SetBranchAddress("dRmin_bb", &dRmin_bb);
theTree->SetBranchAddress("jets40_n", &jets40_n);
theTree->SetBranchAddress("jets_n", &jets_n);
theTree->SetBranchAddress("jets_pt", &jets_pt,NULL);
theTree->SetBranchAddress("mbb_leading_bjets", &mbb_leading_bjets);
theTree->SetBranchAddress("mbb_softest_bjets", &mbb_softest_bjets);
theTree->SetBranchAddress("meff", &meff);
theTree->SetBranchAddress("mtjets_n", &mtjets_n);
theTree->SetBranchAddress("loosemtjets_eta", &loosemtjets_eta,NULL);
theTree->SetBranchAddress("loosemtjets_m", &loosemtjets_m);
theTree->SetBranchAddress("loosemtjets_n", &loosemtjets_n);
theTree->SetBranchAddress("loosemtjets_nconsts", &loosemtjets_nconsts,NULL);
theTree->SetBranchAddress("loosemtjets_phi", &loosemtjets_phi,NULL);
theTree->SetBranchAddress("loosemtjets_pt", &loosemtjets_pt,NULL);
theTree->SetBranchAddress("ttjets_n", &ttjets_n);
theTree->SetBranchAddress("dPhi_lepmet", &dPhi_lepmet);
theTree->SetBranchAddress("dPhi_lepjet", &dPhi_lepjet);
theTree->SetBranchAddress("dPhi_lepbjet", &dPhi_lepbjet);
theTree->SetBranchAddress("dRaverage_jetjet", &dRaverage_jetjet);
theTree->SetBranchAddress("dRmin_jetjet", &dRmin_jetjet);



 
