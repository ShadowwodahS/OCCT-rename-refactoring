// Created on: 1996-12-05
// Created by: Odile Olivier
// Copyright (c) 1996-1999 Matra Datavision
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

#include <PrsDim_Chamf3dDimension.hxx>

#include <PrsDim.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <DsgPrs_Chamf2dPresentation.hxx>
#include <ElCLib.hxx>
#include <Geom_Line.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <ProjLib.hxx>
#include <Prs3d_DimensionAspect.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_Presentation.hxx>
#include <Select3D_SensitiveBox.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_Selection.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PrsDim_Chamf3dDimension, PrsDim_Relation)

//=================================================================================================

PrsDim_Chamf3dDimension::PrsDim_Chamf3dDimension(const TopoShape&               aFShape,
                                                 const Standard_Real               aVal,
                                                 const UtfString& aText)
    : PrsDim_Relation()
{
  myFShape            = aFShape;
  myVal               = aVal;
  myText              = aText;
  mySymbolPrs         = DsgPrs_AS_LASTAR;
  myAutomaticPosition = Standard_True;

  myArrowSize = myVal / 100.;
}

//=================================================================================================

PrsDim_Chamf3dDimension::PrsDim_Chamf3dDimension(const TopoShape&               aFShape,
                                                 const Standard_Real               aVal,
                                                 const UtfString& aText,
                                                 const Point3d&                     aPosition,
                                                 const DsgPrs_ArrowSide            aSymbolPrs,
                                                 const Standard_Real               anArrowSize)
    : PrsDim_Relation()
{
  myFShape    = aFShape;
  myVal       = aVal;
  myText      = aText;
  myPosition  = aPosition;
  mySymbolPrs = aSymbolPrs;
  SetArrowSize(anArrowSize);
  myAutomaticPosition = Standard_False;
}

//=================================================================================================

void PrsDim_Chamf3dDimension::Compute(const Handle(PrsMgr_PresentationManager)&,
                                      const Handle(Prs3d_Presentation)& aPresentation,
                                      const Standard_Integer)
{
  //----------------------------
  // Calcul du centre de la face
  //----------------------------
  BRepAdaptor_Surface surfAlgo(TopoDS::Face(myFShape));
  Standard_Real       uFirst, uLast, vFirst, vLast;
  uFirst             = surfAlgo.FirstUParameter();
  uLast              = surfAlgo.LastUParameter();
  vFirst             = surfAlgo.FirstVParameter();
  vLast              = surfAlgo.LastVParameter();
  Standard_Real uMoy = (uFirst + uLast) / 2;
  Standard_Real vMoy = (vFirst + vLast) / 2;
  Point3d        apos;
  Vector3d        d1u, d1v;
  surfAlgo.D1(uMoy, vMoy, apos, d1u, d1v);
  myPntAttach = apos;

  myDir = d1u ^ d1v;
  //  myDir = surfAlgo.Plane().Axis().Direction();

  //--------------------------------------------
  // Calcul du point de positionnement du texte
  //--------------------------------------------
  Point3d curpos;
  if (myAutomaticPosition)
  {
    Vector3d transVec(myDir);
    transVec *= myVal;
    curpos = myPntAttach.Translated(transVec);

    if (myIsSetBndBox)
      curpos = PrsDim::TranslatePointToBound(curpos, myDir, myBndBox);

    myPosition = curpos;
  }
  else
  {

    Handle(GeomLine) dimLin    = new GeomLine(myPntAttach, myDir);
    Standard_Real     parcurpos = ElCLib1::Parameter(dimLin->Lin(), myPosition);
    curpos                      = ElCLib1::Value(parcurpos, dimLin->Lin());

    if (curpos.Distance(myPntAttach) < 5.)
    {
      Vector3d transVec(myDir);
      transVec *= 5.;
      curpos = myPntAttach.Translated(transVec);
    }
    myPosition = curpos;
  }

  Handle(Prs3d_DimensionAspect) la  = myDrawer->DimensionAspect();
  Handle(Prs3d_ArrowAspect)     arr = la->ArrowAspect();

  //-------------------------------------------------
  // Calcul de la boite englobante du component pour
  // determiner la taille de la fleche
  //-------------------------------------------------
  if (!myArrowSizeIsDefined)
  {
    Standard_Real arrsize = myArrowSize;
    if ((myVal / 4) < arrsize)
      arrsize = myVal / 4;
    if (arrsize > 30.)
      arrsize = 30.;
    else if (arrsize < 8.)
      arrsize = 8.;
    myArrowSize = arrsize;
  }
  arr->SetLength(myArrowSize);

  // Calcul de la presentation
  DsgPrs_Chamf2dPresentation::Add(aPresentation,
                                  myDrawer,
                                  myPntAttach,
                                  curpos,
                                  myText,
                                  mySymbolPrs);
}

//=================================================================================================

void PrsDim_Chamf3dDimension::ComputeSelection(const Handle(SelectionContainer)& aSelection,
                                               const Standard_Integer)
{
  Handle(SelectMgr_EntityOwner)     own = new SelectMgr_EntityOwner(this, 7);
  Handle(Select3D_SensitiveSegment) seg =
    new Select3D_SensitiveSegment(own, myPntAttach, myPosition);
  aSelection->Add(seg);

  // Text
  Standard_Real                 size(Min(myVal / 100. + 1.e-6, myArrowSize + 1.e-6));
  Handle(Select3D_SensitiveBox) box = new Select3D_SensitiveBox(own,
                                                                myPosition.X(),
                                                                myPosition.Y(),
                                                                myPosition.Z(),
                                                                myPosition.X() + size,
                                                                myPosition.Y() + size,
                                                                myPosition.Z() + size);
  aSelection->Add(box);
}
