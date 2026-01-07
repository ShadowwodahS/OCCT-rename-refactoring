// Created on: 2000-01-25
// Created by: Peter KURNEV
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#ifndef _TopOpeBRepBuild_CorrectFace2d_HeaderFile
#define _TopOpeBRepBuild_CorrectFace2d_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Face.hxx>
#include <Standard_Integer.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_IndexedMapOfOrientedShape.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>
class TopoEdge;
class gp_Pnt2d;
class TopoShape;
class gp_Vec2d;
class GeomCurve2d;
class Bnd_Box2d;

class TopOpeBRepBuild_CorrectFace2d
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRepBuild_CorrectFace2d();

  Standard_EXPORT TopOpeBRepBuild_CorrectFace2d(
    const TopoFace&                        aFace,
    const TopTools_IndexedMapOfOrientedShape& anAvoidMap,
    TopTools_IndexedDataMapOfShapeShape&      aMap);

  Standard_EXPORT const TopoFace& Face() const;

  Standard_EXPORT void Perform();

  Standard_EXPORT Standard_Boolean IsDone() const;

  Standard_EXPORT Standard_Integer ErrorStatus() const;

  Standard_EXPORT const TopoFace& CorrectedFace() const;

  Standard_EXPORT void SetMapOfTrans2dInfo(TopTools_IndexedDataMapOfShapeShape& aMap);

  Standard_EXPORT TopTools_IndexedDataMapOfShapeShape& MapOfTrans2dInfo();

  Standard_EXPORT static void GetP2dFL(const TopoFace& aFace,
                                       const TopoEdge& anEdge,
                                       gp_Pnt2d&          P2dF,
                                       gp_Pnt2d&          P2dL);

  Standard_EXPORT static void CheckList(const TopoFace& aFace, ShapeList& aHeadList);

protected:
private:
  Standard_EXPORT void CheckFace();

  Standard_EXPORT Standard_Integer MakeRightWire();

  Standard_EXPORT void MakeHeadList(const TopoShape&   aFirstEdge,
                                    ShapeList& aHeadList) const;

  Standard_EXPORT void TranslateCurve2d(const TopoEdge&    anEdge,
                                        const TopoFace&    aFace,
                                        const gp_Vec2d&       aTranslateVec,
                                        Handle(GeomCurve2d)& aCurve2d);

  Standard_EXPORT Standard_Integer OuterWire(TopoWire& anOuterWire) const;

  Standard_EXPORT void BndBoxWire(const TopoWire& aWire, Bnd_Box2d& aB2d) const;

  Standard_EXPORT void MoveWire2d(TopoWire& aWire, const gp_Vec2d& aTrV);

  Standard_EXPORT void MoveWires2d(TopoWire& aWire);

  Standard_EXPORT void UpdateEdge(const TopoEdge&          E,
                                  const Handle(GeomCurve2d)& C,
                                  const TopoFace&          F,
                                  const Standard_Real         Tol);

  Standard_EXPORT void UpdateEdge(const TopoEdge&          E,
                                  const Handle(GeomCurve2d)& C1,
                                  const Handle(GeomCurve2d)& C2,
                                  const TopoFace&          F,
                                  const Standard_Real         Tol);

  Standard_EXPORT void BuildCopyData(const TopoFace&                        F,
                                     const TopTools_IndexedMapOfOrientedShape& anAvoidMap,
                                     TopoFace&                              aCopyFace,
                                     TopTools_IndexedMapOfOrientedShape&       aCopyAvoidMap,
                                     const Standard_Boolean                    aNeedToUsePMap);

  Standard_EXPORT Standard_Integer
    ConnectWire(TopoFace&                              aCopyFace,
                const TopTools_IndexedMapOfOrientedShape& aCopyAvoidMap,
                const Standard_Boolean                    aTryToConnectFlag);

  TopoFace                         myFace;
  TopoFace                         myCorrectedFace;
  Standard_Boolean                    myIsDone;
  Standard_Integer                    myErrorStatus;
  Standard_Real                       myFaceTolerance;
  TopoWire                         myCurrentWire;
  ShapeList                myOrderedWireList;
  TopTools_IndexedMapOfOrientedShape  myAvoidMap;
  Standard_Address                    myMap;
  TopoFace                         myCopyFace;
  TopTools_IndexedMapOfOrientedShape  myCopyAvoidMap;
  TopTools_IndexedDataMapOfShapeShape myEdMapInversed;
};

#endif // _TopOpeBRepBuild_CorrectFace2d_HeaderFile
