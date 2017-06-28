#include <iostream>
#include <numeric>
#include <functional>

#include <cblas.h>
#include <lapacke.h>

#include "optim/simulated_annealing_module.hpp"
#include "statistics-utilities/misc_stats_utilities.hpp"


/*
 * Constructor.
 *
 * Note that the candidate generation range is [-sqrt(3), sqrt(3)), which gives
 * a standard deviation of 1.0 for a uniform distribution.
 */
SimulatedAnnealingModule::SimulatedAnnealingModule(int stateDim,
                                                   int randomSeed,
												   int maxCoolingIter,
												   int numCostSamples,
												   real xi,
												   real convRelTol,
												   real initTemp,
												   real coolingFactor,
												   real stepLengthFactor,
												   real *initState,
												   costFunction cf,
												   bool useAdaptiveCandidateGeneration)
	: useAdaptiveCandidateGeneration_(useAdaptiveCandidateGeneration)
	, seed_(randomSeed)
	, stateDim_(stateDim)
	, maxCoolingIter_(maxCoolingIter)
	, numCostSamples_(numCostSamples)
	, beta_(0.11f)
	, xi_(xi)
	, convRelTol_(convRelTol)
	, temp_(initTemp)
	, coolingFactor_(coolingFactor)
	, stepLengthFactor_(stepLengthFactor)
	, rng_(seed_)
	, candGenDistr_(-std::sqrt(3.0f), std::sqrt(3.0f))
	, candAccDistr_(0.0, 1.0)
//	, evaluateCost(cf)
	, calculateCovarianceMatrix()
{
	// allocate memory for internal state vectors:
	crntState_ = new real[stateDim_];
	candState_ = new real[stateDim_];
	bestState_ = new real[stateDim_];

	// initialise state vectors:
	cblas_scopy(stateDim_, initState, 1, crntState_, 1);
	cblas_scopy(stateDim_, initState, 1, candState_, 1);
	cblas_scopy(stateDim_, initState, 1, bestState_, 1);

	// get cost of inital states:
//	crntCost_ = evaluateCost(crntState_);
//	candCost_ = evaluateCost(candState_);
//	bestCost_ = evaluateCost(bestState_);

	// allocate candidate cost vector:
	costSamples_ = new real[numCostSamples_];

	// should adaptive candidate generation be used:
	if( useAdaptiveCandidateGeneration_ == true )
	{
		// allocate memory for covariance and adaptation matrix:
		stateSampleMatrix_ = new real[stateDim_ * numCostSamples_];
		adaptationMatrix_ = new real[stateDim_ * stateDim_];

		// initialise adaptation matrix as identity matrix:
		for(int i=0; i<stateDim_; i++)
		{
			for(int j=0; j<stateDim_; j++)
			{
				if(i == j)
				{
					adaptationMatrix_[i*stateDim + j] = 1.0f;
				}
				else
				{
					adaptationMatrix_[i*stateDim + j] = 0.0f;
				}
			}
		}
	}
}


/*!
 * Simple constructor. Creates an SimulatedAnnealingModule object, but does
 * not set any of its properties.
 */
SimulatedAnnealingModule::SimulatedAnnealingModule()
{

}


/*!
 * Custom destructor to free memory.
 */
SimulatedAnnealingModule::~SimulatedAnnealingModule()
{
	// free memory occupied by internal state vectors:
	delete[] crntState_;
	delete[] candState_;
	delete[] bestState_;

	delete[] costSamples_;

	// if necessary, free memory occupied by sample and adaptation matrices:
	if( useAdaptiveCandidateGeneration_ == true )
	{
		// free memory occupied by adaptation algorithm matrices:
		delete[] stateSampleMatrix_;
		delete[] adaptationMatrix_;
	}
}


/*!
 * Sets parameters of the simulated annealing algorithm. Will throw an error
 * if any required parameter without defaults is not set. Ignores unknown 
 * parameters.
 */
