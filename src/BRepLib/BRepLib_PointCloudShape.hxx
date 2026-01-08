// Copyright (c) 2021 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#ifndef _BRepLib_PointCloudShape_HeaderFile
#define _BRepLib_PointCloudShape_HeaderFile

#include <TopTools_DataMapOfShapeInteger.hxx>
#include <TopTools_DataMapOfShapeReal.hxx>
#include <Quantity_Color.hxx>
#include <Precision.hxx>

//! This tool is intended to get points from shape with specified distance from shape along normal.
//! Can be used to simulation of points obtained in result of laser scan of shape.
//! There are 2 ways for generation points by shape:
//! 1. Generation points with specified density
//! 2. Generation points using triangulation Nodes
//! Generation of points by density using the GeneratePointsByDensity() function is not thread safe.
class BRepLib_PointCloudShape
{
public:
  DEFINE_STANDARD_ALLOC

  //! Constructor initialized by shape
  Standard_EXPORT BRepLib_PointCloudShape(const TopoShape& theShape = TopoShape(),
                                          const Standard_Real theTol   = Precision1::Confusion());

  //! Virtual destructor
  Standard_EXPORT virtual ~BRepLib_PointCloudShape();

  //! Return loaded shape.
  const TopoShape& Shape() const { return myShape; }

  //! Set shape.
  void SetShape(const TopoShape& theShape) { myShape = theShape; }

  //! Return tolerance.
  Standard_Real Tolerance() const { return myTol; }

  //! Set tolerance.
  void SetTolerance(Standard_Real theTol) { myTol = theTol; }

  //! Returns value of the distance to define deflection of points from shape along normal to shape;
  //! 0.0 by default.
  Standard_Real GetDistance() const { return myDist; }

  //! Sets value of the distance to define deflection of points from shape along normal to shape.
  //! Negative values of theDist parameter are ignored.
  void SetDistance(const Standard_Real theDist) { myDist = theDist; }

  //! Returns size of the point cloud for specified density.
  Standard_EXPORT Standard_Integer NbPointsByDensity(const Standard_Real theDensity = 0.0);

  //! Returns size of the point cloud for using triangulation.
  Standard_EXPORT Standard_Integer NbPointsByTriangulation() const;

  //! Computes points with specified density for initial shape.
  //! If parameter Density is equal to 0 then density will be computed automatically by criterion:
  //! - 10 points per minimal unreduced face area.
  //!
  //! Note: this function should not be called from concurrent threads without external lock.
  Standard_EXPORT Standard_Boolean GeneratePointsByDensity(const Standard_Real theDensity = 0.0);

  //! Get points from triangulation existing in the shape.
  Standard_EXPORT Standard_Boolean GeneratePointsByTriangulation();

protected:
  //! Compute area of the specified face.
  Standard_EXPORT Standard_Real faceArea(const TopoShape& theShape);

  //! Computes default density points per face.
  Standard_EXPORT Standard_Real computeDensity();

  //! Adds points to face in accordance with the specified density randomly in the specified range
  //! [0, Dist].
  Standard_EXPORT Standard_Boolean addDensityPoints(const TopoShape& theFace);

  //! Adds points to face by nodes of the existing triangulation randomly in the specified range [0,
  //! Dist].
  Standard_EXPORT Standard_Boolean addTriangulationPoints(const TopoShape& theFace);

protected:
  //! Method to clear maps.
  Standard_EXPORT virtual void clear();

  //! Method to add point, normal to surface in this point and face for which point computed.
  //! @param[in] thePoint 3D point on the surface
  //! @param[in] theNorm  surface normal at this point
  //! @param[in] theUV    surface UV parameters
  //! @param[in] theFace  surface (face) definition
  Standard_EXPORT virtual void addPoint(const Point3d&       thePoint,
                                        const Vector3d&       theNorm,
                                        const gp_Pnt2d&     theUV,
                                        const TopoShape& theFace) = 0;

protected:
  TopoShape                   myShape;
  Standard_Real                  myDist;
  Standard_Real                  myTol;
  TopTools_DataMapOfShapeReal    myFaceArea;
  TopTools_DataMapOfShapeInteger myFacePoints;
  Standard_Integer               myNbPoints;
};

#endif // _BRepLib_PointCloudShape_HeaderFile
