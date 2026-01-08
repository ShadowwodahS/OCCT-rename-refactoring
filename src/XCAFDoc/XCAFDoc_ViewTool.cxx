// Created on: 2016-10-19
// Created by: Irina KRYLOVA
// Copyright (c) 2016 OPEN CASCADE SAS
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

#include <XCAFDoc_ViewTool.hxx>

#include <Standard_Type.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_ChildIDIterator.hxx>
#include <TDF_Label.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_View.hxx>

IMPLEMENT_DERIVED_ATTRIBUTE_WITH_TYPE(XCAFDoc_ViewTool, TDataStd_GenericEmpty, "xcaf", "ViewTool")

//=================================================================================================

XCAFDoc_ViewTool::XCAFDoc_ViewTool() {}

//=================================================================================================

Handle(XCAFDoc_ViewTool) XCAFDoc_ViewTool::Set(const DataLabel& L)
{
  Handle(XCAFDoc_ViewTool) A;
  if (!L.FindAttribute(XCAFDoc_ViewTool::GetID(), A))
  {
    A = new XCAFDoc_ViewTool();
    L.AddAttribute(A);
  }
  return A;
}

//=================================================================================================

const Standard_GUID& XCAFDoc_ViewTool::GetID()
{
  static Standard_GUID ViewToolID("efd213e4-6dfd-11d4-b9c8-0060b0ee281b");
  return ViewToolID;
}

//=================================================================================================

