// Created on: 1997-01-09
// Created by: VAUTHIER Jean-Claude & Fricaud Yves
// Copyright (c) 1997-1999 Matra Datavision
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

#include <DNaming.hxx>

#include <BRep_Tool.hxx>
#include <BRepAlgoAPI_BooleanOperation.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>
#include <BRepLib_FindSurface.hxx>
#include <BRepTools.hxx>
#include <DDF.hxx>
#include <Draw.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <gp_Ax1.hxx>
#include <gp_Pln.hxx>
#include <ModelDefinitions.hxx>
#include <TCollection_AsciiString.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_Real.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Data.hxx>
#include <TDF_Label.hxx>
#include <TDF_Reference.hxx>
#include <TDF_TagSource.hxx>
#include <TDF_Tool.hxx>
#include <TFunction_Function.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_Iterator.hxx>
#include <TNaming_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

//=================================================================================================

// Standard_Boolean DNaming_DFandUS(char* a,
// 				 Handle(Data2)&           ND,
// 				 Handle(TNaming_UsedShapes)& US)
// {
//   Handle(DDF_Data) DND = Handle(DDF_Data)::DownCast (Draw1::Get(a));
//   if (DND.IsNull ()) return 0;
//   ND = DND->DataFramework ();
//   ND->Root().FindAttribute(TNaming_UsedShapes::GetID(),US);
//   return 1;
// }
//=================================================================================================

void DNaming1::GetShape(const Standard_CString  LabelName,
                       const Handle(Data2)& DF,
                       ShapeList&   L)
{
  L.Clear();
  DataLabel        Label;
  Standard_Boolean Found = DDF1::AddLabel(DF, LabelName, Label);
  if (Found)
  {
    Iterator1 it(Label, DF->Transaction());
    for (; it.More(); it.Next())
    {
      L.Append(it.NewShape());
    }
  }
}

//=================================================================================================

void DNaming_BuildMap(TDF_LabelMap& Updated, const DataLabel& Lab)
{
  ChildIterator it(Lab);
  for (; it.More(); it.Next())
  {
    Updated.Add(it.Value());
    DNaming_BuildMap(Updated, it.Value());
  }
}

//=================================================================================================

TopoShape DNaming1::CurrentShape(const Standard_CString LabelName, const Handle(Data2)& DF)
{
  TopoShape     S;
  DataLabel        Label;
  Standard_Boolean Found = DDF1::AddLabel(DF, LabelName, Label);
  if (!Found)
  {
#ifdef OCCT_DEBUG
    std::cout << "no labels" << std::endl;
#endif
    return S;
  }
  if (Found)
  {
    Handle(ShapeAttribute) NS;
    Label.FindAttribute(ShapeAttribute::GetID(), NS);
    S = Tool11::CurrentShape(NS);
    if (S.IsNull())
#ifdef OCCT_DEBUG
      std::cout << "current shape from " << LabelName << " is deleted" << std::endl;
#endif
    return S;
  }
  return S;
}

//=================================================================================================

AsciiString1 DNaming1::GetEntry(const TopoShape&     Shape,
                                          const Handle(Data2)& DF,
                                          Standard_Integer&       theStatus)
{
  theStatus = 0;
  // Handle(TNaming_UsedShapes) US;
  // DF->Root().FindAttribute(TNaming_UsedShapes::GetID(),US);

  if (!Tool11::HasLabel(DF->Root(), Shape))
  {
    return AsciiString1();
  }
  Standard_Integer        Transdef;
  DataLabel               Lab = Tool11::Label(DF->Root(), Shape, Transdef);
  AsciiString1 entry;
  Tool3::Entry(Lab, entry);
  // Update theStatus;
  Iterator1 it(Lab, DF->Transaction());
  for (; it.More(); it.Next())
  {
    theStatus++;
    if (theStatus == 2)
      break;
  }
  return entry;
}

//=================================================================================================

void DNaming1::AllCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  DNaming1::BasicCommands(theCommands);
  DNaming1::ToolsCommands(theCommands);
  DNaming1::SelectionCommands(theCommands);
  DNaming1::ModelingCommands(theCommands);
  // define the TCL variable Draw_NamingData
  const char* com = "set Draw_NamingData 1";
  theCommands.Eval(com);
}

