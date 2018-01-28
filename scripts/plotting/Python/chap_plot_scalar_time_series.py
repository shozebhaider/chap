#!/usr/bin/env python2


################################################################################
# CONFIGURATION
################################################################################

# load libraries:
import json                             # read in JSON files
import numpy as np                      # manipulate numeric vectors
from matplotlib import pyplot as pl     # plotting facilities
import argparse                         # parse command line arguments

# get parameters from user input:
parser = argparse.ArgumentParser()
parser.add_argument(
    "-filename",
    nargs = "?",
    const = "output.json",
    default = "output.json")
parser.add_argument("-dpi",
    nargs = "?",
    const = 1200,
    default = 1200,
    type = int)
args = parser.parse_args()


################################################################################
# DATA READ-IN
################################################################################

# load output data from JSON file:
with open(args.filename) as data_file:
    data = json.load(data_file)


################################################################################
# TIME SERIES PLOTS
################################################################################


# Length
#-------------------------------------------------------------------------------

pl.figure("length")

pl.plot(
	np.array(data["pathwayScalarTimeSeries"]["t"]),
    np.array(data["pathwayScalarTimeSeries"]["length"]),
	"k-")

pl.margins(x = 0)
pl.title("Permeation Pathway Length over Time")
pl.xlabel("t (ps)")
pl.ylabel("L (nm)")

pl.savefig(
    "time_series_pathway_length.png",
	dpi = args.dpi)

pl.close("length")


# Volume
#-------------------------------------------------------------------------------

pl.figure("volume")

pl.plot(
	np.array(data["pathwayScalarTimeSeries"]["t"]),
    np.array(data["pathwayScalarTimeSeries"]["volume"]),
	"k-")

pl.margins(x = 0)
pl.title("Permeation Pathway Volume over Time")
pl.xlabel("t (ps)")
pl.ylabel(r"V $\left(\mathrm{nm}^3\right)$")

pl.savefig(
    "time_series_pathway_volume.png",
	dpi = args.dpi)

pl.close("volume")


# Number of Solvent Particles in Pathway
#-------------------------------------------------------------------------------

pl.figure("number")

pl.plot(
	np.array(data["pathwayScalarTimeSeries"]["t"]),
    np.array(data["pathwayScalarTimeSeries"]["numPathway"]),
	"k-")

pl.margins(x = 0)
pl.title("Number of Solvent Particles in Pathway over Time")
pl.xlabel("t (ps)")
pl.ylabel(r"N")

pl.savefig(
    "time_series_solvent_number_in_pathway.png",
	dpi = args.dpi)

pl.close("number")


# Average Solvent Number Density in Pathway
#-------------------------------------------------------------------------------

pl.figure("density")

pl.axhline(
	y = 33.3679,
	linestyle = "dashed")

density = np.array(data["pathwayScalarTimeSeries"]["numPathway"]) / np.array(data["pathwayScalarTimeSeries"]["volume"])
pl.plot(
	np.array(data["pathwayScalarTimeSeries"]["t"]),
    density,
	"k-")

pl.margins(x = 0)
pl.title("Average Density in Pathway over Time")
pl.xlabel("t (ps)")
pl.ylabel(r"N/V $(\mathrm{nm}^{-3})$")

pl.savefig(
    "time_series_avg_solvent_number_density.png",
	dpi = args.dpi)

pl.close("density")


# Average Solvent Number Density in Pathway
#-------------------------------------------------------------------------------

pl.figure("min_radius")

pl.plot(
	np.array(data["pathwayScalarTimeSeries"]["t"]),
	np.array(data["pathwayScalarTimeSeries"]["minRadius"]),
	"k-")

pl.margins(x = 0)
pl.title("Minimal Pore Radius over Time")
pl.xlabel("t (ps)")
pl.ylabel(r"$\min_s$ R $(\mathrm{nm})$")

pl.savefig(
    "time_series_min_pore_radius.png",
	dpi = args.dpi)

pl.close("min_radius")


# Average Solvent Number Density in Pathway
#-------------------------------------------------------------------------------

pl.figure("min_density")

pl.plot(
	np.array(data["pathwayScalarTimeSeries"]["t"]),
	np.array(data["pathwayScalarTimeSeries"]["minSolventDensity"]),
	"k-")

pl.margins(x = 0)
pl.title("Minimal Solvent Number Density over Time")
pl.xlabel("t (ps)")
pl.ylabel(r"$\min_s$ n $(\mathrm{nm}^{-3})$")

pl.savefig(
    "time_series_min_solvent_number_density.png",
	dpi = args.dpi)

pl.close("min_density")


# Average Solvent Number Density in Pathway
#-------------------------------------------------------------------------------

pl.figure("argmin_radius")

pl.plot(
	np.array(data["pathwayScalarTimeSeries"]["t"]),
	np.array(data["pathwayScalarTimeSeries"]["argMinRadius"]),
	"k-")

pl.margins(x = 0)
pl.title("Location of Minimal Pore Radius over Time")
pl.xlabel("t (ps)")
pl.ylabel(r"$\arg\min_s$ R $(\mathrm{nm})$")

pl.savefig(
    "time_series_argmin_pore_radius.png",
	dpi = args.dpi)

pl.close("argmin_radius")


# Average Solvent Number Density in Pathway
#-------------------------------------------------------------------------------

pl.figure("argmin_density")

pl.plot(
	np.array(data["pathwayScalarTimeSeries"]["t"]),
	np.array(data["pathwayScalarTimeSeries"]["argMinSolventDensity"]),
	"k-")

pl.margins(x = 0)
pl.title("Location of Minimal Solvent Number Density over Time")
pl.xlabel("t (ps)")
pl.ylabel(r"$\arg\min_s$ n $(\mathrm{nm})$")

pl.savefig(
    "time_series_argmin_solvent_number_density.png",
	dpi = args.dpi)

pl.close("argmin_density")
