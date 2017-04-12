#include <iostream>

#include <lapacke.h>

#include "geometry/basis_spline.hpp"
#include "geometry/cubic_spline_interp_1D.hpp"


/*
 * Constructor.
 */
CubicSplineInterp1D::CubicSplineInterp1D()
{

}



/*
 * Destructor.
 */
CubicSplineInterp1D::~CubicSplineInterp1D()
{

}


/*
 * Public interface for interpolation. Function takes a one-dimensional data 
 * cloud of (x_i, f(x_i)) points (each in their separate vectors) and finds the
 * cubic spline curve that interpolates between these data points such that:
 *
 *      s(x_i) = f(x_i)
 *
 * Currently only Hermite endpoint conditions are implemented. The relevant 
 * linear system is solved via Gaussian elimination (using LAPACKE) and the 
 * result is returned as a splien curve object.
 *
 * TODO: Implement natural boundary conditions.
 */
SplineCurve1D
CubicSplineInterp1D::interpolate(std::vector<real> x,
                                 std::vector<real> f)
{
    
    // generate knot vector:
    std::vector<real> knotVector = prepareKnotVector(x);


    // Assmble Left Hand Side Matrix:
    //-------------------------------------------------------------------------

    // dimension of system and number of right hand sides:
    int nDat = x.size();
    int nSys = nDat + 2;

    // initialise basis spline (derivative) functor:
    BasisSpline B;
    BasisSplineDerivative D;

    // initialse diagonals:
    real subDiag[nSys - 1];
    real mainDiag[nSys];
    real superDiag[nSys - 1];

    // assemble subdiagonal:
    subDiag[nSys - 2] = D(knotVector, degree_, nSys - 2, x.back());
    for(int i = 0; i < nDat; i++)
    {
        subDiag[i] = B(knotVector, degree_, i, x.at(i));
    }

    // assemble main diagonal:
    mainDiag[0] = D(knotVector, degree_, 0, x.front());
    mainDiag[nSys - 1] = D(knotVector, degree_, nSys - 1, x.back());
    for(int i = 0; i < nDat; i++)
    {
        mainDiag[i + 1] = B(knotVector, degree_, i+1, x.at(i));
    }


    // assemble superdiagonal:
    superDiag[0] = D(knotVector, degree_, 1, x.front());
    for(int i = 1; i < nSys - 1; i++)
    {
        superDiag[i] = B(knotVector, degree_, i + 1, x.at(i - 1));
    }


    // Assemble Right Hand Side Vector
    //-------------------------------------------------------------------------

    // number of right hand sides is one:
    int nRhs = 1;
 
    // initialise right hand side:
    real rhsVec[nSys * nRhs];

    // estimate derivatives at endpoints via finite differences:
    rhsVec[0] = ( f.at(1) - f.at(0) ) / ( x.at(1) - x.at(0) );
    rhsVec[nSys - 1] = ( f.at(nDat - 1) - f.at(nDat - 2) ) / ( x.at(nDat - 1) - x.at(nDat - 2) );

    // assemble internal points:
    for(int i = 0; i < nDat; i++)
    {
        rhsVec[i + 1] = f.at(i);
    }
     
    
    // Solve System
    //-------------------------------------------------------------------------
    
    // solve tridiagonal system by Gaussian elimination:
    int status = LAPACKE_sgtsv(LAPACK_COL_MAJOR,
                               nSys, 
                               nRhs,
                               subDiag, 
                               mainDiag, 
                               superDiag, 
                               rhsVec,
                               nSys);

    // handle solver failure:
    if( status != 0 )
    {
        std::cerr<<"ERROR: Could not solve tridiagonal system!"<<std::endl;
        std::cerr<<"LAPACK error code: "<<status<<std::endl;
        std::abort();
    } 


    // Prepare Output
    //-------------------------------------------------------------------------

    // create vector of control points:
    std::vector<real> ctrlPoints;
    ctrlPoints.resize(nSys);
    for(unsigned int i = 0; i < nSys; i++)
    {
        ctrlPoints.at(i) = rhsVec[i];
    }

    // create spline curve object:
    SplineCurve1D Spl(degree_, knotVector, ctrlPoints);

    // return spline curve:
    return Spl;
}


/*
 * Interpolation interface conveniently defined as operator.
 */
SplineCurve1D
CubicSplineInterp1D::operator()(std::vector<real> x,
                                std::vector<real> f)
{
    // actual computation is handled by interpolate() method:
    return interpolate(x, f);
}


/*
 * Internal helper function for creating a knot vector from a vector of input 
 * data. The knot vector is essentially a copy of the data vector with its 
 * first and last element repeated four times.
 */
std::vector<real> 
CubicSplineInterp1D::prepareKnotVector(std::vector<real> &x)
{
    // initialise knot vector:
    std::vector<real> knotVector;

    // add repeats of first knot:
    for(int i = 0; i < degree_; i++)
    {
        knotVector.push_back(x.front());
    }

    // copy support points:
    for(int i = 0; i < x.size(); i++)
    {
        knotVector.push_back(x.at(i));
    }

    // add repeats of last knot:
    for(int i = 0; i < degree_; i++)
    {
        knotVector.push_back(x.back());
    }

    // return knot vector:
    return knotVector;
}

