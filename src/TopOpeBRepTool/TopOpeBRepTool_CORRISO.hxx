// Created on: 1998-11-25
// Created by: Xuan PHAM PHU
// Copyright (c) 1998-1999 Matra Datavision
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

#ifndef _TopOpeBRepTool_CORRISO_HeaderFile
#define _TopOpeBRepTool_CORRISO_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <TopoDS_Face.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopOpeBRepTool_DataMapOfOrientedShapeC2DF.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapOfOrientedShapeInteger.hxx>
class TopoEdge;
class TopOpeBRepTool_C2DF;
class TopoVertex;

//! Fref is built on x-periodic surface (x=u,v).
//! S built on Fref's geometry, should be UVClosed.
//!
//! Give us E, an edge of S. 2drep(E) is not UV connexed.
//! We translate 2drep(E) in xdir*xperiod if necessary.
//!
//! call : TopOpeBRepTool_CORRISO Tool(Fref);
//! Tool.Init(S);
//! if (!Tool.UVClosed()) {
//! // initialize EdsToCheck,nfybounds,stopatfirst
//!
//! Tool.EdgeWithFaultyUV(EdsToCheck,nfybounds,FyEds,stopatfirst);
//! if (Tool.SetUVClosed()) S = Tool.GetnewS();
//! }
class TopOpeBRepTool_CORRISO
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRepTool_CORRISO();

  Standard_EXPORT TopOpeBRepTool_CORRISO(const TopoFace& FRef);

  Standard_EXPORT const TopoFace& Fref() const;

  Standard_EXPORT const GeomAdaptor_Surface& GASref() const;

  Standard_EXPORT Standard_Boolean Refclosed(const Standard_Integer x,
                                             Standard_Real&         xperiod) const;

  Standard_EXPORT Standard_Boolean Init(const TopoShape& S);

  Standard_EXPORT const TopoShape& S() const;

  Standard_EXPORT const ShapeList& Eds() const;

  Standard_EXPORT Standard_Boolean UVClosed() const;

  Standard_EXPORT Standard_Real Tol(const Standard_Integer I, const Standard_Real tol3d) const;

  Standard_EXPORT Standard_Boolean PurgeFyClosingE(const ShapeList& ClEds,
                                                   ShapeList&       fyClEds) const;

  Standard_EXPORT Standard_Integer EdgeOUTofBoundsUV(const TopoEdge&     E,
                                                     const Standard_Boolean onU,
                                                     const Standard_Real    tolx,
                                                     Standard_Real&         parspE) const;

  Standard_EXPORT Standard_Boolean
    EdgesOUTofBoundsUV(const ShapeList&             EdsToCheck,
                       const Standard_Boolean                  onU,
                       const Standard_Real                     tolx,
                       TopTools_DataMapOfOrientedShapeInteger& FyEds) const;

  Standard_EXPORT Standard_Boolean EdgeWithFaultyUV(const TopoEdge& E,
                                                    Standard_Integer&  Ivfaulty) const;

  Standard_EXPORT Standard_Boolean
    EdgesWithFaultyUV(const ShapeList&             EdsToCheck,
                      const Standard_Integer                  nfybounds,
                      TopTools_DataMapOfOrientedShapeInteger& FyEds,
                      const Standard_Boolean                  stopatfirst = Standard_False) const;

  Standard_EXPORT Standard_Boolean EdgeWithFaultyUV(const ShapeList& EdsToCheck,
                                                    const Standard_Integer      nfybounds,
                                                    TopoShape&               fyE,
                                                    Standard_Integer&           Ifaulty) const;

  Standard_EXPORT Standard_Boolean TrslUV(const Standard_Boolean                        onU,
                                          const TopTools_DataMapOfOrientedShapeInteger& FyEds);

  Standard_EXPORT Standard_Boolean GetnewS(TopoFace& newS) const;

  Standard_EXPORT Standard_Boolean UVRep(const TopoEdge& E, TopOpeBRepTool_C2DF& C2DF) const;

  Standard_EXPORT Standard_Boolean SetUVRep(const TopoEdge& E, const TopOpeBRepTool_C2DF& C2DF);

  Standard_EXPORT Standard_Boolean Connexity(const TopoVertex&  V,
                                             ShapeList& Eds) const;

  Standard_EXPORT Standard_Boolean SetConnexity(const TopoVertex&        V,
                                                const ShapeList& Eds);

  Standard_EXPORT Standard_Boolean AddNewConnexity(const TopoVertex& V, const TopoEdge& E);

  Standard_EXPORT Standard_Boolean RemoveOldConnexity(const TopoVertex& V, const TopoEdge& E);

protected:
private:
  TopoFace                               myFref;
  GeomAdaptor_Surface                       myGAS;
  Standard_Boolean                          myUclosed;
  Standard_Boolean                          myVclosed;
  Standard_Real                             myUper;
  Standard_Real                             myVper;
  TopoShape                              myS;
  ShapeList                      myEds;
  TopOpeBRepTool_DataMapOfOrientedShapeC2DF myERep2d;
  TopTools_DataMapOfShapeListOfShape        myVEds;
};

#endif // _TopOpeBRepTool_CORRISO_HeaderFile
