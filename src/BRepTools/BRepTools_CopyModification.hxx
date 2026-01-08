// Copyright (c) 1999-2022 OPEN CASCADE SAS
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

#ifndef _BRepTools_CopyModification_HeaderFile
#define _BRepTools_CopyModification_HeaderFile

#include <BRepTools_Modification.hxx>

class BRepTools_CopyModification;
DEFINE_STANDARD_HANDLE(BRepTools_CopyModification, ShapeModification)

//! Tool class implementing necessary functionality for copying geometry and triangulation.
class BRepTools_CopyModification : public ShapeModification
{
public:
  //! Constructor.
  //! \param[in] theCopyGeom  indicates that the geometry (surfaces and curves) should be copied
  //! \param[in] theCopyMesh  indicates that the triangulation should be copied
  Standard_EXPORT explicit BRepTools_CopyModification(
    const Standard_Boolean theCopyGeom = Standard_True,
    const Standard_Boolean theCopyMesh = Standard_True);

  //! Returns true if theFace has been modified.
  //! If the face has been modified:
  //! - theSurf is the new geometry of the face,
  //! - theLoc is its new location, and
  //! - theTol is the new tolerance.
  //! theRevWires, theRevFace are always set to false, because the orientation is not changed.
  Standard_EXPORT Standard_Boolean NewSurface(const TopoFace&    theFace,
                                              Handle(GeomSurface)& theSurf,
                                              TopLoc_Location&      theLoc,
                                              Standard_Real&        theTol,
                                              Standard_Boolean&     theRevWires,
                                              Standard_Boolean&     theRevFace) Standard_OVERRIDE;

  //! Returns true if theEdge has been modified.
  //! If the edge has been modified:
  //! - theCurve is the new geometric support of the edge,
  //! - theLoc is the new location, and
  //! - theTol is the new tolerance.
  //! If the edge has not been modified, this function
  //! returns false, and the values of theCurve, theLoc and theTol are not significant.
  Standard_EXPORT Standard_Boolean NewCurve(const TopoEdge&  theEdge,
                                            Handle(GeomCurve3d)& theCurve,
                                            TopLoc_Location&    theLoc,
                                            Standard_Real&      theTol) Standard_OVERRIDE;

  //! Returns true if theVertex has been modified.
  //! If the vertex has been modified:
  //! - thePnt is the new geometry of the vertex, and
  //! - theTol is the new tolerance.
  //! If the vertex has not been modified this function
  //! returns false, and the values of thePnt and theTol are not significant.
  Standard_EXPORT Standard_Boolean NewPoint(const TopoVertex& theVertex,
                                            Point3d&              thePnt,
                                            Standard_Real&       theTol) Standard_OVERRIDE;

  //! Returns true if theEdge has a new curve on surface on theFace.
  //! If a new curve exists:
  //! - theCurve is the new geometric support of the edge,
  //! - theTol the new tolerance.
  //! If no new curve exists, this function returns false, and
  //! the values of theCurve and theTol are not significant.
  Standard_EXPORT Standard_Boolean NewCurve2d(const TopoEdge&    theEdge,
                                              const TopoFace&    theFace,
                                              const TopoEdge&    theNewEdge,
                                              const TopoFace&    theNewFace,
                                              Handle(GeomCurve2d)& theCurve,
                                              Standard_Real&        theTol) Standard_OVERRIDE;

  //! Returns true if theVertex has a new parameter on theEdge.
  //! If a new parameter exists:
  //! - thePnt is the parameter, and
  //! - theTol is the new tolerance.
  //! If no new parameter exists, this function returns false,
  //! and the values of thePnt and theTol are not significant.
  Standard_EXPORT Standard_Boolean NewParameter(const TopoVertex& theVertex,
                                                const TopoEdge&   theEdge,
                                                Standard_Real&       thePnt,
                                                Standard_Real&       theTol) Standard_OVERRIDE;

  //! Returns the continuity of theNewEdge between theNewFace1 and theNewFace2.
  //!
  //! theNewEdge is the new edge created from theEdge.  theNewFace1
  //! (resp. theNewFace2) is the new face created from theFace1 (resp. theFace2).
  Standard_EXPORT GeomAbs_Shape Continuity(const TopoEdge& theEdge,
                                           const TopoFace& theFace1,
                                           const TopoFace& theFace2,
                                           const TopoEdge& theNewEdge,
                                           const TopoFace& theNewFace1,
                                           const TopoFace& theNewFace2) Standard_OVERRIDE;

  //! Returns true if the face has been modified according to changed triangulation.
  //! If the face has been modified:
  //! - theTri is a new triangulation on the face
  Standard_EXPORT Standard_Boolean NewTriangulation(const TopoFace&          theFace,
                                                    Handle(MeshTriangulation)& theTri)
    Standard_OVERRIDE;

  //! Returns true if the edge has been modified according to changed polygon.
  //! If the edge has been modified:
  //! - thePoly is a new polygon
  Standard_EXPORT Standard_Boolean NewPolygon(const TopoEdge&      theEdge,
                                              Handle(Poly_Polygon3D)& thePoly) Standard_OVERRIDE;

  //! Returns true if the edge has been modified according to changed polygon on triangulation.
  //! If the edge has been modified:
  //! - thePoly is a new polygon on triangulation
  Standard_EXPORT Standard_Boolean
    NewPolygonOnTriangulation(const TopoEdge&                   theEdge,
                              const TopoFace&                   theFace,
                              Handle(Poly_PolygonOnTriangulation)& thePoly) Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(BRepTools_CopyModification, ShapeModification)

private:
  Standard_Boolean myCopyGeom;
  Standard_Boolean myCopyMesh;
};

#endif // _BRepTools_CopyModification_HeaderFile