void
SimulatedAnnealingModule::setParams(std::map<std::string, real> params)
{
    // PRNG seed:
    if( params.find("saSeed") != params.end() )
    {
        seed_ = params["saSeed"];
    }
    else
    {
        // TODO: random seed!
    }
    
    // number of cooling iterations:
    if( params.find("saMaxCoolingIter") != params.end() )
    {
        maxCoolingIter_ = params["saMaxCoolingIter"];
    }
    else
    {
        std::cerr<<"ERROR: No maximum number of cooling iterations given!"<<std::endl;
        std::abort();
    }

    // number of cost samples:
    if( params.find("saNumCostSamples") != params.end() )
    {
        numCostSamples_ = params["saNumCostSamples"];
    }
    else
    {
        std::cerr<<"ERROR: No number of cost samples given!"<<std::endl;
        std::abort();
    }

    // convergence tolerance parameter:
    if( params.find("saConvRelTol") != params.end() )
    {
        convRelTol_ = params["saConvRelTol"];
    }
    else
    {
        convRelTol_ = std::sqrt(std::numeric_limits<real>::epsilon());
    }

    // initial temperature:
    if( params.find("saInitTemp") != params.end() )
    {
        temp_ = params["saInitTemp"];
    }
    else
    {
        std::cerr<<"ERROR: No initial temperature given!"<<std::endl;
        std::abort();
    }

    // cooling factor:
    if( params.find("saCoolingFactor") != params.end() )
    {
        coolingFactor_ = params["saCoolingFactor"];
    }
    else
    {
        std::cerr<<"ERROR: No cooling factor given!"<<std::endl;
        std::abort();
    }

    // step length factor:
    if( params.find("saStepLengthFactor") != params.end() )
    {
        stepLengthFactor_ = params["saStepLengthFactor"];
    }
    else
    {
        std::cerr<<"ERROR: No step length factor given!"<<std::endl;
        std::abort();
    }

    // adaptive adaptive candidate generation:
    if( params.find("saUseAdaptiveCandidateGeneration") != params.end() )
    {
        real tmp = params["saUseAdaptiveCandidateGeneration"];
        if( tmp == 1 )
        {
            std::cerr<<"ERROR: Adaptive candidate generation no longer supported!"<<std::endl;
            std::abort();
            useAdaptiveCandidateGeneration_ = true;
        }
        else if (tmp == 0)
        {
            useAdaptiveCandidateGeneration_ = false;
        }
        else
        {
            std::cerr<<"ERROR: Parameter useAdaptiveCandidateGeneration may only be 0 or 1!"<<std::endl;
            std::abort();
        }
    }
    else
    {
        useAdaptiveCandidateGeneration_ = false;
    }

    // adaptive sampling beta parameter:
    if( params.find("saBeta") != params.end() )
    {
        beta_ = params["saBeta"];
    }
    else
    {
        beta_ = 0.11f;
    }

    // adaptive sampling xi parameter:
    if( params.find("saXi") != params.end() )
    {
        xi_ = params["saXi"];
    }
    else
    {
        xi_ = 3.0f;
    }
}


/*!
 * Sets the objective function object.
 */
void
SimulatedAnnealingModule::setObjFun(ObjectiveFunction objFun)
{
     this -> objFun_ = objFun;
}


/*!
 * Sets the initial point in optimisation space from which simulated annealing
 * is started. This function also allocates the memory needed by the arrays
 * containing the internal state.
 */
void
SimulatedAnnealingModule::setInitGuess(std::vector<real> guess)
{
    // set optimisation space dimension:
    stateDim_ = guess.size();

    // allocate memory for internal state vectors:
	crntState_ = new real[stateDim_];
	candState_ = new real[stateDim_];
	bestState_ = new real[stateDim_];

	// initialise state vectors:
    std::copy(guess.begin(), guess.end(), crntState_);
    std::copy(guess.begin(), guess.end(), candState_);
    std::copy(guess.begin(), guess.end(), bestState_);

	// should adaptive candidate generation be used:
	if( useAdaptiveCandidateGeneration_ == true )
	{
		// allocate memory for covariance and adaptation matrix:
		stateSampleMatrix_ = new real[stateDim_ * numCostSamples_];
		adaptationMatrix_ = new real[stateDim_ * stateDim_];

		// initialise adaptation matrix as identity matrix:
		for(int i=0; i<stateDim_; i++)
		{
			for(int j=0; j<stateDim_; j++)
			{
				if(i == j)
				{
					adaptationMatrix_[i*stateDim_ + j] = 1.0f;
				}
				else
				{
					adaptationMatrix_[i*stateDim_ + j] = 0.0f;
				}
			}
		}
	}
}


/*!
 * Implements the optimisation class interface. Wraps around the anneal() 
 * function, which stems from the original version if this class.
 */
void
SimulatedAnnealingModule::optimise()
{
    anneal();
}


/*!
 * Returns the optimisation result (i.e. the best point found) and the 
 * corresponding objective function value as am OptimSpacePoint.
 */
OptimSpacePoint
SimulatedAnnealingModule::getOptimPoint()
{
    OptimSpacePoint res;
    res.first.assign(bestState_, bestState_ + stateDim_);
    res.second = bestCost_;

    return res;
}


