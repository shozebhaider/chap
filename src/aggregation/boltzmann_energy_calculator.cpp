#include "aggregation/boltzmann_energy_calculator.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>


/*!
 * Constructor sets energy units to \f$ k_{\text{B}} T \f$.
 */
BoltzmannEnergyCalculator::BoltzmannEnergyCalculator()
    : energyUnitFactor_(1.0)
    , temperature_(-1.0)
    , temperatureIsSet_(false)
{

}


/*!
 * Public interface for calculation of energy from density. Takes a vector of 
 * densities as inout and returns a vector of energies.
 *
 * This performs a sanity check on the input density and throws an exception
 * if any density value is smaller than zero!
 */
std::vector<real>
BoltzmannEnergyCalculator::calculate(
        const std::vector<real> &density)
{
    // sanity check:
    for(auto d : density)
    {
        if( d < 0.0 )
        {
            throw std::logic_error("Encountered negative density in energy "
                                   " calculation!");
        }
    }

    // simply copy density into local vector:
    std::vector<real> energy;
    energy.resize(density.size());

    // convert to energy assuming Boltzmann statistics:
    std::transform(
            density.begin(),
            density.end(),
            energy.begin(),
            [this](real x) -> real{return -std::log(x)*energyUnitFactor_;});

    // replace infinities by finite values:
    mendInfinities(energy);

    // return vector of energies:
    return energy;
}


/*!
 * Setter method for thermodynamic temperature in Kelvin. Must be called if
 * energy units other than \f$ k_{\text{B}} T \f$ should be used.
 */
void
BoltzmannEnergyCalculator::setTemperature(
        const real temperature)
{
    temperature_ = temperature;
    temperatureIsSet_ = true;
}


/*!
 * Setter method for energy units to be used in calculate() method. Internally
 * this sets a numerical factor by which the negative logarithm of the density
 * is multiplied. This factor is 1 by default yielding energy units of
 * \f$ k_{\text{B}} T \f$ and can be set to \f$ R_{\text{gas}} T \f$ to
 * obtain units of kilojoules per mol or kilocalories per mol respectively.
 * Note that setTemperature must be called before this methof if energy units
 * other than \f$ k_{\text{B}} \f$ are to be used.
 */
void
BoltzmannEnergyCalculator::setEnergyUnits(
        const eEnergyUnit &unit)
{
    if( unit == eEnergyUnitBoltzmann )
    {
        // in this cas esimply multiply by one:
        energyUnitFactor_ = 1.0;
    }
    else if( unit == eEnergyUnitKiloJoulePerMol )
    {
        // sanity check:
        if( !temperatureIsSet_ )
        {
            throw std::logic_error("Temperature must be set for requested "
                                   "energy units.");
        }

        // caluclate energy unit factor:
        energyUnitFactor_ = temperature_ * cGasConstantKiloJoulePerMol_;
    }
    else if( unit == eEnergyUnitKiloCaloriePerMol )
    {
        // sanity check:
        if( !temperatureIsSet_ )
        {
            throw std::logic_error("Temperature must be set for requested "
                                   "energy units.");
        }

        // calculate energy unit factor:
        energyUnitFactor_ = temperature_ * cGasConstantKiloCaloriePerMol_;
    }
    else
    {
        throw std::logic_error("Requested energy unit not recognised!");
    }
}


/*!
 * Auxiliary function for dealing with infinities resulting from zero 
 * densities. Instead of using actual infinities, the relevant values are 
 * replaced by the largest and smallest representable real value. This 
 * facilitates writing output to JSON.
 */
void
BoltzmannEnergyCalculator::mendInfinities(std::vector<real> &energy)
{
    for(auto it = energy.begin(); it != energy.end(); it++)
    {
        if( std::isinf(*it) )
        {
            if( *it < 0.0  )
            {
                *it = -std::numeric_limits<real>::max();
            }
            else
            {
                *it = std::numeric_limits<real>::max();
            }
        }
    }
}

