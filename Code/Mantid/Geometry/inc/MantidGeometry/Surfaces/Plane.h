#ifndef Plane_h
#define Plane_h

namespace Mantid
{

  namespace Geometry
  {

    /*!
    \class Plane
    \brief Holds a simple Plane
    \author S. Ansell
    \date April 2004
    \version 1.0

    Defines a plane and a normal and a distance from
    the origin

    Copyright &copy; 2007 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
    */
	class BaseVisit;
    class DLLExport Plane : public Quadratic
    {
    private:

      static Kernel::Logger& PLog;           ///< The official logger

      Geometry::V3D NormV;         ///< Normal vector
      double Dist;                   ///< Distance 

      int planeType() const;         ///< are we alined on an axis

    public:

      /// Effective typename 
      virtual std::string className() const { return "Plane"; }

      Plane();
      Plane(const Plane&);
      Plane* clone() const;
      Plane& operator=(const Plane&);
      virtual ~Plane();

      virtual void acceptVisitor(BaseVisit& A) const
      {  A.Accept(*this); }

      int setPlane(const Geometry::V3D&,const Geometry::V3D&);
      //  int setPlane(const std::string&);
      int side(const Geometry::V3D&) const;
      int onSurface(const Geometry::V3D&) const;
      // stuff for finding intersections etc.
      double dotProd(const Plane&) const;      ///< returns normal dot product
      Geometry::V3D crossProd(const Plane&) const;      ///< returns normal cross product
      double distance(const Geometry::V3D&) const;      ///< distance from a point

      double getDistance() const { return Dist; }  ///< Distance from origin
      Geometry::V3D getNormal() const { return NormV; }    ///< Normal to plane (+ve surface)

      void rotate(const Geometry::Matrix<double>&);
      void displace(const Geometry::V3D&);

      int setSurface(const std::string&);
      void print() const;
      void write(std::ostream&) const;        ///< Write in MCNPX form

      void setBaseEqn() ;                      ///< set up to be eqn based
	
	  int  LineIntersectionWithPlane(V3D startpt,V3D endpt,V3D& output);
	  void getBoundingBox(double &xmax,double &ymax,double &zmax,double &xmin,double &ymin,double &zmin);
    };

  } // NAMESPACE MonteCarlo

}  // NAMESPACE Mantid

#endif