/*
 * Public interface for the annealing function. Basically handles the 
 * distinction between isotropic and adaptive annealing.
 */
eSimAnTerm
SimulatedAnnealingModule::anneal()
{
    // allocate memory for cost samples:
    costSamples_ = new real[numCostSamples_];

   	// get cost of inital states:
    std::vector<real> crntStateVec(crntState_, crntState_ + stateDim_);
    std::vector<real> candStateVec(candState_, candState_ + stateDim_);
    std::vector<real> bestStateVec(bestState_, bestState_ + stateDim_);
    crntCost_ = objFun_(crntStateVec);
    candCost_ = objFun_(candStateVec);
    bestCost_ = objFun_(bestStateVec);

    // choose annealing procedure:
	if( useAdaptiveCandidateGeneration_ == true )
	{
		return annealAdaptive();
	}
	else
	{
		return annealIsotropic();
	}
}


/*
 * Nonadaptive version of the annealing procedure. At each temperature, the 
 * cost function is evaluated exactly once and candidate states are always 
 * generated by making a small step in a isotropically random direction.
 */
eSimAnTerm
SimulatedAnnealingModule::annealIsotropic()
{
	// initialise counter:
	int nCoolingIter = 0;

	// start annealing loop:
	while(true)
	{
		// start sampling loop:
		for(int i = 0; i < numCostSamples_; i++)
		{
        //std::cout<<"i = "<<i<<std::endl;
			// generate a candidate state:
			generateCandidateStateIsotropic();

			// TODO: check boundary conditions!
			
			// evaluate cost function:
            std::vector<real> candStateVec;
            candStateVec.assign(candState_, candState_ + stateDim_);
			candCost_ = objFun_(candStateVec);

            // accept candidate?
			if( acceptCandidateState() == true )
			{
				// candidate state becomes current state:
				cblas_scopy(stateDim_, candState_, 1, crntState_, 1);
				crntCost_ = candCost_;
				// is new state also the best state?
				if( candCost_ > bestCost_ )
				{
					cblas_scopy(stateDim_, candState_, 1, bestState_, 1);
					bestCost_ = candCost_;
				}
			}

			// save current best cost:
			costSamples_[i] = bestCost_;
	
			// reduce temperature:
			cool();
			nCoolingIter++;

			// maximum step number reached?
			if( nCoolingIter >= maxCoolingIter_ )
			{
				return(MAX_COOLING_ITER);
			}
		}	

		// convergence reached?
		if( isConvergedIsotropic() )
		{
			return(CONVERGENCE);
		}
	}
}


/*
 * Adaptive version of the simulated annealing procedure. At each temperature,
 * the cost function is evaluated multiple times and the resulting sample of 
 * cost function values is used as an estimate for the local shape of the cost 
 * function. During the next iteration (at lower temperature), canidate states
 * are generated by taking larger steps in directions in which the slope of the
 * cost function is comparably flat.
 */
eSimAnTerm
SimulatedAnnealingModule::annealAdaptive()
{
	// initialise counter:
	int nCoolingIter = 0;

	// start annealing loop:
	while(true)
	{
		// reset counter of accepted and rejected moves:
		int nAccepted = 0;
		int nRejected = 0;

		// start sampling loop:
		for(int i = 0; i < numCostSamples_; i++)
		{
			// generate a candidate state:
			generateCandidateStateAdaptive();

			// TODO: check boundary conditions!
			
			// evaluate cost function:
            std::vector<real> candStateVec;
            candStateVec.assign(candState_, candState_ + stateDim_);
			candCost_ = objFun_(candStateVec);
			costSamples_[i] = candCost_;

			// accept candidate?
			if( acceptCandidateState() == true )
			{
				// candidate state becomes current state:
				cblas_scopy(stateDim_, candState_, 1, crntState_, 1);
				crntCost_ = candCost_;

				// increment acceptance counter:
				nAccepted++;

				// is new state also the best state?
				if( candCost_ > bestCost_ )
				{
					cblas_scopy(stateDim_, candState_, 1, bestState_, 1);
					bestCost_ = candCost_;
				}
			}
			else
			{
				// increment rejection counter:
				nRejected++;
			}

			// copy current state to sample matrix:
			LAPACKE_slacpy(LAPACK_ROW_MAJOR, 'A', stateDim_, 1, crntState_,
						   1, &stateSampleMatrix_[i], numCostSamples_);
		}

		// no candidates accepted?
		if( nAccepted <= 0 )
		{
			return(NO_CAND_ACCEPTED);
		}

		// update statistics for adaptive candidate generation:
		updateAdaptationMatrix();

		// reduce temperature:
		cool();
		nCoolingIter++;

		// convergence reached?
		if( isConvergedAdaptive() )
		{
			return(CONVERGENCE);
		}

		// maximum step number reached?
		if( nCoolingIter >= maxCoolingIter_ )
		{
			return(MAX_COOLING_ITER);
		}	
	}	
}


