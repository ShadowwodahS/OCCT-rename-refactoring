// Created on: 2000-08-03
// Created by: data exchange team
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <XCAFDoc_ShapeTool.hxx>

#include <BRep_Builder.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <Standard_Type.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TColStd_SequenceOfHAsciiString.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_ChildIDIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelMap.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDF_Tool.hxx>
#include <TDocStd_Document.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_Tool.hxx>
#include <TopLoc_IndexedMapOfLocation.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfOrientedShape.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_ShapeMapTool.hxx>
#include <TopLoc_Datum3D.hxx>

IMPLEMENT_DERIVED_ATTRIBUTE_WITH_TYPE(XCAFDoc_ShapeTool, TDataStd_GenericEmpty, "xcaf", "ShapeTool")

static Standard_Boolean theAutoNaming = Standard_True;

// attribute methods //////////////////////////////////////////////////

//=================================================================================================

const Standard_GUID& XCAFDoc_ShapeTool::GetID()
{
  static Standard_GUID ShapeToolID("efd212ee-6dfd-11d4-b9c8-0060b0ee281b");
  return ShapeToolID;
}

//=================================================================================================

Handle(XCAFDoc_ShapeTool) XCAFDoc_ShapeTool::Set(const DataLabel& L)
{
  Handle(XCAFDoc_ShapeTool) A;
  if (!L.FindAttribute(XCAFDoc_ShapeTool::GetID(), A))
  {
    A = new XCAFDoc_ShapeTool();
    L.AddAttribute(A);
  }
  A->Init();
  return A;
}

//=================================================================================================

XCAFDoc_ShapeTool::XCAFDoc_ShapeTool()
{
  hasSimpleShapes = Standard_False;
}

//=================================================================================================

const Standard_GUID& XCAFDoc_ShapeTool::ID() const
{
  return GetID();
}

// Auxiliary methods //////////////////////////////////////////////////

//=================================================================================================

static void SetLabelNameByLink(const DataLabel L)
{
  Handle(TDataStd_TreeNode) Node;
  if (!L.FindAttribute(XCAFDoc1::ShapeRefGUID(), Node) || !Node->HasFather())
  {
#ifdef OCCT_DEBUG
    std::cout << "Error: XCAFDoc_ShapeTool, SetLabelNameByLink(): NO NODE" << std::endl;
#endif
    return;
  }
  AsciiString1 Entry;
  Tool3::Entry(Node->Father()->Label(), Entry);
  Entry.Insert(1, "=>[");
  Entry += "]";

  NameAttribute::Set(L, UtfString(Entry));
}

//=================================================================================================

