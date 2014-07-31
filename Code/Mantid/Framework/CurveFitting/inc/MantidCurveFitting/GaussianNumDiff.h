#ifndef MANTID_CURVEFITTING_GAUSSIANNUMDIFF_H_
#define MANTID_CURVEFITTING_GAUSSIANNUMDIFF_H_

#include "MantidKernel/System.h"

#include "MantidAPI/IFunction1D.h"
#include "MantidAPI/ParamFunction.h"


namespace Mantid
{
namespace CurveFitting
{

  /** GaussianNumDiff : TODO: DESCRIPTION
    
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
  class DLLExport GaussianNumDiff : public API::IFunction1D, public API::ParamFunction
  {
  public:
    GaussianNumDiff();
    virtual ~GaussianNumDiff();

    std::string name() const { return "GaussianNumDiff"; }

    void function1D(double *out, const double *xValues, const size_t nData) const;

  protected:
    void init();
    
  };


} // namespace CurveFitting
} // namespace Mantid

#endif  /* MANTID_CURVEFITTING_GAUSSIANNUMDIFF_H_ */
