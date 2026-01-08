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

#include <XCAFDoc_DimTolTool.hxx>

#include <Standard_Type.hxx>
#include <TColStd_MapOfAsciiString.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_ChildIDIterator.hxx>
#include <TDF_Label.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDF_Tool.hxx>
#include <XCAFDimTolObjects_DatumObject.hxx>
#include <XCAFDimTolObjects_DimensionObject.hxx>
#include <XCAFDimTolObjects_GeomToleranceObject.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_Dimension.hxx>
#include <XCAFDoc_GeomTolerance.hxx>
#include <XCAFDoc_Datum.hxx>
#include <XCAFDoc_DimTol.hxx>
#include <XCAFDoc_DimTolTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <XCAFDoc_ShapeTool.hxx>

IMPLEMENT_DERIVED_ATTRIBUTE_WITH_TYPE(XCAFDoc_DimTolTool,
                                      TDataStd_GenericEmpty,
                                      "xcaf",
                                      "DimTolTool")

//=================================================================================================

XCAFDoc_DimTolTool::XCAFDoc_DimTolTool() {}

//=================================================================================================

Handle(XCAFDoc_DimTolTool) XCAFDoc_DimTolTool::Set(const DataLabel& L)
{
  Handle(XCAFDoc_DimTolTool) A;
  if (!L.FindAttribute(XCAFDoc_DimTolTool::GetID(), A))
  {
    A = new XCAFDoc_DimTolTool();
    L.AddAttribute(A);
    A->myShapeTool = XCAFDoc_DocumentTool::ShapeTool(L);
  }
  return A;
}

//=================================================================================================

const Standard_GUID& XCAFDoc_DimTolTool::GetID()
{
  static Standard_GUID DGTTblID("72afb19b-44de-11d8-8776-001083004c77");
  return DGTTblID;
}

//=================================================================================================

DataLabel XCAFDoc_DimTolTool::BaseLabel() const
{
  return Label();
}

//=================================================================================================