static void SetLabelNameByShape(const DataLabel L)
{
  TopoShape S;
  if (XCAFDoc_ShapeTool::GetShape(L, S) && !L.IsAttribute(NameAttribute::GetID()))
  {
    Standard_SStream Stream;
    //    TopAbs_ShapeEnum Type = S.ShapeType();
    //    if (Type == TopAbs_COMPOUND) Stream<<"ASSEMBLY";
    //    else
    TopAbs1::Print(S.ShapeType(), Stream);
    AsciiString1 aName(Stream.str().c_str());
    NameAttribute::Set(L, UtfString(aName));
  }
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::SearchUsingMap(const TopoShape&    S,
                                                   DataLabel&             L,
                                                   const Standard_Boolean findWithoutLoc,
                                                   const Standard_Boolean findSubShape) const
{

  if (myShapeLabels.IsBound(S))
  {
    L = myShapeLabels.Find(S);
    return Standard_True;
  }
  TopoShape    S0 = S;
  TopLoc_Location loc;
  S0.Location(loc);
  if (myShapeLabels.IsBound(S0))
  {
    DataLabel         L1 = myShapeLabels.Find(S0);
    TDF_LabelSequence Labels;
    if (GetUsers(L1, Labels, Standard_True))
    {
      for (Standard_Integer i = 1; i <= Labels.Length(); i++)
      {
        TopoShape c = GetShape(Labels.Value(i));
        if (c.IsSame(S))
        {
          L = Labels.Value(i);
          return Standard_True;
        }
      }
    }
    if (findWithoutLoc)
    {
      L = L1;
      return Standard_True;
    }
  }

  if (hasSimpleShapes)
  {
    if (mySimpleShapes.IsBound(S))
    {
      L = mySimpleShapes.Find(S);
      return Standard_True;
    }
    if (mySimpleShapes.IsBound(S0))
    {
      L = mySimpleShapes.Find(S0);
      return Standard_True;
    }
  }
  // search subshapes
  if (!findSubShape)
    return Standard_False;
  DataLabel mainL = FindMainShapeUsingMap(S);
  if (mainL.IsNull())
    return Standard_False;
  L = AddSubShape(mainL, S);
  return !L.IsNull(); // Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::Search(const TopoShape&    S,
                                           DataLabel&             L,
                                           const Standard_Boolean findInstance,
                                           const Standard_Boolean findComponent,
                                           const Standard_Boolean findSubShape) const
{
  // search among shapes
  Standard_Boolean isLocated = !S.Location().IsIdentity();

  if (isLocated)
  {
    // try to find top-level instance
    if (findInstance && FindShape(S, L, Standard_True))
      return Standard_True;
    // try to find component of assembly
    if (findComponent)
    {
      TDF_LabelSequence labels;
      GetShapes(labels);
      for (Standard_Integer i = 1; i <= labels.Length(); i++)
      {
        if (!IsAssembly(labels.Value(i)))
          continue;
        TDF_LabelSequence comp;
        GetComponents(labels.Value(i), comp);
        for (Standard_Integer j = 1; j <= comp.Length(); j++)
        {
          TopoShape c = GetShape(comp.Value(j));
          if (c.IsSame(S))
          {
            L = comp.Value(j);
            return Standard_True;
          }
        }
      }
    }
  }
  // try to find top-level simple shape
  if (FindShape(S, L, Standard_False))
    return Standard_True;

  // search subshapes
  if (!findSubShape)
    return Standard_False;
  DataLabel mainL = FindMainShape(S);
  if (mainL.IsNull())
    return Standard_False;
  L = AddSubShape(mainL, S);
  return !L.IsNull(); // Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::FindShape(const TopoShape&    S,
                                              DataLabel&             L,
                                              const Standard_Boolean findInstance) const
{
  // search for null-located shape
  TopoShape S0 = S;
  if (!findInstance)
  {
    TopLoc_Location loc;
    S0.Location(loc);
  }

  // this code is used instead of the following for performance reasons
  if (Tool11::HasLabel(Label(), S0))
  {
    int TransDef = 0;
    L            = Tool11::Label(Label(), S0, TransDef);
  }
  else
    return Standard_False;

  if (IsTopLevel(L))
    return Standard_True;

  // Try to find shape manually
  TDF_ChildIDIterator it(Label(), ShapeAttribute::GetID());
  for (; it.More(); it.Next())
  {
    DataLabel                  aLabel = it.Value()->Label();
    Handle(ShapeAttribute) NS;
    if (aLabel.FindAttribute(ShapeAttribute::GetID(), NS)
        && S0.IsSame(Tool11::GetShape(NS)))
    {
      L = aLabel;
      return Standard_True;
    }
  }

  L = DataLabel();
  return Standard_False;
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::FindShape(const TopoShape&    S,
                                       const Standard_Boolean findInstance) const
{
  DataLabel L;
  if (FindShape(S, L, findInstance))
    return L;
  return DataLabel();
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::GetShape(const DataLabel& L, TopoShape& S)
{
  Handle(XCAFDoc_Location) LocationAttribute;

  if (IsExternRef(L))
  {
    TopoCompound EmptyComp;
    ShapeBuilder    B;
    B.MakeCompound(EmptyComp);
    S = EmptyComp;
  }

  // for instance, get referred shape
  Handle(TDataStd_TreeNode) Node;
  if (L.FindAttribute(XCAFDoc1::ShapeRefGUID(), Node) && Node->HasFather()
      && L.FindAttribute(XCAFDoc_Location::GetID(), LocationAttribute))
  {
    if (!GetShape(Node->Father()->Label(), S))
      return Standard_False;
    S.Move(LocationAttribute->Get(), Standard_False);
    return Standard_True;
  }

  // else just return shape on this label
  Handle(ShapeAttribute) NS;
  if (!L.FindAttribute(ShapeAttribute::GetID(), NS))
    return Standard_False;
  S = Tool11::GetShape(NS);
  return Standard_True;
}

//=================================================================================================

TopoShape XCAFDoc_ShapeTool::GetShape(const DataLabel& L)
{
  TopoShape aShape;
  GetShape(L, aShape);
  return aShape;
}

//=================================================================================================

TopoShape XCAFDoc_ShapeTool::GetOneShape(const TDF_LabelSequence& theLabels)
{
  TopoShape aShape;
  if (theLabels.Length() == 1)
  {
    return GetShape(theLabels.Value(1));
  }
  TopoCompound aCompound;
  ShapeBuilder    aBuilder;
  aBuilder.MakeCompound(aCompound);
  for (TDF_LabelSequence::Iterator anIt(theLabels); anIt.More(); anIt.Next())
  {
    TopoShape aFreeShape;
    if (!GetShape(anIt.Value(), aFreeShape))
    {
      continue;
    }
    aBuilder.Add(aCompound, aFreeShape);
  }
  if (aCompound.NbChildren() > 0)
  {
    aShape = aCompound;
  }
  return aShape;
}

//=================================================================================================

TopoShape XCAFDoc_ShapeTool::GetOneShape() const
{
  TDF_LabelSequence aLabels;
  GetFreeShapes(aLabels);
  return GetOneShape(aLabels);
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::NewShape() const
{
  TopoCompound aShape;
  ShapeBuilder    tdsB;
  tdsB.MakeCompound(aShape);

  TDF_TagSource aTag;
  DataLabel     aLabel = aTag.NewChild(Label());

  TNaming_Builder tnBuild(aLabel);
  tnBuild.Generated(aShape);

  return aLabel;
}

//=================================================================================================

void XCAFDoc_ShapeTool::SetShape(const DataLabel& L, const TopoShape& S)
{
  TNaming_Builder tnBuild(L);
  tnBuild.Generated(S);
  Handle(XCAFDoc_ShapeMapTool) A = XCAFDoc_ShapeMapTool::Set(L);
  //  if ( ! L.FindAttribute(XCAFDoc_ShapeMapTool::GetID(), A) ) {
  //    A = XCAFDoc_ShapeMapTool::Set(L);
  //    L.AddAttribute(A);
  //  }
  A->SetShape(S);

  if (!myShapeLabels.IsBound(S))
  {
    myShapeLabels.Bind(S, L);
  }
}

//=================================================================================================

void XCAFDoc_ShapeTool::MakeReference(const DataLabel&       L,
                                      const DataLabel&       refL,
                                      const TopLoc_Location& loc)
{
  // store location
  XCAFDoc_Location::Set(L, loc);

  // set reference
  Handle(TDataStd_TreeNode) refNode, mainNode;
  mainNode = TDataStd_TreeNode::Set(refL, XCAFDoc1::ShapeRefGUID());
  refNode  = TDataStd_TreeNode::Set(L, XCAFDoc1::ShapeRefGUID());
  refNode->Remove(); // abv: fix against bug in TreeNode::Append()
  mainNode->Append(refNode);

  if (theAutoNaming)
    SetLabelNameByLink(L);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::SetLocation(const DataLabel&       theShapeLabel,
                                                const TopLoc_Location& theLoc,
                                                DataLabel&             theRefLabel)
{
  if (theLoc.IsIdentity())
  {
    theRefLabel = theShapeLabel;
    return Standard_True;
  }
  // if input label is reference -> just change the location attribute
  if (IsReference(theShapeLabel))
  {
    TopLoc_Location anOldLoc;
    anOldLoc = GetLocation(theShapeLabel);
    TopLoc_Location aNewLoc(theLoc.Transformation() * anOldLoc.Transformation());
    XCAFDoc_Location::Set(theShapeLabel, aNewLoc);
    theRefLabel = theShapeLabel;
    return Standard_True;
  }
  // if input label is shape, and it is free -> create reference to the shape
  if (IsShape(theShapeLabel) && IsFree(theShapeLabel))
  {
    theRefLabel = TDF_TagSource::NewChild(Label());
    MakeReference(theRefLabel, theShapeLabel, theLoc);
    return Standard_True;
  }
  // other cases of label meaning doesn't need to apply new location
  return Standard_False;
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::addShape(const TopoShape& S, const Standard_Boolean makeAssembly)
{
  DataLabel     ShapeLabel;
  TDF_TagSource aTag;

  // search if the shape already exists (with the same location)
  if (S.IsNull() || FindShape(S, ShapeLabel, Standard_True))
    return ShapeLabel;

  // else add a new label
  ShapeLabel = aTag.NewChild(Label());

  // if shape has location, make a reference to the same shape without location
  if (!S.Location().IsIdentity() /*&& FindShape ( S, L )*/)
  {
    TopoShape    S0 = S;
    TopLoc_Location loc;
    S0.Location(loc);
    DataLabel L = addShape(S0, makeAssembly);
    MakeReference(ShapeLabel, L, S.Location());
    return ShapeLabel;
  }

  // else add a shape to a label
  TNaming_Builder tnBuild(ShapeLabel);
  tnBuild.Generated(S);

  Handle(XCAFDoc_ShapeMapTool) A = XCAFDoc_ShapeMapTool::Set(ShapeLabel);
  //  if ( ! ShapeLabel.FindAttribute(XCAFDoc_ShapeMapTool::GetID(), A) ) {
  //    A = XCAFDoc_ShapeMapTool::Set(ShapeLabel);
  //    ShapeLabel.AddAttribute(A);
  //  }
  A->SetShape(S);

  if (theAutoNaming)
    SetLabelNameByShape(ShapeLabel);

  // if shape is Compound and flag is set, create assembly
  if (makeAssembly && S.ShapeType() == TopAbs_COMPOUND)
  {
    // mark assembly by assigning UAttribute1
    Handle(TDataStd_UAttribute) Uattr;
    Uattr = TDataStd_UAttribute::Set(ShapeLabel, XCAFDoc1::AssemblyGUID());
    if (theAutoNaming)
      NameAttribute::Set(ShapeLabel, UtfString("ASSEMBLY"));

    // iterate on components
    TopoDS_Iterator Iterator(S);
    for (; Iterator.More(); Iterator.Next())
    {
      // get label for component`s shape
      TopoShape    Scomp = Iterator.Value(), S0 = Scomp;
      TopLoc_Location loc;
      S0.Location(loc);
      DataLabel compL = addShape(S0, makeAssembly);

      // add a component as reference
      DataLabel RefLabel = aTag.NewChild(ShapeLabel);
      MakeReference(RefLabel, compL, Scomp.Location());
    }
  }

  if (!IsAssembly(ShapeLabel))
  {
    // const TopTools_IndexedMapOfShape tmpMap = A->GetMap();
    // for(Standard_Integer i=1; i<=tmpMap.Extent(); i++)
    // mySubShapes.Bind(tmpMap.FindKey(i),ShapeLabel);
    for (Standard_Integer i = 1; i <= A->GetMap().Extent(); i++)
    {
      TopoShape aSh = A->GetMap().FindKey(i);
      mySubShapes.Bind(aSh, ShapeLabel);
      // if shape has location, make a reference to the same shape without location
      if (!aSh.Location().IsIdentity())
      {
        TopoShape    S0 = aSh;
        TopLoc_Location loc;
        S0.Location(loc);
        mySubShapes.Bind(S0, ShapeLabel);
      }
    }
    // mySubShapes.Bind(ShapeLabel,A->GetMap());
  }

  return ShapeLabel;
}

//=================================================================================================

static Standard_Boolean prepareAssembly(const TopoShape& theShape, TopoShape& theOUTShape)
{
  // iterate on components
  theOUTShape = theShape;
  if (theShape.ShapeType() == TopAbs_COMPOUND)
  {
    ShapeBuilder B;
    // check if shape if frosen
    if (!theOUTShape.Free())
      theOUTShape.Free(Standard_True);

    TopTools_SequenceOfShape aSubShapeSeq;
    TopoDS_Iterator          Iterator(theShape);
    for (; Iterator.More(); Iterator.Next())
      aSubShapeSeq.Append(Iterator.Value());
    for (Standard_Integer i = 1; i <= aSubShapeSeq.Length(); i++)
    {
      const TopoShape& Scomp = aSubShapeSeq.Value(i);
      TopoShape        aNewScomp;
      B.Remove(theOUTShape, Scomp);
      prepareAssembly(Scomp, aNewScomp);
      TopLoc_Location aLoc;
      aLoc = aNewScomp.Location();
      if (aLoc.IsIdentity())
      {
        // create an "empty" location
        Transform3d aTrsf;
        aTrsf.SetScale(Point3d(0, 0, 0), 1);
        aLoc = TopLoc_Location(aTrsf);
        aNewScomp.Location(aLoc, Standard_False);
      }
      B.Add(theOUTShape, aNewScomp);
    }
  }
  return Standard_True;
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::AddShape(const TopoShape&    theShape,
                                      const Standard_Boolean makeAssembly,
                                      const Standard_Boolean makePrepare)
{
  // PTV 17.02.2003 to avoid components without location.
  TopoShape S = theShape;
  if (makePrepare && makeAssembly && S.ShapeType() == TopAbs_COMPOUND)
    prepareAssembly(theShape, S); // OCC1669

  DataLabel L = addShape(S, makeAssembly);

  if (!myShapeLabels.IsBound(S))
  {
    myShapeLabels.Bind(S, L);
  }

  return L;

  // return addShape( S, makeAssembly );
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::RemoveShape(const DataLabel&       L,
                                                const Standard_Boolean removeCompletely) const
{
  if (!IsTopLevel(L) || !IsFree(L))
    return Standard_False;

  Handle(TDataStd_TreeNode) aNode;
  DataLabel                 aLabel;
  if (removeCompletely && L.FindAttribute(XCAFDoc1::ShapeRefGUID(), aNode) && aNode->HasFather()
      && L.IsAttribute(XCAFDoc_Location::GetID()))
  {
    aLabel = aNode->Father()->Label();
  }

  L.ForgetAllAttributes(Standard_True);

  if (removeCompletely && !aLabel.IsNull())
  {
    return RemoveShape(aLabel);
  }
  return Standard_True;
}

//=================================================================================================

void XCAFDoc_ShapeTool::Init()
{
  hasSimpleShapes = Standard_False;
}

//=================================================================================================

void XCAFDoc_ShapeTool::SetAutoNaming(const Standard_Boolean V)
{
  theAutoNaming = V;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::AutoNaming()
{
  return theAutoNaming;
}

//=================================================================================================

void XCAFDoc_ShapeTool::ComputeShapes(const DataLabel& L)
{
  ChildIterator it(L);
  for (; it.More(); it.Next())
  {
    DataLabel    L1 = it.Value();
    TopoShape S;
    if (GetShape(L1, S))
    {
      if (!myShapeLabels.IsBound(S))
      {
        mySimpleShapes.Bind(S, L1);
      }
    }
    ComputeShapes(L1);
  }
}

//=================================================================================================

void XCAFDoc_ShapeTool::ComputeSimpleShapes()
{
  ComputeShapes(Label());
  hasSimpleShapes = Standard_True;
}

//=================================================================================================

void XCAFDoc_ShapeTool::GetShapes(TDF_LabelSequence& Labels) const
{
  Labels.Clear();

  ChildIterator it(Label());
  for (; it.More(); it.Next())
  {
    DataLabel    L = it.Value();
    TopoShape S;
    if (GetShape(L, S))
      Labels.Append(L);
  }
}

//=================================================================================================

void XCAFDoc_ShapeTool::GetFreeShapes(TDF_LabelSequence& FreeLabels) const
{
  FreeLabels.Clear();

  ChildIterator it(Label());
  for (; it.More(); it.Next())
  {
    DataLabel    L = it.Value();
    TopoShape S;
    if (GetShape(L, S) && IsFree(L))
      FreeLabels.Append(L);
  }
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsTopLevel(const DataLabel& L) const
{
  return L.Father() == Label();
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsShape(const DataLabel& L)
{
  return IsSimpleShape(L) || IsAssembly(L) || IsReference(L);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsSimpleShape(const DataLabel& L)
{
  Handle(ShapeAttribute) NS;
  return L.FindAttribute(ShapeAttribute::GetID(), NS) && !IsAssembly(L) && !IsReference(L);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsReference(const DataLabel& L)
{
  Handle(TDataStd_TreeNode) Node;
  return L.FindAttribute(XCAFDoc1::ShapeRefGUID(), Node) && Node->HasFather();
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsAssembly(const DataLabel& L)
{
  Handle(TDataStd_UAttribute) Uattr;
  return L.FindAttribute(XCAFDoc1::AssemblyGUID(), Uattr);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsComponent(const DataLabel& L)
{
  return IsReference(L) && IsAssembly(L.Father());
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsCompound(const DataLabel& L)
{
  Handle(NameAttribute) Name;
  if (L.FindAttribute(NameAttribute::GetID(), Name))
  {
    UtfString estr1 = Name->Get();
    UtfString estr2("COMPOUND");
    if (estr1 == estr2)
    {
      return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsSubShape(const DataLabel& L)
{
  return IsSimpleShape(L) && IsShape(L.Father());
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsFree(const DataLabel& L)
{
  Handle(TDataStd_TreeNode) Node;
  if (!L.FindAttribute(XCAFDoc1::ShapeRefGUID(), Node) || !Node->HasFirst())
    return Standard_True;

  return Standard_False;
}

//=======================================================================
// function : GetUsers
// purpose  : Returns number of users (0 if shape is free)
//=======================================================================

Standard_Integer XCAFDoc_ShapeTool::GetUsers(const DataLabel&       L,
                                             TDF_LabelSequence&     Labels,
                                             const Standard_Boolean getsubchilds)
{
  Standard_Integer          NbUsers = 0;
  Handle(TDataStd_TreeNode) Node;

  if (!L.FindAttribute(XCAFDoc1::ShapeRefGUID(), Node))
    return NbUsers;

  Node = Node->First();
  while (!Node.IsNull())
  {

    if (getsubchilds)
    {
      DataLabel underL = Node->Label().Father();
      NbUsers += GetUsers(underL, Labels, getsubchilds);
    }

    Labels.Append(Node->Label());
    Node = Node->Next();
    NbUsers++;
  }
  return NbUsers;
}

//=================================================================================================

Standard_Integer XCAFDoc_ShapeTool::NbComponents(const DataLabel&       L,
                                                 const Standard_Boolean getsubchilds)
{
  TDF_LabelSequence subLabels;
  GetComponents(L, subLabels, getsubchilds);
  return subLabels.Length();
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::GetComponents(const DataLabel&       L,
                                                  TDF_LabelSequence&     Labels,
                                                  const Standard_Boolean getsubchilds)
{
  if (!IsAssembly(L))
    return Standard_False;

  ChildIterator It(L);
  for (; It.More(); It.Next())
  {
    DataLabel comp = It.Value();
    if (IsComponent(comp))
    {
      if (getsubchilds)
      {
        DataLabel underL;
        if (GetReferredShape(comp, underL))
          GetComponents(underL, Labels, getsubchilds);
      }
      Labels.Append(comp);
    }
  }
  return Standard_True;
}

//=================================================================================================

TopLoc_Location XCAFDoc_ShapeTool::GetLocation(const DataLabel& L)
{
  Handle(XCAFDoc_Location) LocationAttribute;
  if (L.FindAttribute(XCAFDoc_Location::GetID(), LocationAttribute))
    return LocationAttribute->Get();

  Handle(ShapeAttribute) NS;
  TopoShape               S;
  if (L.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    S = Tool11::GetShape(NS);
  }
  return S.Location();
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::GetReferredShape(const DataLabel& L, DataLabel& Label)
{
  if (!IsReference(L))
    return Standard_False;

  Handle(TDataStd_TreeNode) Node;
  L.FindAttribute(XCAFDoc1::ShapeRefGUID(), Node);
  Label = Node->Father()->Label();
  return Standard_True;
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::AddComponent(const DataLabel&       assembly,
                                          const DataLabel&       compL,
                                          const TopLoc_Location& Loc)
{
  DataLabel L;

  // check that shape is assembly
  if (!IsAssembly(assembly))
  {
    // if it is simple shape, make it assembly
    if (IsSimpleShape(assembly))
      TDataStd_UAttribute::Set(assembly, XCAFDoc1::AssemblyGUID());
    else
      return L;
  }

  // add a component as reference
  TDF_TagSource aTag;
  L = aTag.NewChild(assembly);
  MakeReference(L, compL, Loc);

  // map shape to label
  TopoShape aShape;
  if (GetShape(L, aShape))
  {
    if (!myShapeLabels.IsBound(aShape))
      myShapeLabels.Bind(aShape, L);
  }

  return L;
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::AddComponent(const DataLabel&       assembly,
                                          const TopoShape&    comp,
                                          const Standard_Boolean expand)
{
  // get label for component`s shape
  TopoShape    S0 = comp;
  TopLoc_Location loc;
  S0.Location(loc);
  DataLabel compL;
  compL = AddShape(S0, expand);

  // add component by its label
  return AddComponent(assembly, compL, comp.Location());
}

//=================================================================================================

void XCAFDoc_ShapeTool::RemoveComponent(const DataLabel& comp) const
{
  if (IsComponent(comp))
  {
    comp.ForgetAllAttributes();
  }
}

//=================================================================================================

void XCAFDoc_ShapeTool::UpdateAssemblies()
{
  // We start from the free shapes (roots in the assembly structure)
  TDF_LabelSequence aRootLabels;
  GetFreeShapes(aRootLabels);

  // Iterate over the free shapes
  TDF_LabelMap anUpdated;
  for (TDF_LabelSequence::Iterator anIt(aRootLabels); anIt.More(); anIt.Next())
  {
    DataLabel aRefLabel = anIt.Value();
    if (IsReference(aRefLabel))
    {
      GetReferredShape(aRefLabel, aRefLabel);
    }
    const DataLabel& aRootLab = aRefLabel;
    TopoShape     anAssemblyShape;
    updateComponent(aRootLab, anAssemblyShape, anUpdated);
  }
}

//=================================================================================================

// static Standard_Boolean CheckSubShape (const TopoShape &S, const TopoShape &sub)
//{
//   if ( S.IsSame ( sub ) ) return Standard_True;
//
//   if ( S.ShapeType() >= sub.ShapeType() ) return Standard_False;
//
//   for ( TopoDS_Iterator it(S); it.More(); it.Next() ) {
//     if ( CheckSubShape ( it.Value(), sub ) ) return Standard_True;
//   }
//   return Standard_False;
// }

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsSubShape(const DataLabel&    shapeL,
                                               const TopoShape& sub) const
{
  Handle(XCAFDoc_ShapeMapTool) A;
  if (!shapeL.FindAttribute(XCAFDoc_ShapeMapTool::GetID(), A))
  {
    TopoShape aShape = GetShape(shapeL);
    if (aShape.IsNull())
      return Standard_False;
    A = XCAFDoc_ShapeMapTool::Set(shapeL);
    A->SetShape(aShape);
  }

  return A->IsSubShape(sub);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::FindSubShape(const DataLabel&    shapeL,
                                                 const TopoShape& sub,
                                                 DataLabel&          L) const
{
  if (sub.IsNull())
    return Standard_False;

  if (Tool11::HasLabel(Label(), sub))
  {
    int TransDef = 0;
    L            = Tool11::Label(Label(), sub, TransDef);
    if (L.IsNull())
      return Standard_False;
    if (L.Father() == shapeL)
      return Standard_True;
  }
  else
  {
    return Standard_False;
  }

  // if subshape was found wrong, try to do it manually
  // it can be possible if several part shapes has the same subshapes
  L = DataLabel();
  ChildIterator aChldLabIt(shapeL);
  for (; aChldLabIt.More(); aChldLabIt.Next())
  {
    DataLabel                  aSubLabel = aChldLabIt.Value();
    Handle(ShapeAttribute) NS;
    if (!aSubLabel.FindAttribute(ShapeAttribute::GetID(), NS))
      continue;
    TopoShape aSubShape = Tool11::GetShape(NS);
    if (!aSubShape.IsNull() && aSubShape.IsSame(sub))
    {
      L = aSubLabel;
      return Standard_True;
    }
  }

  return Standard_False;
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::AddSubShape(const DataLabel& shapeL, const TopoShape& sub) const
{
  DataLabel L;
  AddSubShape(shapeL, sub, L);

  return L;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::AddSubShape(const DataLabel&    shapeL,
                                                const TopoShape& sub,
                                                DataLabel&          addedSubShapeL) const
{
  addedSubShapeL = DataLabel();
  // Check if adding subshape is possible
  if (!IsSimpleShape(shapeL) || !IsTopLevel(shapeL))
    return Standard_False;

  TopoShape     aSubShape = sub;
  Standard_Boolean isDefined = Standard_True;
  if (!IsSubShape(shapeL, sub))
  {
    isDefined = Standard_False;
    // Try to find a subshape as a part of the main shape.
    // If location of subshape has been removed,
    // take the shape with the location from the main shape
    if (sub.Location().IsIdentity())
    {
      TDF_LabelSequence aShapeLSeq;
      for (TopoDS_Iterator it(GetShape(shapeL)); it.More() && !isDefined; it.Next())
      {
        const TopoShape& aShape = it.Value();
        if (sub.IsSame(aShape.Located(TopLoc_Location())))
        {
          isDefined = Standard_True;
          aSubShape = aShape;
        }
      }
    }
  }
  if (!isDefined)
    return Standard_False;

  // Try to find already existed subshape
  if (FindSubShape(shapeL, aSubShape, addedSubShapeL))
    return Standard_False;

  TDF_TagSource aTag;
  addedSubShapeL = aTag.NewChild(shapeL);
  TNaming_Builder tnBuild(addedSubShapeL);
  tnBuild.Generated(aSubShape);

  return Standard_True;
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::FindMainShapeUsingMap(const TopoShape& sub) const
{
  // for(Standard_Integer i=1; i<=myNotAssemblies.Length(); i++) {
  //   DataLabel L = myNotAssemblies.Value(i);
  //   if(IsSubShape(L,sub)) return L;
  // }
  if (mySubShapes.IsBound(sub))
    return mySubShapes.Find(sub);
  DataLabel L0;
  return L0;
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::FindMainShape(const TopoShape& sub) const
{
  ChildIterator it(Label());
  for (; it.More(); it.Next())
  {
    DataLabel L = it.Value();

    if (IsSimpleShape(L) && IsSubShape(L, sub))
      return L;
  }
  DataLabel L0;
  return L0;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::GetSubShapes(const DataLabel& L, TDF_LabelSequence& Labels)
{
  ChildIterator It(L);
  for (; It.More(); It.Next())
  {
    DataLabel sub = It.Value();
    if (IsSubShape(sub))
      Labels.Append(sub);
  }
  return Labels.Length() > 0;
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::BaseLabel() const
{
  return Label();
}

//=======================================================================
// function : DumpAssembly
// purpose  : recursive part of Dump()
//=======================================================================

static void DumpAssembly(Standard_OStream&      theDumpLog,
                         const DataLabel        L,
                         const Standard_Integer level,
                         const Standard_Boolean deep)
{
  TopoShape S;
  XCAFDoc_ShapeTool::GetShape(L, S);
  if (S.IsNull())
    return;
  for (Standard_Integer i = 0; i < level; i++)
    theDumpLog << "\t";

  AsciiString1 Entry;
  Tool3::Entry(L, Entry);

  if (XCAFDoc_ShapeTool::IsAssembly(L))
  {
    theDumpLog << "ASSEMBLY ";
  }
  else if (XCAFDoc_ShapeTool::IsSimpleShape(L))
  {
    if (L.Father().Father().Father().IsRoot())
      theDumpLog << "PART ";
  }
  else
  {
    theDumpLog << "INSTANCE ";
  }
  TopAbs1::Print(S.ShapeType(), theDumpLog);

  theDumpLog << " " << Entry;
  if (XCAFDoc_ShapeTool::IsReference(L))
  {
    Handle(TDataStd_TreeNode) aRef;
    L.FindAttribute(XCAFDoc1::ShapeRefGUID(), aRef);
    Tool3::Entry(aRef->Father()->Label(), Entry);
    theDumpLog << " (refers to " << Entry << ")";
  }
  Handle(NameAttribute) Name;
  if (L.FindAttribute(NameAttribute::GetID(), Name))
    theDumpLog << " \"" << Name->Get() << "\" ";

  if (deep)
  {
    theDumpLog << "(" << *(void**)&S.TShape();
    if (!S.Location().IsIdentity())
      theDumpLog << ", " << *(void**)&S.Location();
    theDumpLog << ") ";
  }
  theDumpLog << std::endl;

  Handle(TDataStd_TreeNode) Node;
  ChildIterator         NodeIterator(L);
  for (; NodeIterator.More(); NodeIterator.Next())
  {
    DumpAssembly(theDumpLog, NodeIterator.Value(), level + 1, deep);
  }
  if (level == 0)
    theDumpLog << std::endl;
}

//=================================================================================================

Standard_OStream& XCAFDoc_ShapeTool::Dump(Standard_OStream&      theDumpLog,
                                          const Standard_Boolean deep) const
{
  Standard_Integer level = 0;
  //   TopTools_SequenceOfShape SeqShapes;
  TDF_LabelSequence SeqLabels;
  GetShapes(SeqLabels);

  if (SeqLabels.Length() > 0)
    theDumpLog << std::endl;
  Standard_Integer i;
  for (i = 1; i <= SeqLabels.Length(); i++)
  {
    DumpAssembly(theDumpLog, SeqLabels.Value(i), level, deep);
  }

  SeqLabels.Clear();
  GetFreeShapes(SeqLabels);
  theDumpLog << std::endl << "Free Shapes: " << SeqLabels.Length() << std::endl;
  for (i = 1; i <= SeqLabels.Length(); i++)
  {
    DumpShape(theDumpLog, SeqLabels.Value(i), level, deep);
    theDumpLog << std::endl;
  }
  return theDumpLog;
}

//=================================================================================================

Standard_OStream& XCAFDoc_ShapeTool::Dump(Standard_OStream& theDumpLog) const
{
  TDF_Attribute::Dump(theDumpLog);
  Dump(theDumpLog, Standard_False);
  return theDumpLog;
}

//=================================================================================================

void XCAFDoc_ShapeTool::DumpShape(Standard_OStream&      theDumpLog,
                                  const DataLabel&       L,
                                  const Standard_Integer level,
                                  const Standard_Boolean deep)
{
  TopoShape S;
  if (!XCAFDoc_ShapeTool::GetShape(L, S))
    return;
  for (Standard_Integer i = 0; i < level; i++)
    theDumpLog << "\t";

  if (XCAFDoc_ShapeTool::IsAssembly(L))
  {
    theDumpLog << "ASSEMBLY ";
  }
  else if (XCAFDoc_ShapeTool::IsSimpleShape(L))
  {
    if (L.Father().Father().Father().IsRoot())
      theDumpLog << "PART ";
  }
  else
  {
    theDumpLog << "INSTANCE ";
  }
  TopAbs1::Print(S.ShapeType(), theDumpLog);

  AsciiString1 Entry;
  Tool3::Entry(L, Entry);
  theDumpLog << "  " << Entry;
  if (XCAFDoc_ShapeTool::IsReference(L))
  {
    Handle(TDataStd_TreeNode) aRef;
    L.FindAttribute(XCAFDoc1::ShapeRefGUID(), aRef);
    Tool3::Entry(aRef->Father()->Label(), Entry);
    theDumpLog << " (refers to " << Entry << ")";
  }
  // std::cout<<std::endl;
  Handle(NameAttribute) Name;
  if (L.FindAttribute(NameAttribute::GetID(), Name))
    theDumpLog << " \"" << Name->Get() << "\" ";

  if (deep)
  {
    theDumpLog << "(" << *(void**)&S.TShape();
    if (!S.Location().IsIdentity())
      theDumpLog << ", " << *(void**)&S.Location();
    theDumpLog << ") ";
  }
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::IsExternRef(const DataLabel& L)
{
  Handle(TDataStd_UAttribute) Uattr;
  return L.FindAttribute(XCAFDoc1::ExternRefGUID(), Uattr);
}

//=================================================================================================

void XCAFDoc_ShapeTool::SetExternRefs(const DataLabel&                      L,
                                      const TColStd_SequenceOfHAsciiString& SHAS) const
{
  DataLabel ShapeLabel = L.NewChild();
  TDataStd_UAttribute::Set(ShapeLabel, XCAFDoc1::ExternRefGUID());
  for (Standard_Integer i = 1; i <= SHAS.Length(); i++)
  {
    DataLabel                               tmplbl = ShapeLabel.FindChild(i, Standard_True);
    const Handle(TCollection_HAsciiString)& str    = SHAS(i);
    UtfString              extstr(str->String());
    NameAttribute::Set(tmplbl, extstr);
  }
}

//=================================================================================================

DataLabel XCAFDoc_ShapeTool::SetExternRefs(const TColStd_SequenceOfHAsciiString& SHAS) const
{
  DataLabel     ShapeLabel;
  TDF_TagSource aTag;
  // add a new label
  ShapeLabel = aTag.NewChild(Label());
  TDataStd_UAttribute::Set(ShapeLabel, XCAFDoc1::ExternRefGUID());
  for (Standard_Integer i = 1; i <= SHAS.Length(); i++)
  {
    DataLabel                               tmplbl = ShapeLabel.FindChild(i, Standard_True);
    const Handle(TCollection_HAsciiString)& str    = SHAS(i);
    UtfString              extstr(str->String());
    NameAttribute::Set(tmplbl, extstr);
  }
  return ShapeLabel;
}

//=================================================================================================

void XCAFDoc_ShapeTool::GetExternRefs(const DataLabel& L, TColStd_SequenceOfHAsciiString& SHAS)
{
  Handle(NameAttribute) TDN;
  DataLabel             tmplbl;
  for (Standard_Integer i = 1; i <= L.NbChildren(); i++)
  {
    tmplbl = L.FindChild(i);
    if (tmplbl.FindAttribute(NameAttribute::GetID(), TDN))
    {
      UtfString       extstr = TDN->Get();
      Handle(TCollection_HAsciiString) str =
        new TCollection_HAsciiString(AsciiString1(extstr));
      SHAS.Append(str);
    }
  }
}

// API: API work with SHUO (Specified Higher Usage Occurrence) structure

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::GetSHUO(const DataLabel&           SHUOLabel,
                                            Handle(XCAFDoc_GraphNode)& aSHUOAttr)
{
  if (!SHUOLabel.FindAttribute(XCAFDoc1::SHUORefGUID(), aSHUOAttr))
    return Standard_False;
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::GetAllComponentSHUO(const DataLabel&       theCompLabel,
                                                        TDF_AttributeSequence& theSHUOAttrs)
{
  ChildIterator it(theCompLabel);
  for (; it.More(); it.Next())
  {
    DataLabel                 L = it.Value();
    Handle(XCAFDoc_GraphNode) aSHUOAttr;
    if (GetSHUO(L, aSHUOAttr))
      theSHUOAttrs.Append(aSHUOAttr);
  }
  return (theSHUOAttrs.Length() > 0);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::SetSHUO(const TDF_LabelSequence&   labels,
                                            Handle(XCAFDoc_GraphNode)& MainSHUOAttr) const
{
  MainSHUOAttr.Nullify();
  // check number of labels
  if (labels.Length() < 2)
    return Standard_False;
  // check is all labels contains components of any assemblyies
  Standard_Integer i;
  for (i = 1; i <= labels.Length(); i++)
    if (!IsComponent(labels.Value(i)))
      return Standard_False;

  TDF_TagSource aTag;
  DataLabel     UpperSubL = aTag.NewChild(labels(1));
  if (theAutoNaming)
  {
    UtfString Entry("SHUO");
    NameAttribute::Set(UpperSubL, UtfString(Entry));
  }
  Handle(XCAFDoc_GraphNode) aUpperSHUO;
  aUpperSHUO = XCAFDoc_GraphNode::Set(UpperSubL, XCAFDoc1::SHUORefGUID());
  // init out argument by main upper usage SHUO
  MainSHUOAttr = aUpperSHUO;
  // add other next_usage occurrences.
  for (i = 2; i <= labels.Length(); i++)
  {
    DataLabel NextSubL = aTag.NewChild(labels(i));
    if (theAutoNaming)
    {
      UtfString EntrySub("SHUO-");
      EntrySub += i;
      NameAttribute::Set(NextSubL, UtfString(EntrySub));
    }
    Handle(XCAFDoc_GraphNode) aNextSHUO;
    aNextSHUO = XCAFDoc_GraphNode::Set(NextSubL, XCAFDoc1::SHUORefGUID());
    // set references
    aUpperSHUO->SetChild(aNextSHUO);
    aNextSHUO->SetFather(aUpperSHUO);
    // now lets next_usage become upper_usage for next level of SHUO
    aUpperSHUO = aNextSHUO;
    UpperSubL  = NextSubL;
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::GetSHUOUpperUsage(const DataLabel&   NextUsageL,
                                                      TDF_LabelSequence& aLabels)
{
  Handle(XCAFDoc_GraphNode) aNextSHUO;
  if (!GetSHUO(NextUsageL, aNextSHUO) || aNextSHUO->NbFathers() < 1)
    return Standard_False;

  // get upper_usage SHAO
  for (Standard_Integer i = 1; i <= aNextSHUO->NbFathers(); i++)
    aLabels.Append(aNextSHUO->GetFather(i)->Label());
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::GetSHUONextUsage(const DataLabel&   UpperUsageL,
                                                     TDF_LabelSequence& aLabels)
{
  Handle(XCAFDoc_GraphNode) aUpperSHUO;
  if (!GetSHUO(UpperUsageL, aUpperSHUO) || aUpperSHUO->NbChildren() < 1)
    return Standard_False;
  // get upper_usage SHAO
  for (Standard_Integer i = 1; i <= aUpperSHUO->NbChildren(); i++)
    aLabels.Append(aUpperSHUO->GetChild(i)->Label());
  return Standard_True;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::RemoveSHUO(const DataLabel& L) const
{
  L.ForgetAllAttributes(Standard_True);
  return Standard_True;
}

//=================================================================================================

static Standard_Boolean IsEqual(const TopLoc_Location& theLoc1, const TopLoc_Location& theLoc2)
{
  if (theLoc1.IsEqual(theLoc2))
  {
    return Standard_True;
  }
  if (theLoc1.IsIdentity() || theLoc2.IsIdentity())
  {
    return Standard_False;
  }
  const Handle(TopLoc_Datum3D)& aDatum1 = theLoc1.FirstDatum();
  const Handle(TopLoc_Datum3D)& aDatum2 = theLoc2.FirstDatum();
  if (aDatum1 && aDatum2)
  {
    NCollection_Mat4<double> aMat41;
    NCollection_Mat4<double> aMat42;
    theLoc1.FirstDatum()->Transformation().GetMat4(aMat41);
    theLoc2.FirstDatum()->Transformation().GetMat4(aMat42);
    if (!aMat41.IsEqual(aMat42))
    {
      return Standard_False;
    }
  }
  else if (aDatum1 || aDatum2)
  {
    return Standard_False;
  }
  if (theLoc1.FirstPower() != theLoc2.FirstPower())
  {
    return Standard_False;
  }
  else
  {
    return IsEqual(theLoc1.NextLocation(), theLoc2.NextLocation());
  }
}

static Standard_Boolean IsSame(const TopoShape& theShape1, const TopoShape& theShape2)
{

  return theShape1.TShape() == theShape2.TShape()
         && theShape1.Orientation() == theShape2.Orientation()
         && IsEqual(theShape1.Location(), theShape2.Location());
}

static Standard_Boolean checkForShape(const TopoShape& theShape,
                                      const TopoShape& theCurSh,
                                      const DataLabel&    theUserL,
                                      TDF_LabelSequence&  theLabels)
{
  // the label of an assembly which contains this component
  DataLabel       aSuperUserL = theUserL.Father();
  TopLoc_Location aSupLoc, aCompLoc;
  aSupLoc              = ::XCAFDoc_ShapeTool::GetLocation(aSuperUserL);
  aCompLoc             = ::XCAFDoc_ShapeTool::GetLocation(theUserL);
  TopoShape aCopySh = theCurSh;
  aCompLoc             = aCompLoc.Multiplied(theCurSh.Location());
  aSupLoc              = aSupLoc.Multiplied(aCompLoc);
  aCopySh.Location(aSupLoc, Standard_False);
  if (IsSame(theShape, aCopySh))
  {
    theLabels.Prepend(theUserL);
    return Standard_True;
  }
  // try to search deeply (upper by assembly structure)
  TDF_LabelSequence aNewLabels;
  for (Standard_Integer j = 1; j <= theLabels.Length(); j++)
    aNewLabels.Append(theLabels.Value(j));
  aNewLabels.Prepend(theUserL);
  TDF_LabelSequence aUsers;
  ::XCAFDoc_ShapeTool::GetUsers(aSuperUserL, aUsers);
  for (Standard_Integer i = 1; i <= aUsers.Length(); i++)
    if (checkForShape(theShape, aCopySh, aUsers.Value(i), aNewLabels))
    {
      // get solution
      theLabels = aNewLabels;
      return Standard_True;
    }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::FindComponent(const TopoShape& theShape,
                                                  TDF_LabelSequence&  theLabels) const
{
  theLabels.Clear();
  // search for a top-level shape that corresponds to this component
  TopoShape    S0 = theShape;
  TopLoc_Location loc;
  S0.Location(loc);
  DataLabel aRefL = FindShape(S0);
  if (aRefL.IsNull())
    return Standard_False; // cannot find top-level shape.

  TDF_LabelSequence aUsers;
  ::XCAFDoc_ShapeTool::GetUsers(aRefL, aUsers);
  for (Standard_Integer i = 1; i <= aUsers.Length(); i++)
    if (checkForShape(theShape, S0, aUsers.Value(i), theLabels))
      break;

  return (theLabels.Length() > 0);
}

//=================================================================================================

static Standard_Boolean getShapesOfSHUO(TopLoc_IndexedMapOfLocation&     theaPrevLocMap,
                                        const Handle(XCAFDoc_ShapeTool)& theSTool,
                                        const DataLabel&                 theSHUOlab,
                                        TopoShape&                    theShape)
{
  Handle(XCAFDoc_GraphNode) SHUO;
  TDF_LabelSequence         aLabSeq;
  theSTool->GetSHUONextUsage(theSHUOlab, aLabSeq);
  if (aLabSeq.Length() >= 1)
    for (Standard_Integer i = 1; i <= aLabSeq.Length(); i++)
    {
      DataLabel       aSubCompL = aLabSeq.Value(i);
      TopLoc_Location compLoc   = XCAFDoc_ShapeTool::GetLocation(aSubCompL.Father());
      // create new map of laocation (to not merge locations from different shapes)
      TopLoc_IndexedMapOfLocation aNewPrevLocMap;
      for (Standard_Integer m = 1; m <= theaPrevLocMap.Extent(); m++)
        aNewPrevLocMap.Add(theaPrevLocMap.FindKey(m));
      aNewPrevLocMap.Add(compLoc);
      // got for the new sublocations and corresponding shape
      getShapesOfSHUO(aNewPrevLocMap, theSTool, aSubCompL, theShape);
    }
  else
  {
    TopoShape aSHUO_NUSh = theSTool->GetShape(theSHUOlab.Father());
    if (aSHUO_NUSh.IsNull())
      return Standard_False;
    // cause got shape with location already.
    TopLoc_Location nullLoc;
    aSHUO_NUSh.Location(nullLoc);
    // multiply the locations
    Standard_Integer intMapLenght = theaPrevLocMap.Extent();
    if (intMapLenght < 1)
      return Standard_False; // should not be, but to avoid exception...?
    TopLoc_Location SupcompLoc;
    SupcompLoc = theaPrevLocMap.FindKey(intMapLenght);
    if (intMapLenght > 1)
    {
      Standard_Integer l = intMapLenght - 1;
      while (l >= 1)
      {
        SupcompLoc = theaPrevLocMap.FindKey(l).Multiplied(SupcompLoc);
        l--;
      }
    }
    aSHUO_NUSh.Location(SupcompLoc, Standard_False);
    theShape = aSHUO_NUSh;
  }
  return (!theShape.IsNull());
}

//=================================================================================================

TopoShape XCAFDoc_ShapeTool::GetSHUOInstance(const Handle(XCAFDoc_GraphNode)& theSHUO) const
{
  TopoShape aShape;
  if (theSHUO.IsNull())
    return aShape;

  DataLabel aSHUOlab = theSHUO->Label();
  // get location of the assembly
  TopLoc_Location loc = XCAFDoc_ShapeTool::GetLocation(aSHUOlab.Father().Father());
  // get location of the component
  TopLoc_Location             compLoc = XCAFDoc_ShapeTool::GetLocation(aSHUOlab.Father());
  TopLoc_IndexedMapOfLocation aPrevLocMap;
  // get previous set location
  if (!loc.IsIdentity())
    aPrevLocMap.Add(loc);
  aPrevLocMap.Add(compLoc);
  // get shape by recurse method
  const Handle(XCAFDoc_ShapeTool)& STool = this;
  getShapesOfSHUO(aPrevLocMap, STool, aSHUOlab, aShape);

  return aShape;
}

//=================================================================================================

static Standard_Boolean getUsersShapesOfSHUO(TopLoc_IndexedMapOfLocation&     aPrevLocMap,
                                             const Handle(XCAFDoc_ShapeTool)& STool,
                                             const DataLabel&                 aSHUOlab,
                                             const DataLabel&                 theUserL,
                                             TopTools_SequenceOfShape&        theSHUOShapeSeq)
{
  TopLoc_IndexedMapOfLocation aNewPrevLocMap;
  // get location of the assembly
  TopLoc_Location loc = XCAFDoc_ShapeTool::GetLocation(theUserL.Father());
  // get location of the component
  TopLoc_Location compLoc = XCAFDoc_ShapeTool::GetLocation(theUserL);
  // get previous set location
  aNewPrevLocMap.Add(loc);
  aNewPrevLocMap.Add(compLoc);
  Standard_Integer i;
  for (i = 1; i <= aPrevLocMap.Extent(); i++)
    aNewPrevLocMap.Add(aPrevLocMap.FindKey(i));
  DataLabel         L = theUserL.Father();
  TDF_LabelSequence usersLab;
  ::XCAFDoc_ShapeTool::GetUsers(L, usersLab);
  if (usersLab.Length() == 0)
  {
    TopoShape aShape;
    getShapesOfSHUO(aNewPrevLocMap, STool, aSHUOlab, aShape);
    if (!aShape.IsNull())
    {
      theSHUOShapeSeq.Append(aShape);
      return Standard_True;
    }
  }
  // now iterates on users of this assembly as component
  for (i = 1; i <= usersLab.Length(); i++)
  {
    DataLabel aNewUserL = usersLab.Value(i);
    getUsersShapesOfSHUO(aNewPrevLocMap, STool, aSHUOlab, aNewUserL, theSHUOShapeSeq);
  }

  return (theSHUOShapeSeq.Length() > 1);
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::GetAllSHUOInstances(
  const Handle(XCAFDoc_GraphNode)& theSHUO,
  TopTools_SequenceOfShape&        theSHUOShapeSeq) const
{
  if (theSHUO.IsNull())
    return Standard_False;

  DataLabel                   aSHUOlab = theSHUO->Label();
  TopLoc_IndexedMapOfLocation aPrevLocMap;
  // get location of the assembly
  TopLoc_Location loc = XCAFDoc_ShapeTool::GetLocation(aSHUOlab.Father().Father());
  // get location of the component
  TopLoc_Location compLoc = XCAFDoc_ShapeTool::GetLocation(aSHUOlab.Father());
  // get previous set location
  if (!loc.IsIdentity())
    aPrevLocMap.Add(loc);
  aPrevLocMap.Add(compLoc);
  // get label of assembly
  DataLabel         L = aSHUOlab.Father().Father();
  TDF_LabelSequence usersLab;
  ::XCAFDoc_ShapeTool::GetUsers(L, usersLab);
  TopoShape                     aShape;
  const Handle(XCAFDoc_ShapeTool)& STool = this;
  if (usersLab.Length() == 0)
  {
    getShapesOfSHUO(aPrevLocMap, STool, aSHUOlab, aShape);
    if (!aShape.IsNull())
    {
      theSHUOShapeSeq.Append(aShape);
      return Standard_True;
    }
  }
  // now iterates on users of this assembly as component
  for (Standard_Integer i = 1; i <= usersLab.Length(); i++)
  {
    DataLabel aUserL = usersLab.Value(i);
    getUsersShapesOfSHUO(aPrevLocMap, STool, aSHUOlab, aUserL, theSHUOShapeSeq);
  }

  return (theSHUOShapeSeq.Length() > 1);
}

//=================================================================================================

Handle(XCAFDoc_GraphNode) XCAFDoc_ShapeTool::SetInstanceSHUO(const TopoShape& theShape) const
{
  Handle(XCAFDoc_GraphNode) SHUO;
  TDF_LabelSequence         aLabels;
  if (FindComponent(theShape, aLabels))
    // set shuo structure on labels of component-assembly structure
    SetSHUO(aLabels, SHUO);
  return SHUO;
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::FindSHUO(const TDF_LabelSequence&   theLabels,
                                             Handle(XCAFDoc_GraphNode)& theSHUOAttr)
{
  TDF_AttributeSequence SHUOAttrs;
  DataLabel             aCompLabel = theLabels.Value(1);
  if (!::XCAFDoc_ShapeTool::GetAllComponentSHUO(aCompLabel, SHUOAttrs))
    return Standard_False;
  // WARNING: manage that each SHUO upper_usage have only one SHUO next_usage
  for (Standard_Integer i = 1; i <= SHUOAttrs.Length(); i++)
  {
    TDF_LabelSequence         aCondidate;
    Handle(XCAFDoc_GraphNode) anSHUO = Handle(XCAFDoc_GraphNode)::DownCast(SHUOAttrs.Value(i));
    aCondidate.Append(anSHUO->Label().Father());
    while (anSHUO->NbChildren())
    {
      anSHUO = anSHUO->GetChild(1);
      aCondidate.Append(anSHUO->Label().Father());
    }
    // check the label sequences
    Standard_Boolean isEqual = Standard_True;
    if (theLabels.Length() != aCondidate.Length())
      isEqual = Standard_False;
    else
      for (Standard_Integer li = 1; li <= theLabels.Length(); li++)
        if (theLabels.Value(li) != aCondidate.Value(li))
        {
          isEqual = Standard_False;
          break;
        }
    if (!isEqual)
      continue;

    theSHUOAttr = Handle(XCAFDoc_GraphNode)::DownCast(SHUOAttrs.Value(i));
    break;
  }
  return (!theSHUOAttr.IsNull());
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::Expand(const DataLabel& theShapeL)
{
  if (theShapeL.IsNull() || IsAssembly(theShapeL))
    return Standard_False;

  TopoShape aShape = GetShape(theShapeL);
  if (aShape.IsNull())
    return Standard_False;

  TopAbs_ShapeEnum aShapeType     = aShape.ShapeType();
  Standard_Boolean isExpandedType = aShapeType == TopAbs_COMPOUND || aShapeType == TopAbs_COMPSOLID
                                    || aShapeType == TopAbs_SHELL || aShapeType == TopAbs_WIRE;
  if (isExpandedType)
  {
    TopoDS_Iterator anIter(aShape);
    for (; anIter.More(); anIter.Next())
    {
      const TopoShape& aChildShape = anIter.Value();
      DataLabel           aChild, aPart;

      // Find child shape as subshape of expanded shape
      FindSubShape(theShapeL, aChildShape, aChild);
      Handle(NameAttribute) anAttr;
      // make child (if color isn't set or if it is compound)
      if (aChild.IsNull())
      {
        aChild = AddSubShape(theShapeL, aChildShape);
      }
      else
      {
        // get name
        aChild.FindAttribute(NameAttribute::GetID(), anAttr);
      }

      // Try to find child shape as already existed part
      aPart = FindShape(aChildShape.Located(TopLoc_Location()));
      if (aPart.IsNull())
      {
        // Create new part to link child shape
        aPart = AddShape(aChildShape.Located(TopLoc_Location()), Standard_False, Standard_False);
      }
      // Add shape manually, if already existed subshape found instead of creation of new part
      if (!aPart.IsNull() && !IsTopLevel(aPart))
      {
        if (!GetReferredShape(aPart, aPart))
        {
          TDF_TagSource aTag;
          aPart = aTag.NewChild(Label());
          SetShape(aPart, aChildShape.Located(TopLoc_Location()));
        }
      }

      // set name to part
      if (!anAttr.IsNull())
      {
        NameAttribute::Set(aPart, anAttr->Get());
      }
      else
      {
        Standard_SStream Stream;
        TopAbs1::Print(aChildShape.ShapeType(), Stream);
        AsciiString1 aName(Stream.str().c_str());
        NameAttribute::Set(aPart, UtfString(aName));
      }
      MakeReference(aChild, aPart, aChildShape.Location());
      makeSubShape(theShapeL, aPart, aChildShape, aChildShape.Location());
    }
    // set assembly attribute
    TDataStd_UAttribute::Set(theShapeL, XCAFDoc1::AssemblyGUID());
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

void XCAFDoc_ShapeTool::makeSubShape(const DataLabel&       theMainShapeL,
                                     const DataLabel&       thePart,
                                     const TopoShape&    theShape,
                                     const TopLoc_Location& theLoc)
{
  TopoDS_Iterator  anIter(theShape);
  Standard_Boolean isCompoundPart = (GetShape(thePart).ShapeType() == TopAbs_COMPOUND);
  Standard_Boolean isAssembly     = IsAssembly(thePart);

  for (; anIter.More(); anIter.Next())
  {
    const TopoShape& aChildShape = anIter.Value();
    DataLabel           aChildLabel;
    FindSubShape(theMainShapeL, aChildShape, aChildLabel);
    if (!aChildLabel.IsNull())
    {
      if (isAssembly)
      {
        aChildLabel.ForgetAllAttributes();
        makeSubShape(theMainShapeL, thePart, aChildShape, theLoc);
        continue;
      }
      // get name
      Handle(NameAttribute) anAttr;
      aChildLabel.FindAttribute(NameAttribute::GetID(), anAttr);
      TopLoc_Location aSubLoc;
      // Calculate location for subshapes of compound parts
      aSubLoc = aChildShape.Location();
      if (isCompoundPart)
        aSubLoc = theLoc.Inverted() * aSubLoc;
      // make subshape
      DataLabel aSubLabel;
      // Identical location and empty location are not the same for ShapeTool, so try to process
      // both in case of aSubLoc is not identical, the second Add try will not affect algorithm.
      Standard_Boolean isNewSubL;
      isNewSubL = AddSubShape(thePart, aChildShape.Located(aSubLoc, Standard_False), aSubLabel);
      if (aSubLabel.IsNull())
      {
        isNewSubL = AddSubShape(thePart, aChildShape.Located(TopLoc_Location()), aSubLabel);
      }
      if (isNewSubL)
      {
        // set name to sub shape
        if (!anAttr.IsNull())
        {
          NameAttribute::Set(aSubLabel, anAttr->Get());
        }
        else
        {
          Standard_SStream Stream;
          TopAbs1::Print(aChildShape.ShapeType(), Stream);
          AsciiString1 aName(Stream.str().c_str());
          NameAttribute::Set(aSubLabel, UtfString(aName));
        }
        // Create auxiliary link, it will be removed during moving attributes
        MakeReference(aSubLabel, aChildLabel, aChildShape.Location());
      }
      else
      {
        aChildLabel.ForgetAllAttributes();
      }
    }

    makeSubShape(theMainShapeL, thePart, aChildShape, theLoc);
  }
}

//=================================================================================================

Standard_Boolean XCAFDoc_ShapeTool::updateComponent(const DataLabel& theItemLabel,
                                                    TopoShape&    theUpdatedShape,
                                                    TDF_LabelMap&    theUpdated) const
{
  if (!IsAssembly(theItemLabel))
    return Standard_False; // Do nothing for non-assemblies

  // Get the currently stored compound for the assembly
  TopoShape aCurrentRootShape;
  GetShape(theItemLabel, aCurrentRootShape);

  // Check if the given assembly is already updated
  if (theUpdated.Contains(theItemLabel))
  {
    theUpdatedShape = aCurrentRootShape;
    return Standard_True;
  }

  TopTools_MapOfOrientedShape aCurrentRootShapeMap(aCurrentRootShape.NbChildren());

  // Get components of the assembly
  TDF_LabelSequence aComponentLabs;
  GetComponents(theItemLabel, aComponentLabs);

  // This flag indicates whether to update the compound of the assembly
  Standard_Boolean isModified = Standard_False;

  // Compare the number of components in XDE structure with the number of
  // components in topological structure. A component may happen to be removed,
  // so we have to update the assembly compound
  const Standard_Integer aNumTopoComponents = aCurrentRootShape.NbChildren();
  //
  if (aNumTopoComponents != aComponentLabs.Length())
    isModified = Standard_True;

  // Iterate over the assembly components. If at least one component is
  // modified (this is the recursive check), then the actually stored
  // compound has to be updated
  ShapeList aComponentShapes;
  //
  for (TDF_LabelSequence::Iterator aCompIt(aComponentLabs); aCompIt.More(); aCompIt.Next())
  {
    const DataLabel& aComponentLab = aCompIt.Value();

    // Take the referred assembly item (ultimately, a part for an instance)
    DataLabel aComponentRefLab;
    GetReferredShape(aComponentLab, aComponentRefLab);

    // Shape comes with some placement transformation here
    TopoShape aComponentShape;
    GetShape(aComponentLab, aComponentShape);
    TopLoc_Location aComponentLoc = aComponentShape.Location();

    // If the component is a sub-assembly, then its associated compound
    // has to be processed in the same manner
    if (IsAssembly(aComponentRefLab))
    {
      // Recursive call
      if (updateComponent(aComponentRefLab, aComponentShape, theUpdated))
      {
        isModified = Standard_True;
        aComponentShape.Location(aComponentLoc, Standard_False); // Apply placement
      }
    }
    else
    {
      // Search for a part in the actual compound of the ultimate assembly.
      // If the part is there, then the compound is up-to-date, so it does not require rebuilding
      if (!isModified)
      {
        if (aCurrentRootShapeMap.IsEmpty())
        {
          // optimize search for next labels in aComponentLabs
          for (TopoDS_Iterator aTopoIt(aCurrentRootShape); aTopoIt.More(); aTopoIt.Next())
          {
            aCurrentRootShapeMap.Add(aTopoIt.Value());
          }
        }
        if (!aCurrentRootShapeMap.Contains(aComponentShape))
        {
          // Part has been modified somewhere, so the compound has to be rebuilt
          isModified = Standard_True;
        }
      }
    }

    // Fill the list of shapes composing a new compound for the assembly
    aComponentShapes.Append(aComponentShape);
  }

  // If any component is modified, we update the currently stored shape
  if (isModified)
  {
    TopoCompound anUpdatedCompound;
    ShapeBuilder    aBB;
    aBB.MakeCompound(anUpdatedCompound);

    // Compose new compound
    for (TopTools_ListIteratorOfListOfShape aShapeIt(aComponentShapes); aShapeIt.More();
         aShapeIt.Next())
    {
      aBB.Add(anUpdatedCompound, aShapeIt.Value());
    }

    // Store the updated shape as an output
    theUpdatedShape = anUpdatedCompound;

    // Use topological naming services to store the updated shape in XDE
    TNaming_Builder NB(theItemLabel);
    NB.Generated(theUpdatedShape);
  }

  if (isModified)
    theUpdated.Add(theItemLabel);

  return isModified;
}

//=================================================================================================

Handle(TDataStd_NamedData) XCAFDoc_ShapeTool::GetNamedProperties(
  const DataLabel&       theLabel,
  const Standard_Boolean theToCreate) const
{
  Handle(TDataStd_NamedData) aNamedProperty;
  if (!theLabel.FindAttribute(TDataStd_NamedData::GetID(), aNamedProperty) && theToCreate)
  {
    aNamedProperty = TDataStd_NamedData::Set(theLabel);
  }

  return aNamedProperty;
}

//=================================================================================================

Handle(TDataStd_NamedData) XCAFDoc_ShapeTool::GetNamedProperties(
  const TopoShape&    theShape,
  const Standard_Boolean theToCreate) const
{
  Handle(TDataStd_NamedData) aNamedProperty;
  DataLabel                  aLabel;
  if (!Search(theShape, aLabel))
    return aNamedProperty;

  aNamedProperty = GetNamedProperties(aLabel, theToCreate);

  return aNamedProperty;
}

//=================================================================================================

void XCAFDoc_ShapeTool::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TDF_Attribute)

  for (XCAFDoc_DataMapOfShapeLabel::Iterator aShapeLabelIt(myShapeLabels); aShapeLabelIt.More();
       aShapeLabelIt.Next())
  {
    const TopoShape& aShape = aShapeLabelIt.Key1();
    OCCT_DUMP_FIELD_VALUE_POINTER(theOStream, &aShape)

    AsciiString1 aShapeLabel;
    Tool3::Entry(aShapeLabelIt.Value(), aShapeLabel);
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aShapeLabel)
  }

  for (XCAFDoc_DataMapOfShapeLabel::Iterator aSubShapeIt(mySubShapes); aSubShapeIt.More();
       aSubShapeIt.Next())
  {
    const TopoShape& aSubShape = aSubShapeIt.Key1();
    OCCT_DUMP_FIELD_VALUE_POINTER(theOStream, &aSubShape)

    AsciiString1 aSubShapeLabel;
    Tool3::Entry(aSubShapeIt.Value(), aSubShapeLabel);
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aSubShapeLabel)
  }

  for (XCAFDoc_DataMapOfShapeLabel::Iterator aSimpleShapeIt(mySimpleShapes); aSimpleShapeIt.More();
       aSimpleShapeIt.Next())
  {
    const TopoShape& aSimpleShape = aSimpleShapeIt.Key1();
    OCCT_DUMP_FIELD_VALUE_POINTER(theOStream, &aSimpleShape)

    AsciiString1 aSimpleShapeLabel;
    Tool3::Entry(aSimpleShapeIt.Value(), aSimpleShapeLabel);
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aSimpleShapeLabel)
  }

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, hasSimpleShapes)
}
