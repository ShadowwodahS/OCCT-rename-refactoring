// Copyright (c) 2017-2019 OPEN CASCADE SAS
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

#ifndef _RWMesh_FaceIterator_HeaderFile
#define _RWMesh_FaceIterator_HeaderFile

#include <RWMesh_ShapeIterator.hxx>

#include <BRepLProp_SLProps.hxx>
#include <Poly_Triangulation.hxx>
#include <TopoDS_Face.hxx>

#include <algorithm>

class DataLabel;

//! Auxiliary class to iterate through triangulated faces.
//! Class is designed to provide an interface for iterating over the faces
//! of a shape, specifically focusing on triangulated faces.
//! It inherits from the `RWMesh_ShapeIterator` base class and
//! extends its functionality to handle faces with triangulation data.
class RWMesh_FaceIterator : public RWMesh_ShapeIterator
{
public:
  //! Main constructor.
  //! @param[in] theLabel Label containing the face data
  //! @param[in] theLocation Location of the face
  //! @param[in] theToMapColors Flag to indicate if colors should be mapped
  //! @param[in] theStyle Style information for the face
  Standard_EXPORT RWMesh_FaceIterator(const DataLabel&       theLabel,
                                      const TopLoc_Location& theLocation,
                                      const Standard_Boolean theToMapColors = false,
                                      const XCAFPrs_Style&   theStyle       = XCAFPrs_Style());

  //! Auxiliary constructor.
  //! @param[in] theShape Shape containing the face data
  //! @param[in] theStyle Style information for the face
  Standard_EXPORT RWMesh_FaceIterator(const TopoShape&  theShape,
                                      const XCAFPrs_Style& theStyle = XCAFPrs_Style());

  //! Return true if iterator points to the valid triangulation.
  bool More() const Standard_OVERRIDE { return !myPolyTriang.IsNull(); }

  //! Find next value.
  Standard_EXPORT void Next() Standard_OVERRIDE;

  //! Return current face.
  const TopoFace& Face() const { return myFace; }

  //! Return current face.
  const TopoShape& Shape() const Standard_OVERRIDE { return myFace; }

  //! Return current face triangulation.
  const Handle(MeshTriangulation)& Triangulation() const { return myPolyTriang; }

  //! Return true if mesh data is defined.
  bool IsEmptyMesh() const { return IsEmpty(); }

  //! Return true if mesh data is defined.
  bool IsEmpty() const Standard_OVERRIDE
  {
    return myPolyTriang.IsNull()
           || (myPolyTriang->NbNodes() < 1 && myPolyTriang->NbTriangles() < 1);
  }

public:
  //! Return face material.
  const XCAFPrs_Style& FaceStyle() const { return myStyle; }

  //! Return TRUE if face color is set.
  bool HasFaceColor() const { return myHasColor; }

  //! Return face color.
  const Quantity_ColorRGBA& FaceColor() const { return myColor; }

public:
  //! Return number of elements of specific type for the current face.
  Standard_Integer NbTriangles() const { return myPolyTriang->NbTriangles(); }

  //! Lower element index in current triangulation.
  Standard_Integer ElemLower() const Standard_OVERRIDE { return 1; }

  //! Upper element index in current triangulation.
  Standard_Integer ElemUpper() const Standard_OVERRIDE { return myPolyTriang->NbTriangles(); }

  //! Return triangle with specified index with applied Face orientation.
  Triangle2 TriangleOriented(Standard_Integer theElemIndex) const
  {
    Triangle2 aTri = triangle(theElemIndex);
    if ((myFace.Orientation() == TopAbs_REVERSED) ^ myIsMirrored)
    {
      return Triangle2(aTri.Value(1), aTri.Value(3), aTri.Value(2));
    }
    return aTri;
  }

public:
  //! Return true if triangulation has defined normals.
  bool HasNormals() const { return myHasNormals; }

  //! Return true if triangulation has defined normals.
  bool HasTexCoords() const { return !myPolyTriang.IsNull() && myPolyTriang->HasUVNodes(); }

  //! Return normal at specified node index with face transformation applied and face orientation
  //! applied.
  Dir3d NormalTransformed(Standard_Integer theNode) const
  {
    Dir3d aNorm = normal(theNode);
    if (myTrsf.Form() != gp_Identity)
    {
      aNorm.Transform(myTrsf);
    }
    if (myFace.Orientation() == TopAbs_REVERSED)
    {
      aNorm.Reverse();
    }
    return aNorm;
  }

  //! Return number of nodes for the current face.
  Standard_Integer NbNodes() const Standard_OVERRIDE
  {
    return !myPolyTriang.IsNull() ? myPolyTriang->NbNodes() : 0;
  }

  //! Lower node index in current triangulation.
  Standard_Integer NodeLower() const Standard_OVERRIDE { return 1; }

  //! Upper node index in current triangulation.
  Standard_Integer NodeUpper() const Standard_OVERRIDE { return myPolyTriang->NbNodes(); }

  //! Return texture coordinates for the node.
  gp_Pnt2d NodeTexCoord(const Standard_Integer theNode) const
  {
    return myPolyTriang->HasUVNodes() ? myPolyTriang->UVNode(theNode) : gp_Pnt2d();
  }

public:
  //! Return the node with specified index with applied transformation.
  Point3d node(const Standard_Integer theNode) const Standard_OVERRIDE
  {
    return myPolyTriang->Node(theNode);
  }

  //! Return normal at specified node index without face transformation applied.
  Standard_EXPORT Dir3d normal(Standard_Integer theNode) const;

  //! Return triangle with specified index.
  Triangle2 triangle(Standard_Integer theElemIndex) const
  {
    return myPolyTriang->Triangle1(theElemIndex);
  }

private:
  //! Reset information for current face.
  void resetFace()
  {
    myPolyTriang.Nullify();
    myFace.Nullify();
    myHasNormals = false;
    resetShape();
  }

  //! Initialize face properties.
  void initFace();

private:
  // clang-format off
  TopoFace                myFace;        //!< current face
  Handle(MeshTriangulation) myPolyTriang;  //!< triangulation of current face
  mutable BRepLProp_SLProps  mySLTool;      //!< auxiliary tool for fetching normals from surface
  BRepAdaptor_Surface        myFaceAdaptor; //!< surface adaptor for fetching normals from surface
  Standard_Boolean           myHasNormals;  //!< flag indicating that current face has normals
  Standard_Boolean           myIsMirrored;  //!< flag indicating that face triangles should be mirrored
  // clang-format on
};

#endif // _RWMesh_FaceIterator_HeaderFile
