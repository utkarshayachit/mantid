#include "MantidCurveFitting/LorentzianFamily.h"

namespace Mantid
{
namespace CurveFitting
{

namespace Lorentzians {

using namespace API;

void LorentzianHandCoded::function1D(double *out, const double *xValues, const size_t nData) const
{
    double h = getParameter("Height");
    double x0 = getParameter("Centre");
    double s = getParameter("Gamma");

    for(size_t i = 0; i < nData; ++i) {
        double diff = xValues[i] - x0;
        out[i] = h * s * s / (diff*diff + s * s);
    }
}

void LorentzianHandCoded::functionDeriv1D(Jacobian *out, const double *xValues, const size_t nData)
{
    double h = getParameter("Height");
    double x0 = getParameter("Centre");
    double s = getParameter("Gamma");

    for(size_t i = 0; i < nData; ++i) {
        double diff = xValues[i] - x0;
        double num = diff*diff + s * s;
        double ssquared = s * s;

        out->set(i, 0, 2 * h * ssquared * diff / (num * num));
        out->set(i, 1, ssquared / (num));
        out->set(i, 2, 2 * h * s / num * (1 - ssquared / num));
    }
}

void LorentzianHandCoded::init()
{
    declareParameter("Centre");
    declareParameter("Height");    
    declareParameter("Gamma");
}


void LorentzianNumDiff::function1D(double *out, const double *xValues, const size_t nData) const
{
    double h = getParameter("Height");
    double x0 = getParameter("Centre");
    double s = getParameter("Gamma");

    for(size_t i = 0; i < nData; ++i) {
        double diff = xValues[i] - x0;
        out[i] = h * s * s / (diff*diff + s * s);
    }
}

void LorentzianNumDiff::init()
{
    declareParameter("Centre");
    declareParameter("Height");    
    declareParameter("Gamma");
}

void LorentzianAutoDiff::function1DAutoDiff(const FunctionDomain1D &domain, std::vector<adept::adouble> &y, const AutoDiffParameterAdapter &parameters) const
{
    adept::adouble h = parameters.getParameter("Height");
    adept::adouble x0 = parameters.getParameter("Centre");
    adept::adouble s = parameters.getParameter("Gamma");

    for(size_t i = 0; i < y.size(); ++i) {
        adept::adouble diff = domain[i] - x0;
        y[i] = h * s * s / (diff*diff + s * s);
    }
}


void LorentzianAutoDiff::init()
{
    declareParameter("Centre");
    declareParameter("Height");    
    declareParameter("Gamma");
}

}
  


} // namespace CurveFitting
} // namespace Mantid
