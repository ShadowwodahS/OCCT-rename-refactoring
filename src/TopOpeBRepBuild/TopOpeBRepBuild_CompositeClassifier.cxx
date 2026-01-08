// Created on: 1996-01-05
// Created by: Jean Yves LEBEY
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

#include <BRepTools.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRepBuild_BlockBuilder.hxx>
#include <TopOpeBRepBuild_CompositeClassifier.hxx>
#include <TopOpeBRepBuild_Loop.hxx>

#define MYBB ((BlockBuilder*)myBlockBuilder)

// sourvenir d'un raise sur FrozenShape lors du Add(myShell,aFace)
// avec un shell qui a ete deja ete place dans le solide interne du
// TopOpeBRepTool_SolidClassifier par LoadShell.

#ifdef OCCT_DEBUG
// static Standard_Integer dddjyl = 0;
// static Standard_Integer dddebi = 0;
// static Standard_Integer dddebi2 = 0;
// static void SAVSS(const TopoShape& S1,const TopoShape& S2)
//{
//   AsciiString1 aname_1("cc_1"), aname_2("cc_2");
//   Standard_CString name_1 = aname_1.ToCString(), name_2 = aname_2.ToCString();
//   std::cout<<"compositeclassifier : "<<name_1<<","<<name_2<<std::endl;
//   BRepTools1::Write(S1,name_1); BRepTools1::Write(S2,name_2);
// }
#endif

//=================================================================================================

TopOpeBRepBuild_CompositeClassifier::TopOpeBRepBuild_CompositeClassifier(
  const BlockBuilder& BB)
    : myBlockBuilder((void*)&BB)
{
}

//=================================================================================================

TopAbs_State TopOpeBRepBuild_CompositeClassifier::Compare(const Handle(TopOpeBRepBuild_Loop)& L1,
                                                          const Handle(TopOpeBRepBuild_Loop)& L2)
{
  TopAbs_State state = TopAbs_UNKNOWN;

  Standard_Boolean isshape1 = L1->IsShape();
  Standard_Boolean isshape2 = L2->IsShape();

  if (isshape2 && isshape1)
  { // L1 is Shape , L2 is Shape
    const TopoShape& s1 = L1->Shape();
    const TopoShape& s2 = L2->Shape();
    state                  = CompareShapes(s1, s2);
  }
  else if (isshape2 && !isshape1)
  { // L1 is Block1 , L2 is Shape
    TopOpeBRepBuildBlockIterator Bit1 = L1->BlockIterator();
    Bit1.Initialize();
    Standard_Boolean yena1 = Bit1.More();
    while (yena1)
    {
      const TopoShape& s1 = MYBB->Element(Bit1);
      const TopoShape& s2 = L2->Shape();
      state                  = CompareElementToShape(s1, s2);
      yena1                  = Standard_False;
      if (state == TopAbs_UNKNOWN)
      {
        if (Bit1.More())
          Bit1.Next();
        yena1 = Bit1.More();
      }
    }
  }
  else if (!isshape2 && isshape1)
  { // L1 is Shape , L2 is Block1
    const TopoShape& s1 = L1->Shape();
    ResetShape(s1);
    TopOpeBRepBuildBlockIterator Bit2 = L2->BlockIterator();
    for (Bit2.Initialize(); Bit2.More(); Bit2.Next())
    {
      const TopoShape& s2 = MYBB->Element(Bit2);
      if (!CompareElement(s2))
        break;
    }
    state = State();
  }
  else if (!isshape2 && !isshape1)
  { // L1 is Block1 , L2 is Block1
    TopOpeBRepBuildBlockIterator Bit1 = L1->BlockIterator();
    Bit1.Initialize();
    Standard_Boolean yena1 = Bit1.More();
    while (yena1)
    {
      const TopoShape& s1 = MYBB->Element(Bit1);
      ResetElement(s1);
      TopOpeBRepBuildBlockIterator Bit2 = L2->BlockIterator();
      for (Bit2.Initialize(); Bit2.More(); Bit2.Next())
      {
        const TopoShape& s2 = MYBB->Element(Bit2);
        CompareElement(s2);
      }
      state = State();
      yena1 = Standard_False;
      if (state == TopAbs_UNKNOWN)
      {
        if (Bit1.More())
          Bit1.Next();
        yena1 = Bit1.More();
      }
    }
  }

  return state;
}
