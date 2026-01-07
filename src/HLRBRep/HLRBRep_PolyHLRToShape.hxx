// Created on: 1993-10-11
// Created by: Christophe MARION
// Copyright (c) 1993-1999 Matra Datavision
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

#ifndef _HLRBRep_PolyHLRToShape_HeaderFile
#define _HLRBRep_PolyHLRToShape_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <HLRBRep_ListOfBPnt2D.hxx>
#include <TopoDS_Shape.hxx>
#include <Standard_Integer.hxx>
class HLRBRep_PolyAlgo;
class TopoShape;

//! A framework for filtering the computation
//! results of an HLRBRep_Algo algorithm by extraction.
//! From the results calculated by the algorithm on
//! a shape, a filter returns the type of edge you
//! want to identify. You can choose any of the following types of output:
//! -   visible sharp edges
//! -   hidden sharp edges
//! -   visible smooth edges
//! -   hidden smooth edges
//! -   visible sewn edges
//! -   hidden sewn edges
//! -   visible outline edges
//! -   hidden outline edges.
//! -   visible isoparameters and
//! -   hidden isoparameters.
//! Sharp edges present a C0 continuity (non G1).
//! Smooth edges present a G1 continuity (non G2).
//! Sewn edges present a C2 continuity.
//! The result is composed of 2D edges in the
//! projection plane of the view which the
//! algorithm has worked with. These 2D edges
//! are not included in the data structure of the visualized shape.
//! In order to obtain a complete image, you must
//! combine the shapes given by each of the chosen filters.
//! The construction of the shape does not call a
//! new computation of the algorithm, but only
//! reads its internal results.
class HLRBRep_PolyHLRToShape
{
public:
  DEFINE_STANDARD_ALLOC

  //! Constructs a framework for filtering the results
  //! of the HLRBRep_Algo algorithm, A.
  //! Use the extraction filters to obtain the results you want for A.
  Standard_EXPORT HLRBRep_PolyHLRToShape();

  Standard_EXPORT void Update(const Handle(HLRBRep_PolyAlgo)& A);

  void Show();

  void Hide();

  TopoShape VCompound();

  TopoShape VCompound(const TopoShape& S);

  //! Sets the extraction filter for visible smooth edges.
  TopoShape Rg1LineVCompound();

  TopoShape Rg1LineVCompound(const TopoShape& S);

  //! Sets the extraction filter for visible sewn edges.
  TopoShape RgNLineVCompound();

  TopoShape RgNLineVCompound(const TopoShape& S);

  TopoShape OutLineVCompound();

  //! Sets the extraction filter for visible outlines.
  TopoShape OutLineVCompound(const TopoShape& S);

  TopoShape HCompound();

  TopoShape HCompound(const TopoShape& S);

  TopoShape Rg1LineHCompound();

  //! Sets the extraction filter for hidden smooth edges.
  TopoShape Rg1LineHCompound(const TopoShape& S);

  TopoShape RgNLineHCompound();

  //! Sets the extraction filter for hidden sewn edges.
  TopoShape RgNLineHCompound(const TopoShape& S);

  TopoShape OutLineHCompound();

  //! Sets the extraction filter for hidden outlines.
  //! Hidden outlines occur, for instance, in tori. In
  //! this case, the inner outlines of the torus seen on its side are hidden.
  TopoShape OutLineHCompound(const TopoShape& S);

protected:
private:
  Standard_EXPORT TopoShape InternalCompound(const Standard_Integer typ,
                                                const Standard_Boolean visible,
                                                const TopoShape&    S);

  Handle(HLRBRep_PolyAlgo) myAlgo;
  HLRBRep_ListOfBPnt2D     myBiPntVis;
  HLRBRep_ListOfBPnt2D     myBiPntHid;
  Standard_Boolean         myHideMode;
};

#include <HLRBRep_PolyHLRToShape.lxx>

#endif // _HLRBRep_PolyHLRToShape_HeaderFile
