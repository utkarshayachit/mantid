#ifndef MANTID_API_IFUNCTIONAUTODIFF_H_
#define MANTID_API_IFUNCTIONAUTODIFF_H_

#include "MantidKernel/System.h"
#include "MantidAPI/IFunction.h"
#include "MantidAPI/Jacobian.h"
#include "MantidAPI/ParamFunction.h"

#include <adept.h>

namespace Mantid
{
namespace API
{

/** IFunctionAutoDiff : TODO: DESCRIPTION

    Copyright &copy; 2014 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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
class AutoDiffJacobian : public Jacobian
{
public:
    AutoDiffJacobian() : Jacobian(),
        m_nParams(0), m_nValues(0), m_jacobian(), m_isValid(false)
    { }

    virtual ~AutoDiffJacobian() { }

    void setSize(size_t nParams, size_t nValues) {
        m_nParams = nParams;
        m_nValues = nValues;
        m_jacobian.resize(nParams * nValues);
    }

    double *rawValues() {
        return &m_jacobian[0];
    }

    virtual void set(size_t iY, size_t iP, double value) {
        m_jacobian[iY + iP * m_nParams] = value;
    }

    virtual double get(size_t iY, size_t iP) {
        return m_jacobian[iY + iP * m_nParams];
    }

    void setValid() {
        m_isValid = true;
    }

    void setInvalid() {
        m_isValid = false;
    }

    bool isValid() {
        return m_isValid;
    }

protected:
    size_t m_nParams;
    size_t m_nValues;
    std::vector<double> m_jacobian;

    bool m_isValid;
};


class DLLExport IFunctionAutoDiff : virtual public IFunction, virtual public ParamFunction
{
public:
    IFunctionAutoDiff();
    virtual ~IFunctionAutoDiff();

    void function(const FunctionDomain &domain, FunctionValues &values) const;
    void functionDeriv(const FunctionDomain &domain, Jacobian &jacobian);

    virtual void functionAutoDiff(std::vector<adept::adouble> &x, std::vector<adept::adouble> &y, std::vector<adept::adouble> &parameters) const = 0;

    virtual void setParameter(size_t i, const double &value, bool explicitlySet) {
        ParamFunction::setParameter(i, value, explicitlySet);

        m_jacobian->setInvalid();
    }

    virtual void setParameter(const std::string& name, const double &value, bool explicitlySet) {
        ParamFunction::setParameter(name, value, explicitlySet);

        m_jacobian->setInvalid();
    }

protected:
    std::vector<adept::adouble> getAdeptParameters() const;

    adept::Stack *m_stack;
    AutoDiffJacobian *m_jacobian;
};


} // namespace API
} // namespace Mantid

#endif  /* MANTID_API_IFUNCTIONAUTODIFF_H_ */
