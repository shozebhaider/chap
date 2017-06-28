#include <cmath>
#include <iostream>

#include "path-finding/naive_cylindrical_path_finder.hpp"


/*
 * Constructor.
 */
NaiveCylindricalPathFinder::NaiveCylindricalPathFinder(std::map<std::string, real> params,
                                                       gmx::RVec centrePoint,
                                                       gmx::RVec dirVec)
    : AbstractPathFinder(params)
    , centrePoint_(centrePoint)
    , dirVec_(dirVec)
{
    // assign parameters:
    if( params.find("pfCylStepLength") != params.end() )
    {
        stepLength_ = params["pfCylStepLength"];
    }
    else
    {
        std::cerr<<"ERROR: Step length not given!"<<std::endl;
        std::abort();
    }

    if( params.find("pfCylRad") != params.end() )
    {
        cylRad_ = params["pfCylRad"];
    }
    else
    {
        std::cerr<<"ERROR: Cylinder radius not given!"<<std::endl;
        std::abort();
    }

    if( params.find("pfCylNumSteps") != params.end() )
    {
        nSteps_ = params["pfCylNumSteps"];
    }
    else
    {
        std::cerr<<"ERROR: Number of steps not given!"<<std::endl;
        std::abort();
    }
}


/*
 * Destructor.
 */
NaiveCylindricalPathFinder::~NaiveCylindricalPathFinder()
{

}


/*
 *
 */
void
NaiveCylindricalPathFinder::findPath()
{
    // normalise direction vector:
    gmx::RVec normDirVec = dirVec_;
    real norm = std::sqrt( dirVec_[0]*dirVec_[0] + 
                           dirVec_[1]*dirVec_[1] +
                           dirVec_[2]*dirVec_[2] );
    normDirVec[0] = normDirVec[0]/norm;
    normDirVec[1] = normDirVec[1]/norm;
    normDirVec[2] = normDirVec[2]/norm;

    // build up path points and radii:
    for(int i = 0; i <= 2*nSteps_; i++)
    {
        real x = centrePoint_[0] + (i - nSteps_)*stepLength_*normDirVec[0];
        real y = centrePoint_[1] + (i - nSteps_)*stepLength_*normDirVec[1];
        real z = centrePoint_[2] + (i - nSteps_)*stepLength_*normDirVec[2];

        path_.push_back(gmx::RVec(x, y, z));
        radii_.push_back(cylRad_);
    }
}
