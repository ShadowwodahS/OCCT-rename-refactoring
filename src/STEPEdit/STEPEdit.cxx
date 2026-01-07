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

#include <STEPEdit.hxx>

#include <IFSelect_SelectModelEntities.hxx>
#include <IFSelect_SelectModelRoots.hxx>
#include <IFSelect_SelectSignature.hxx>
#include <Standard_Mutex.hxx>
#include <StepAP214.hxx>
#include <StepAP214_Protocol.hxx>
#include <StepData_StepModel.hxx>
#include <StepSelect_StepType.hxx>

Handle(Interface_Protocol) STEPEdit1::Protocol()
{
  /*
    static Handle(StepData_FileProtocol) proto;
    if (!proto.IsNull()) return proto;
    proto =  new StepData_FileProtocol;
    proto->Add (StepAP2141::Protocol());
    proto->Add (HeaderSection1::Protocol());
    return proto;
  */
  return StepAP2141::Protocol();
}

Handle(StepData_StepModel) STEPEdit1::NewModel()
{
  Handle(StepData_StepModel) stepmodel = new StepData_StepModel;
  stepmodel->SetProtocol(STEPEdit1::Protocol());
  return stepmodel;
}

Handle(IFSelect_Signature) STEPEdit1::SignType()
{
  static Standard_Mutex              aMutex;
  Standard_Mutex::Sentry             aSentry(aMutex);
  static Handle(StepSelect_StepType) sty;
  if (!sty.IsNull())
    return sty;
  sty = new StepSelect_StepType;
  sty->SetProtocol(STEPEdit1::Protocol());
  return sty;
}

Handle(IFSelect_SelectSignature) STEPEdit1::NewSelectSDR()
{
  Handle(IFSelect_SelectSignature) sel =
    new IFSelect_SelectSignature(STEPEdit1::SignType(), "SHAPE_DEFINITION_REPRESENTATION");
  sel->SetInput(new IFSelect_SelectModelRoots);
  return sel;
}

Handle(IFSelect_SelectSignature) STEPEdit1::NewSelectPlacedItem()
{
  Handle(IFSelect_SelectSignature) sel =
    new IFSelect_SelectSignature(STEPEdit1::SignType(),
                                 "MAPPED_ITEM|CONTEXT_DEPENDENT_SHAPE_REPRESENTATION",
                                 Standard_False);
  sel->SetInput(new IFSelect_SelectModelEntities);
  return sel;
}

Handle(IFSelect_SelectSignature) STEPEdit1::NewSelectShapeRepr()
{
  Handle(IFSelect_SelectSignature) sel =
    new IFSelect_SelectSignature(STEPEdit1::SignType(), "SHAPE_REPRESENTATION", Standard_False);
  // REPRESENTATION_RELATIONSHIP passe par CONTEXT_DEPENDENT_SHAPE_REPRESENTATION
  sel->SetInput(new IFSelect_SelectModelEntities);
  return sel;
}
