#include "MantidAPI/IFunction1DAutoDiff.h"
#include "MantidAPI/FunctionDomain1D.h"

namespace Mantid
{
namespace API
{

IFunction1DAutoDiff::IFunction1DAutoDiff() :
    IFunction()
{
}

IFunction1DAutoDiff::~IFunction1DAutoDiff()
{
}

/**
 *
 *
 * @param domain
 * @param values
 */
void IFunction1DAutoDiff::function(const FunctionDomain &domain, FunctionValues &values) const
{
    functionWrapper(domain, values);
}

void IFunction1DAutoDiff::functionDeriv(const FunctionDomain &domain, Jacobian &jacobian)
{
    AutoDiffJacobianAdapter localJacobian;

    FunctionValues values(domain);
    functionWrapper(domain, values, &localJacobian);

    localJacobian.copyValuesToJacobian(jacobian);
}

void IFunction1DAutoDiff::functionWrapper(const FunctionDomain &domain, FunctionValues &values, AutoDiffJacobianAdapter *jacobian) const
{
    try {
        const FunctionDomain1D &d1d = dynamic_cast<const FunctionDomain1D &>(domain);

        size_t domainSize = d1d.size();

        adept::Stack stack;
        std::vector<adept::adouble> y(domainSize);
        AutoDiffParameterAdapter parameters(this);

        if(jacobian != 0) {
            stack.new_recording();
        } else {
            stack.pause_recording();
        }

        function1DAutoDiff(d1d, y, parameters);

        if(jacobian != 0) {
            size_t numberParameters = parameters.size();
            jacobian->setSize(numberParameters, domainSize);

            stack.independent(&parameters[0], numberParameters);
            stack.dependent(&y[0], domainSize);
            stack.jacobian(jacobian->rawValues());
        } else {
            double *first = values.getPointerToCalculated(0);
            for(size_t i = 0; i < values.size(); ++i) {
                *(first + i) = y[i].value();
            }
        }
    } catch(std::bad_cast) {
        throw std::invalid_argument("Provided domain is not of type FunctionDomain1D.");
    }
}

} // namespace API
} // namespace Mantid