//=======================================================================
//=======================================================================
// function : LoadC0Vertices
// purpose  : Method for internal use. It is used by Load() method.
//=======================================================================

static void LoadC0Vertices(const TopoShape& S, const Handle(TDF_TagSource)& Tagger)
{
  TopTools_DataMapOfShapeListOfShape vertexNaborFaces;
  ShapeList               empty;
  ShapeExplorer                    explF(S, TopAbs_FACE);
  for (; explF.More(); explF.Next())
  {
    const TopoShape& aFace = explF.Current();
    ShapeExplorer     explV(aFace, TopAbs_VERTEX);
    for (; explV.More(); explV.Next())
    {
      const TopoShape& aVertex = explV.Current();
      if (!vertexNaborFaces.IsBound(aVertex))
        vertexNaborFaces.Bind(aVertex, empty);
      Standard_Boolean                   faceIsNew = Standard_True;
      TopTools_ListIteratorOfListOfShape itrF(vertexNaborFaces.Find(aVertex));
      for (; itrF.More(); itrF.Next())
      {
        if (itrF.Value().IsSame(aFace))
        {
          faceIsNew = Standard_False;
          break;
        }
      }
      if (faceIsNew)
      {
        vertexNaborFaces.ChangeFind(aVertex).Append(aFace);
      }
    }
  }

  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itr(vertexNaborFaces);
  for (; itr.More(); itr.Next())
  {
    const ShapeList& naborFaces = itr.Value();
    if (naborFaces.Extent() < 3)
    {
      TNaming_Builder bC0Vertex(Tagger->NewChild());
      bC0Vertex.Generated(itr.Key1());
    }
  }
}

//=======================================================================
// function : LoadC0Edges
// purpose  : Method for internal use. It is used by Load() method.
//=======================================================================

static void LoadC0Edges(const TopoShape& S, const Handle(TDF_TagSource)& Tagger)
{
  TopTools_DataMapOfShapeListOfShape edgeNaborFaces;
  ShapeList               empty;
  ShapeExplorer                    explF(S, TopAbs_FACE);
  for (; explF.More(); explF.Next())
  {
    const TopoShape& aFace = explF.Current();
    ShapeExplorer     explV(aFace, TopAbs_EDGE);
    for (; explV.More(); explV.Next())
    {
      const TopoShape& anEdge = explV.Current();
      if (!edgeNaborFaces.IsBound(anEdge))
        edgeNaborFaces.Bind(anEdge, empty);
      Standard_Boolean                   faceIsNew = Standard_True;
      TopTools_ListIteratorOfListOfShape itrF(edgeNaborFaces.Find(anEdge));
      for (; itrF.More(); itrF.Next())
      {
        if (itrF.Value().IsSame(aFace))
        {
          faceIsNew = Standard_False;
          break;
        }
      }
      if (faceIsNew)
      {
        edgeNaborFaces.ChangeFind(anEdge).Append(aFace);
      }
    }
  }

  TopTools_MapOfShape anEdgesToDelete;
  // clang-format off
  ShapeExplorer anEx(S,TopAbs_EDGE); // mpv: new explorer iterator because we need keep edges order
  // clang-format on
  for (; anEx.More(); anEx.Next())
  {
    Standard_Boolean    aC0     = Standard_False;
    const TopoShape& anEdge1 = anEx.Current();
    if (edgeNaborFaces.IsBound(anEdge1))
    {
      const ShapeList& aList1 = edgeNaborFaces.Find(anEdge1);
      if (aList1.Extent() < 2)
        continue; // mpv (06.09.2002): these edges already was loaded
      TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itr(edgeNaborFaces);
      for (; itr.More(); itr.Next())
      {
        const TopoShape& anEdge2 = itr.Key1();
        if (anEdgesToDelete.Contains(anEdge2))
          continue;
        if (anEdge1.IsSame(anEdge2))
          continue;
        const ShapeList& aList2 = itr.Value();
        // compare lists of the neighbour faces of edge1 and edge2
        if (aList1.Extent() == aList2.Extent())
        {
          Standard_Integer aMatches = 0;
          for (TopTools_ListIteratorOfListOfShape aLIter1(aList1); aLIter1.More(); aLIter1.Next())
            for (TopTools_ListIteratorOfListOfShape aLIter2(aList2); aLIter2.More(); aLIter2.Next())
              if (aLIter1.Value().IsSame(aLIter2.Value()))
                aMatches++;
          if (aMatches == aList1.Extent())
          {
            aC0 = Standard_True;
            TNaming_Builder bC0Edge(Tagger->NewChild());
            bC0Edge.Generated(anEdge2);
            // edgeNaborFaces.UnBind(anEdge2);
            anEdgesToDelete.Add(anEdge2);
          }
        }
      }
      // VUN (10/2/2005) avoid UnBind during iterating -^
      TopTools_MapIteratorOfMapOfShape itDelete(anEdgesToDelete);
      for (; itDelete.More(); itDelete.Next())
      {
        edgeNaborFaces.UnBind(itDelete.Key1());
      }
      edgeNaborFaces.UnBind(anEdge1);
    }
    if (aC0)
    {
      TNaming_Builder bC0Edge(Tagger->NewChild());
      bC0Edge.Generated(anEdge1);
    }
  }
}

