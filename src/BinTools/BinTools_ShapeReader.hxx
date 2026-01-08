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

#ifndef _BinTools_ShapeReader_HeaderFile
#define _BinTools_ShapeReader_HeaderFile

#include <BinTools_ShapeSetBase.hxx>
#include <BinTools_IStream.hxx>
#include <NCollection_DataMap.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Shape.hxx>

class GeomCurve3d;
class GeomSurface;
class GeomCurve2d;
class Poly_Polygon3D;
class Poly_PolygonOnTriangulation;
class MeshTriangulation;

//! Reads topology from IStream in binary format without grouping of objects by types
//! and using relative positions in a file as references.
class BinaryShapeReader : public ShapeSetBase
{
public:
  DEFINE_STANDARD_ALLOC

  //! Initializes a shape reader.
  Standard_EXPORT BinaryShapeReader();

  Standard_EXPORT virtual ~BinaryShapeReader();

  //! Clears the content of the set.
  Standard_EXPORT virtual void Clear() Standard_OVERRIDE;

  //! Reads the shape from stream using previously restored shapes and objects by references.
  Standard_EXPORT void Read(Standard_IStream& theStream, TopoShape& theShape) Standard_OVERRIDE;

  //! Reads location from the stream.
  Standard_EXPORT const TopLoc_Location* ReadLocation(BinaryInputStream& theStream);

private:
  //! Reads the shape from stream using previously restored shapes and objects by references.
  TopoShape ReadShape(BinaryInputStream& theStream);
  //! Reads curve from the stream.
  Handle(GeomCurve3d) ReadCurve(BinaryInputStream& theStream);
  //! Reads curve2d from the stream.
  Handle(GeomCurve2d) ReadCurve2d(BinaryInputStream& theStream);
  //! Reads surface from the stream.
  Handle(GeomSurface) ReadSurface(BinaryInputStream& theStream);
  //! Reads ploygon3d from the stream.
  Handle(Poly_Polygon3D) ReadPolygon3d(BinaryInputStream& theStream);
  //! Reads polygon on triangulation from the stream.
  Handle(Poly_PolygonOnTriangulation) ReadPolygon(BinaryInputStream& theStream);
  //! Reads triangulation from the stream.
  Handle(MeshTriangulation) ReadTriangulation(BinaryInputStream& theStream);

  /// position of the shape previously restored
  NCollection_DataMap<uint64_t, TopoShape>                        myShapePos;
  NCollection_DataMap<uint64_t, TopLoc_Location>                     myLocationPos;
  NCollection_DataMap<uint64_t, Handle(GeomCurve3d)>                  myCurvePos;
  NCollection_DataMap<uint64_t, Handle(GeomCurve2d)>                myCurve2dPos;
  NCollection_DataMap<uint64_t, Handle(GeomSurface)>                mySurfacePos;
  NCollection_DataMap<uint64_t, Handle(Poly_Polygon3D)>              myPolygon3dPos;
  NCollection_DataMap<uint64_t, Handle(Poly_PolygonOnTriangulation)> myPolygonPos;
  NCollection_DataMap<uint64_t, Handle(MeshTriangulation)>          myTriangulationPos;
};

#endif // _BinTools_ShapeReader_HeaderFile
