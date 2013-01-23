#ifndef MANTID_CURVEFITTING_GRAMCHARLIERCOMPTONPROFILE_H_
#define MANTID_CURVEFITTING_GRAMCHARLIERCOMPTONPROFILE_H_

#include "MantidCurveFitting/DllConfig.h"
#include "MantidCurveFitting/ComptonProfile.h"


namespace Mantid
{
namespace CurveFitting
{
  /**

    Implements a function to calculate the Compton profile of a nucleus using a Gram-Charlier approximation
    convoluted with an instrument resolution function that is approximated by a Voigt function.
    
    Copyright &copy; 2013 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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
  class DLLExport GramCharlierComptonProfile : public ComptonProfile
  {
  public:
    /// Default constructor required by factory
    GramCharlierComptonProfile();
    
  private:
    /// A string identifier for this function
    std::string name() const;
    /// Declare the function parameters
    void declareParameters();
    /// Declare non-parameter attributes
    void declareAttributes();
    /// Set an attribute value (and possibly cache its value)
    void setAttribute(const std::string& name,const Attribute& value);
    /// Parse the active hermite polynomial coefficents
    void setHermiteCoefficients(const std::string & coeffs);
    /// Declare the Gram-Charlier (Hermite) coefficients
    void declareGramCharlierParameters();

    /// Compute the function
    void massProfile(std::vector<double> & result,const double lorentzFWHM, const double resolutionFWHM) const;


    /// The active hermite coefficents
    std::vector<short> m_hermite;
  };


} // namespace CurveFitting
} // namespace Mantid

#endif  /* MANTID_CURVEFITTING_GRAMCHARLIERCOMPTONPROFILE_H_ */