//
//=======================================================================
// function : GetDangleShapes
// purpose  : Returns dangle sub shapes Generator - Dangle.
//=======================================================================

static Standard_Boolean GetDangleShapes(const TopoShape&           ShapeIn,
                                        const TopAbs_ShapeEnum        GeneratedFrom,
                                        TopTools_DataMapOfShapeShape& Dangles)
{
  Dangles.Clear();
  TopTools_IndexedDataMapOfShapeListOfShape subShapeAndAncestors;
  TopAbs_ShapeEnum                          GeneratedTo;
  if (GeneratedFrom == TopAbs_FACE)
    GeneratedTo = TopAbs_EDGE;
  else if (GeneratedFrom == TopAbs_EDGE)
    GeneratedTo = TopAbs_VERTEX;
  else
    return Standard_False;
  TopExp1::MapShapesAndAncestors(ShapeIn, GeneratedTo, GeneratedFrom, subShapeAndAncestors);
  for (Standard_Integer i = 1; i <= subShapeAndAncestors.Extent(); i++)
  {
    const TopoShape&         mayBeDangle = subShapeAndAncestors.FindKey(i);
    const ShapeList& ancestors   = subShapeAndAncestors.FindFromIndex(i);
    if (ancestors.Extent() == 1)
      Dangles.Bind(ancestors.First(), mayBeDangle);
  }
  return !Dangles.IsEmpty();
}

//=================================================================================================

static void LoadGeneratedDangleShapes(const TopoShape&    ShapeIn,
                                      const TopAbs_ShapeEnum GeneratedFrom,
                                      TNaming_Builder&       Builder)
{
  TopTools_DataMapOfShapeShape dangles;
  if (!GetDangleShapes(ShapeIn, GeneratedFrom, dangles))
    return;
  TopTools_DataMapIteratorOfDataMapOfShapeShape itr(dangles);
  for (; itr.More(); itr.Next())
    Builder.Generated(itr.Key1(), itr.Value());
}

//=======================================================================
// function : LoadNextLevels
// purpose  : Method for internal use. Is used by LoadFirstLevel()
//=======================================================================