const Handle(XCAFDoc_ShapeTool)& XCAFDoc_DimTolTool::ShapeTool()
{
  if (myShapeTool.IsNull())
    myShapeTool = XCAFDoc_DocumentTool::ShapeTool(Label());
  return myShapeTool;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::IsDimTol(const DataLabel& theDimTolL) const
{
  Handle(XCAFDoc_DimTol) aDimTolAttr;
  if (theDimTolL.FindAttribute(XCAFDoc_DimTol::GetID(), aDimTolAttr))
  {
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::IsDimension(const DataLabel& theDimTolL) const
{
  Handle(XCAFDoc_Dimension) aDimTolAttr;
  if (theDimTolL.FindAttribute(XCAFDoc_Dimension::GetID(), aDimTolAttr))
  {
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::IsGeomTolerance(const DataLabel& theDimTolL) const
{
  Handle(XCAFDoc_GeomTolerance) aDimTolAttr;
  if (theDimTolL.FindAttribute(XCAFDoc_GeomTolerance::GetID(), aDimTolAttr))
  {
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

void XCAFDoc_DimTolTool::GetDimTolLabels(TDF_LabelSequence& theLabels) const
{
  theLabels.Clear();
  ChildIterator aChildIterator(Label());
  for (; aChildIterator.More(); aChildIterator.Next())
  {
    DataLabel aL = aChildIterator.Value();
    if (IsDimTol(aL))
      theLabels.Append(aL);
  }
}

//=================================================================================================

void XCAFDoc_DimTolTool::GetDimensionLabels(TDF_LabelSequence& theLabels) const
{
  theLabels.Clear();
  ChildIterator aChildIterator(Label());
  for (; aChildIterator.More(); aChildIterator.Next())
  {
    DataLabel aL = aChildIterator.Value();
    if (IsDimension(aL))
      theLabels.Append(aL);
  }
}

//=================================================================================================

void XCAFDoc_DimTolTool::GetGeomToleranceLabels(TDF_LabelSequence& theLabels) const
{
  theLabels.Clear();
  ChildIterator aChildIterator(Label());
  for (; aChildIterator.More(); aChildIterator.Next())
  {
    DataLabel aL = aChildIterator.Value();
    if (IsGeomTolerance(aL))
      theLabels.Append(aL);
  }
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::FindDimTol(
  const Standard_Integer                  kind,
  const Handle(TColStd_HArray1OfReal)&    aVal,
  const Handle(TCollection_HAsciiString)& aName,
  const Handle(TCollection_HAsciiString)& aDescription,
  DataLabel&                              lab) const
{
  TDF_ChildIDIterator it(Label(), XCAFDoc_DimTol::GetID());
  for (; it.More(); it.Next())
  {
    DataLabel              DimTolL = it.Value()->Label();
    Handle(XCAFDoc_DimTol) DimTolAttr;
    if (!DimTolL.FindAttribute(XCAFDoc_DimTol::GetID(), DimTolAttr))
      continue;
    Standard_Integer                 kind1         = DimTolAttr->GetKind();
    Handle(TColStd_HArray1OfReal)    aVal1         = DimTolAttr->GetVal();
    Handle(TCollection_HAsciiString) aName1        = DimTolAttr->GetName();
    Handle(TCollection_HAsciiString) aDescription1 = DimTolAttr->GetDescription();
    Standard_Boolean                 IsEqual       = Standard_True;
    if (!(kind1 == kind))
      continue;
    if (!(aName == aName1))
      continue;
    if (!(aDescription == aDescription1))
      continue;
    if (kind < 20)
    { // dimension
      for (Standard_Integer i = 1; i <= aVal->Length(); i++)
      {
        if (Abs(aVal->Value(i) - aVal1->Value(i)) > Precision1::Confusion())
          IsEqual = Standard_False;
      }
    }
    else if (kind < 50)
    { // tolerance
      if (Abs(aVal->Value(1) - aVal1->Value(1)) > Precision1::Confusion())
        IsEqual = Standard_False;
    }
    if (IsEqual)
    {
      lab = DimTolL;
      return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

DataLabel XCAFDoc_DimTolTool::FindDimTol(const Standard_Integer                  kind,
                                         const Handle(TColStd_HArray1OfReal)&    aVal,
                                         const Handle(TCollection_HAsciiString)& aName,
                                         const Handle(TCollection_HAsciiString)& aDescription) const
{
  DataLabel L;
  FindDimTol(kind, aVal, aName, aDescription, L);
  return L;
}

//=================================================================================================

DataLabel XCAFDoc_DimTolTool::AddDimTol(const Standard_Integer                  kind,
                                        const Handle(TColStd_HArray1OfReal)&    aVal,
                                        const Handle(TCollection_HAsciiString)& aName,
                                        const Handle(TCollection_HAsciiString)& aDescription) const
{
  DataLabel     DimTolL;
  TDF_TagSource aTag;
  DimTolL = aTag.NewChild(Label());
  XCAFDoc_DimTol::Set(DimTolL, kind, aVal, aName, aDescription);
  AsciiString1 str = "DGT:";
  if (kind < 20)
    str.AssignCat("Dimension");
  else
    str.AssignCat("Tolerance");
  NameAttribute::Set(DimTolL, str);
  return DimTolL;
}

//=================================================================================================

DataLabel XCAFDoc_DimTolTool::AddDimension()
{
  DataLabel     aDimTolL;
  TDF_TagSource aTag;
  aDimTolL                       = aTag.NewChild(Label());
  Handle(XCAFDoc_Dimension) aDim = XCAFDoc_Dimension::Set(aDimTolL);
  AsciiString1   aStr = "DGT:Dimension";
  NameAttribute::Set(aDimTolL, aStr);
  return aDimTolL;
}

//=================================================================================================

DataLabel XCAFDoc_DimTolTool::AddGeomTolerance()
{
  DataLabel     aDimTolL;
  TDF_TagSource aTag;
  aDimTolL                           = aTag.NewChild(Label());
  Handle(XCAFDoc_GeomTolerance) aTol = XCAFDoc_GeomTolerance::Set(aDimTolL);
  AsciiString1       aStr = "DGT:Tolerance";
  NameAttribute::Set(aDimTolL, aStr);
  return aDimTolL;
}

//=================================================================================================

void XCAFDoc_DimTolTool::SetDimension(const DataLabel& theL, const DataLabel& theDimTolL) const
{
  DataLabel nullLab;
  SetDimension(theL, nullLab, theDimTolL);
}

//=================================================================================================

void XCAFDoc_DimTolTool::SetDimension(const DataLabel& theFirstL,
                                      const DataLabel& theSecondL,
                                      const DataLabel& theDimTolL) const
{
  TDF_LabelSequence aFirstLS, aSecondLS;
  if (!theFirstL.IsNull())
    aFirstLS.Append(theFirstL);
  if (!theSecondL.IsNull())
    aSecondLS.Append(theSecondL);
  SetDimension(aFirstLS, aSecondLS, theDimTolL);
}

//=================================================================================================

void XCAFDoc_DimTolTool::SetDimension(const TDF_LabelSequence& theFirstL,
                                      const TDF_LabelSequence& theSecondL,
                                      const DataLabel&         theDimTolL) const
{
  if (!IsDimension(theDimTolL) || theFirstL.Length() == 0)
  {
    return;
  }

  Handle(XCAFDoc_GraphNode) aChGNode;
  Handle(XCAFDoc_GraphNode) aFGNode;
  Handle(XCAFDoc_GraphNode) aSecondFGNode;

  if (theDimTolL.FindAttribute(XCAFDoc1::DimensionRefFirstGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aFGNode = aChGNode->GetFather(1);
      aFGNode->UnSetChild(aChGNode);
      if (aFGNode->NbChildren() == 0)
        aFGNode->ForgetAttribute(XCAFDoc1::DimensionRefFirstGUID());
    }
    theDimTolL.ForgetAttribute(XCAFDoc1::DimensionRefFirstGUID());
  }
  if (theDimTolL.FindAttribute(XCAFDoc1::DimensionRefSecondGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aFGNode = aChGNode->GetFather(1);
      aFGNode->UnSetChild(aChGNode);
      if (aFGNode->NbChildren() == 0)
        aFGNode->ForgetAttribute(XCAFDoc1::DimensionRefSecondGUID());
    }
    theDimTolL.ForgetAttribute(XCAFDoc1::DimensionRefSecondGUID());
  }

  if (!theDimTolL.FindAttribute(XCAFDoc1::DimensionRefFirstGUID(), aChGNode))
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theDimTolL);
    aChGNode->SetGraphID(XCAFDoc1::DimensionRefFirstGUID());
  }
  for (Standard_Integer i = theFirstL.Lower(); i <= theFirstL.Upper(); i++)
  {
    if (!theFirstL.Value(i).FindAttribute(XCAFDoc1::DimensionRefFirstGUID(), aFGNode))
    {
      aFGNode = new XCAFDoc_GraphNode;
      aFGNode = XCAFDoc_GraphNode::Set(theFirstL.Value(i));
    }
    aFGNode->SetGraphID(XCAFDoc1::DimensionRefFirstGUID());
    aFGNode->SetChild(aChGNode);
    aChGNode->SetFather(aFGNode);
  }

  if (!theDimTolL.FindAttribute(XCAFDoc1::DimensionRefSecondGUID(), aChGNode)
      && theSecondL.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theDimTolL);
    aChGNode->SetGraphID(XCAFDoc1::DimensionRefSecondGUID());
  }
  for (Standard_Integer i = theSecondL.Lower(); i <= theSecondL.Upper(); i++)
  {
    if (!theSecondL.Value(i).FindAttribute(XCAFDoc1::DimensionRefSecondGUID(), aSecondFGNode))
    {
      aSecondFGNode = new XCAFDoc_GraphNode;
      aSecondFGNode = XCAFDoc_GraphNode::Set(theSecondL.Value(i));
    }
    aSecondFGNode->SetGraphID(XCAFDoc1::DimensionRefSecondGUID());
    aSecondFGNode->SetChild(aChGNode);
    aChGNode->SetFather(aSecondFGNode);
  }
}

//=================================================================================================

void XCAFDoc_DimTolTool::SetGeomTolerance(const DataLabel& theL, const DataLabel& theGeomTolL) const
{
  TDF_LabelSequence aSeq;
  aSeq.Append(theL);
  SetGeomTolerance(aSeq, theGeomTolL);
}

//=================================================================================================

void XCAFDoc_DimTolTool::SetGeomTolerance(const TDF_LabelSequence& theL,
                                          const DataLabel&         theGeomTolL) const
{
  //  // set reference
  //  Handle(TDataStd_TreeNode) refNode, mainNode;
  //  refNode = TDataStd_TreeNode::Set ( theDimTolL, XCAFDoc1::GeomToleranceRefGUID() );
  //  mainNode  = TDataStd_TreeNode::Set ( theL,       XCAFDoc1::GeomToleranceRefGUID() );
  //  refNode->Remove(); // abv: fix against bug in TreeNode::Append()
  //  mainNode->Append(refNode);

  if (!IsGeomTolerance(theGeomTolL) || theL.Length() == 0)
  {
    return;
  }

  Handle(XCAFDoc_GraphNode) aChGNode;
  Handle(XCAFDoc_GraphNode) aFGNode;

  if (theGeomTolL.FindAttribute(XCAFDoc1::GeomToleranceRefGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aFGNode = aChGNode->GetFather(1);
      aFGNode->UnSetChild(aChGNode);
      if (aFGNode->NbChildren() == 0)
        aFGNode->ForgetAttribute(XCAFDoc1::GeomToleranceRefGUID());
    }
    theGeomTolL.ForgetAttribute(XCAFDoc1::GeomToleranceRefGUID());
  }

  if (!theGeomTolL.FindAttribute(XCAFDoc1::GeomToleranceRefGUID(), aChGNode))
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theGeomTolL);
    aChGNode->SetGraphID(XCAFDoc1::GeomToleranceRefGUID());
  }
  for (Standard_Integer i = theL.Lower(); i <= theL.Upper(); i++)
  {
    if (!theL.Value(i).FindAttribute(XCAFDoc1::GeomToleranceRefGUID(), aFGNode))
    {
      aFGNode = new XCAFDoc_GraphNode;
      aFGNode = XCAFDoc_GraphNode::Set(theL.Value(i));
    }
    aFGNode->SetGraphID(XCAFDoc1::GeomToleranceRefGUID());
    aFGNode->SetChild(aChGNode);
    aChGNode->SetFather(aFGNode);
  }
}

//=================================================================================================

void XCAFDoc_DimTolTool::SetDimTol(const DataLabel& theL, const DataLabel& theDimTolL) const
{
  // set reference
  Handle(TDataStd_TreeNode) refNode, mainNode;
  refNode  = TDataStd_TreeNode::Set(theDimTolL, XCAFDoc1::DimTolRefGUID());
  mainNode = TDataStd_TreeNode::Set(theL, XCAFDoc1::DimTolRefGUID());
  refNode->Remove(); // abv: fix against bug in TreeNode::Append()
  mainNode->Append(refNode);
}

//=================================================================================================

DataLabel XCAFDoc_DimTolTool::SetDimTol(const DataLabel&                        L,
                                        const Standard_Integer                  kind,
                                        const Handle(TColStd_HArray1OfReal)&    aVal,
                                        const Handle(TCollection_HAsciiString)& aName,
                                        const Handle(TCollection_HAsciiString)& aDescription) const
{
  DataLabel DimTolL = AddDimTol(kind, aVal, aName, aDescription);
  SetDimTol(L, DimTolL);
  return DimTolL;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::GetRefShapeLabel(const DataLabel&   theL,
                                                      TDF_LabelSequence& theShapeLFirst,
                                                      TDF_LabelSequence& theShapeLSecond)
{
  theShapeLFirst.Clear();
  theShapeLSecond.Clear();
  Handle(TDataStd_TreeNode) aNode;
  if (!theL.FindAttribute(XCAFDoc1::DimTolRefGUID(), aNode) || !aNode->HasFather())
  {
    if (!theL.FindAttribute(XCAFDoc1::DatumRefGUID(), aNode) || !aNode->HasFather())
    {
      Handle(XCAFDoc_GraphNode) aGNode;
      if (theL.FindAttribute(XCAFDoc1::GeomToleranceRefGUID(), aGNode) && aGNode->NbFathers() > 0)
      {
        for (Standard_Integer i = 1; i <= aGNode->NbFathers(); i++)
        {
          theShapeLFirst.Append(aGNode->GetFather(i)->Label());
        }
        return Standard_True;
      }
      else if (theL.FindAttribute(XCAFDoc1::DatumRefGUID(), aGNode) && aGNode->NbFathers() > 0)
      {
        for (Standard_Integer i = 1; i <= aGNode->NbFathers(); i++)
        {
          theShapeLFirst.Append(aGNode->GetFather(i)->Label());
        }
        return Standard_True;
      }
      else if (theL.FindAttribute(XCAFDoc1::DimensionRefFirstGUID(), aGNode)
               && aGNode->NbFathers() > 0)
      {
        for (Standard_Integer i = 1; i <= aGNode->NbFathers(); i++)
        {
          theShapeLFirst.Append(aGNode->GetFather(i)->Label());
        }
        if (theL.FindAttribute(XCAFDoc1::DimensionRefSecondGUID(), aGNode)
            && aGNode->NbFathers() > 0)
        {
          for (Standard_Integer i = 1; i <= aGNode->NbFathers(); i++)
          {
            theShapeLSecond.Append(aGNode->GetFather(i)->Label());
          }
        }
        return Standard_True;
      }
      else
      {
        return Standard_False;
      }
    }
  }

  theShapeLFirst.Append(aNode->Father()->Label());
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::GetRefDimensionLabels(const DataLabel&   theShapeL,
                                                           TDF_LabelSequence& theDimTols) const
{
  Handle(XCAFDoc_GraphNode) aGNode;
  Standard_Boolean          aResult = Standard_False;
  if (theShapeL.FindAttribute(XCAFDoc1::DimensionRefFirstGUID(), aGNode) && aGNode->NbChildren() > 0)
  {
    for (Standard_Integer i = 1; i <= aGNode->NbChildren(); i++)
    {
      theDimTols.Append(aGNode->GetChild(i)->Label());
    }
    aResult = Standard_True;
  }
  if (theShapeL.FindAttribute(XCAFDoc1::DimensionRefSecondGUID(), aGNode)
      && aGNode->NbChildren() > 0)
  {
    for (Standard_Integer i = 1; i <= aGNode->NbChildren(); i++)
    {
      theDimTols.Append(aGNode->GetChild(i)->Label());
    }
    aResult = Standard_True;
  }
  return aResult;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::GetRefGeomToleranceLabels(const DataLabel&   theShapeL,
                                                               TDF_LabelSequence& theDimTols) const
{
  Handle(XCAFDoc_GraphNode) aGNode;
  if (!theShapeL.FindAttribute(XCAFDoc1::GeomToleranceRefGUID(), aGNode)
      || aGNode->NbChildren() == 0)
  {
    return Standard_False;
  }
  for (Standard_Integer i = 1; i <= aGNode->NbChildren(); i++)
  {
    theDimTols.Append(aGNode->GetChild(i)->Label());
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::GetRefDatumLabel(const DataLabel&   theShapeL,
                                                      TDF_LabelSequence& theDatum) const
{
  Handle(XCAFDoc_GraphNode) aGNode;
  if (!theShapeL.FindAttribute(XCAFDoc1::DatumRefGUID(), aGNode))
  {
    return Standard_False;
  }
  for (Standard_Integer i = 1; i <= aGNode->NbChildren(); i++)
  {
    theDatum.Append(aGNode->GetChild(i)->Label());
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::GetDimTol(const DataLabel&                  DimTolL,
                                               Standard_Integer&                 kind,
                                               Handle(TColStd_HArray1OfReal)&    aVal,
                                               Handle(TCollection_HAsciiString)& aName,
                                               Handle(TCollection_HAsciiString)& aDescription) const
{
  Handle(XCAFDoc_DimTol) DimTolAttr;
  if (!DimTolL.FindAttribute(XCAFDoc_DimTol::GetID(), DimTolAttr))
  {
    return Standard_False;
  }
  kind         = DimTolAttr->GetKind();
  aVal         = DimTolAttr->GetVal();
  aName        = DimTolAttr->GetName();
  aDescription = DimTolAttr->GetDescription();

  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::IsDatum(const DataLabel& theDimTolL) const
{
  Handle(XCAFDoc_Datum) aDatumAttr;
  if (theDimTolL.FindAttribute(XCAFDoc_Datum::GetID(), aDatumAttr))
  {
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

void XCAFDoc_DimTolTool::GetDatumLabels(TDF_LabelSequence& theLabels) const
{
  theLabels.Clear();
  ChildIterator aChildIterator(Label());
  for (; aChildIterator.More(); aChildIterator.Next())
  {
    DataLabel L = aChildIterator.Value();
    if (IsDatum(L))
      theLabels.Append(L);
  }
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::FindDatum(
  const Handle(TCollection_HAsciiString)& aName,
  const Handle(TCollection_HAsciiString)& aDescription,
  const Handle(TCollection_HAsciiString)& anIdentification,
  DataLabel&                              lab) const
{
  TDF_ChildIDIterator it(Label(), XCAFDoc_Datum::GetID());
  for (; it.More(); it.Next())
  {
    Handle(TCollection_HAsciiString) aName1, aDescription1, anIdentification1;
    DataLabel                        aLabel = it.Value()->Label();
    if (!GetDatum(aLabel, aName1, aDescription1, anIdentification1))
      continue;
    if (!(aName == aName1))
      continue;
    if (!(aDescription == aDescription1))
      continue;
    if (!(anIdentification == anIdentification1))
      continue;
    lab = aLabel;
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

DataLabel XCAFDoc_DimTolTool::AddDatum(
  const Handle(TCollection_HAsciiString)& aName,
  const Handle(TCollection_HAsciiString)& aDescription,
  const Handle(TCollection_HAsciiString)& anIdentification) const
{
  DataLabel     DatumL;
  TDF_TagSource aTag;
  DatumL = aTag.NewChild(Label());
  XCAFDoc_Datum::Set(DatumL, aName, aDescription, anIdentification);
  NameAttribute::Set(DatumL, "DGT:Datum");
  return DatumL;
}

//=================================================================================================

DataLabel XCAFDoc_DimTolTool::AddDatum()
{
  DataLabel     aDatumL;
  TDF_TagSource aTag;
  aDatumL                    = aTag.NewChild(Label());
  Handle(XCAFDoc_Datum) aDat = XCAFDoc_Datum::Set(aDatumL);
  NameAttribute::Set(aDatumL, "DGT:Datum");
  return aDatumL;
}

//=================================================================================================

void XCAFDoc_DimTolTool::SetDatum(const TDF_LabelSequence& theL, const DataLabel& theDatumL) const
{
  if (!IsDatum(theDatumL))
  {
    return;
  }

  Handle(XCAFDoc_GraphNode) aChGNode;
  Handle(XCAFDoc_GraphNode) aFGNode;

  if (theDatumL.FindAttribute(XCAFDoc1::DatumRefGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aFGNode = aChGNode->GetFather(1);
      aFGNode->UnSetChild(aChGNode);
      if (aFGNode->NbChildren() == 0)
        aFGNode->ForgetAttribute(XCAFDoc1::DatumRefGUID());
    }
    theDatumL.ForgetAttribute(XCAFDoc1::DatumRefGUID());
  }

  if (!theDatumL.FindAttribute(XCAFDoc1::DatumRefGUID(), aChGNode))
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theDatumL);
    aChGNode->SetGraphID(XCAFDoc1::DatumRefGUID());
  }
  for (Standard_Integer i = theL.Lower(); i <= theL.Upper(); i++)
  {
    if (!theL.Value(i).FindAttribute(XCAFDoc1::DatumRefGUID(), aFGNode))
    {
      aFGNode = new XCAFDoc_GraphNode;
      aFGNode = XCAFDoc_GraphNode::Set(theL.Value(i));
    }
    aFGNode->SetGraphID(XCAFDoc1::DatumRefGUID());
    aFGNode->SetChild(aChGNode);
    aChGNode->SetFather(aFGNode);
  }
}

//=================================================================================================

void XCAFDoc_DimTolTool::SetDatum(const DataLabel&                        L,
                                  const DataLabel&                        TolerL,
                                  const Handle(TCollection_HAsciiString)& aName,
                                  const Handle(TCollection_HAsciiString)& aDescription,
                                  const Handle(TCollection_HAsciiString)& anIdentification) const
{
  DataLabel DatumL;
  if (!FindDatum(aName, aDescription, anIdentification, DatumL))
    DatumL = AddDatum(aName, aDescription, anIdentification);
  TDF_LabelSequence aLabels;
  aLabels.Append(L);
  SetDatum(aLabels, DatumL);
  // set reference
  Handle(XCAFDoc_GraphNode) FGNode;
  Handle(XCAFDoc_GraphNode) ChGNode;
  if (!TolerL.FindAttribute(XCAFDoc1::DatumTolRefGUID(), FGNode))
  {
    FGNode = new XCAFDoc_GraphNode;
    FGNode = XCAFDoc_GraphNode::Set(TolerL);
  }
  if (!DatumL.FindAttribute(XCAFDoc1::DatumTolRefGUID(), ChGNode))
  {
    ChGNode = new XCAFDoc_GraphNode;
    ChGNode = XCAFDoc_GraphNode::Set(DatumL);
  }
  FGNode->SetGraphID(XCAFDoc1::DatumTolRefGUID());
  ChGNode->SetGraphID(XCAFDoc1::DatumTolRefGUID());
  FGNode->SetChild(ChGNode);
  ChGNode->SetFather(FGNode);
}

//=================================================================================================

void XCAFDoc_DimTolTool::SetDatumToGeomTol(const DataLabel& theDatumL,
                                           const DataLabel& theTolerL) const
{
  // set reference
  Handle(XCAFDoc_GraphNode) aFGNode;
  Handle(XCAFDoc_GraphNode) aChGNode;
  if (!theTolerL.FindAttribute(XCAFDoc1::DatumTolRefGUID(), aFGNode))
  {
    aFGNode = new XCAFDoc_GraphNode;
    aFGNode = XCAFDoc_GraphNode::Set(theTolerL);
  }
  if (!theDatumL.FindAttribute(XCAFDoc1::DatumTolRefGUID(), aChGNode))
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theDatumL);
  }
  aFGNode->SetGraphID(XCAFDoc1::DatumTolRefGUID());
  aChGNode->SetGraphID(XCAFDoc1::DatumTolRefGUID());
  aFGNode->SetChild(aChGNode);
  aChGNode->SetFather(aFGNode);
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::GetDatum(
  const DataLabel&                  theDatumL,
  Handle(TCollection_HAsciiString)& theName,
  Handle(TCollection_HAsciiString)& theDescription,
  Handle(TCollection_HAsciiString)& theIdentification) const
{
  Handle(XCAFDoc_Datum) aDatumAttr;
  if (theDatumL.IsNull() || !theDatumL.FindAttribute(XCAFDoc_Datum::GetID(), aDatumAttr))
    return Standard_False;

  theName           = aDatumAttr->GetName();
  theDescription    = aDatumAttr->GetDescription();
  theIdentification = aDatumAttr->GetIdentification();
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::GetDatumOfTolerLabels(const DataLabel&   theDimTolL,
                                                           TDF_LabelSequence& theDatums)
{
  Handle(XCAFDoc_GraphNode) aNode;
  if (!theDimTolL.FindAttribute(XCAFDoc1::DatumTolRefGUID(), aNode))
    return Standard_False;

  for (Standard_Integer i = 1; i <= aNode->NbChildren(); i++)
  {
    Handle(XCAFDoc_GraphNode) aDatumNode = aNode->GetChild(i);
    theDatums.Append(aDatumNode->Label());
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::GetDatumWithObjectOfTolerLabels(const DataLabel&   theDimTolL,
                                                                     TDF_LabelSequence& theDatums)
{
  Handle(XCAFDoc_GraphNode) aNode;
  if (!theDimTolL.FindAttribute(XCAFDoc1::DatumTolRefGUID(), aNode))
    return Standard_False;

  TColStd_MapOfAsciiString aDatumNameMap;
  for (Standard_Integer i = 1; i <= aNode->NbChildren(); i++)
  {
    Handle(XCAFDoc_GraphNode) aDatumNode = aNode->GetChild(i);
    DataLabel                 aDatumL    = aDatumNode->Label();
    Handle(XCAFDoc_Datum)     aDatumAttr;
    if (!aDatumL.FindAttribute(XCAFDoc_Datum::GetID(), aDatumAttr))
      continue;
    Handle(XCAFDimTolObjects_DatumObject) aDatumObj = aDatumAttr->GetObject();
    if (aDatumObj.IsNull())
      continue;
    Handle(TCollection_HAsciiString) aName = aDatumObj->GetName();
    if (!aDatumNameMap.Add(aName->String()))
    {
      // the datum has already been appended to sequence, due to one of its datum targets
      continue;
    }
    theDatums.Append(aDatumNode->Label());
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::GetTolerOfDatumLabels(const DataLabel&   theDatumL,
                                                           TDF_LabelSequence& theTols) const
{
  Handle(XCAFDoc_GraphNode) aNode;
  if (!theDatumL.FindAttribute(XCAFDoc1::DatumTolRefGUID(), aNode))
    return Standard_False;
  for (Standard_Integer i = 1; i <= aNode->NbFathers(); i++)
  {
    Handle(XCAFDoc_GraphNode) aDatumNode = aNode->GetFather(i);
    theTols.Append(aDatumNode->Label());
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_DimTolTool::IsLocked(const DataLabel& theViewL) const
{
  Handle(TDataStd_UAttribute) anAttr;
  return theViewL.FindAttribute(XCAFDoc1::LockGUID(), anAttr);
}

//=================================================================================================

void XCAFDoc_DimTolTool::Lock(const DataLabel& theViewL) const
{
  TDataStd_UAttribute::Set(theViewL, XCAFDoc1::LockGUID());
}

//=================================================================================================

void XCAFDoc_DimTolTool::Unlock(const DataLabel& theViewL) const
{
  theViewL.ForgetAttribute(XCAFDoc1::LockGUID());
}

//=================================================================================================

const Standard_GUID& XCAFDoc_DimTolTool::ID() const
{
  return GetID();
}

//=================================================================================================

void XCAFDoc_DimTolTool::GetGDTPresentations(
  NCollection_IndexedDataMap<DataLabel, TopoShape>& theGDTLabelToShape) const
{
  TDF_LabelSequence aGDTs;
  GetDimensionLabels(aGDTs);
  for (Standard_Integer i = 1; i <= aGDTs.Length(); i++)
  {
    Handle(XCAFDoc_Dimension) aDimAttr;
    const DataLabel&          aCL = aGDTs.Value(i);
    if (!aCL.FindAttribute(XCAFDoc_Dimension::GetID(), aDimAttr))
      continue;
    Handle(XCAFDimTolObjects_DimensionObject) anObject = aDimAttr->GetObject();
    if (anObject.IsNull())
      continue;
    TopoShape aShape = anObject->GetPresentation();
    if (!aShape.IsNull())
      theGDTLabelToShape.Add(aCL, aShape);
  }

  aGDTs.Clear();
  GetGeomToleranceLabels(aGDTs);
  for (Standard_Integer i = 1; i <= aGDTs.Length(); i++)
  {
    Handle(XCAFDoc_GeomTolerance) aGTAttr;
    const DataLabel&              aCL = aGDTs.Value(i);
    if (!aCL.FindAttribute(XCAFDoc_GeomTolerance::GetID(), aGTAttr))
      continue;
    Handle(XCAFDimTolObjects_GeomToleranceObject) anObject = aGTAttr->GetObject();
    if (anObject.IsNull())
      continue;
    TopoShape aShape = anObject->GetPresentation();
    if (!aShape.IsNull())
      theGDTLabelToShape.Add(aCL, aShape);
  }

  aGDTs.Clear();
  GetDatumLabels(aGDTs);
  for (Standard_Integer i = 1; i <= aGDTs.Length(); i++)
  {
    Handle(XCAFDoc_Datum) aGTAttr;
    const DataLabel&      aCL = aGDTs.Value(i);
    if (!aCL.FindAttribute(XCAFDoc_Datum::GetID(), aGTAttr))
      continue;
    Handle(XCAFDimTolObjects_DatumObject) anObject = aGTAttr->GetObject();
    if (anObject.IsNull())
      continue;
    TopoShape aShape = anObject->GetPresentation();
    if (!aShape.IsNull())
      theGDTLabelToShape.Add(aCL, aShape);
  }
}

//=================================================================================================

void XCAFDoc_DimTolTool::SetGDTPresentations(
  NCollection_IndexedDataMap<DataLabel, TopoShape>& theGDTLabelToPrs)
{
  for (Standard_Integer i = 1; i <= theGDTLabelToPrs.Extent(); i++)
  {
    const DataLabel&          aCL = theGDTLabelToPrs.FindKey(i);
    Handle(XCAFDoc_Dimension) aDimAttrDim;
    if (aCL.FindAttribute(XCAFDoc_Dimension::GetID(), aDimAttrDim))
    {
      Handle(XCAFDimTolObjects_DimensionObject) anObject = aDimAttrDim->GetObject();
      if (anObject.IsNull())
        continue;
      const TopoShape& aPrs = theGDTLabelToPrs.FindFromIndex(i);
      anObject->SetPresentation(aPrs, anObject->GetPresentationName());
      aDimAttrDim->SetObject(anObject);
      continue;
    }
    Handle(XCAFDoc_GeomTolerance) aDimAttrG;
    if (aCL.FindAttribute(XCAFDoc_GeomTolerance::GetID(), aDimAttrG))
    {
      Handle(XCAFDimTolObjects_GeomToleranceObject) anObject = aDimAttrG->GetObject();
      if (anObject.IsNull())
        continue;
      const TopoShape& aPrs = theGDTLabelToPrs.FindFromIndex(i);
      anObject->SetPresentation(aPrs, anObject->GetPresentationName());
      aDimAttrG->SetObject(anObject);
      continue;
    }
    Handle(XCAFDoc_Datum) aDimAttrD;
    if (aCL.FindAttribute(XCAFDoc_Datum::GetID(), aDimAttrD))
    {
      Handle(XCAFDimTolObjects_DatumObject) anObject = aDimAttrD->GetObject();
      if (anObject.IsNull())
        continue;
      const TopoShape& aPrs = theGDTLabelToPrs.FindFromIndex(i);
      anObject->SetPresentation(aPrs, anObject->GetPresentationName());
      aDimAttrD->SetObject(anObject);
      continue;
    }
  }
}

//=================================================================================================

void XCAFDoc_DimTolTool::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TDF_Attribute)

  TDF_LabelSequence aLabels;
  GetDimensionLabels(aLabels);
  for (TDF_LabelSequence::Iterator aDimLabelIt(aLabels); aDimLabelIt.More(); aDimLabelIt.Next())
  {
    AsciiString1 aDimensionLabel;
    Tool3::Entry(aDimLabelIt.Value(), aDimensionLabel);
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aDimensionLabel)
  }

  aLabels.Clear();
  GetGeomToleranceLabels(aLabels);
  for (TDF_LabelSequence::Iterator aGeomToleranceLabelIt(aLabels); aGeomToleranceLabelIt.More();
       aGeomToleranceLabelIt.Next())
  {
    AsciiString1 aGeomToleranceLabel;
    Tool3::Entry(aGeomToleranceLabelIt.Value(), aGeomToleranceLabel);
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aGeomToleranceLabel)
  }

  aLabels.Clear();
  GetDimTolLabels(aLabels);
  for (TDF_LabelSequence::Iterator aDimTolLabelIt(aLabels); aDimTolLabelIt.More();
       aDimTolLabelIt.Next())
  {
    AsciiString1 aDimTolLabelLabel;
    Tool3::Entry(aDimTolLabelIt.Value(), aDimTolLabelLabel);
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aDimTolLabelLabel)
  }

  aLabels.Clear();
  GetDatumLabels(aLabels);
  for (TDF_LabelSequence::Iterator aDatumLabelIt(aLabels); aDatumLabelIt.More();
       aDatumLabelIt.Next())
  {
    AsciiString1 aDatumLabel;
    Tool3::Entry(aDatumLabelIt.Value(), aDatumLabel);
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aDatumLabel)
  }
}
