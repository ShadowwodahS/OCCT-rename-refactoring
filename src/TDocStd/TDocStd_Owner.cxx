// Created on: 1999-07-12
// Created by: Denis PASCAL
// Copyright (c) 1999-1999 Matra Datavision
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

#include <TDocStd_Owner.hxx>

#include <Standard_Dump.hxx>
#include <Standard_GUID.hxx>
#include <Standard_Type.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_Data.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDocStd_Document.hxx>

IMPLEMENT_STANDARD_RTTIEXT(TDocStd_Owner, TDF_Attribute)

//=================================================================================================

const Standard_GUID& TDocStd_Owner::GetID()
{
  static Standard_GUID TDocStd_OwnerID("2a96b617-ec8b-11d0-bee7-080009dc3333");
  return TDocStd_OwnerID;
}

//=================================================================================================

void TDocStd_Owner::SetDocument(const Handle(Data2)& indata, const Handle(AppDocument)& doc)
{
  Handle(TDocStd_Owner) A;
  if (!indata->Root().FindAttribute(TDocStd_Owner::GetID(), A))
  {
    A = new TDocStd_Owner();
    A->SetDocument(doc);
    indata->Root().AddAttribute(A);
  }
  else
  {
    throw Standard_DomainError("TDocStd_Owner::SetDocument : already called");
  }
}

//=================================================================================================

void TDocStd_Owner::SetDocument(const Handle(Data2)& indata, AppDocument* doc)
{
  Handle(TDocStd_Owner) A;
  if (!indata->Root().FindAttribute(TDocStd_Owner::GetID(), A))
  {
    A = new TDocStd_Owner();
    A->SetDocument(doc);
    indata->Root().AddAttribute(A);
  }
  else
  {
    throw Standard_DomainError("TDocStd_Owner::SetDocument : already called");
  }
}

//=================================================================================================

Handle(AppDocument) TDocStd_Owner::GetDocument(const Handle(Data2)& ofdata)
{
  Handle(TDocStd_Owner) A;
  if (!ofdata->Root().FindAttribute(TDocStd_Owner::GetID(), A))
  {
    throw Standard_DomainError("TDocStd_Owner::GetDocument : document not found");
  }
  return A->GetDocument();
}

//=================================================================================================

TDocStd_Owner::TDocStd_Owner() {}

//=================================================================================================

void TDocStd_Owner::SetDocument(const Handle(AppDocument)& document)
{
  myDocument = document.get();
}

//=================================================================================================

void TDocStd_Owner::SetDocument(AppDocument* document)
{
  myDocument = document;
}

//=================================================================================================

Handle(AppDocument) TDocStd_Owner::GetDocument() const
{
  return Handle(AppDocument)(myDocument);
}

//=================================================================================================

const Standard_GUID& TDocStd_Owner::ID() const
{
  return GetID();
}

//=================================================================================================

Handle(TDF_Attribute) TDocStd_Owner::NewEmpty() const
{
  Handle(TDF_Attribute) dummy;
  return dummy;
}

//=================================================================================================

void TDocStd_Owner::Restore(const Handle(TDF_Attribute)&) {}

//=================================================================================================

void TDocStd_Owner::Paste(const Handle(TDF_Attribute)&, const Handle(RelocationTable1)&) const {}

//=================================================================================================

Standard_OStream& TDocStd_Owner::Dump(Standard_OStream& anOS) const
{
  anOS << "Owner";
  return anOS;
}

//=================================================================================================

void TDocStd_Owner::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myDocument)
}
