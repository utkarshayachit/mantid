#ifndef MANTID_API_IFUNCTIONAUTODIFF_H_
#define MANTID_API_IFUNCTIONAUTODIFF_H_

#include "MantidKernel/System.h"
#include "MantidAPI/IFunction.h"
#include "MantidAPI/FunctionDomain1D.h"
#include "MantidAPI/Jacobian.h"

#include <adept.h>

namespace Mantid
{
namespace API
{

/** IFunction1DAutoDiff :

    IFunction1DAutoDiff is an implementation of IFunction that provides "Automatic differentiation" for 1D domains.

    By sub-classing IFunction1DAutoDiff and implementing function1DAutoDiff, the resulting
    function does not need a manual implementation of partial derivatives. These are calculated
    using automatic differentiation, in this case provided by the adept-library[1]. The performance
    is much better than numerical derivatives as well as more accurate (accurate within machine precision
    - see unit tests), but slower than hand-coded derivatives (it depends on the function how much
    slower).

    In order to actually use the automatic differentiation feature, the parameters need to be
    transformed to a special type provided by adept, adept::adouble. For this reason,
    the use of parameters inside function1DAutoDiff is slightly different from the normal
    use. Instead of calling IFunction::getParameter directly, an adapter object (AutoDiffParameterAdapter)
    is passed to the function. The parameters with correct type can be retrieved through an interface
    that is identical to the methods provided by IFunction:

        ConcreteFunction::function1DAutoDiff(..., const AutoDiffParameterAdapter &parameters) const {
            adept::adouble height = parameters.getParameter("Height");
            adept::adouble width = parameters.getParameter(1);

            adept::adobule area  = width * height;

            ...
        }

    The function values are supposed to be stored in the adept::adouble vector that is passed as second argument.
    It is important not to use the set_value-method of the adept::adouble-object, as that does not produce
    the necessary recording on the active adept-stack:

        ConcreteFunction::function1DAutoDiff(const FunctionDomain1D &domain, std::vector<adept::adouble> &y, ...) const {
            ...

            for(size_t i = 0; i < y.size(); ++i) {
                y[i] = domain[i] * area;
            }
        }

    The sizes of the domain and the vector are guaranteed to be identical, so additional checks are not required.
    Implementing this function is enough, calling ConcreteFunction::functionDerivative will yield the
    correct partial derivatives for all parameters at all points defined by the domain.

    [1] http://www.met.reading.ac.uk/clouds/adept/

      @author Michael Wedel, Paul Scherrer Institut - SINQ
      @date 02/07/2014

    Copyright © 2014 PSI-MSS

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://github.com/mantidproject/mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
  */


/**
 * @class AutoDiffJacobianAdapter
 *
 * This class serves as an interface between API::Jacobian,
 * which defines the methods get and set to interact with the values
 * and adept, which requires a pointer for setting the Jacobian.
 *
 * In the end, the values have to be transfered to the Jacobian object
 * that has been passed into the functionDerivative method of
 * IFunction1DAutoDiff. This is performed by copyValuesToJacobian.
 *
 * The rawValues method provides raw pointer access to the underlying data.
 * In this case it's probably okay to put this kind of method into the
 * public interface, because the use of this class is strictly limited
 * to IFunction1DAutoDiff.
 */
class AutoDiffJacobianAdapter : public Jacobian
{
public:
    /// Constructor
    AutoDiffJacobianAdapter(size_t nValues, size_t nParams) : Jacobian(),
         m_nValues(nValues), m_nParams(nParams), m_jacobian(nValues * nParams)
    { }

    /// Destructor
    ~AutoDiffJacobianAdapter() { }

    /// Implementation of API::Jacobian::set. Throws std::out_of_range on invalid index.
    void set(size_t iY, size_t iP, double value) {
        m_jacobian[safeIndex(iY, iP)] = value;
    }

    /// Implementation of API::Jacobian::get. Throws std::out_of_range on invalid index.
    double get(size_t iY, size_t iP) {
        return m_jacobian[safeIndex(iY, iP)];
    }

    /// Provides raw pointer access to the underlying std::vector. Required for adept-interface.
    double *rawValues() {
        return &m_jacobian[0];
    }

