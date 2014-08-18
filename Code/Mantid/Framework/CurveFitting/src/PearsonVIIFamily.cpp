#include "MantidCurveFitting/PearsonVIIFamily.h"

namespace Mantid
{
namespace CurveFitting
{

namespace Pearsons {

using namespace API;

void PearsonVIIHandCoded::function1D(double *out, const double *xValues, const size_t nData) const
{
    double h = getParameter("Height");
    double x0 = getParameter("Centre");
    double s = getParameter("Gamma");
    double m = getParameter("m");

    for(size_t i = 0; i < nData; ++i) {
        double diff = xValues[i] - x0;
        out[i] = h * pow(s, 2.0 * m) / pow(diff*diff * (pow(2.0, 1.0/m) - 1.0) + s * s, m);
    }
}

void PearsonVIIHandCoded::functionDeriv1D(Jacobian *out, const double *xValues, const size_t nData)
{
    double h = getParameter("Height");
    double x0 = getParameter("Centre");
    double s = getParameter("Gamma");
    double m = getParameter("m");

    for(size_t i = 0; i < nData; ++i) {
        double diff = xValues[i] - x0;
        double ssquared = s * s;
        double factor = pow(2.0, (1.0/m)) - 1.0;
        double sraised = pow(s, 2.0*m);
        double factoredSquaredDiff = factor*diff*diff;
        double diffM = pow(ssquared + factoredSquaredDiff, m);
        double diffM1 = pow(ssquared + factoredSquaredDiff, m + 1.0);

        out->set(i, 0, 2.0 * (h*m*sraised*diff*factor)/diffM1);
        out->set(i, 1, sraised/diffM);
        out->set(i, 2, (2.0*h*m) * (pow(s, 2.0*m - 1.0)/diffM - s * sraised/diffM1));
        out->set(i, 3, (2.0*h*sraised*log(s))/diffM
                        - h*sraised*(log(ssquared + factoredSquaredDiff)/diffM
                        - ((factor+1.0)*log(2.0)*diff*diff)/(m*diffM1)));
    }
}

void PearsonVIIHandCoded::init()
{
    declareParameter("Centre");
    declareParameter("Height");
    declareParameter("Gamma");
    declareParameter("m");
}


void PearsonVIINumDiff::function1D(double *out, const double *xValues, const size_t nData) const
{
    double h = getParameter("Height");
    double x0 = getParameter("Centre");
    double s = getParameter("Gamma");
    double m = getParameter("m");

    for(size_t i = 0; i < nData; ++i) {
        double diff = xValues[i] - x0;
        out[i] = h * pow(s, 2.0 * m) / pow(diff*diff * (pow(2.0, 1.0/m) - 1.0) + s * s, m);
    }
}

void PearsonVIINumDiff::init()
{
    declareParameter("Centre");
    declareParameter("Height");
    declareParameter("Gamma");
    declareParameter("m");
}

void PearsonVIIAutoDiff::function1DAutoDiff(const FunctionDomain1D &domain, std::vector<adept::adouble> &y, const AutoDiffParameterAdapter &parameters) const
{
    adept::adouble h = parameters.getParameter("Height");
    adept::adouble x0 = parameters.getParameter("Centre");
    adept::adouble s = parameters.getParameter("Gamma");
    adept::adouble m = parameters.getParameter("m");

    for(size_t i = 0; i < y.size(); ++i) {
        adept::adouble diff = domain[i] - x0;
        y[i] = h * pow(s, 2.0 * m) / pow(diff*diff * (pow(2.0, 1.0/m) - 1.0) + s * s, m);
    }
}


void PearsonVIIAutoDiff::init()
{
    declareParameter("Centre");
    declareParameter("Height");
    declareParameter("Gamma");
    declareParameter("m");
}

}



} // namespace CurveFitting
} // namespace Mantid
