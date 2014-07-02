#include "MantidCurveFitting/GaussianAutoDiff.h"
#include "MantidAPI/FunctionFactory.h"

namespace Mantid
{
namespace CurveFitting
{

DECLARE_FUNCTION(GaussianAutoDiff)

using namespace API;

//----------------------------------------------------------------------------------------------
/** Destructor
   */
GaussianAutoDiff::~GaussianAutoDiff()
{
}

void GaussianAutoDiff::function1DAutoDiff(const FunctionDomain1D &domain, std::vector<adept::adouble> &y, const AutoDiffParameterAdapter &parameters) const
{
    adept::adouble height = parameters.getParameter("Height");
    adept::adouble centre = parameters.getParameter("PeakCentre");
    adept::adouble sigma = parameters.getParameter("Sigma");

    for(size_t i = 0; i < y.size(); ++i) {
        y[i] = height * exp(-0.5 * pow( (domain[i] - centre)/sigma, 2.0) );
    }
}

void GaussianAutoDiff::init() {
    declareParameter("Height");
    declareParameter("PeakCentre");
    declareParameter("Sigma");
}

} // namespace CurveFitting
} // namespace Mantid
