// Created on: 1999-09-30
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

#include <TDF_ChildIterator.hxx>
#include <TDF_IDFilter.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TNaming.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_Identifier.hxx>
#include <TNaming_NamedShape.hxx>
#include <TNaming_NameType.hxx>
#include <TNaming_Naming.hxx>
#include <TNaming_NamingTool.hxx>
#include <TNaming_NewShapeIterator.hxx>
#include <TNaming_Selector.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

// #define MDTV_DEB_SEL
#ifdef OCCT_DEBUG_SEL
  // #define MDTV_DEB_BNP
  #include <TopExp_Explorer.hxx>
  #include <TCollection_AsciiString.hxx>
  #include <TNaming_Tool.hxx>
  #include <BRep_Tool.hxx>
  #include <TopoDS.hxx>
  #include <TNaming_UsedShapes.hxx>

void PrintEntry(const DataLabel& label, const Standard_Boolean allLevels)
{
  AsciiString1 entry;
  Tool3::Entry(label, entry);
  std::cout << "LabelEntry = " << entry << std::endl;
  if (allLevels)
  {
    ChildIterator it(label, allLevels);
    for (; it.More(); it.Next())
    {
      Tool3::Entry(it.Value(), entry);
      std::cout << "ChildLabelEntry = " << entry << std::endl;
    }
  }
}

  #include <BRepTools.hxx>

static void Write(const TopoShape& shape, const Standard_CString filename)
{
  char buf[256];
  if (strlen(filename) > 255)
    return;
  #if defined _MSC_VER
  strcpy_s(buf, filename);
  #else
  strcpy(buf, filename);
  #endif
  char* p = buf;
  while (*p)
  {
    if (*p == ':')
      *p = '-';
    p++;
  }
  std::ofstream save(buf);
  if (!save)
    std::cout << "File " << buf << " was not created: rdstate = " << save.rdstate() << std::endl;
  save << "DBRep_DrawableShape" << std::endl << std::endl;
  if (!shape.IsNull())
    BRepTools1::Write(shape, save);
  save.close();
}
#endif

#define ORIENTATION_DSOPT
#ifdef ORIENTATION_DSOPT
  #include <TopTools_MapIteratorOfMapOfOrientedShape.hxx>
  #include <TDF_ChildIDIterator.hxx>
  #include <TNaming_Tool.hxx>
  #include <TNaming_Iterator.hxx>

//==========================================================================================
inline static void MapOfOrientedShapes(const TopoShape& S, TopTools_MapOfOrientedShape& M)
{
  M.Add(S);
  TopoDS_Iterator It(S, Standard_True, Standard_True);
  while (It.More())
  {
    MapOfOrientedShapes(It.Value(), M);
    It.Next();
  }
}

//=======================================================================
static void BuildAtomicMap(const TopoShape& S, TopTools_MapOfOrientedShape& M)
{
  if (S.ShapeType() > TopAbs_COMPSOLID)
    return;
  TopoDS_Iterator it(S);
  for (; it.More(); it.Next())
  {
    if (it.Value().ShapeType() > TopAbs_COMPSOLID)
      M.Add(it.Value());
    else
      BuildAtomicMap(it.Value(), M);
  }
}

//==========================================================================================
static const Handle(ShapeAttribute) FindPrevNDS(const Handle(ShapeAttribute)& CNS)
{
  Handle(ShapeAttribute) aNS;
  Iterator1           it(CNS);
  if (it.More())
  {
    if (!it.OldShape().IsNull())
    {
      aNS = Tool11::NamedShape1(it.OldShape(), CNS->Label());
      return aNS;
    }
  }
  return aNS;
}