DataLabel XCAFDoc_ViewTool::BaseLabel() const
{
  return Label();
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::IsView(const DataLabel& theLabel) const
{
  Handle(XCAFDoc_View) aViewAttr;
  if (theLabel.FindAttribute(XCAFDoc_View::GetID(), aViewAttr))
  {
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

void XCAFDoc_ViewTool::GetViewLabels(TDF_LabelSequence& theLabels) const
{
  theLabels.Clear();
  ChildIterator aChildIterator(Label());
  for (; aChildIterator.More(); aChildIterator.Next())
  {
    DataLabel aLabel = aChildIterator.Value();
    if (IsView(aLabel))
      theLabels.Append(aLabel);
  }
}

//=================================================================================================

DataLabel XCAFDoc_ViewTool::AddView()
{
  DataLabel     aViewL;
  TDF_TagSource aTag;
  aViewL                        = aTag.NewChild(Label());
  Handle(XCAFDoc_View)    aView = XCAFDoc_View::Set(aViewL);
  AsciiString1 aStr  = "View";
  NameAttribute::Set(aViewL, aStr);
  return aViewL;
}

//=================================================================================================

void XCAFDoc_ViewTool::SetView(const TDF_LabelSequence& theShapes,
                               const TDF_LabelSequence& theGDTs,
                               const TDF_LabelSequence& theClippingPlanes,
                               const TDF_LabelSequence& theNotes,
                               const TDF_LabelSequence& theAnnotations,
                               const DataLabel&         theViewL) const
{
  if (!IsView(theViewL))
    return;

  Handle(XCAFDoc_GraphNode) aChGNode;
  Handle(XCAFDoc_GraphNode) aShapeGNode;
  Handle(XCAFDoc_GraphNode) aGDTGNode;
  Handle(XCAFDoc_GraphNode) aPlaneGNode;
  Handle(XCAFDoc_GraphNode) aNoteGNode;
  Handle(XCAFDoc_GraphNode) aAnnotGNode;

  if (theViewL.FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aShapeGNode = aChGNode->GetFather(1);
      aShapeGNode->UnSetChild(aChGNode);
      if (aShapeGNode->NbChildren() == 0)
        aShapeGNode->ForgetAttribute(XCAFDoc1::ViewRefShapeGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefShapeGUID());
  }
  if (theViewL.FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aGDTGNode = aChGNode->GetFather(1);
      aGDTGNode->UnSetChild(aChGNode);
      if (aGDTGNode->NbChildren() == 0)
        aGDTGNode->ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
  }
  if (theViewL.FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aPlaneGNode = aChGNode->GetFather(1);
      aPlaneGNode->UnSetChild(aChGNode);
      if (aPlaneGNode->NbChildren() == 0)
        aPlaneGNode->ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefPlaneGUID());
  }
  if (theViewL.FindAttribute(XCAFDoc1::ViewRefNoteGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aPlaneGNode = aChGNode->GetFather(1);
      aPlaneGNode->UnSetChild(aChGNode);
      if (aPlaneGNode->NbChildren() == 0)
        aPlaneGNode->ForgetAttribute(XCAFDoc1::ViewRefNoteGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefNoteGUID());
  }
  if (theViewL.FindAttribute(XCAFDoc1::ViewRefAnnotationGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aPlaneGNode = aChGNode->GetFather(1);
      aPlaneGNode->UnSetChild(aChGNode);
      if (aPlaneGNode->NbChildren() == 0)
        aPlaneGNode->ForgetAttribute(XCAFDoc1::ViewRefAnnotationGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefAnnotationGUID());
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aChGNode) && theShapes.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefShapeGUID());
  }
  for (Standard_Integer i = theShapes.Lower(); i <= theShapes.Upper(); i++)
  {
    if (!theShapes.Value(i).FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aShapeGNode))
    {
      aShapeGNode = new XCAFDoc_GraphNode;
      aShapeGNode = XCAFDoc_GraphNode::Set(theShapes.Value(i));
    }
    aShapeGNode->SetGraphID(XCAFDoc1::ViewRefShapeGUID());
    aShapeGNode->SetChild(aChGNode);
    aChGNode->SetFather(aShapeGNode);
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aChGNode) && theGDTs.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefGDTGUID());
  }
  for (Standard_Integer i = theGDTs.Lower(); i <= theGDTs.Upper(); i++)
  {
    if (!theGDTs.Value(i).FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aGDTGNode))
    {
      aGDTGNode = new XCAFDoc_GraphNode;
      aGDTGNode = XCAFDoc_GraphNode::Set(theGDTs.Value(i));
    }
    aGDTGNode->SetGraphID(XCAFDoc1::ViewRefGDTGUID());
    aGDTGNode->SetChild(aChGNode);
    aChGNode->SetFather(aGDTGNode);
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aChGNode)
      && theClippingPlanes.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefPlaneGUID());
  }
  for (Standard_Integer i = theClippingPlanes.Lower(); i <= theClippingPlanes.Upper(); i++)
  {
    if (!theClippingPlanes.Value(i).FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aPlaneGNode))
    {
      aPlaneGNode = new XCAFDoc_GraphNode;
      aPlaneGNode = XCAFDoc_GraphNode::Set(theClippingPlanes.Value(i));
    }
    aPlaneGNode->SetGraphID(XCAFDoc1::ViewRefPlaneGUID());
    aPlaneGNode->SetChild(aChGNode);
    aChGNode->SetFather(aPlaneGNode);
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefNoteGUID(), aChGNode) && theNotes.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefPlaneGUID());
  }
  for (Standard_Integer i = theNotes.Lower(); i <= theNotes.Upper(); i++)
  {
    if (!theNotes.Value(i).FindAttribute(XCAFDoc1::ViewRefNoteGUID(), aNoteGNode))
    {
      aNoteGNode = new XCAFDoc_GraphNode;
      aNoteGNode = XCAFDoc_GraphNode::Set(theNotes.Value(i));
    }
    aNoteGNode->SetGraphID(XCAFDoc1::ViewRefNoteGUID());
    aNoteGNode->SetChild(aChGNode);
    aChGNode->SetFather(aNoteGNode);
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefAnnotationGUID(), aChGNode)
      && theAnnotations.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefAnnotationGUID());
  }
  for (Standard_Integer i = theAnnotations.Lower(); i <= theAnnotations.Upper(); i++)
  {
    if (!theAnnotations.Value(i).FindAttribute(XCAFDoc1::ViewRefAnnotationGUID(), aNoteGNode))
    {
      aAnnotGNode = new XCAFDoc_GraphNode;
      aAnnotGNode = XCAFDoc_GraphNode::Set(theNotes.Value(i));
    }
    aAnnotGNode->SetGraphID(XCAFDoc1::ViewRefAnnotationGUID());
    aAnnotGNode->SetChild(aChGNode);
    aChGNode->SetFather(aAnnotGNode);
  }
}

