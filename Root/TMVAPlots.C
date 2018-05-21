Bool_t isRegression = kFALSE; 
Bool_t useTMVAStyle = kFALSE; 

Bool_t do_efficiencies = kFALSE;

//based upon https://root.cern.ch/doc/v606/correlations_8cxx_source.html with fixes to graphics.. 
void PlotCorrelation(TString fin)
{
  TFile* file = new TFile( fin );  
  TString hName[2] = { "CorrelationMatrixS", "CorrelationMatrixB" };
  const Int_t width = 800;

  for (Int_t ic=0; ic<2; ic++) {

    TH2* h2 = dynamic_cast<TH2*> (file->Get( hName[ic] ));
    if(!h2) 
      {
	std::cout << "Did not find histogram " << hName[ic] << " in " << fin << std::endl;
	continue;
      }
    
    TCanvas* c = new TCanvas( hName[ic], Form("Correlations between MVA input variables (%s)", (ic==0 ? "signal" : "background")), ic*(width+5)+200, 0, width, width ); 
    Float_t newMargin1 = 0.12;
    Float_t newMargin2 = 0.18;
    
    c->SetGrid();
    c->SetTicks();
    c->SetLeftMargin  ( 0.17 );
    c->SetBottomMargin( 0.17 );
    c->SetRightMargin ( 0.13 );
    c->SetTopMargin   ( 0.04 );
    gStyle->SetPalette( kTemperatureMap );
    
    gStyle->SetPaintTextFormat( "3g" );
    h2->SetMarkerSize( 1. );
    h2->SetMarkerColor( 0 );

    Float_t labelSize = 0.018;
    h2->GetXaxis()->SetLabelSize( labelSize );
    h2->GetYaxis()->SetLabelSize( labelSize );
    h2->GetZaxis()->SetLabelSize( labelSize );
    h2->LabelsOption( "d" );
    h2->SetLabelOffset( 0.01 );// label offset on x axis    
    
    h2->Draw("colz"); // color pads   
    c->Update();
    
    // modify properties of paletteAxis
    TPaletteAxis* paletteAxis = (TPaletteAxis*)h2->GetListOfFunctions()->FindObject( "palette" );
    paletteAxis->SetLabelSize( 0.01 );
    paletteAxis->SetX1NDC( paletteAxis->GetX1NDC() + 0.02 );
    
    h2->Draw("textsame");  // add text
    
    c->Update();
    
    TString fname = "plots/";
    fname += hName[ic]+".eps";
    c->SaveAs(fname);

  }

}

int TMVAPlots(TString signal="bbhtt" , TString output_dir="plots_v0_2017-02-17_" , Bool_t do_interactive=kFALSE)
{
  gROOT  ->ProcessLine(".x AtlasStyle.C");
  std::cout<<"Opening root file "<<signal<<std::endl;

  TString input=signal;

  if(do_interactive)TMVA::TMVAGui( input );
  else{
    std::vector<std::string> input_variables;
    input_variables.push_back("InputVariables_Id");
    input_variables.push_back("InputVariables_Deco");
    input_variables.push_back("InputVariables_PCA");
    input_variables.push_back("InputVariables_Gauss_Deco");

    std::cout<<"-- TMVA::variables"<<std::endl;
    for( auto var_name : input_variables ) TMVA::variables(input,var_name,"TMVA Input Variables",isRegression,useTMVAStyle);
    //TMVA::CorrGui(input,"InputVariables_Id","TMVA Input Variable",isRegression);
    
    std::cout<<"-- TMVA::correlations"<<std::endl;
    gROOT  ->ProcessLine(".x AtlasStyle.C");
    //Original TMVA method TMVA::correlations(input,false,false,false);
    PlotCorrelation(input);

    std::cout<<"-- TMVA::mvas"<<std::endl;
    TMVA::mvas(input,TMVA::HistType::kMVAType,useTMVAStyle);
    TMVA::mvas(input,TMVA::HistType::kCompareType,useTMVAStyle);
    //not available for BDT's
    //TMVA::mvas(input,TMVA::HistType::kRarityType,useTMVAStyle);
    
    if(do_efficiencies)
      {
	std::cout<<"-- TMVA::mvaeffs"<<std::endl;
	TMVA::mvaeffs(input,useTMVAStyle);
	//Second argument as in here https://root.cern.ch/doc/v606/efficiencies_8cxx_source.html
	std::cout<<"-- TMVA::efficiencies"<<std::endl;
	TMVA::efficiencies(input,1);
	TMVA::efficiencies(input,2);
	TMVA::efficiencies(input,3);
      }

    output_dir=output_dir+signal;
    std::cout<<"Final output directory is "<< output_dir<<std::endl;
    TString root_mv=".! mv plots "+output_dir;
    gROOT  ->ProcessLine(root_mv);
  }
  
  return 0;
}