//==========================================================================================
// Purpose: checks correspondens between orientation of sub-shapes of Context and orientation
//          of sub-shapes registered in DF and put under result label
//==========================================================================================
static Standard_Boolean IsSpecificCase(const DataLabel& F, const TopoShape& Context)
{
  Standard_Boolean            isFound(Standard_False);
  TopTools_MapOfOrientedShape shapesOfContext;
  MapOfOrientedShapes(Context, shapesOfContext);
  Handle(ShapeAttribute) CNS = Tool11::NamedShape1(Context, F);
  #ifdef OCCT_DEBUG_BNP
  PrintEntry(CNS->Label(), 0);
  #endif
  if (!CNS.IsNull())
  {
    TNaming_ListOfNamedShape aLNS;
    TDF_ChildIDIterator      cit(CNS->Label(), ShapeAttribute::GetID(), Standard_False);
    if (!cit.More())
    {
      // Naming1 data structure is empty - no sub-shapes under resulting shape
      // clang-format off
      const Handle(ShapeAttribute) aNS = FindPrevNDS(CNS); //look to old shape data structure if exist
      // clang-format on
      if (!aNS.IsNull())
      {
  #ifdef OCCT_DEBUG_BNP
        PrintEntry(aNS->Label(), 0);
  #endif
        cit.Initialize(aNS->Label(), ShapeAttribute::GetID(), Standard_False);
      }
      else
        return Standard_True;
    }

    for (; cit.More(); cit.Next())
    {
      Handle(ShapeAttribute) NS(Handle(ShapeAttribute)::DownCast(cit.Value()));
      if (!NS.IsNull())
      {
        TopoShape aS = Tool11::CurrentShape(NS);
        if (aS.IsNull())
          continue;
  #ifdef OCCT_DEBUG_BNP
        PrintEntry(NS->Label(), 0);
        std::cout << "ShapeType =" << aS.ShapeType() << std::endl;
        Write(aS, "BNProblem.brep");
  #endif
        if (aS.ShapeType() != TopAbs_COMPOUND)
        { // single shape at the child label
          if (!shapesOfContext.Contains(aS))
          {
            isFound = Standard_True;
            break;
          }
        }
        else
        {
          TopTools_MapOfOrientedShape M;
          BuildAtomicMap(aS, M);
          TopTools_MapIteratorOfMapOfOrientedShape it(M);
          for (; it.More(); it.Next())
          {
            if (!shapesOfContext.Contains(it.Key1()))
            {
  #ifdef OCCT_DEBUG_BNP
              std::cout << "BNProblem: ShapeType in AtomicMap = " << it.Key1().ShapeType()
                        << " TShape = " << it.Key1().TShape() << " OR = " << it.Key1().Orientation()
                        << std::endl;
              Write(it.Key1(), "BNProblem_AtomicMap_Item.brep");
              TopTools_MapIteratorOfMapOfOrientedShape itC(shapesOfContext);
              for (; itC.More(); itC.Next())
                std::cout << " ShapeType = " << itC.Key1().ShapeType()
                          << " TShape = " << itC.Key1().TShape()
                          << " OR = " << itC.Key1().Orientation() << std::endl;

  #endif
              isFound = Standard_True;
              break;
            }
            if (isFound)
              break;
          }
        }
      }
    }
  }
  return isFound;
}

//==========================================================================================
static Standard_Boolean IsSpecificCase2(const DataLabel& F, const TopoShape& Selection)
{
  Standard_Boolean isTheCase(Standard_False);
  if (Selection.ShapeType() == TopAbs_EDGE)
  {
    Handle(ShapeAttribute) aNS = Tool11::NamedShape1(Selection, F);
    if (!aNS.IsNull())
    { // presented in DF
  #ifdef OCCT_DEBUG_BNP
      PrintEntry(aNS->Label(), 0);
  #endif
      const TopoShape& aS = Tool11::CurrentShape(aNS);
      if (!aS.IsNull() && aS.ShapeType() == Selection.ShapeType())
      {
        if (Selection.Orientation() != aS.Orientation())
        {
          isTheCase = Standard_True;
        }
      }
    }
  }
  return isTheCase;
}
#endif
//=======================================================================
// function : FindGenerated
// purpose  : Finds all generated from the <S>
//=======================================================================