/*!
 * Reduces temperature of SA module. Currently only simple exponential 
 * cooling is implemented.
 */
void
SimulatedAnnealingModule::cool()
{
	temp_ *= coolingFactor_;
}


/*
 * Generates a candidate state in the neighbourhood of the current state, where
 * the step direction generated isotropically at random.
 */
void 
SimulatedAnnealingModule::generateCandidateStateIsotropic()
{
	// generate random direction in state space:
	real stateDir[stateDim_];
	for(int i = 0; i < stateDim_; i++)
	{
		stateDir[i] = candGenDistr_(rng_);
	}

	// copy current state to candidate state:
	cblas_scopy(stateDim_, crntState_, 1, candState_, 1);	

	// candidate state is current state plus random direction:
	cblas_saxpy(stateDim_, stepLengthFactor_, stateDir, 1, candState_, 1);
}


/*
 * Generates a candidate state in the neighbourhood of the current state, where
 * the step direction is chosen to reflect the local shape of the cost 
 * function. 
 */
void 
SimulatedAnnealingModule::generateCandidateStateAdaptive()
{
	// generate random direction in state space:
	real stateDir[stateDim_];
	for(int i = 0; i < stateDim_; i++)
	{
		stateDir[i] = candGenDistr_(rng_);
	}

	// copy current state to candidate state:
	cblas_scopy(stateDim_, crntState_, 1, candState_, 1);	

	// should adaptive candidate generation be used?
	// candidate state is current state plus adapted random direction:
	cblas_sgemv(CblasRowMajor, CblasNoTrans, stateDim_, stateDim_, 
	            stepLengthFactor_, adaptationMatrix_, stateDim_, stateDir, 1, 1.0, 
				candState_, 1);
}


/*
 * Decides whether to accept or reject a candidate state. Returns true if 
 * candidate state is accepted.
 */
bool
SimulatedAnnealingModule::acceptCandidateState()
{
	// calculate acceptance probability according to Boltzmann statistics:
	real accProb = std::min(std::exp( (candCost_ - crntCost_)/temp_ ), 1.0f);

	// draw unfiform random number on interval [0,1):
	real r = candAccDistr_(rng_);
	
	// should candidate be accepted:
	return (r < accProb);
}


/*
 * Checks is isotropic algorithm has converged. The criterion used here is that
 * the improvement of best cost over the last numCostSamples samples has been 
 * smaller than a tolerance threshold.
 */
bool
SimulatedAnnealingModule::isConvergedIsotropic()
{
	// get mean and minimum of candidate costs:
	real meanCost =  calculateArrayMean(numCostSamples_, costSamples_);
	real minCost = *std::min_element(costSamples_, costSamples_ + numCostSamples_);

	// check convergence criterion:
	if( std::abs( (meanCost - minCost)/minCost ) < convRelTol_ )
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*
 * Checks if adaptive algorithm has converged. The criterion here is that the
 * difference in cost between the mean and minimum over all evaluations at the
 * current temperature is smaller than a tolerance threshold.
 */
bool
SimulatedAnnealingModule::isConvergedAdaptive()
{
	// get mean and minimum of candidate costs:
	real meanCost =  calculateArrayMean(numCostSamples_, costSamples_);
	real minCost = *std::min_element(costSamples_, costSamples_ + numCostSamples_);

	// check convergence criterion:
	if( std::abs( (meanCost - minCost)/minCost ) < convRelTol_ )
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*
 * Updates the adaptation matrix as described in Vanderbilt & Louie. 
 */
void
SimulatedAnnealingModule::updateAdaptationMatrix()
{
	// calculate covariance matrix:
	calculateCovarianceMatrix(stateDim_, numCostSamples_, stateSampleMatrix_, 
	                          adaptationMatrix_);

	// get matrix square root via Cholesky decomposition:
	LAPACKE_spotrf_work(LAPACK_COL_MAJOR, 'L', stateDim_, adaptationMatrix_, 
	                    stateDim_);

	// scale with growth factor:
	cblas_sscal(stateDim_ * stateDim_, xi_/(beta_*numCostSamples_), 
	            adaptationMatrix_, 1);	
}