//=================================================================================================

void XCAFDoc_ViewTool::SetView(const TDF_LabelSequence& theShapeLabels,
                               const TDF_LabelSequence& theGDTLabels,
                               const TDF_LabelSequence& theClippingPlaneLabels,
                               const DataLabel&         theViewL) const
{
  if (!IsView(theViewL))
    return;

  Handle(XCAFDoc_GraphNode) aChGNode;
  Handle(XCAFDoc_GraphNode) aShapeGNode;
  Handle(XCAFDoc_GraphNode) aGDTGNode;
  Handle(XCAFDoc_GraphNode) aPlaneGNode;

  if (theViewL.FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aShapeGNode = aChGNode->GetFather(1);
      aShapeGNode->UnSetChild(aChGNode);
      if (aShapeGNode->NbChildren() == 0)
        aShapeGNode->ForgetAttribute(XCAFDoc1::ViewRefShapeGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefShapeGUID());
  }
  if (theViewL.FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aGDTGNode = aChGNode->GetFather(1);
      aGDTGNode->UnSetChild(aChGNode);
      if (aGDTGNode->NbChildren() == 0)
        aGDTGNode->ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
  }
  if (theViewL.FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aPlaneGNode = aChGNode->GetFather(1);
      aPlaneGNode->UnSetChild(aChGNode);
      if (aPlaneGNode->NbChildren() == 0)
        aPlaneGNode->ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefPlaneGUID());
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aChGNode) && theShapeLabels.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefShapeGUID());
  }
  for (Standard_Integer i = theShapeLabels.Lower(); i <= theShapeLabels.Upper(); i++)
  {
    if (!theShapeLabels.Value(i).FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aShapeGNode))
    {
      aShapeGNode = new XCAFDoc_GraphNode;
      aShapeGNode = XCAFDoc_GraphNode::Set(theShapeLabels.Value(i));
    }
    aShapeGNode->SetGraphID(XCAFDoc1::ViewRefShapeGUID());
    aShapeGNode->SetChild(aChGNode);
    aChGNode->SetFather(aShapeGNode);
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aChGNode) && theGDTLabels.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefGDTGUID());
  }
  for (Standard_Integer i = theGDTLabels.Lower(); i <= theGDTLabels.Upper(); i++)
  {
    if (!theGDTLabels.Value(i).FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aGDTGNode))
    {
      aGDTGNode = new XCAFDoc_GraphNode;
      aGDTGNode = XCAFDoc_GraphNode::Set(theGDTLabels.Value(i));
    }
    aGDTGNode->SetGraphID(XCAFDoc1::ViewRefGDTGUID());
    aGDTGNode->SetChild(aChGNode);
    aChGNode->SetFather(aGDTGNode);
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aChGNode)
      && theClippingPlaneLabels.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefPlaneGUID());
  }
  for (Standard_Integer i = theClippingPlaneLabels.Lower(); i <= theClippingPlaneLabels.Upper();
       i++)
  {
    if (!theClippingPlaneLabels.Value(i).FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aPlaneGNode))
    {
      aPlaneGNode = new XCAFDoc_GraphNode;
      aPlaneGNode = XCAFDoc_GraphNode::Set(theClippingPlaneLabels.Value(i));
    }
    aPlaneGNode->SetGraphID(XCAFDoc1::ViewRefPlaneGUID());
    aPlaneGNode->SetChild(aChGNode);
    aChGNode->SetFather(aPlaneGNode);
  }
}

//=================================================================================================

