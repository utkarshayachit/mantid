#include "MantidCurveFitting/GaussianNumDiff.h"

namespace Mantid
{
namespace CurveFitting
{


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  GaussianNumDiff::GaussianNumDiff()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  GaussianNumDiff::~GaussianNumDiff()
  {
  }

  void GaussianNumDiff::function1D(double *out, const double *xValues, const size_t nData) const
  {
      double height = getParameter("Height");
      double centre = getParameter("PeakCentre");
      double sigma = getParameter("Sigma");

      for(size_t i = 0; i < nData; ++i) {
          double term = (xValues[i] - centre)/sigma;
          out[i] = height * exp(-0.5 * term * term );
      }
  }

  void GaussianNumDiff::init() {
      declareParameter("Height");
      declareParameter("PeakCentre");
      declareParameter("Sigma");
  }
  


} // namespace CurveFitting
} // namespace Mantid