static void LoadNextLevels(const TopoShape& S, const Handle(TDF_TagSource)& Tagger)
{

  if (S.ShapeType() == TopAbs_SOLID)
  {
    ShapeExplorer aExp(S, TopAbs_FACE);
    for (; aExp.More(); aExp.Next())
    {
      TNaming_Builder bFace(Tagger->NewChild());
      bFace.Generated(aExp.Current());
    }
  }
  else if (S.ShapeType() == TopAbs_SHELL || S.ShapeType() == TopAbs_FACE)
  {
    // load faces and all the free edges
    TopTools_IndexedMapOfShape Faces;
    TopExp1::MapShapes(S, TopAbs_FACE, Faces);
    if (Faces.Extent() > 1 || (S.ShapeType() == TopAbs_SHELL && Faces.Extent() == 1))
    {
      ShapeExplorer aExp(S, TopAbs_FACE);
      for (; aExp.More(); aExp.Next())
      {
        TNaming_Builder bFace(Tagger->NewChild());
        bFace.Generated(aExp.Current());
      }
    }
    TopTools_IndexedDataMapOfShapeListOfShape anEdgeAndNeighbourFaces;
    TopExp1::MapShapesAndAncestors(S, TopAbs_EDGE, TopAbs_FACE, anEdgeAndNeighbourFaces);
    for (Standard_Integer i = 1; i <= anEdgeAndNeighbourFaces.Extent(); i++)
    {
      const ShapeList& aLL = anEdgeAndNeighbourFaces.FindFromIndex(i);
      if (aLL.Extent() < 2)
      {
        TNaming_Builder bFreeEdges(Tagger->NewChild());
        bFreeEdges.Generated(anEdgeAndNeighbourFaces.FindKey(i));
      }
      else
      {
        TopTools_ListIteratorOfListOfShape anIter(aLL);
        const TopoFace&                 aFace = TopoDS::Face(anIter.Value());
        anIter.Next();
        if (aFace.IsEqual(anIter.Value()))
        {
          TNaming_Builder bFreeEdges(Tagger->NewChild());
          bFreeEdges.Generated(anEdgeAndNeighbourFaces.FindKey(i));
        }
      }
    }
  }
  else if (S.ShapeType() == TopAbs_WIRE)
  {
    TopTools_IndexedMapOfShape Edges;
    BRepTools1::Map3DEdges(S, Edges);
    if (Edges.Extent() == 1)
    {
      TNaming_Builder bEdge(Tagger->NewChild());
      bEdge.Generated(Edges.FindKey(1));
      ShapeExplorer aExp(S, TopAbs_VERTEX);
      for (; aExp.More(); aExp.Next())
      {
        TNaming_Builder bVertex(Tagger->NewChild());
        bVertex.Generated(aExp.Current());
      }
    }
    else
    {
      ShapeExplorer aExp(S, TopAbs_EDGE);
      for (; aExp.More(); aExp.Next())
      {
        TNaming_Builder bEdge(Tagger->NewChild());
        bEdge.Generated(aExp.Current());
      }
      // and load generated vertices.
      TopTools_DataMapOfShapeShape generated;
      if (GetDangleShapes(S, TopAbs_EDGE, generated))
      {
        TNaming_Builder bGenVertices(Tagger->NewChild());
        LoadGeneratedDangleShapes(S, TopAbs_EDGE, bGenVertices);
      }
    }
  }
  else if (S.ShapeType() == TopAbs_EDGE)
  {
    ShapeExplorer aExp(S, TopAbs_VERTEX);
    for (; aExp.More(); aExp.Next())
    {
      TNaming_Builder bVertex(Tagger->NewChild());
      bVertex.Generated(aExp.Current());
    }
  }
}

//=======================================================================
// function : LoadFirstLevel
// purpose  : Method for internal use. Is used by Load()
//=======================================================================

static void LoadFirstLevel(const TopoShape& S, const Handle(TDF_TagSource)& Tagger)
{
  if (S.ShapeType() == TopAbs_COMPOUND || S.ShapeType() == TopAbs_COMPSOLID)
  {
    TopoDS_Iterator itr(S);
    for (; itr.More(); itr.Next())
    {
      TNaming_Builder bIndependentShapes(Tagger->NewChild());
      bIndependentShapes.Generated(itr.Value());
      if (itr.Value().ShapeType() == TopAbs_COMPOUND || itr.Value().ShapeType() == TopAbs_COMPSOLID)
      {
        LoadFirstLevel(itr.Value(), Tagger);
      }
      else
        LoadNextLevels(itr.Value(), Tagger);
    }
  }
  else
    LoadNextLevels(S, Tagger);
}

//=======================================================================
// function : Load
// purpose  : To load an ImportShape
//           Use this method for a topological naming of an imported shape
//=======================================================================

void DNaming1::LoadImportedShape(const DataLabel& theResultLabel, const TopoShape& theShape)
{
  theResultLabel.ForgetAllAttributes();
  TNaming_Builder aBuilder(theResultLabel);
  aBuilder.Generated(theShape);

  Handle(TDF_TagSource) aTagger = TDF_TagSource::Set(theResultLabel);
  if (aTagger.IsNull())
    return;
  aTagger->Set(0);

  LoadFirstLevel(theShape, aTagger);
  LoadC0Edges(theShape, aTagger);
  LoadC0Vertices(theShape, aTagger);
}

//=================================================================================================

