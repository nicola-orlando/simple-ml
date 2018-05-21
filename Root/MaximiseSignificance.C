// ------------------------------ call of Genetic algorithm  ----------------------------------------------------------------
//With the application tree, the significance is maximized with the help of the TMVA genetic algrorithm.
void MaximizeSignificance(){
       
  // define all the parameters by their minimum and maximum value
  // in this example 3 parameters (=cuts on the classifiers) are defined. 
  vector<Interval*> ranges;
  ranges.push_back( new Interval(-1,1) ); // for some classifiers (especially LD) the ranges have to be taken larger
  ranges.push_back( new Interval(-1,1) );
  ranges.push_back( new Interval(-1,1) );

  std::cout << "Classifier ranges (defined by the user)" << std::endl;
  for( std::vector<Interval*>::iterator it = ranges.begin(); it != ranges.end(); it++ ){
    std::cout << " range: " << (*it)->GetMin() << "   " << (*it)->GetMax() << std::endl;
  }

  TChain* chain = new TChain("multiBkg");
  chain->Add("tmva_example_multiple_backgrounds__applied.root");

  IFitterTarget* myFitness = new MyFitness( chain );

  // prepare the genetic algorithm with an initial population size of 20
  // mind: big population sizes will help in searching the domain space of the solution
  // but you have to weight this out to the number of generations
  // the extreme case of 1 generation and populationsize n is equal to 
  // a Monte Carlo calculation with n tries

  const TString name( "multipleBackgroundGA" );
  const TString opts( "PopSize=100:Steps=30" );

  GeneticFitter mg( *myFitness, name, ranges, opts);
  // mg.SetParameters( 4, 30, 200, 10,5, 0.95, 0.001 );

  std::vector<Double_t> result;
  Double_t estimator = mg.Run(result);

  dynamic_cast<MyFitness*>(myFitness)->Print();
  std::cout << std::endl;

  int n = 0;
  for( std::vector<Double_t>::iterator it = result.begin(); it<result.end(); it++ ){
    std::cout << "  cutValue[" << n << "] = " << (*it) << ";"<< std::endl;
    n++;
  }
	
}