void XCAFDoc_ViewTool::SetView(const TDF_LabelSequence& theShapeLabels,
                               const TDF_LabelSequence& theGDTLabels,
                               const DataLabel&         theViewL) const
{
  if (!IsView(theViewL))
    return;

  Handle(XCAFDoc_GraphNode) aChGNode;
  Handle(XCAFDoc_GraphNode) aShapeGNode;
  Handle(XCAFDoc_GraphNode) aGDTGNode;

  if (theViewL.FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aShapeGNode = aChGNode->GetFather(1);
      aShapeGNode->UnSetChild(aChGNode);
      if (aShapeGNode->NbChildren() == 0)
        aShapeGNode->ForgetAttribute(XCAFDoc1::ViewRefShapeGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefShapeGUID());
  }
  if (theViewL.FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aGDTGNode = aChGNode->GetFather(1);
      aGDTGNode->UnSetChild(aChGNode);
      if (aGDTGNode->NbChildren() == 0)
        aGDTGNode->ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aChGNode) && theShapeLabels.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefShapeGUID());
  }
  for (Standard_Integer i = theShapeLabels.Lower(); i <= theShapeLabels.Upper(); i++)
  {
    if (!theShapeLabels.Value(i).FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aShapeGNode))
    {
      aShapeGNode = new XCAFDoc_GraphNode;
      aShapeGNode = XCAFDoc_GraphNode::Set(theShapeLabels.Value(i));
    }
    aShapeGNode->SetGraphID(XCAFDoc1::ViewRefShapeGUID());
    aShapeGNode->SetChild(aChGNode);
    aChGNode->SetFather(aShapeGNode);
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aChGNode) && theGDTLabels.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefGDTGUID());
  }
  for (Standard_Integer i = theGDTLabels.Lower(); i <= theGDTLabels.Upper(); i++)
  {
    if (!theGDTLabels.Value(i).FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aGDTGNode))
    {
      aGDTGNode = new XCAFDoc_GraphNode;
      aGDTGNode = XCAFDoc_GraphNode::Set(theGDTLabels.Value(i));
    }
    aGDTGNode->SetGraphID(XCAFDoc1::ViewRefGDTGUID());
    aGDTGNode->SetChild(aChGNode);
    aChGNode->SetFather(aGDTGNode);
  }
}

//=================================================================================================

void XCAFDoc_ViewTool::SetClippingPlanes(const TDF_LabelSequence& theClippingPlaneLabels,
                                         const DataLabel&         theViewL) const
{
  if (!IsView(theViewL))
    return;

  Handle(XCAFDoc_GraphNode) aChGNode;
  Handle(XCAFDoc_GraphNode) aPlaneGNode;

  if (theViewL.FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aPlaneGNode = aChGNode->GetFather(1);
      aPlaneGNode->UnSetChild(aChGNode);
      if (aPlaneGNode->NbChildren() == 0)
        aPlaneGNode->ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
    }
    theViewL.ForgetAttribute(XCAFDoc1::ViewRefPlaneGUID());
  }

  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aChGNode)
      && theClippingPlaneLabels.Length() > 0)
  {
    aChGNode = new XCAFDoc_GraphNode;
    aChGNode = XCAFDoc_GraphNode::Set(theViewL);
    aChGNode->SetGraphID(XCAFDoc1::ViewRefPlaneGUID());
  }
  for (Standard_Integer i = theClippingPlaneLabels.Lower(); i <= theClippingPlaneLabels.Upper();
       i++)
  {
    if (!theClippingPlaneLabels.Value(i).FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aPlaneGNode))
    {
      aPlaneGNode = new XCAFDoc_GraphNode;
      aPlaneGNode = XCAFDoc_GraphNode::Set(theClippingPlaneLabels.Value(i));
    }
    aPlaneGNode->SetGraphID(XCAFDoc1::ViewRefPlaneGUID());
    aPlaneGNode->SetChild(aChGNode);
    aChGNode->SetFather(aPlaneGNode);
  }
}

//=================================================================================================

