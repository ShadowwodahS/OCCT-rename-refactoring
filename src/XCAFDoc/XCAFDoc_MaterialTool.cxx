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

#include <XCAFDoc_MaterialTool.hxx>

#include <Standard_Type.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_ChildIDIterator.hxx>
#include <TDF_Label.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDF_Tool.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Material.hxx>
#include <XCAFDoc_ShapeTool.hxx>

IMPLEMENT_DERIVED_ATTRIBUTE_WITH_TYPE(XCAFDoc_MaterialTool,
                                      TDataStd_GenericEmpty,
                                      "xcaf",
                                      "MaterialTool")

//=================================================================================================

XCAFDoc_MaterialTool::XCAFDoc_MaterialTool() {}

//=================================================================================================

Handle(XCAFDoc_MaterialTool) XCAFDoc_MaterialTool::Set(const DataLabel& L)
{
  Handle(XCAFDoc_MaterialTool) A;
  if (!L.FindAttribute(XCAFDoc_MaterialTool::GetID(), A))
  {
    A = new XCAFDoc_MaterialTool();
    L.AddAttribute(A);
    A->myShapeTool = XCAFDoc_DocumentTool::ShapeTool(L);
  }
  return A;
}

//=================================================================================================

const Standard_GUID& XCAFDoc_MaterialTool::GetID()
{
  static Standard_GUID MatTblID("efd212f9-6dfd-11d4-b9c8-0060b0ee281b");
  return MatTblID;
}

//=================================================================================================

DataLabel XCAFDoc_MaterialTool::BaseLabel() const
{
  return Label();
}

//=================================================================================================

const Handle(XCAFDoc_ShapeTool)& XCAFDoc_MaterialTool::ShapeTool()
{
  if (myShapeTool.IsNull())
    myShapeTool = XCAFDoc_DocumentTool::ShapeTool(Label());
  return myShapeTool;
}

//=================================================================================================

Standard_Boolean XCAFDoc_MaterialTool::IsMaterial(const DataLabel& lab) const
{
  Handle(XCAFDoc_Material) MatAttr;
  if (lab.FindAttribute(XCAFDoc_Material::GetID(), MatAttr))
  {
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

void XCAFDoc_MaterialTool::GetMaterialLabels(TDF_LabelSequence& Labels) const
{
  Labels.Clear();
  ChildIterator ChildIterator(Label());
  for (; ChildIterator.More(); ChildIterator.Next())
  {
    DataLabel L = ChildIterator.Value();
    if (IsMaterial(L))
      Labels.Append(L);
  }
}

//=================================================================================================

DataLabel XCAFDoc_MaterialTool::AddMaterial(
  const Handle(TCollection_HAsciiString)& aName,
  const Handle(TCollection_HAsciiString)& aDescription,
  const Standard_Real                     aDensity,
  const Handle(TCollection_HAsciiString)& aDensName,
  const Handle(TCollection_HAsciiString)& aDensValType) const
{
  DataLabel     MatL;
  TDF_TagSource aTag;
  MatL = aTag.NewChild(Label());
  XCAFDoc_Material::Set(MatL, aName, aDescription, aDensity, aDensName, aDensValType);
  NameAttribute::Set(MatL, AsciiString1(aName->ToCString()));
  return MatL;
}

//=================================================================================================

void XCAFDoc_MaterialTool::SetMaterial(const DataLabel& L, const DataLabel& MatL) const
{
  // set reference
  Handle(TDataStd_TreeNode) refNode, mainNode;
  mainNode = TDataStd_TreeNode::Set(MatL, XCAFDoc::MaterialRefGUID());
  refNode  = TDataStd_TreeNode::Set(L, XCAFDoc::MaterialRefGUID());
  refNode->Remove(); // abv: fix against bug in TreeNode::Append()
  mainNode->Append(refNode);
}

//=================================================================================================

void XCAFDoc_MaterialTool::SetMaterial(const DataLabel&                        L,
                                       const Handle(TCollection_HAsciiString)& aName,
                                       const Handle(TCollection_HAsciiString)& aDescription,
                                       const Standard_Real                     aDensity,
                                       const Handle(TCollection_HAsciiString)& aDensName,
                                       const Handle(TCollection_HAsciiString)& aDensValType) const
{
  DataLabel MatL = AddMaterial(aName, aDescription, aDensity, aDensName, aDensValType);
  SetMaterial(L, MatL);
}

//=================================================================================================

Standard_Boolean XCAFDoc_MaterialTool::GetMaterial(const DataLabel&                  MatL,
                                                   Handle(TCollection_HAsciiString)& aName,
                                                   Handle(TCollection_HAsciiString)& aDescription,
                                                   Standard_Real&                    aDensity,
                                                   Handle(TCollection_HAsciiString)& aDensName,
                                                   Handle(TCollection_HAsciiString)& aDensValType)
{
  Handle(XCAFDoc_Material) MatAttr;
  if (!MatL.FindAttribute(XCAFDoc_Material::GetID(), MatAttr))
  {
    return Standard_False;
  }
  aName        = MatAttr->GetName();
  aDescription = MatAttr->GetDescription();
  aDensity     = MatAttr->GetDensity();
  aDensName    = MatAttr->GetDensName();
  aDensValType = MatAttr->GetDensValType();

  return Standard_True;
}

//=================================================================================================

Standard_Real XCAFDoc_MaterialTool::GetDensityForShape(const DataLabel& ShapeL)
{
  Standard_Real             Dens = 0.0;
  Handle(TDataStd_TreeNode) Node;
  if (!ShapeL.FindAttribute(XCAFDoc::MaterialRefGUID(), Node) || !Node->HasFather())
    return Dens;
  DataLabel                MatL = Node->Father()->Label();
  Handle(XCAFDoc_Material) MatAttr;
  if (!MatL.FindAttribute(XCAFDoc_Material::GetID(), MatAttr))
  {
    return Dens;
  }
  // default dimension fo density - gram/sm^3
  // we transfer "sm" into "mm"
  Dens = MatAttr->GetDensity() * 0.001;
  return Dens;
}

//=================================================================================================

const Standard_GUID& XCAFDoc_MaterialTool::ID() const
{
  return GetID();
}

//=================================================================================================

void XCAFDoc_MaterialTool::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TDF_Attribute)

  TDF_LabelSequence aLabels;
  GetMaterialLabels(aLabels);
  for (TDF_LabelSequence::Iterator aMaterialLabelIt(aLabels); aMaterialLabelIt.More();
       aMaterialLabelIt.Next())
  {
    AsciiString1 aMaterialLabel;
    Tool3::Entry(aMaterialLabelIt.Value(), aMaterialLabel);
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aMaterialLabel)
  }
}