void DNaming1::LoadPrime(const DataLabel& theResultLabel, const TopoShape& theShape)
{

  Handle(TDF_TagSource) aTagger = TDF_TagSource::Set(theResultLabel);
  if (aTagger.IsNull())
    return;
  aTagger->Set(0);

  LoadFirstLevel(theShape, aTagger);
  LoadC0Edges(theShape, aTagger);
  LoadC0Vertices(theShape, aTagger);
}

//
//=======================================================================
// function : Real
// purpose  : Gives the access to a real argument
//=======================================================================
Handle(TDataStd_Real) DNaming1::GetReal(const Handle(TFunction_Function)& theFunction,
                                       const Standard_Integer            thePosition)
{
  Handle(TDataStd_Real) aReal;
  if (!POSITION(theFunction, thePosition).FindAttribute(TDataStd_Real::GetID(), aReal))
    aReal = TDataStd_Real::Set(POSITION(theFunction, thePosition), 0.0);
  return aReal;
}

//=======================================================================
// function : Integer1
// purpose  : Give an access to integer attribute
//=======================================================================
Handle(IntAttribute) DNaming1::GetInteger(const Handle(TFunction_Function)& theFunction,
                                             const Standard_Integer            thePosition)
{
  Handle(IntAttribute) anInteger;
  if (!POSITION(theFunction, thePosition).FindAttribute(IntAttribute::GetID(), anInteger))
    anInteger = IntAttribute::Set(POSITION(theFunction, thePosition), 0);
  return anInteger;
}

//=======================================================================
// function : String
// purpose  : Returns Name attribute
//=======================================================================
Handle(NameAttribute) DNaming1::GetString(const Handle(TFunction_Function)& theFunction,
                                         const Standard_Integer            thePosition)
{
  Handle(NameAttribute) aString;
  if (!POSITION(theFunction, thePosition).FindAttribute(NameAttribute::GetID(), aString))
    aString = NameAttribute::Set(POSITION(theFunction, thePosition), "");
  return aString;
}

//=======================================================================
// function : GetResult
// purpose  : Returns a result of a function, which is stored on a second label
//=======================================================================
Handle(ShapeAttribute) DNaming1::GetFunctionResult(const Handle(TFunction_Function)& theFunction)
{
  Handle(ShapeAttribute) aNShape;
  theFunction->Label()
    .FindChild(FUNCTION_RESULT_LABEL)
    .FindAttribute(ShapeAttribute::GetID(), aNShape);
  return aNShape;
}

//=======================================================================
// function : Object
// purpose  : Returns UAttribute1 associated with Object
//=======================================================================
Handle(TDataStd_UAttribute) DNaming1::GetObjectArg(const Handle(TFunction_Function)& theFunction,
                                                  const Standard_Integer            thePosition)
{
  Handle(TDataStd_UAttribute) anObject;
  Handle(TDF_Reference)       aReference;
  if (POSITION(theFunction, thePosition).FindAttribute(TDF_Reference::GetID(), aReference))
    aReference->Get().FindAttribute(GEOMOBJECT_GUID, anObject);
  return anObject;
}

//=======================================================================
// function : SetObject
// purpose  : Replace the argument by new value.
//=======================================================================
void DNaming1::SetObjectArg(const Handle(TFunction_Function)&  theFunction,
                           const Standard_Integer             thePosition,
                           const Handle(TDataStd_UAttribute)& theNewValue)
{

  if (theNewValue.IsNull())
    return;
  TDF_Reference::Set(POSITION(theFunction, thePosition), theNewValue->Label());
}

//=======================================================================
// function : GetObjectValue
// purpose  : Returns NamedShape1 of the Object
//=======================================================================
Handle(ShapeAttribute) DNaming1::GetObjectValue(const Handle(TDataStd_UAttribute)& theObject)
{
  Handle(ShapeAttribute) aNS;

  if (!theObject.IsNull() && theObject->ID() == GEOMOBJECT_GUID)
  {

    Handle(TDF_Reference) aReference;
    if (theObject->FindAttribute(TDF_Reference::GetID(), aReference))
      aReference->Get().FindAttribute(ShapeAttribute::GetID(), aNS);
  }
  return aNS;

  /*
    Handle(TFunction_Function) aFun;
    Handle(TDataStd_TreeNode) aNode;
    objLabel.FindAttribute(TDataStd_TreeNode::GetDefaultTreeID(), aNode);
    if(aNode.IsNull()) return aFun;
    if(!aNode->HasFirst()) return aFun;
    else
      aNode = aNode->First();
    while(!aNode.IsNull()) {
      if(aNode->FindAttribute(TFunction_Function::GetID(), aFun)) {
        const Standard_GUID& aGUID = aFun->GetDriverGUID();
        if(aGUID == funGUID) break;
        else aFun.Nullify();
      }
      aNode = aNode->Next();
    }
  */
}