static void FindGenerated(const Handle(ShapeAttribute)& NS,
                          const TopoShape&               S,
                          ShapeList&             theList)

{
  const DataLabel& LabNS = NS->Label();
  for (NewShapeIterator it(S, LabNS); it.More(); it.Next())
  {
    if (it.Label() == LabNS)
    {
      theList.Append(it.Shape());
    }
  }
}

//=================================================================================================

Standard_Boolean TNaming_Selector::IsIdentified(const DataLabel&            L,
                                                const TopoShape&         Selection,
                                                Handle(ShapeAttribute)& NS,
                                                const Standard_Boolean      Geometry1)
{
  TopoShape       Context;
  Standard_Boolean   OnlyOne = !Geometry1;
  TNaming_Identifier Ident(L, Selection, Context, OnlyOne);
  if (Ident.IsFeature())
  {
    if (!OnlyOne)
      return Standard_False;
    else
    {
      NS = Ident.FeatureArg();

      // mpv : external condition
      TDF_LabelMap               Forbiden, Valid;
      TopTools_IndexedMapOfShape MS;
      NamingTool1::CurrentShape(Valid, Forbiden, NS, MS);
      return (MS.Contains(Selection) && MS.Extent() == 1);
    }
  }
  else if (Ident.Type() == TNaming_GENERATION)
  {
    NS = Ident.NamedShapeOfGeneration();
    if (!NS.IsNull())
    {
      TDF_LabelMap               Forbiden, Valid;
      TopTools_IndexedMapOfShape MS;
      NamingTool1::CurrentShape(Valid, Forbiden, NS, MS);
      if (MS.Contains(Selection) && MS.Extent() == 1)
      {
        const TopoShape&  aS = Ident.ShapeArg();
        ShapeList aList;
        FindGenerated(NS, aS, aList);
        Ident.NextArg();
        while (Ident.MoreArgs())
        {
          const TopoShape& aShape = Ident.ShapeArg();
          FindGenerated(NS, aShape, aList);
          Ident.NextArg();
        }
        const TopoShape&                aC = MS(1);
        Standard_Boolean                   isEq(Standard_False);
        TopTools_ListIteratorOfListOfShape itl(aList);
        for (; itl.More(); itl.Next())
        {
          if (itl.Value() == aC)
            isEq = Standard_True;
          else
          {
            isEq = Standard_False;
            break;
          }
        }
        return isEq;
      }
    }
    else
      return Standard_False;
  }
  return Standard_False;
}

//=================================================================================================

TNaming_Selector::TNaming_Selector(const DataLabel& L)
{
  myLabel = L;
}

//=================================================================================================

