#include "geometry/bspline_basis_element.hpp"


/*!
 * Evaluates the B-spline basis function \f$ B_{i,p}(x) \f$, where \f$ p \f$
 * is the spline's degree, \f$ x \f$ is the evaluation point, and \f$ i \f$
 * is the index of the requested basis element.
 *
 * This implements algorithm A2.4 from the NURBS Book.
 */
real
BSplineBasisElement::operator()(
        real eval,
        int idx,
        const std::vector<real> &knots,
        unsigned int degree)
{
    // uppermost knot index:
    int m = knots.size() - 1;

    // handle special cases:
    if( idx == 0 && eval == knots[0]  )
    {
        return 1.0;
    }
    if( idx == m - degree - 1 && eval == knots[m] )
    {
        return 1.0;
    }

    // use local support property to simply return zeros:
    if( eval < knots[idx] || eval >= knots[idx + degree + 1] )
    {
        return 0.0;
    }

    // allocate memory for coefficients in triangle table:
    std::vector<real> coefs(degree + 1);

    // initialise degree zero basis functions (bottom of Cox-deBoor recursion):
    for(int i = 0; i <= degree; i++)
    {
        if( eval >= knots[idx + i] &&
            eval < knots[idx + i + 1] )
        {
            coefs[i] = 1.0;
        }
        else
        {
            coefs[i] = 0.0;
        }
    }

    // compute basis spline via triangular table:
    for(int i = 1; i <= degree; i++)
    {
        real saved;
        if( coefs[0] == 0.0 )
        {
            saved = 0.0;
        }
        else
        {
            saved = ((eval - knots[idx])*coefs[0])/(knots[idx+i] - knots[idx]);
        }

        for(int j = 0; j <= degree - i + 1; j++)
        {
            // knots to the left and right:
            real left = knots[idx + j + 1];
            real right = knots[idx + j + i + 1];

            // compute coefficients:
            if( coefs[j + 1] == 0.0 )
            {
                coefs[j] = saved;
                saved = 0.0;
            }
            else
            {
                real tmp = coefs[j + 1]/(right - left);
                coefs[j] = saved + (right - eval)*tmp;
                saved = (eval - left)*tmp;
            }
        }
    }

    // return value of basis function:
    return coefs[0];
}


/*
 *
 */
real
BSplineBasisElement::operator()(
        real eval,
        const std::vector<real> &knots,
        unsigned int degree,
        unsigned int idx,
        unsigned int deriv)
{
    // evaluate all derivatives up to degree deriv:
    std::vector<real> ders = evaluateDerivatives(
            eval,
            knots,
            degree,
            idx,
            deriv);

    // return only the requested derivativ:
    return ders[deriv];
}


/*
 *
 */
std::vector<real>
BSplineBasisElement::evaluateDerivatives(
        real eval,
        const std::vector<real> &knots,
        unsigned int degree,
        unsigned int idx,
        unsigned int deriv)
{
    // allocate memory for output vector:
    std::vector<real> ders(deriv + 1);

    // use local support property:
    if( eval < knots[idx] || eval >= knots[idx + degree + 1] )
    {
        std::cout<<"idx = "<<idx<<"  "
                 <<"degree = "<<degree<<"  "
                 <<"knots[idx + degree + 1] = "<<knots[idx + degree + 1]<<"  "
                 <<"knots.size = "<<knots.size()<<"  "
                 <<std::endl;

        for(int i = 0; i <= deriv; i++)
        {
            // NOTE: this if is missing in The NURBS Book, but us necessary
            // to handle the special case of the last basis function if the
            // evaluation point is identical to that knot
            if( eval == knots.back() && idx == knots.size() - degree - 2 )
            {
                ders[i] = 1.0;
            }
            else
            {
                ders[i] = 0.0;
            }
        }

        return ders;
    }

    // allocate memory for B-spline coefficients:
    std::vector<std::vector<real>> coefs(degree + 1, std::vector<real>(degree));

    // initialise degree zero basis functions (bottom of Cox-deBoor recursion):
    for(int i = 0; i <= degree; i++)
    {
        if( eval >= knots[idx + i] && eval < knots[idx + i + 1] )
        {
            coefs[i][0] = 1.0;
        }
        else
        {
            coefs[i][0] = 0.0;
        }
    }

    // compute all basis function coefficients via triangular table:
    for(int i = 1; i <= degree; i++ )
    {
        real saved;
        if( coefs[0][i - 1] == 0.0 )
        {
            saved = 0.0;
        }
        else
        {
            saved = ((eval - knots[idx])*coefs[0][i-1]) 
                    / (knots[idx+i] - knots[idx] );
        }

        for(int j = 0; j < degree - i + 1; j++)
        {
            // left and right knots:
            real left = knots[idx + j + 1];
            real right = knots[idx + j + i + 1];

            if( coefs[j + 1][i - 1] == 0.0 )
            {
                coefs[j][i] = saved;
                saved = 0.0;
            }
            else
            {
                real tmp = coefs[j + 1][i - 1] / (right - left);
                coefs[j][i] = saved + (right - eval)*tmp;
                saved = (eval - left)*tmp;
            }
        }
    }

    // copy function value to output vector:
    ders[0] = coefs[0][degree];
   
    // compute the derivatives:
    for(int i = 1; i <= deriv; i++)
    {
        // 
        for(int j = 0; j <= i; j++)
        {
            
        }
    }

    // return vector of derivatives up to order deriv:
    return ders;
}































