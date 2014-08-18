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
          double diffTerm = (xValues[i] - centre)/sigma;
          out[i] = height * exp(-0.5 * diffTerm * diffTerm );
      }
  }

  void GaussianHandCoded::functionDeriv1D(Jacobian *out, const double *xValues, const size_t nData)
  {
      double height = getParameter("Height");
      double centre = getParameter("PeakCentre");
      double sigma = getParameter("Sigma");

      for(size_t i = 0; i < nData; ++i) {
          double xDiff = xValues[i] - centre;
          double term = (xDiff)/sigma;
          double expTerm = exp(-0.5 * term * term);

          out->set(i, 0, (height*expTerm*(xDiff))/(sigma * sigma));
          out->set(i, 1, expTerm);
          out->set(i, 2, (height*expTerm*term * term)/(sigma));
      }
  }

  void GaussianHandCoded::init() {
      declareParameter("PeakCentre");
      declareParameter("Height");      
      declareParameter("Sigma");
  }



} // namespace CurveFitting
} // namespace Mantid