Standard_Boolean TNaming_Selector::Select(const TopoShape&    Selection,
                                          const TopoShape&    Context,
                                          const Standard_Boolean Geometry1,
                                          const Standard_Boolean KeepOrientation) const
{
  myLabel.ForgetAllAttributes();
  Handle(ShapeAttribute) NS;
  Standard_Boolean aKeepOrientation((Selection.ShapeType() == TopAbs_VERTEX) ? Standard_False
                                                                             : KeepOrientation);
  if (Selection.ShapeType() == TopAbs_COMPOUND)
  {
    Standard_Boolean isVertex(Standard_True);
    TopoDS_Iterator  it(Selection);
    for (; it.More(); it.Next())
      if (it.Value().ShapeType() != TopAbs_VERTEX)
      {
        isVertex = Standard_False;
        break;
      }
    if (isVertex)
      aKeepOrientation = Standard_False;
  }
  /*
 // for debug opposite orientation
 TopoShape selection;
 Standard_Boolean found(Standard_False);
 ShapeExplorer exp(Context,TopAbs_EDGE);
 for(;exp.More();exp.Next()) {
   TopoShape E = exp.Current();
   if(E.IsSame(Selection) && E.Orientation() != Selection.Orientation()) {
     selection = E;
   found = Standard_True;
   std::cout <<" FOUND: Entity orientation = " << selection.Orientation() <<std::endl;
   }
 }
 if (!found)
   selection = Selection;
  */

#ifdef OCCT_DEBUG_SEL
  std::cout << "SELECTION ORIENTATION = " << Selection.Orientation()
            << ", TShape = " << Selection.TShape() << std::endl;
  // std::cout << "SELECTION ORIENTATION = " << selection.Orientation() <<", TShape = " <<
  // selection.TShape() <<std::endl;
  PrintEntry(myLabel, 0);
  TNaming1::Print(myLabel, std::cout);
#endif

  if (aKeepOrientation)
  {
#ifdef ORIENTATION_DSOPT
    const Standard_Boolean aBNproblem =
      IsSpecificCase(myLabel, Context) || IsSpecificCase2(myLabel, Selection);

    NS = TNaming_Naming::Name(myLabel, Selection, Context, Geometry1, aKeepOrientation, aBNproblem);
#else
    NS = TNaming_Naming::Name(myLabel, Selection, Context, Geometry1, aKeepOrientation);
#endif
  }
  else if (!IsIdentified(myLabel, Selection, NS, Geometry1))
  {
    NS = TNaming_Naming::Name(myLabel, Selection, Context, Geometry1, aKeepOrientation);
  }
  if (NS.IsNull())
    return Standard_False;
  //
  // namedshape with SELECTED Evolution
  //
  TNaming_Builder B(myLabel);
  // mpv: if oldShape for selection is some shape from used map of shapes,
  //      then naming structure becomes more complex1, can be cycles
  const TopoShape& aSelection = Tool11::CurrentShape(NS); // szy
#ifdef OCCT_DEBUG_CHECK_TYPE
  if (!Selection.IsSame(aSelection) && Selection.ShapeType() != TopAbs_COMPOUND)
  {
    AsciiString1 entry;
    Tool3::Entry(NS->Label(), entry);
    std::cout << "Selection is Not Same (NSLabel = " << entry
              << "): TShape1 = " << Selection.TShape()->This()
              << " TShape2 = " << aSelection.TShape()->This() << std::endl;
  }
#endif
  if (aSelection.ShapeType() == TopAbs_COMPOUND && aSelection.ShapeType() != Selection.ShapeType())
    B.Select(aSelection, aSelection); // type migration
  else
    B.Select(Selection, Selection);
  //
  // naming with IDENTITY NameType
  //
  Handle(TNaming_Naming) N = new TNaming_Naming();
  N->ChangeName().Type(TNaming_IDENTITY);
  N->ChangeName().Append(NS);
  N->ChangeName().Orientation(Selection.Orientation());
  // inserted by vro 06.09.00:
  N->ChangeName().ShapeType(Selection.ShapeType());

  myLabel.AddAttribute(N);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TNaming_Selector::Select(const TopoShape&    Selection,
                                          const Standard_Boolean Geometry1,
                                          const Standard_Boolean KeepOrientation) const
{
  // we give a Null shape. How to guess what is the good context ?
  TopoShape Context;
  //  return Select (Selection,Context,Geometry1);
  // temporary!!!
  return Select(Selection, Selection, Geometry1, KeepOrientation);
}

//=================================================================================================

Standard_Boolean TNaming_Selector::Solve(TDF_LabelMap& Valid) const
{
  Handle(TNaming_Naming) name;
#ifdef OCCT_DEBUG_SEL
  std::cout << "TNaming_Selector::Solve==> ";
  PrintEntry(myLabel, 0);
#endif
  if (myLabel.FindAttribute(TNaming_Naming::GetID(), name))
  {
    return name->Solve(Valid);
  }
  return Standard_False;
}

//=================================================================================================

void TNaming_Selector::Arguments(TDF_AttributeMap& args) const
{
  Tool3::OutReferences(myLabel, args);
}

//=================================================================================================

Handle(ShapeAttribute) TNaming_Selector::NamedShape1() const
{
  Handle(ShapeAttribute) NS;
  myLabel.FindAttribute(ShapeAttribute::GetID(), NS);
  return NS;
}