    /// Assign values to jacobian
    void copyValuesToJacobian(Jacobian &jacobian) {
        for(size_t y = 0; y < m_nValues; ++y) {
            for(size_t p = 0; p < m_nParams; ++p) {
                jacobian.set(y, p, getRaw(y, p));
            }
        }
    }

protected:
    /// Get-method without checks.
    inline double getRaw(size_t iY, size_t iP) const {
        return m_jacobian[index(iY, iP)];
    }

    /// Index-calculation for vector access.
    inline size_t index(size_t iY, size_t iP) const {
        return iY + iP * m_nValues;
    }

    /// Get index and check for validity. Throws std::out_of_range on invalid index.
    size_t safeIndex(size_t iY, size_t iP) const {
        size_t i = index(iY, iP);

        if(i >= m_jacobian.size()) {
            throw std::out_of_range("Index is not valid for this Jacobian.");
        }

        return i;
    }

    size_t m_nValues;
    size_t m_nParams;
    std::vector<double> m_jacobian;
};

/**
 * @class AutoDiffParameterAdapter
 *
 * Adept requires that parameters are of type adept::adouble, whereas the
 * IFunction interface works with doubles. adept::adouble variables cause
 * problems when there is no adept::Stack object present and for multithreading-
 * reasons and simplicity of the function interface, adept::Stack is created
 * on the stack each time function values or derivatives are calculated. The overhead
 * for this seems to be small.
 *
 * This class constructs an array of adept::adouble from a raw IFunction-pointer (this), see
 * the implementation of IFunction1DAutoDiff::functionWrapper.
 *
 * After construction, the parameters are passed to IFunction1DAutoDiff::function1DAutoDiff
 * and are accessible via getParameter(size_t) and getParameter(std::string) methods,
 * as well as through operator[](size_t). The latter is required for interfacing with adept.
 *
 * There is a small performance hit connected to this, but in the tests it was
 * acceptable with usual parameter sizes (< 10) and this way does not interfere
 * with the parameter part of the IFunction interface.
 *
 */
class DLLExport AutoDiffParameterAdapter
{
public:
    /// Constructor, requires valid IFunction-pointer.
    AutoDiffParameterAdapter(const IFunction *fromFunction)
        : m_names(), m_parameters()
    {
        if(!fromFunction) {
            throw std::invalid_argument("Cannot construct parameter list from null-function.");
        }

        size_t np = fromFunction->nParams();

        m_parameters.resize(np);

        for(size_t i = 0; i < np; ++i) {
            m_parameters[i].set_value(fromFunction->getParameter(i));
            m_names[fromFunction->parameterName(i)] = i;
        }
    }

    /// Destructor
    ~AutoDiffParameterAdapter() { }

    /// Returns the number of paramters.
    size_t size() const { return m_parameters.size(); }

    /// Returns the parameter at index i. Bounds are not checked.
    adept::adouble &operator[](size_t i)
    {
        return m_parameters[i];
    }

    /// Returns the parameter at index i. Throws std::out_of_range on bad index.
    const adept::adouble &getParameter(size_t i) const
    {
        return m_parameters.at(i);
    }

    /// Returns the parameter with supplied name. Throws std::
    const adept::adouble &getParameter(std::string name) const
    {
        auto it = m_names.find(name);
        if(it == m_names.end()) {
            throw std::out_of_range("Trying to access non-existing function parameter.");
        }

        return m_parameters[it->second];
    }

private:
    std::map<std::string, size_t> m_names;
    std::vector<adept::adouble> m_parameters;
};

class DLLExport IFunction1DAutoDiff : virtual public IFunction
{
public:
    IFunction1DAutoDiff();
    virtual ~IFunction1DAutoDiff();

    void function(const FunctionDomain &domain, FunctionValues &values) const;
    void functionDeriv(const FunctionDomain &domain, Jacobian &jacobian);

    virtual void function1DAutoDiff(const FunctionDomain1D &domain, std::vector<adept::adouble> &y, const AutoDiffParameterAdapter &parameters) const = 0;

protected:
    void functionWrapper(const FunctionDomain &domain, FunctionValues &values, AutoDiffJacobianAdapter *jacobian = 0) const;

};


} // namespace API
} // namespace Mantid

#endif  /* MANTID_API_IFUNCTIONAUTODIFF_H_ */
