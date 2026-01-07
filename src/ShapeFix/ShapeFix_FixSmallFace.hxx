// Created on: 1999-09-13
// Created by: data exchange team
// Copyright (c) 1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#ifndef _ShapeFix_FixSmallFace_HeaderFile
#define _ShapeFix_FixSmallFace_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TopoDS_Shape.hxx>
#include <Standard_Integer.hxx>
#include <ShapeAnalysis_CheckSmallFace.hxx>
#include <ShapeFix_Root.hxx>
class TopoFace;
class TopoEdge;
class TopoCompound;

class ShapeFix_FixSmallFace;
DEFINE_STANDARD_HANDLE(ShapeFix_FixSmallFace, ShapeFix_Root)

//! Fixing face with small size
class ShapeFix_FixSmallFace : public ShapeFix_Root
{

public:
  Standard_EXPORT ShapeFix_FixSmallFace();

  Standard_EXPORT void Init(const TopoShape& S);

  //! Fixing case of spot face
  Standard_EXPORT void Perform();

  //! Fixing case of spot face, if tol = -1 used local tolerance.
  Standard_EXPORT TopoShape FixSpotFace();

  //! Compute average vertex and replacing vertices by new one.
  Standard_EXPORT Standard_Boolean ReplaceVerticesInCaseOfSpot(TopoFace&        F,
                                                               const Standard_Real tol) const;

  //! Remove spot face from compound
  Standard_EXPORT Standard_Boolean RemoveFacesInCaseOfSpot(const TopoFace& F) const;

  //! Fixing case of strip face, if tol = -1 used local tolerance
  Standard_EXPORT TopoShape FixStripFace(const Standard_Boolean wasdone = Standard_False);

  //! Replace veretces and edges.
  Standard_EXPORT Standard_Boolean ReplaceInCaseOfStrip(TopoFace&        F,
                                                        TopoEdge&        E1,
                                                        TopoEdge&        E2,
                                                        const Standard_Real tol) const;

  //! Remove strip face from compound.
  Standard_EXPORT Standard_Boolean RemoveFacesInCaseOfStrip(const TopoFace& F) const;

  //! Compute average edge for strip face
  Standard_EXPORT TopoEdge ComputeSharedEdgeForStripFace(const TopoFace&  F,
                                                            const TopoEdge&  E1,
                                                            const TopoEdge&  E2,
                                                            const TopoFace&  F1,
                                                            const Standard_Real tol) const;

  Standard_EXPORT TopoShape FixSplitFace(const TopoShape& S);

  //! Compute data for face splitting.
  Standard_EXPORT Standard_Boolean SplitOneFace(TopoFace& F, TopoCompound& theSplittedFaces);

  Standard_EXPORT TopoFace FixFace(const TopoFace& F);

  Standard_EXPORT TopoShape FixShape();

  Standard_EXPORT TopoShape Shape();

  Standard_EXPORT Standard_Boolean FixPinFace(TopoFace& F);

  DEFINE_STANDARD_RTTIEXT(ShapeFix_FixSmallFace, ShapeFix_Root)

protected:
private:
  TopoShape                 myShape;
  TopoShape                 myResult;
  Standard_Integer             myStatus;
  ShapeAnalysis_CheckSmallFace myAnalyzer;
};

#endif // _ShapeFix_FixSmallFace_HeaderFile
