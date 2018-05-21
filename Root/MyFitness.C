//Removed from main code, to be pushed back or fixed later if/when necessary
// ------------------------------ Genetic Algorithm Fitness definition ----------------------------------------------------------------
class MyFitness : public IFitterTarget 
{
public:
  // constructor
  MyFitness( TChain* _chain ) : IFitterTarget() {
    chain = _chain;

    hSignal = new TH1F("hsignal","hsignal",100,-1,1);
    hFP = new TH1F("hfp","hfp",100,-1,1);
    hTP = new TH1F("htp","htp",100,-1,1);

    TString cutsAndWeightSignal  = "weight*(classID==0)";
    nSignal = chain->Draw("Entry$/Entries$>>hsignal",cutsAndWeightSignal,"goff");
    weightsSignal = hSignal->Integral();

  }
       
  // the output of this function will be minimized
  Double_t EstimatorFunction( std::vector<Double_t> & factors ){

    TString cutsAndWeightTruePositive  = Form("weight*((classID==0) && cls_bbhtt>%f && cls_tthbb>%f && cls_tthtt>%f )",factors.at(0), factors.at(1), factors.at(2));
    TString cutsAndWeightFalsePositive = Form("weight*((classID >0) && cls_bbhtt>%f && cls_tthbb>%f && cls_tthtt>%f )",factors.at(0), factors.at(1), factors.at(2));
	  
    // Entry$/Entries$ just draws something reasonable. Could in principle anything
    Float_t nTP = chain->Draw("Entry$/Entries$>>htp",cutsAndWeightTruePositive,"goff");
    Float_t nFP = chain->Draw("Entry$/Entries$>>hfp",cutsAndWeightFalsePositive,"goff");

    weightsTruePositive = hTP->Integral();
    weightsFalsePositive = hFP->Integral();

    efficiency = 0;
    if( weightsSignal > 0 )
      efficiency = weightsTruePositive/weightsSignal;
	  
    purity = 0;
    if( weightsTruePositive+weightsFalsePositive > 0 )
      purity = weightsTruePositive/(weightsTruePositive+weightsFalsePositive);

    Float_t effTimesPur = efficiency*purity;

    Float_t toMinimize = std::numeric_limits<float>::max(); // set to the highest existing number 
    if( effTimesPur > 0 ) // if larger than 0, take 1/x. This is the value to minimize
      toMinimize = 1./(effTimesPur); // we want to minimize 1/efficiency*purity

    // Print();

    return toMinimize;
  }


  void Print(){
    std::cout << std::endl;
    std::cout << "======================" << std::endl
	      << "Efficiency : " << efficiency << std::endl
	      << "Purity     : " << purity << std::endl << std::endl
	      << "True positive weights : " << weightsTruePositive << std::endl
	      << "False positive weights: " << weightsFalsePositive << std::endl
	      << "Signal weights        : " << weightsSignal << std::endl;
  }

  Float_t nSignal;

  Float_t efficiency;
  Float_t purity;
  Float_t weightsTruePositive;
  Float_t weightsFalsePositive;
  Float_t weightsSignal;


private:
  TChain* chain;
  TH1F* hSignal;
  TH1F* hFP;
  TH1F* hTP;

}
