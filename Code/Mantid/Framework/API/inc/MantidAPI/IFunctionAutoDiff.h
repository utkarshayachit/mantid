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
        m_jacobian[iY + iP * m_nValues] = value;
    }

    virtual double get(size_t iY, size_t iP) {
        return m_jacobian[iY + iP * m_nValues];
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

class AutoDiffParameters
{
public:
    AutoDiffParameters(const IFunction *fromFunction)
        : m_names(), m_parameters()
    {
        size_t np = fromFunction->nParams();

        m_parameters.resize(np);

        for(size_t i = 0; i < np; ++i) {
            m_names[fromFunction->parameterName(i)] = i;
            m_parameters[i].set_value(fromFunction->getParameter(i));
        }
    }

    size_t size() const { return m_parameters.size(); }

    adept::adouble &operator[](size_t i) { return m_parameters[i]; }

    adept::adouble getParameter(size_t i) const
    {
        return m_parameters[i];
    }

    adept::adouble getParameter(std::string &name) const
    {
        return m_parameters[m_names.at(name)];
    }

private:
    std::map<std::string, size_t> m_names;
    std::vector<adept::adouble> m_parameters;
};


class DLLExport IFunctionAutoDiff : virtual public IFunction
{
public:
    IFunctionAutoDiff();
    virtual ~IFunctionAutoDiff();

    void function(const FunctionDomain &domain, FunctionValues &values) const;
    void functionDeriv(const FunctionDomain &domain, Jacobian &jacobian);

    void functionWrapper(const FunctionDomain &domain, FunctionValues &values, AutoDiffJacobian *jacobian) const;

    virtual void functionAutoDiff(std::vector<adept::adouble> &x, std::vector<adept::adouble> &y, const AutoDiffParameters &parameters) const = 0;
};


} // namespace API
} // namespace Mantid

#endif  /* MANTID_API_IFUNCTIONAUTODIFF_H_ */