//=======================================================================
// function : GetPrevFunction
// purpose  : Returns previous function
//=======================================================================
Handle(TFunction_Function) DNaming1::GetPrevFunction(const Handle(TFunction_Function)& theFunction)
{
  Handle(TFunction_Function) aPrevFun;
  if (!theFunction.IsNull())
  {
    Handle(TDataStd_TreeNode) aNode;
    theFunction->FindAttribute(TDataStd_TreeNode::GetDefaultTreeID(), aNode);
    while (!aNode.IsNull())
    {
      if (!aNode->HasPrevious())
        return aPrevFun;
      else
        aNode = aNode->Previous();
      aNode->FindAttribute(TFunction_Function::GetID(), aPrevFun);
      if (!aPrevFun.IsNull())
        break;
    }
  }
  return aPrevFun;
  /*
      while(!aNode.IsNull()) {
      if(aNode->FindAttribute(TFunction_Function::GetID(), aFun)) {
        const Standard_GUID& aGUID = aFun->GetDriverGUID();
        if(aGUID == funGUID) break;
        else aFun.Nullify();
      }
      aNode = aNode->Next();
    }
  */
}

//=======================================================================
// function : GetFirstFunction
// purpose  : Returns first function
//=======================================================================
Handle(TFunction_Function) DNaming1::GetFirstFunction(const Handle(TDataStd_UAttribute)& theObject)
{
  Handle(TFunction_Function) aFirstFun;
  if (!theObject.IsNull())
  {
    Handle(TDataStd_TreeNode) aNode;
    theObject->FindAttribute(TDataStd_TreeNode::GetDefaultTreeID(), aNode);
    if (aNode.IsNull())
      return aFirstFun;
    if (!aNode->HasFirst())
      return aFirstFun;
    else
      aNode = aNode->First();

    while (!aNode.IsNull())
    {
      aNode->FindAttribute(TFunction_Function::GetID(), aFirstFun);
      if (!aFirstFun.IsNull())
        break;
      aNode = aNode->Next();
    }
  }
  return aFirstFun;
}

//=======================================================================
// function : GetLastFunction
// purpose  : Returns Last function
//=======================================================================
Handle(TFunction_Function) DNaming1::GetLastFunction(const Handle(TDataStd_UAttribute)& theObject)
{
  Handle(TFunction_Function) aLastFun;
  if (!theObject.IsNull())
  {
    Handle(TDataStd_TreeNode) aNode;
    theObject->FindAttribute(TDataStd_TreeNode::GetDefaultTreeID(), aNode);
    if (aNode.IsNull())
      return aLastFun;
    if (!aNode->HasFirst())
      return aLastFun;
    else
      aNode = aNode->First();

    while (!aNode.IsNull())
    {
      if (aNode->IsAttribute(TFunction_Function::GetID()))
        aNode->FindAttribute(TFunction_Function::GetID(), aLastFun);
      aNode = aNode->Next();
    }
  }
  return aLastFun;
}

//=======================================================================
// function : GetObjectFromFunction
// purpose  : Returns Object
//=======================================================================
Handle(TDataStd_UAttribute) DNaming1::GetObjectFromFunction(
  const Handle(TFunction_Function)& theFunction)
{
  Handle(TDataStd_UAttribute) anObject;
  if (!theFunction.IsNull())
  {
    Handle(TDataStd_TreeNode) aNode;
    theFunction->FindAttribute(TDataStd_TreeNode::GetDefaultTreeID(), aNode);
    if (!aNode.IsNull())
    {
      if (!aNode->HasFather())
        return anObject;
      else
        aNode = aNode->Father();
      aNode->FindAttribute(GEOMOBJECT_GUID, anObject);
    }
  }
  return anObject;
  /*
      while(!aNode.IsNull()) {
      if(aNode->FindAttribute(TFunction_Function::GetID(), aFun)) {
        const Standard_GUID& aGUID = aFun->GetDriverGUID();
        if(aGUID == funGUID) break;
        else aFun.Nullify();
      }
      aNode = aNode->Next();
    }
  */
}