void XCAFDoc_ViewTool::RemoveView(const DataLabel& theViewL)
{
  if (!IsView(theViewL))
    return;

  Handle(XCAFDoc_GraphNode) aChGNode;
  Handle(XCAFDoc_GraphNode) aShapeGNode;
  Handle(XCAFDoc_GraphNode) aGDTGNode;
  Handle(XCAFDoc_GraphNode) aPlaneGNode;

  if (theViewL.FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aShapeGNode = aChGNode->GetFather(1);
      aShapeGNode->UnSetChild(aChGNode);
      if (aShapeGNode->NbChildren() == 0)
        aShapeGNode->ForgetAttribute(XCAFDoc1::ViewRefShapeGUID());
    }
  }
  if (theViewL.FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aGDTGNode = aChGNode->GetFather(1);
      aGDTGNode->UnSetChild(aChGNode);
      if (aGDTGNode->NbChildren() == 0)
        aGDTGNode->ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
    }
  }
  if (theViewL.FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aChGNode))
  {
    while (aChGNode->NbFathers() > 0)
    {
      aPlaneGNode = aChGNode->GetFather(1);
      aPlaneGNode->UnSetChild(aChGNode);
      if (aPlaneGNode->NbChildren() == 0)
        aPlaneGNode->ForgetAttribute(XCAFDoc1::ViewRefGDTGUID());
    }
  }
  theViewL.ForgetAllAttributes();
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::GetRefShapeLabel(const DataLabel&   theViewL,
                                                    TDF_LabelSequence& theShapeLabels) const
{
  theShapeLabels.Clear();
  Handle(TDataStd_TreeNode) aNode;
  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefGUID(), aNode) || !aNode->HasFather())
  {
    Handle(XCAFDoc_GraphNode) aGNode;
    if (theViewL.FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aGNode) && aGNode->NbFathers() > 0)
    {
      for (Standard_Integer i = 1; i <= aGNode->NbFathers(); i++)
        theShapeLabels.Append(aGNode->GetFather(i)->Label());
      return Standard_True;
    }
    else
      return Standard_False;
  }

  theShapeLabels.Append(aNode->Father()->Label());
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::GetRefGDTLabel(const DataLabel&   theViewL,
                                                  TDF_LabelSequence& theGDTLabels) const
{
  theGDTLabels.Clear();
  Handle(TDataStd_TreeNode) aNode;
  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefGUID(), aNode) || !aNode->HasFather())
  {
    Handle(XCAFDoc_GraphNode) aGNode;
    if (theViewL.FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aGNode) && aGNode->NbFathers() > 0)
    {
      for (Standard_Integer i = 1; i <= aGNode->NbFathers(); i++)
        theGDTLabels.Append(aGNode->GetFather(i)->Label());
      return Standard_True;
    }
    else
      return Standard_False;
  }

  theGDTLabels.Append(aNode->Father()->Label());
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::GetRefClippingPlaneLabel(
  const DataLabel&   theViewL,
  TDF_LabelSequence& theClippingPlaneLabels) const
{
  theClippingPlaneLabels.Clear();
  Handle(TDataStd_TreeNode) aNode;
  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefGUID(), aNode) || !aNode->HasFather())
  {
    Handle(XCAFDoc_GraphNode) aGNode;
    if (theViewL.FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aGNode) && aGNode->NbFathers() > 0)
    {
      for (Standard_Integer i = 1; i <= aGNode->NbFathers(); i++)
        theClippingPlaneLabels.Append(aGNode->GetFather(i)->Label());
      return Standard_True;
    }
    else
      return Standard_False;
  }

  theClippingPlaneLabels.Append(aNode->Father()->Label());
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::GetRefNoteLabel(const DataLabel&   theViewL,
                                                   TDF_LabelSequence& theNoteLabels) const
{
  theNoteLabels.Clear();
  Handle(TDataStd_TreeNode) aNode;
  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefGUID(), aNode) || !aNode->HasFather())
  {
    Handle(XCAFDoc_GraphNode) aGNode;
    if (theViewL.FindAttribute(XCAFDoc1::ViewRefNoteGUID(), aGNode) && aGNode->NbFathers() > 0)
    {
      for (Standard_Integer i = 1; i <= aGNode->NbFathers(); i++)
        theNoteLabels.Append(aGNode->GetFather(i)->Label());
      return Standard_True;
    }
    else
      return Standard_False;
  }

  theNoteLabels.Append(aNode->Father()->Label());
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::GetRefAnnotationLabel(
  const DataLabel&   theViewL,
  TDF_LabelSequence& theAnnotationLabels) const
{
  theAnnotationLabels.Clear();
  Handle(TDataStd_TreeNode) aNode;
  if (!theViewL.FindAttribute(XCAFDoc1::ViewRefGUID(), aNode) || !aNode->HasFather())
  {
    Handle(XCAFDoc_GraphNode) aGNode;
    if (theViewL.FindAttribute(XCAFDoc1::ViewRefAnnotationGUID(), aGNode) && aGNode->NbFathers() > 0)
    {
      for (Standard_Integer i = 1; i <= aGNode->NbFathers(); i++)
        theAnnotationLabels.Append(aGNode->GetFather(i)->Label());
      return Standard_True;
    }
    else
      return Standard_False;
  }

  theAnnotationLabels.Append(aNode->Father()->Label());
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::GetViewLabelsForShape(const DataLabel&   theShapeL,
                                                         TDF_LabelSequence& theViews) const
{
  Handle(XCAFDoc_GraphNode) aGNode;
  Standard_Boolean          aResult = Standard_False;
  if (theShapeL.FindAttribute(XCAFDoc1::ViewRefShapeGUID(), aGNode) && aGNode->NbChildren() > 0)
  {
    for (Standard_Integer i = 1; i <= aGNode->NbChildren(); i++)
    {
      theViews.Append(aGNode->GetChild(i)->Label());
    }
    aResult = Standard_True;
  }
  return aResult;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::GetViewLabelsForGDT(const DataLabel&   theGDTL,
                                                       TDF_LabelSequence& theViews) const
{
  Handle(XCAFDoc_GraphNode) aGNode;
  Standard_Boolean          aResult = Standard_False;
  if (theGDTL.FindAttribute(XCAFDoc1::ViewRefGDTGUID(), aGNode) && aGNode->NbChildren() > 0)
  {
    for (Standard_Integer i = 1; i <= aGNode->NbChildren(); i++)
    {
      theViews.Append(aGNode->GetChild(i)->Label());
    }
    aResult = Standard_True;
  }
  return aResult;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::GetViewLabelsForClippingPlane(const DataLabel& theClippingPlaneL,
                                                                 TDF_LabelSequence& theViews) const
{
  Handle(XCAFDoc_GraphNode) aGNode;
  Standard_Boolean          aResult = Standard_False;
  if (theClippingPlaneL.FindAttribute(XCAFDoc1::ViewRefPlaneGUID(), aGNode)
      && aGNode->NbChildren() > 0)
  {
    for (Standard_Integer i = 1; i <= aGNode->NbChildren(); i++)
    {
      theViews.Append(aGNode->GetChild(i)->Label());
    }
    aResult = Standard_True;
  }
  return aResult;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::GetViewLabelsForNote(const DataLabel&   theNoteL,
                                                        TDF_LabelSequence& theViews) const
{
  Handle(XCAFDoc_GraphNode) aGNode;
  Standard_Boolean          aResult = Standard_False;
  if (theNoteL.FindAttribute(XCAFDoc1::ViewRefNoteGUID(), aGNode) && aGNode->NbChildren() > 0)
  {
    for (Standard_Integer i = 1; i <= aGNode->NbChildren(); i++)
    {
      theViews.Append(aGNode->GetChild(i)->Label());
    }
    aResult = Standard_True;
  }
  return aResult;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::GetViewLabelsForAnnotation(const DataLabel&   theAnnotationL,
                                                              TDF_LabelSequence& theViews) const
{
  Handle(XCAFDoc_GraphNode) aGNode;
  Standard_Boolean          aResult = Standard_False;
  if (theAnnotationL.FindAttribute(XCAFDoc1::ViewRefAnnotationGUID(), aGNode)
      && aGNode->NbChildren() > 0)
  {
    for (Standard_Integer i = 1; i <= aGNode->NbChildren(); i++)
    {
      theViews.Append(aGNode->GetChild(i)->Label());
    }
    aResult = Standard_True;
  }
  return aResult;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ViewTool::IsLocked(const DataLabel& theViewL) const
{
  Handle(TDataStd_UAttribute) anAttr;
  return theViewL.FindAttribute(XCAFDoc1::LockGUID(), anAttr);
}

//=================================================================================================

void XCAFDoc_ViewTool::Lock(const DataLabel& theViewL) const
{
  TDataStd_UAttribute::Set(theViewL, XCAFDoc1::LockGUID());
}

//=================================================================================================

void XCAFDoc_ViewTool::Unlock(const DataLabel& theViewL) const
{
  theViewL.ForgetAttribute(XCAFDoc1::LockGUID());
}

//=================================================================================================

const Standard_GUID& XCAFDoc_ViewTool::ID() const
{
  return GetID();
}
