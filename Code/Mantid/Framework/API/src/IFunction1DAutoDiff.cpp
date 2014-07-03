#include "MantidAPI/IFunction1DAutoDiff.h"
#include "MantidAPI/FunctionDomain1D.h"

namespace Mantid
{
namespace API
{

/// Constructor
IFunction1DAutoDiff::IFunction1DAutoDiff() :
    IFunction()
{
}

/// Destructor
IFunction1DAutoDiff::~IFunction1DAutoDiff()
{
}

/**
 * Implementation of IFunction::function. Calculates function values.
 *
 * Internally, this method simply calls IFunction1DAutoDiff::functionWrapper, which takes the proper actions.
 *
 * @param domain :: Function domain. Needs to be a FunctionDomain1D.
 * @param values :: Function values object to store the calculated function values.
 */
void IFunction1DAutoDiff::function(const FunctionDomain &domain, FunctionValues &values) const
{
    functionWrapper(domain, values);
}

/**
 * Implementation of IFunction::functionDeriv. Calculates the Jacobian.
 *
 * This method constructs an instance of AutoDiffJacobianAdapter and passes it as a reference to IFunction1DAutoDiff::functionWrapper.
 * Since a Function-object is required by functionWrapper but not actually used in this case, an empty one is created.
 *
 * Finally, the Jacobian is copied from the adapter into the actual Jacobian object.
 *
 * @param domain :: Function domain. Must be of type FunctionDomain1D.
 * @param jacobian :: Jacobian.
 */
void IFunction1DAutoDiff::functionDeriv(const FunctionDomain &domain, Jacobian &jacobian)
{
    AutoDiffJacobianAdapter localJacobian(domain.size(), nParams());
    FunctionValues values;

    functionWrapper(domain, values, &localJacobian);

    localJacobian.copyValuesToJacobian(jacobian);
}

/**
 * Computes function values or Jacobian
 *
 * This method forms the actual interface between IFunction and adept. An adept::Stack is constructed
 * and activated if the jacobian-parameter is a valid pointer to an AutoDiffJacobianAdapter-object (i.e. not null).
 * In this case, the Jacobian is calculated at the end, while the values-parameter is ignored completely.
 *
 * For calculating function values, a valid FunctionValues object is required and the AutoDiffJacobianAdapter must
 * be null. This stops the adept::Stack from recording and copies the calculated values from the adept-vector
 * into the FunctionValues-object.
 *
 * If the domain is not a FunctionDomain1D, the method throws std::invalid_argument.
 *
 * @param domain :: Function domain. Is checked for correct type (FunctionDomain1D or its subclasses).
 * @param values :: Function values. Must match domain for calculation of function values, can be empty otherwise.
 * @param jacobian :: Jacobian adapter. If null, derivatives are not calculated, but function values are stored in values-parameter.
 */
void IFunction1DAutoDiff::functionWrapper(const FunctionDomain &domain, FunctionValues &values, AutoDiffJacobianAdapter *jacobian) const
{
    try {
        const FunctionDomain1D &d1d = dynamic_cast<const FunctionDomain1D &>(domain);

        /* Set up the adept stack and a y-array of correct size.
         * Initialize parameter adapter from this-pointer
         * to make adept::adouble parameters available.
         */
        size_t domainSize = d1d.size();

        adept::Stack stack;
        std::vector<adept::adouble> y(domainSize);
        AutoDiffParameterAdapter parameters(this);

        if(jacobian != 0) {
            stack.new_recording();
        } else {
            /* This branch is executed when no AutoDiffJacobianAdapter is supplied,
             * which means that only function values are computed and thus, recording
             * of the execution stack is not required.
             */
            if(values.size() != domain.size()) {
                throw std::invalid_argument("FunctionValues does not match FunctionDomain in size.");
            }

            stack.pause_recording();
        }

        /* Call the actual function implementation to calculate values in y.
         * adept::adouble parameters are passed.
         */
        function1DAutoDiff(d1d, y, parameters);

        if(jacobian != 0) {
            /* If the "Jacobian branch" is executed, it is calculated by identifying
             * dependent and independent variables and telling the stack object where
             * to put the calculated values, using the raw pointer access of AutoDiffJacobianAdapter.
             */
            size_t numberParameters = parameters.size();

            stack.independent(&parameters[0], numberParameters);
            stack.dependent(&y[0], domainSize);
            stack.jacobian(jacobian->rawValues());
        } else {
            /* Otherwise, the function values need to be copied to the right place.
             * For performance reasons it's done via raw pointers.
             */
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