//=================================================================================================

void DNaming1::LoadResult(const DataLabel& ResultLabel, BRepAlgoAPI_BooleanOperation& MS)
{
  Handle(TDF_TagSource) Tagger = TDF_TagSource::Set(ResultLabel);
  if (Tagger.IsNull())
    return;
  Tagger->Set(0);
  TNaming_Builder Builder(ResultLabel);
  TopoShape    aResult = MS.Shape();
  if (aResult.ShapeType() == TopAbs_COMPOUND)
  {
    if (aResult.NbChildren() == 1)
    {
      TopoDS_Iterator itr(aResult);
      if (itr.More())
        aResult = itr.Value();
    }
  }
  if (MS.Shape1().IsNull())
    Builder.Generated(aResult);
  else
  {
    Builder.Modify(MS.Shape1(), aResult);
  }
}

//=================================================================================================

void DNaming1::LoadAndOrientModifiedShapes(BRepBuilderAPI_MakeShape&           MS,
                                          const TopoShape&                 ShapeIn,
                                          const TopAbs_ShapeEnum              KindOfShape,
                                          TNaming_Builder&                    Builder,
                                          const TopTools_DataMapOfShapeShape& SubShapes)
{
  TopTools_MapOfShape View;
  ShapeExplorer     ShapeExplorer(ShapeIn, KindOfShape);
  for (; ShapeExplorer.More(); ShapeExplorer.Next())
  {
    const TopoShape& Root = ShapeExplorer.Current();
    if (!View.Add(Root))
      continue;
    const ShapeList&        Shapes = MS.Modified(Root);
    TopTools_ListIteratorOfListOfShape ShapesIterator(Shapes);
    for (; ShapesIterator.More(); ShapesIterator.Next())
    {
      TopoShape newShape = ShapesIterator.Value();
      if (SubShapes.IsBound(newShape))
      {
        newShape.Orientation((SubShapes(newShape)).Orientation());
      }
      if (!Root.IsSame(newShape))
        Builder.Modify(Root, newShape);
    }
  }
}

//=================================================================================================

void DNaming1::LoadDeletedShapes(BRepBuilderAPI_MakeShape& MS,
                                const TopoShape&       ShapeIn,
                                const TopAbs_ShapeEnum    KindOfShape,
                                TNaming_Builder&          Builder)
{
  TopTools_MapOfShape View;
  ShapeExplorer     ShapeExplorer(ShapeIn, KindOfShape);
  for (; ShapeExplorer.More(); ShapeExplorer.Next())
  {
    const TopoShape& Root = ShapeExplorer.Current();
    if (!View.Add(Root))
      continue;
    if (MS.IsDeleted(Root))
    {
      Builder.Delete(Root);
    }
  }
}

//=================================================================================================

void DNaming1::LoadAndOrientGeneratedShapes(BRepBuilderAPI_MakeShape&           MS,
                                           const TopoShape&                 ShapeIn,
                                           const TopAbs_ShapeEnum              KindOfShape,
                                           TNaming_Builder&                    Builder,
                                           const TopTools_DataMapOfShapeShape& SubShapes)
{
  TopTools_MapOfShape View;
  ShapeExplorer     ShapeExplorer(ShapeIn, KindOfShape);
  for (; ShapeExplorer.More(); ShapeExplorer.Next())
  {
    const TopoShape& Root = ShapeExplorer.Current();
    if (!View.Add(Root))
      continue;
    const ShapeList&        Shapes = MS.Generated(Root);
    TopTools_ListIteratorOfListOfShape ShapesIterator(Shapes);
    for (; ShapesIterator.More(); ShapesIterator.Next())
    {
      TopoShape newShape = ShapesIterator.Value();
      if (SubShapes.IsBound(newShape))
      {
        newShape.Orientation((SubShapes(newShape)).Orientation());
      }
      if (!Root.IsSame(newShape))
        Builder.Generated(Root, newShape);
    }
  }
}

