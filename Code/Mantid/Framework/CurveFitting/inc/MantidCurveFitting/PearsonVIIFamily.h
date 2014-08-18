#ifndef MANTID_CURVEFITTING_PEARSONVIIFAMILY_H_
#define MANTID_CURVEFITTING_PEARSONVIIFAMILY_H_

#include "MantidKernel/System.h"
#include "MantidAPI/IFunction1D.h"
#include "MantidAPI/ParamFunction.h"
#include "MantidAPI/IFunction1DAutoDiff.h"


namespace Mantid
{
namespace CurveFitting
{

  /** PearsonVIIFamily : TODO: DESCRIPTION

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

namespace Pearsons {

  class DLLExport PearsonVIIHandCoded : public API::IFunction1D, public API::ParamFunction
  {
  public:
    virtual ~PearsonVIIHandCoded() {}

    std::string name() const { return "PearsonVIIHandCoded"; }

    void function1D(double *out, const double *xValues, const size_t nData) const;
    void functionDeriv1D(API::Jacobian *out, const double *xValues, const size_t nData);

  protected:
    void init();
  };

  class DLLExport PearsonVIINumDiff : public API::IFunction1D, public API::ParamFunction
  {
  public:
    virtual ~PearsonVIINumDiff() {}

    std::string name() const { return "PearsonVIINumDiff"; }

    void function1D(double *out, const double *xValues, const size_t nData) const;

  protected:
    void init();
  };

  class DLLExport PearsonVIIAutoDiff : virtual public API::IFunction1DAutoDiff, virtual public API::ParamFunction
  {
  public:
      virtual ~PearsonVIIAutoDiff() {}

      std::string name() const { return "PearsonVIIAutoDiff"; }

      void function1DAutoDiff(const API::FunctionDomain1D &domain, std::vector<adept::adouble> &y, const API::AutoDiffParameterAdapter &parameters) const;

  protected:
      void init();

  };

} // namespace PearsonVIIFamily
} // namespace CurveFitting
} // namespace Mantid

#endif  /* MANTID_CURVEFITTING_PEARSONVIIFAMILY_H_ */
