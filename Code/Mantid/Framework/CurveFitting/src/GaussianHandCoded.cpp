#include "MantidCurveFitting/GaussianHandCoded.h"

namespace Mantid
{
namespace CurveFitting
{

using namespace API;
  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  GaussianHandCoded::GaussianHandCoded()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  GaussianHandCoded::~GaussianHandCoded()
  {
  }
  
  void GaussianHandCoded::function1D(double *out, const double *xValues, const size_t nData) const
  {
      double height = getParameter("Height");
      double centre = getParameter("PeakCentre");
      double sigma = getParameter("Sigma");

      for(size_t i = 0; i < nData; ++i) {
          out[i] = height * exp(-0.5 * pow( (xValues[i] - centre)/sigma, 2.0) );
      }
  }

  void GaussianHandCoded::functionDeriv1D(Jacobian *out, const double *xValues, const size_t nData)
  {
      double height = getParameter("Height");
      double centre = getParameter("PeakCentre");
      double sigma = getParameter("Sigma");

      for(size_t i = 0; i < nData; ++i) {
          double xDiff = xValues[i] - centre;
          double expTerm = exp(-0.5 * pow( (xDiff)/sigma, 2.0));

          out->set(i, 0, expTerm);
          out->set(i, 1, (height*expTerm*(2 * xDiff))/(2*pow(sigma, 2)));
          out->set(i, 2, (height*expTerm*pow(xDiff, 2))/pow(sigma, 3));
      }
  }

  void GaussianHandCoded::init() {
      declareParameter("Height");
      declareParameter("PeakCentre");
      declareParameter("Sigma");
  }



} // namespace CurveFitting
} // namespace Mantid
