#ifndef MANTID_CURVEFITTING_GAUSSIANAUTODIFF_H_
#define MANTID_CURVEFITTING_GAUSSIANAUTODIFF_H_

#include "MantidKernel/System.h"

#include "MantidAPI/IFunction1DAutoDiff.h"
#include "MantidAPI/ParamFunction.h"


namespace Mantid
{
namespace CurveFitting
{

/** GaussianAutoDiff : TODO: DESCRIPTION

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
class DLLExport GaussianAutoDiff : virtual public API::IFunction1DAutoDiff, virtual public API::ParamFunction
{
public:
    virtual ~GaussianAutoDiff();

    std::string name() const { return "GaussianAutoDiff"; }

    void function1DAutoDiff(const API::FunctionDomain1D &domain, std::vector<adept::adouble> &y, const API::AutoDiffParameterAdapter &parameters) const;

protected:
    void init();
    
};


} // namespace CurveFitting
} // namespace Mantid

#endif  /* MANTID_CURVEFITTING_GAUSSIANAUTODIFF_H_ */
