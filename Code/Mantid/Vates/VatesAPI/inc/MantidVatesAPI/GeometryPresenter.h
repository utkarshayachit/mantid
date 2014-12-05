#ifndef GEOMETRY_PRESENTER_H
#define GEOMETRY_PRESENTER_H

#include "MantidKernel/System.h"
#include "MantidGeometry/MDGeometry/IMDDimension.h"
#include <map>

namespace Mantid
{
  namespace VATES
  {
    class GeometryView;
    class DimensionView;
    class DimensionPresenter;

    /** @class GeometryPresenter

    Abstract type for MVP style presenter for a Multi-dimensional workspace geometry.

    @author Owen Arnold, Tessella Support Services plc
    @date 24/05/2011

    Copyright &copy; 2007-11 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge National Laboratory & European Spallation Source

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

    File change history is stored at: <https://github.com/mantidproject/mantid>.
    Code Documentation is available at: <http://doxygen.mantidproject.org>
    */

    
    class DimensionPresenter;
    class DLLExport GeometryPresenter
    {
    public:
      typedef std::map<std::string, boost::shared_ptr<DimensionPresenter> > MappingType;
      virtual void dimensionResized(DimensionPresenter* pDimensionPresenter) = 0;
      virtual void dimensionRealigned(DimensionPresenter* pDimensionPresenter) = 0;
      virtual Mantid::Geometry::VecIMDDimension_sptr getNonIntegratedDimensions() const = 0;
      virtual MappingType getMappings() const = 0;
      virtual std::string getGeometryXML() const = 0;
      virtual ~GeometryPresenter() {}
      virtual void acceptView(GeometryView*)=0;
      virtual void setModified() = 0;
      virtual void setDimensionModeChanged() = 0;
    };
  }
}

#endif
