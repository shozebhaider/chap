#ifndef COLOUR_HPP
#define COLOUR_HPP

#include <map>
#include <string>
#include <vector>

#include <gromacs/math/vec.h> 

#include "external/rapidjson/document.h"


/*!
 * \brief Container object for colour palette.
 */
class ColourPalette
{
    public:

        // constructor:
        ColourPalette(){};
        ColourPalette(
                std::vector<real> values,
                std::vector<gmx::RVec> colours);
        ColourPalette(
                const ColourPalette &other);                

        // getter methods:
        std::vector<real> values() const {return values_;};
        std::vector<gmx::RVec> colours() const {return colours_;};


    private:

        std::vector<real> values_;
        std::vector<gmx::RVec> colours_;
};


/*!
 * \brief Named scale of named colours.
 *
 * This object facilitates the creation of a continuous colour scale from a 
 * ColourPalette of discrete RGB colours by linear interpolation. In addition
 * it managed names of these individual colours and the scale as a whole to
 * facilitate writing colour definitions to Wavefront MTL files.
 */
class ColourScale
{
    public:

        // constructor:
        ColourScale(std::string name);

        // setter functions:
        void setPalette(ColourPalette palette);
        void setRange(real min, real max);
        void setResolution(size_t res);

        // access all colours in scale:
        std::map<std::string, gmx::RVec> getColours();

        // convert scalar value to colour:
        std::string scalarToColourName(real scalar);
        gmx::RVec scalarToColour(real scalar);

    private:

        // palette of colours between which to interpolate:
        ColourPalette palette_;

        // scale properties:
        std::string name_;
        real rangeMin_;
        real rangeMax_;
        size_t numColours_;
        std::vector<std::string> colourNames_;
};


/*!
 * \brief Imports colour palettes from JSON files.
 */
class ColourPaletteProvider
{
    public:

        static std::map<std::string, ColourPalette> fromJsonDoc(
                const rapidjson::Document &doc);
        static std::map<std::string, ColourPalette> fromJsonFile(
                std::string filename);

    private:
};


#endif

