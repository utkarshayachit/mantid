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

    Implementation of IFunction that provides "Automatic differentiation" for 1D domains.

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
class AutoDiffJacobianAdapter : public Jacobian
{
public:
    AutoDiffJacobianAdapter() : Jacobian(),
        m_nParams(0), m_nValues(0), m_jacobian(), m_isValid(false)
    { }

    virtual ~AutoDiffJacobianAdapter() { }

    void setSize(size_t nParams, size_t nValues) {
        m_nParams = nParams;
        m_nValues = nValues;
        m_jacobian.resize(nParams * nValues);
    }

    double *rawValues() {
        return &m_jacobian[0];
    }

    inline double getRaw(size_t iY, size_t iP) const {
        return m_jacobian[index(iY, iP)];
    }

    virtual void set(size_t iY, size_t iP, double value) {
        m_jacobian[index(iY, iP)] = value;
    }

    virtual double get(size_t iY, size_t iP) {
        return getRaw(iY, iP);
    }

    void copyValuesToJacobian(Jacobian &jacobian) {
        for(size_t y = 0; y < m_nValues; ++y) {
            for(size_t p = 0; p < m_nParams; ++p) {
                jacobian.set(y, p, getRaw(y, p));
            }
        }
    }

protected:
    inline size_t index(size_t iY, size_t iP) const {
        return iY + iP * m_nValues;
    }

    size_t m_nParams;
    size_t m_nValues;
    std::vector<double> m_jacobian;

    bool m_isValid;
};



class DLLExport AutoDiffParameterAdapter
{
public:
    AutoDiffParameterAdapter(const IFunction *fromFunction)
        : m_names(), m_parameters()
    {
        size_t np = fromFunction->nParams();

        m_parameters.resize(np);

        for(size_t i = 0; i < np; ++i) {
            m_parameters[i].set_value(fromFunction->getParameter(i));
            m_names[fromFunction->parameterName(i)] = i;
        }
    }

    size_t size() const { return m_parameters.size(); }

    adept::adouble &operator[](size_t i) { return m_parameters[i]; }

    const adept::adouble &getParameter(size_t i) const
    {
        return m_parameters[i];
    }

    const adept::adouble &getParameter(std::string name) const
    {

        return m_parameters[m_names.at(name)];
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

    void functionWrapper(const FunctionDomain &domain, FunctionValues &values, AutoDiffJacobianAdapter *jacobian = 0) const;

    virtual void function1DAutoDiff(const FunctionDomain1D &domain, std::vector<adept::adouble> &y, const AutoDiffParameterAdapter &parameters) const = 0;
};


} // namespace API
} // namespace Mantid

#endif  /* MANTID_API_IFUNCTIONAUTODIFF_H_ */
