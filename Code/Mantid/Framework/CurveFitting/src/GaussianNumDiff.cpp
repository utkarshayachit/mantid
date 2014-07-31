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
          out[i] = height * exp(-0.5 * pow( (xValues[i] - centre)/sigma, 2.0) );
      }
  }

  void GaussianNumDiff::init() {
      declareParameter("Height");
      declareParameter("PeakCentre");
      declareParameter("Sigma");
  }
  


} // namespace CurveFitting
} // namespace Mantid
