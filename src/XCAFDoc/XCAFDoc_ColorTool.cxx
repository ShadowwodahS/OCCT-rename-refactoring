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

#include <XCAFDoc_ColorTool.hxx>

#include <Standard_Type.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_ChildIDIterator.hxx>
#include <TDF_Label.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDF_Tool.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_Color.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <XCAFDoc_ShapeTool.hxx>

IMPLEMENT_DERIVED_ATTRIBUTE_WITH_TYPE(XCAFDoc_ColorTool, TDataStd_GenericEmpty, "xcaf", "ColorTool")

static Standard_Boolean XCAFDoc_ColorTool_AutoNaming = Standard_True;

//=================================================================================================

void XCAFDoc_ColorTool::SetAutoNaming(Standard_Boolean theIsAutoNaming)
{
  XCAFDoc_ColorTool_AutoNaming = theIsAutoNaming;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::AutoNaming()
{
  return XCAFDoc_ColorTool_AutoNaming;
}

//=================================================================================================

DataLabel XCAFDoc_ColorTool::BaseLabel() const
{
  return Label();
}

//=================================================================================================

const Handle(XCAFDoc_ShapeTool)& XCAFDoc_ColorTool::ShapeTool()
{
  if (myShapeTool.IsNull())
    myShapeTool = XCAFDoc_DocumentTool::ShapeTool(Label());
  return myShapeTool;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::IsColor(const DataLabel& lab) const
{
  Quantity_Color C;
  return GetColor(lab, C);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::GetColor(const DataLabel& lab, Quantity_Color& col)
{
  Quantity_ColorRGBA aCol;
  Standard_Boolean   isDone = GetColor(lab, aCol);
  if (isDone)
    col = aCol.GetRGB();
  return isDone;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::GetColor(const DataLabel& lab, Quantity_ColorRGBA& col)
{
  Handle(XCAFDoc_Color) ColorAttribute;
  if (!lab.FindAttribute(XCAFDoc_Color::GetID(), ColorAttribute))
    return Standard_False;

  col = ColorAttribute->GetColorRGBA();

  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::FindColor(const Quantity_Color& col, DataLabel& lab) const
{
  Quantity_ColorRGBA aCol;
  aCol.SetRGB(col);
  return FindColor(aCol, lab);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::FindColor(const Quantity_ColorRGBA& col, DataLabel& lab) const
{
  TDF_ChildIDIterator it(Label(), XCAFDoc_Color::GetID());
  for (; it.More(); it.Next())
  {
    DataLabel          aLabel = it.Value()->Label();
    Quantity_ColorRGBA C;
    if (!GetColor(aLabel, C))
      continue;
    if (C.IsEqual(col))
    {
      lab = aLabel;
      return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

DataLabel XCAFDoc_ColorTool::FindColor(const Quantity_ColorRGBA& col) const
{
  DataLabel L;
  FindColor(col, L);
  return L;
}

//=================================================================================================

DataLabel XCAFDoc_ColorTool::FindColor(const Quantity_Color& col) const
{
  DataLabel L;
  FindColor(col, L);
  return L;
}

//=================================================================================================

DataLabel XCAFDoc_ColorTool::AddColor(const Quantity_Color& col) const
{
  Quantity_ColorRGBA aCol;
  aCol.SetRGB(col);
  return AddColor(aCol);
}

//=================================================================================================

DataLabel XCAFDoc_ColorTool::AddColor(const Quantity_ColorRGBA& theColor) const
{
  DataLabel aLab;
  if (FindColor(theColor, aLab))
  {
    return aLab;
  }

  // create a new color entry
  TDF_TagSource aTag;
  aLab = aTag.NewChild(Label());
  XCAFDoc_Color::Set(aLab, theColor);

  if (XCAFDoc_ColorTool_AutoNaming)
  {
    // set name according to color value
    const AsciiString1 aName =
      AsciiString1(Quantity_Color::StringName(theColor.GetRGB().Name())) + " ("
      + Quantity_ColorRGBA::ColorToHex(theColor) + ")";
    NameAttribute::Set(aLab, aName);
  }

  return aLab;
}

//=================================================================================================

void XCAFDoc_ColorTool::RemoveColor(const DataLabel& lab) const
{
  lab.ForgetAllAttributes(Standard_True);
}

//=================================================================================================

void XCAFDoc_ColorTool::GetColors(TDF_LabelSequence& Labels) const
{
  Labels.Clear();

  TDF_ChildIDIterator ChildIDIterator(Label(), XCAFDoc_Color::GetID());
  for (; ChildIDIterator.More(); ChildIDIterator.Next())
  {
    DataLabel L = ChildIDIterator.Value()->Label();
    if (IsColor(L))
      Labels.Append(L);
  }
}

//=================================================================================================

void XCAFDoc_ColorTool::SetColor(const DataLabel&        L,
                                 const DataLabel&        colorL,
                                 const XCAFDoc_ColorType type) const
{
  // set reference
  Handle(TDataStd_TreeNode) refNode, mainNode;
  mainNode = TDataStd_TreeNode::Set(colorL, XCAFDoc1::ColorRefGUID(type));
  refNode  = TDataStd_TreeNode::Set(L, XCAFDoc1::ColorRefGUID(type));
  refNode->Remove(); // abv: fix against bug in TreeNode::Append()
  mainNode->Prepend(refNode);
}

//=================================================================================================

void XCAFDoc_ColorTool::SetColor(const DataLabel&        L,
                                 const Quantity_Color&   Color,
                                 const XCAFDoc_ColorType type) const
{
  DataLabel colorL = AddColor(Color);
  SetColor(L, colorL, type);
}

//=================================================================================================

void XCAFDoc_ColorTool::SetColor(const DataLabel&          L,
                                 const Quantity_ColorRGBA& Color,
                                 const XCAFDoc_ColorType   type) const
{
  DataLabel colorL = AddColor(Color);
  SetColor(L, colorL, type);
}

//=================================================================================================

void XCAFDoc_ColorTool::UnSetColor(const DataLabel& L, const XCAFDoc_ColorType type) const
{
  L.ForgetAttribute(XCAFDoc1::ColorRefGUID(type));
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::IsSet(const DataLabel& L, const XCAFDoc_ColorType type) const
{
  Handle(TDataStd_TreeNode) Node;
  return L.FindAttribute(XCAFDoc1::ColorRefGUID(type), Node) && Node->HasFather();
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::GetColor(const DataLabel&        L,
                                             const XCAFDoc_ColorType type,
                                             DataLabel&              colorL)
{
  Handle(TDataStd_TreeNode) Node;
  if (!L.FindAttribute(XCAFDoc1::ColorRefGUID(type), Node) || !Node->HasFather())
    return Standard_False;
  colorL = Node->Father()->Label();
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::GetColor(const DataLabel&        L,
                                             const XCAFDoc_ColorType type,
                                             Quantity_Color&         color)
{
  DataLabel colorL;
  if (!GetColor(L, type, colorL))
    return Standard_False;
  return GetColor(colorL, color);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::GetColor(const DataLabel&        L,
                                             const XCAFDoc_ColorType type,
                                             Quantity_ColorRGBA&     color)
{
  DataLabel colorL;
  if (!GetColor(L, type, colorL))
    return Standard_False;
  return GetColor(colorL, color);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::SetColor(const TopoShape&     S,
                                             const DataLabel&        colorL,
                                             const XCAFDoc_ColorType type)
{
  DataLabel L;
  if (!ShapeTool()->Search(S, L))
    return Standard_False;
  SetColor(L, colorL, type);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::SetColor(const TopoShape&     S,
                                             const Quantity_Color&   Color,
                                             const XCAFDoc_ColorType type)
{
  DataLabel colorL = AddColor(Color);
  return SetColor(S, colorL, type);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::SetColor(const TopoShape&       S,
                                             const Quantity_ColorRGBA& Color,
                                             const XCAFDoc_ColorType   type)
{
  DataLabel colorL = AddColor(Color);
  return SetColor(S, colorL, type);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::UnSetColor(const TopoShape& S, const XCAFDoc_ColorType type)
{
  DataLabel L;
  if (!ShapeTool()->Search(S, L))
    return Standard_False;
  UnSetColor(L, type);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::IsSet(const TopoShape& S, const XCAFDoc_ColorType type)
{
  DataLabel L;
  if (!ShapeTool()->Search(S, L))
    return Standard_False;
  return IsSet(L, type);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::GetColor(const TopoShape&     S,
                                             const XCAFDoc_ColorType type,
                                             DataLabel&              colorL)
{
  DataLabel L;
  if (!ShapeTool()->Search(S, L))
    return Standard_False;
  return GetColor(L, type, colorL);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::GetColor(const TopoShape&     S,
                                             const XCAFDoc_ColorType type,
                                             Quantity_Color&         color)
{
  DataLabel colorL;
  if (!GetColor(S, type, colorL))
    return Standard_False;
  return GetColor(colorL, color);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::GetColor(const TopoShape&     S,
                                             const XCAFDoc_ColorType type,
                                             Quantity_ColorRGBA&     color)
{
  DataLabel colorL;
  if (!GetColor(S, type, colorL))
    return Standard_False;
  return GetColor(colorL, color);
}

//=================================================================================================

const Standard_GUID& XCAFDoc_ColorTool::GetID()
{
  static Standard_GUID ColorTblID("efd212ed-6dfd-11d4-b9c8-0060b0ee281b");
  return ColorTblID;
}

//=================================================================================================

Handle(XCAFDoc_ColorTool) XCAFDoc_ColorTool::Set(const DataLabel& L)
{
  Handle(XCAFDoc_ColorTool) A;
  if (!L.FindAttribute(XCAFDoc_ColorTool::GetID(), A))
  {
    A = new XCAFDoc_ColorTool();
    L.AddAttribute(A);
    A->myShapeTool = XCAFDoc_DocumentTool::ShapeTool(L);
  }
  return A;
}

//=================================================================================================

const Standard_GUID& XCAFDoc_ColorTool::ID() const
{
  return GetID();
}

//=================================================================================================

XCAFDoc_ColorTool::XCAFDoc_ColorTool() {}

// PTV 23.01.2003 add visibility flag for objects (CAX-IF TRJ11)
//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::IsVisible(const DataLabel& L)
{
  Handle(TDataStd_UAttribute) aUAttr;
  return (!L.FindAttribute(XCAFDoc1::InvisibleGUID(), aUAttr));
}

//=================================================================================================

void XCAFDoc_ColorTool::SetVisibility(const DataLabel& L, const Standard_Boolean isvisible)
{
  Handle(TDataStd_UAttribute) aUAttr;
  if (!isvisible)
  {
    Handle(XCAFDoc_GraphNode) aSHUO;
    if (ShapeTool()->IsShape(L) || ShapeTool()->GetSHUO(L, aSHUO))
      if (!L.FindAttribute(XCAFDoc1::InvisibleGUID(), aUAttr))
        TDataStd_UAttribute::Set(L, XCAFDoc1::InvisibleGUID());
  }
  else
    L.ForgetAttribute(XCAFDoc1::InvisibleGUID());
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::IsColorByLayer(const DataLabel& L) const
{
  Handle(TDataStd_UAttribute) aUAttr;
  return L.FindAttribute(XCAFDoc1::ColorByLayerGUID(), aUAttr);
}

//=================================================================================================

void XCAFDoc_ColorTool::SetColorByLayer(const DataLabel& L, const Standard_Boolean isColorByLayer)
{
  Handle(TDataStd_UAttribute) aUAttr;
  if (isColorByLayer)
  {
    Handle(XCAFDoc_GraphNode) aSHUO;
    if (ShapeTool()->IsShape(L) || ShapeTool()->GetSHUO(L, aSHUO))
      if (!L.FindAttribute(XCAFDoc1::ColorByLayerGUID(), aUAttr))
        TDataStd_UAttribute::Set(L, XCAFDoc1::ColorByLayerGUID());
  }
  else
    L.ForgetAttribute(XCAFDoc1::ColorByLayerGUID());
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::SetInstanceColor(const TopoShape&     theShape,
                                                     const XCAFDoc_ColorType type,
                                                     const Quantity_Color&   color,
                                                     const Standard_Boolean  IsCreateSHUO)
{
  Quantity_ColorRGBA aCol;
  aCol.SetRGB(color);
  return SetInstanceColor(theShape, type, aCol, IsCreateSHUO);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::SetInstanceColor(const TopoShape&       theShape,
                                                     const XCAFDoc_ColorType   type,
                                                     const Quantity_ColorRGBA& color,
                                                     const Standard_Boolean    IsCreateSHUO)
{
  // find shuo label structure
  TDF_LabelSequence aLabels;
  if (!ShapeTool()->FindComponent(theShape, aLabels))
    return Standard_False;
  Handle(XCAFDoc_GraphNode) aSHUO;
  // set the SHUO structure for this component if it is not exist
  if (!ShapeTool()->FindSHUO(aLabels, aSHUO))
  {
    if (aLabels.Length() == 1)
    {
      // set color directly for component as NAUO
      SetColor(aLabels.Value(1), color, type);
      return Standard_True;
    }
    else if (!IsCreateSHUO || !ShapeTool()->SetSHUO(aLabels, aSHUO))
    {
      return Standard_False;
    }
  }
  DataLabel aSHUOLabel = aSHUO->Label();
  SetColor(aSHUOLabel, color, type);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::GetInstanceColor(const TopoShape&     theShape,
                                                     const XCAFDoc_ColorType type,
                                                     Quantity_Color&         color)
{
  Quantity_ColorRGBA aCol;
  Standard_Boolean   isDone = GetInstanceColor(theShape, type, aCol);
  if (isDone)
    color = aCol.GetRGB();
  return isDone;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::GetInstanceColor(const TopoShape&     theShape,
                                                     const XCAFDoc_ColorType type,
                                                     Quantity_ColorRGBA&     color)
{
  // find shuo label structure
  TDF_LabelSequence aLabels;
  if (!ShapeTool()->FindComponent(theShape, aLabels))
    return Standard_False;
  Handle(XCAFDoc_GraphNode) aSHUO;
  // get shuo from document by label structure
  DataLabel aCompLab = aLabels.Value(aLabels.Length());
  while (aLabels.Length() > 1)
  {
    if (!ShapeTool()->FindSHUO(aLabels, aSHUO))
    {
      // try to find other shuo
      aLabels.Remove(aLabels.Length());
      continue;
    }
    else
    {
      DataLabel aSHUOLabel = aSHUO->Label();
      if (GetColor(aSHUOLabel, type, color))
        return Standard_True;
      else
        // try to find other shuo
        aLabels.Remove(aLabels.Length());
    }
  }
  // attempt to get color exactly of component
  if (GetColor(aCompLab, type, color))
    return Standard_True;

  // attempt to get color of solid
  TopLoc_Location aLoc;
  TopoShape    S0 = theShape;
  S0.Location(aLoc);
  DataLabel aRefLab = ShapeTool()->FindShape(S0);
  if (!aRefLab.IsNull())
    return GetColor(aRefLab, type, color);
  // no color assigned
  return Standard_False;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::IsInstanceVisible(const TopoShape& theShape)
{
  // check visibility status of top-level solid, cause it is have highest priority
  TopLoc_Location NullLoc;
  TopoShape    S0 = theShape;
  S0.Location(NullLoc);
  DataLabel aRefL = ShapeTool()->FindShape(S0);
  if (!aRefL.IsNull() && !IsVisible(aRefL))
    return Standard_False;
  // find shuo label structure
  TDF_LabelSequence aLabels;
  if (!ShapeTool()->FindComponent(theShape, aLabels))
    return Standard_True;
  DataLabel aCompLab = aLabels.Value(aLabels.Length());
  // visibility status of component withouts SHUO.
  if (!IsVisible(aCompLab))
    return Standard_False;
  // check by SHUO structure
  TDF_LabelSequence aCurLabels;
  aCurLabels.Append(aCompLab);
  Standard_Integer i = aLabels.Length() - 1;
  //   while (aCurLabels.Length() < aLabels.Length()) {
  while (i >= 1)
  {
    aCurLabels.Prepend(aLabels.Value(i--));
    // get shuo from document by label structure
    Handle(XCAFDoc_GraphNode) aSHUO;
    if (!ShapeTool()->FindSHUO(aCurLabels, aSHUO))
      continue;
    if (!IsVisible(aSHUO->Label()))
      return Standard_False;
  }
  return Standard_True; // visible, cause cannot find invisibility status
}

//=================================================================================================

static void ReverseTreeNodes(Handle(TDataStd_TreeNode)& mainNode)
{
  if (mainNode->HasFirst())
  {
    Handle(TDataStd_TreeNode) tmpNode;
    Handle(TDataStd_TreeNode) pNode = mainNode->First();
    Handle(TDataStd_TreeNode) nNode = pNode->Next();
    while (!nNode.IsNull())
    {
      tmpNode = pNode->Previous();
      pNode->SetPrevious(nNode);
      pNode->SetNext(tmpNode);
      pNode = nNode;
      nNode = pNode->Next();
    }
    tmpNode = pNode->Previous();
    pNode->SetPrevious(nNode);
    pNode->SetNext(tmpNode);
    mainNode->SetFirst(pNode);
  }
}

//=================================================================================================

Standard_Boolean XCAFDoc_ColorTool::ReverseChainsOfTreeNodes()
{
  TDF_ChildIDIterator it(Label(), XCAFDoc_Color::GetID());
  for (; it.More(); it.Next())
  {
    DataLabel                 aLabel = it.Value()->Label();
    Handle(TDataStd_TreeNode) mainNode;
    if (aLabel.FindAttribute(XCAFDoc1::ColorRefGUID(XCAFDoc_ColorSurf), mainNode))
    {
      ReverseTreeNodes(mainNode);
    }
    if (aLabel.FindAttribute(XCAFDoc1::ColorRefGUID(XCAFDoc_ColorCurv), mainNode))
    {
      ReverseTreeNodes(mainNode);
    }
    if (aLabel.FindAttribute(XCAFDoc1::ColorRefGUID(XCAFDoc_ColorGen), mainNode))
    {
      ReverseTreeNodes(mainNode);
    }
  }
  return Standard_True;
}

//=================================================================================================

void XCAFDoc_ColorTool::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TDF_Attribute)

  TDF_LabelSequence aLabels;
  GetColors(aLabels);
  for (TDF_LabelSequence::Iterator aColorLabelIt(aLabels); aColorLabelIt.More();
       aColorLabelIt.Next())
  {
    AsciiString1 aColorLabel;
    Tool3::Entry(aColorLabelIt.Value(), aColorLabel);
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aColorLabel)
  }
}
