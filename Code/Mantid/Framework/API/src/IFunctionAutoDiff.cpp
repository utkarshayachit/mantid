#include "MantidAPI/IFunctionAutoDiff.h"
#include "MantidAPI/FunctionDomain1D.h"

namespace Mantid
{
namespace API
{

//----------------------------------------------------------------------------------------------
/** Constructor
   */
IFunctionAutoDiff::IFunctionAutoDiff() :
    IFunction()
{
}

//----------------------------------------------------------------------------------------------
/** Destructor
   */
IFunctionAutoDiff::~IFunctionAutoDiff()
{
}

/**
 *
 *
 * @param domain
 * @param values
 */
void IFunctionAutoDiff::function(const FunctionDomain &domain, FunctionValues &values) const
{
    functionWrapper(domain, values, 0);
}

void IFunctionAutoDiff::functionDeriv(const FunctionDomain &domain, Jacobian &jacobian)
{
    AutoDiffJacobian localJacobian;

    FunctionValues values(domain);
    functionWrapper(domain, values, &localJacobian);

    size_t np = nParams();
    size_t ny = domain.size();

    for(size_t p = 0; p < np; ++p) {
        for(size_t y = 0; y < ny; ++y) {
            jacobian.set(y, p, localJacobian.get(y, p));
        }
    }
}

void IFunctionAutoDiff::functionWrapper(const FunctionDomain &domain, FunctionValues &values, AutoDiffJacobian *jacobian) const
{
    const FunctionDomain1D &d1d = dynamic_cast<const FunctionDomain1D &>(domain);

    const double *domainStart = d1d.getPointerAt(0);
    size_t domainSize = d1d.size();

    adept::Stack stack;

    std::vector<adept::adouble> x(domainStart, domainStart + domainSize);
    std::vector<adept::adouble> y(d1d.size());

    if(jacobian != 0) {
        stack.new_recording();
    } else {
        stack.pause_recording();
    }

    AutoDiffParameters params(this);

    functionAutoDiff(x, y, params);

    for(size_t i = 0; i < values.size(); ++i) {
        *(values.getPointerToCalculated(i)) = y[i].value();
    }

    if(jacobian != 0) {
        stack.independent(&params[0], params.size());
        stack.dependent(&y[0], domainSize);

        jacobian->setSize(params.size(), domainSize);
        stack.jacobian(jacobian->rawValues());
    }
}

} // namespace API
} // namespace Mantid
