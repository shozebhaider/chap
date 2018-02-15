#include <gtest/gtest.h>

#include "geometry/cubic_spline_interp_3D.hpp"


/*!
 * \brief Test fixture for cubic spline interpolation in 3D.
 */
class CubicSplineInterp3DTest : public ::testing::Test
{


};


/*!
 * Tests interpolation algorithm on a problem which should yield a linear 
 * polynomial. Correct evaluation is checked at the support points and the 
 * interval midpoints. Uses Hermite boundary conditions.
 */
TEST_F(CubicSplineInterp3DTest, CubicSplineInterpHermiteLinearTest)
{
    // set tolerance threshold for numerical comparison:
    real eps = std::numeric_limits<real>::epsilon();

    // create a set of points on a line:
    std::vector<gmx::RVec> points = {gmx::RVec( 3.0,  2.0,  0.5),
                                     gmx::RVec( 3.0,  1.0,  0.5),
                                     gmx::RVec( 3.0,  0.0,  0.5),
                                     gmx::RVec( 3.0, -1.0,  0.5),
                                     gmx::RVec( 3.0, -2.0,  0.5)};

    // instantiate interpolation functor:
    CubicSplineInterp3D Interp;

    // find interpolating spline curve:
    SplineCurve3D Spl = Interp(points, eSplineInterpBoundaryHermite);

    // check that spline curve goes through support points:
    for(unsigned int i = 0; i < points.size(); i++)
    {
        real evalPoint = 1.0*i;
        gmx::RVec val = Spl.evaluate(evalPoint, 0);

        ASSERT_NEAR(points[i][XX], val[XX], eps);
        ASSERT_NEAR(points[i][YY], val[YY], eps);
        ASSERT_NEAR(points[i][ZZ], val[ZZ], eps);
    }

    // check interpolation at interval midpoints:
    for(unsigned int i = 0; i < points.size() - 1; i++)
    {
        real evalPoint = 1.0*i + 0.5;
        gmx::RVec val = Spl.evaluate(evalPoint, 0);

        ASSERT_NEAR((points[i][XX] + points[i+1][XX])/2.0, val[XX], eps);
        ASSERT_NEAR((points[i][YY] + points[i+1][YY])/2.0, val[YY], eps);
        ASSERT_NEAR((points[i][ZZ] + points[i+1][ZZ])/2.0, val[ZZ], eps);
    }
}

