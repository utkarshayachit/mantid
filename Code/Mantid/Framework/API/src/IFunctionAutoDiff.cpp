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
    IFunction(),
    ParamFunction(),
    m_stack(new adept::Stack),
    m_jacobian(new AutoDiffJacobian)
{
}

//----------------------------------------------------------------------------------------------
/** Destructor
   */
IFunctionAutoDiff::~IFunctionAutoDiff()
{
}

void IFunctionAutoDiff::function(const FunctionDomain &domain, FunctionValues &values) const
{
    const FunctionDomain1D &d1d = dynamic_cast<const FunctionDomain1D &>(domain);

    const double *domainStart = d1d.getPointerAt(0);
    size_t domainSize = d1d.size();

    std::vector<adept::adouble> x(domainStart, domainStart + domainSize);
    std::vector<adept::adouble> y(d1d.size());
    std::vector<adept::adouble> parameters = getAdeptParameters();

    m_stack->new_recording();

    functionAutoDiff(x, y, parameters);

    for(size_t i = 0; i < values.size(); ++i) {
        *(values.getPointerToCalculated(i)) = y[i].value();
    }

    m_stack->independent(&parameters[0], parameters.size());
    m_stack->dependent(&y[0], domainSize);
    m_stack->jacobian(m_jacobian->rawValues());
}

void IFunctionAutoDiff::functionDeriv(const FunctionDomain &domain, Jacobian &jacobian)
{
    if(!m_jacobian->isValid()) {
        FunctionValues values(domain);
        function(domain, values);
    }

    size_t np = nParams();
    size_t ny = domain.size();

    for(size_t p = 0; p < np; ++p) {
        for(size_t y = 0; y < ny; ++y) {
            jacobian.set(y, p, m_jacobian->get(y, p));
        }
    }
}

std::vector<adept::adouble> IFunctionAutoDiff::getAdeptParameters() const
{
    size_t np = nParams();

    std::vector<adept::adouble> adeptParameters(np);
    for(size_t i = 0; i < np; ++np) {
        adeptParameters[i].set_value(getParameter(i));
    }

    return adeptParameters;
}


} // namespace API
} // namespace Mantid