//=======================================================================
// function : ComputeNormalizedVector
// purpose  : Computes normalized vector from shape if it is possible
//=======================================================================
Standard_Boolean DNaming1::ComputeAxis(const Handle(ShapeAttribute)& theNS, Axis3d& theAx1)
{
  if (theNS.IsNull() || theNS->IsEmpty())
    return Standard_False;
  TopoShape aShape = theNS->Get();
  if (aShape.IsNull())
    return Standard_False;
  if (aShape.ShapeType() == TopAbs_EDGE || aShape.ShapeType() == TopAbs_WIRE)
  {
    if (aShape.ShapeType() == TopAbs_WIRE)
    {
      ShapeExplorer anExplorer(aShape, TopAbs_EDGE);
      aShape = anExplorer.Current();
    }
    const TopoEdge& anEdge = TopoDS::Edge(aShape);
    Standard_Real      aFirst, aLast;
    Handle(GeomCurve3d) aCurve = BRepInspector::Curve(anEdge, aFirst, aLast);
    if (aCurve->IsKind(STANDARD_TYPE(GeomLine)))
    {
      Handle(GeomLine) aLine = Handle(GeomLine)::DownCast(aCurve);
      if (!aLine.IsNull())
      {
        theAx1 = aLine->Position1();
        return Standard_True;
      }
    }
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean DNaming1::IsAttachment(const Handle(TDataStd_UAttribute)& anObj)
{

  Handle(TFunction_Function) aFun = GetFirstFunction(anObj);
  if (!aFun.IsNull())
  {
    const Standard_GUID& aGUID = aFun->GetDriverGUID();
    if (aGUID == ATTCH_GUID || aGUID == XTTCH_GUID)
    {
      return aFun->Label()
        .FindChild(FUNCTION_ARGUMENTS_LABEL)
        .FindChild(ATTACH_ARG)
        .IsAttribute(TDF_Reference::GetID());
    }
  }
  return Standard_False;
}

//=================================================================================================

Handle(ShapeAttribute) DNaming1::GetAttachmentsContext(const Handle(TDataStd_UAttribute)& anObj)
{
  Handle(ShapeAttribute) aNS;
  Handle(TFunction_Function) aFun = GetFirstFunction(anObj);
  if (!aFun.IsNull())
  {
    const Standard_GUID& aGUID = aFun->GetDriverGUID();
    if (aGUID == ATTCH_GUID)
    {
      const DataLabel& aLabel =
        aFun->Label().FindChild(FUNCTION_ARGUMENTS_LABEL).FindChild(ATTACH_ARG);
      Handle(TDF_Reference)      aRef;
      Handle(TFunction_Function) aFunCnt;
      if (aLabel.FindAttribute(TDF_Reference::GetID(), aRef))
      {
        if (aRef->Get().FindAttribute(TFunction_Function::GetID(), aFunCnt))
        {
          const DataLabel& aResultLabel =
            aFunCnt->Label().FindChild(FUNCTION_RESULT_LABEL, Standard_True);
          aResultLabel.FindAttribute(ShapeAttribute::GetID(), aNS);
        }
      }
    }
  }
  return aNS;
}

//=======================================================================
// function : ComputeSweepDir
// purpose  : Computes direction for extrusion
//=======================================================================
Standard_Boolean DNaming1::ComputeSweepDir(const TopoShape& theShape, Axis3d& theAxis)
{
  // Find surface
  Handle(GeomPlane) aPlane;

  if (theShape.ShapeType() == TopAbs_FACE)
  {
    Handle(GeomSurface) aSurf = BRepInspector::Surface(TopoDS::Face(theShape));
#ifdef OCCT_DEBUG
    Standard_CString s = aSurf->DynamicType()->Name();
    std::cout << "Surface Dynamic TYPE = " << s << std::endl;
#endif
    if (aSurf->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
      aSurf = Handle(Geom_RectangularTrimmedSurface)::DownCast(aSurf)->BasisSurface();
    aPlane = Handle(GeomPlane)::DownCast(aSurf);
  }

  if (aPlane.IsNull())
  {
    BRepLib_FindSurface aFinder(theShape, 0., Standard_True);
    if (!aFinder.Found())
      return Standard_False;
    aPlane = Handle(GeomPlane)::DownCast(aFinder.Surface());
  }

  if (aPlane.IsNull())
    return Standard_False;

  theAxis = aPlane->Pln().Axis();
  if (!aPlane->Pln().Direct())
    theAxis.Reverse();

  if (theShape.Orientation() == TopAbs_REVERSED)
    theAxis.Reverse();

  return Standard_True;
}
