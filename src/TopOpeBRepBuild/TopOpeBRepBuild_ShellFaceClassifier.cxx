// Created on: 1993-06-17
// Created by: Jean Yves LEBEY
// Copyright (c) 1993-1999 Matra Datavision
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

#include <BRepAdaptor_Surface.hxx>
#include <Precision.hxx>
#include <Standard_ProgramError.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRepBuild_BlockBuilder.hxx>
#include <TopOpeBRepBuild_ShellFaceClassifier.hxx>

#ifdef OCCT_DEBUG
extern Standard_Boolean TopOpeBRepBuild_GettraceCHK();
#endif

//=================================================================================================

TopOpeBRepBuild_ShellFaceClassifier::TopOpeBRepBuild_ShellFaceClassifier(
  const BlockBuilder& BB)
    : TopOpeBRepBuild_CompositeClassifier(BB)
{
}

//=================================================================================================

void TopOpeBRepBuild_ShellFaceClassifier::Clear()
{
  mySolidClassifier.Clear();
  myFaceShellMap.Clear();
}

//=================================================================================================

TopAbs_State TopOpeBRepBuild_ShellFaceClassifier::CompareShapes(const TopoShape& B1,
                                                                const TopoShape& B2)
{
#ifdef OCCT_DEBUG
//  const TopAbs_ShapeEnum t1 = B1.ShapeType();
//  const TopAbs_ShapeEnum t2 = B2.ShapeType();
#endif

  ResetShape(B1);
  myShell = TopoDS::Shell(B2);
  mySolidClassifier.LoadShell(myShell);
  //  myShell.Free(Standard_True);
  TopAbs_State state = State();
  return state;
}

//=================================================================================================

TopAbs_State TopOpeBRepBuild_ShellFaceClassifier::CompareElementToShape(const TopoShape& F,
                                                                        const TopoShape& SHE)
{
#ifdef OCCT_DEBUG
//  const TopAbs_ShapeEnum t1 = F.ShapeType();
//  const TopAbs_ShapeEnum t2 = SHE.ShapeType();
#endif

  ResetElement(F);
  myShell = TopoDS::Shell(SHE);
  mySolidClassifier.LoadShell(myShell);
  //  myShell.Free(Standard_True);
  TopAbs_State state = State();
  return state;
}

//=================================================================================================

void TopOpeBRepBuild_ShellFaceClassifier::ResetShape(const TopoShape& SHE)
{
#ifdef OCCT_DEBUG
//  const TopAbs_ShapeEnum t1 = SHE.ShapeType();
#endif

  ShapeExplorer    ex(SHE, TopAbs_FACE);
  const TopoFace& F = TopoDS::Face(ex.Current());
  ResetElement(F);
}

//=================================================================================================

void TopOpeBRepBuild_ShellFaceClassifier::ResetElement(const TopoShape& F)
{
  const TopAbs_ShapeEnum t = F.ShapeType();

  // initialize myPoint3d with first vertex of face <E>
  myFirstCompare = Standard_True;

  ShapeExplorer ex(F, TopAbs_VERTEX);
  if (ex.More())
  {
    const TopoVertex& V = TopoDS::Vertex(ex.Current());
    myPoint3d              = BRepInspector::Pnt(V);
  }
  else
  {
    if (t == TopAbs_FACE)
    {
      BRepAdaptor_Surface BAS(TopoDS::Face(F));
      myPoint3d = BAS.Value(.5 * (BAS.FirstUParameter() + BAS.LastUParameter()),
                            .5 * (BAS.FirstVParameter() + BAS.LastVParameter()));
    }
    else
    {
      myPoint3d.SetCoord(0., 0., 0.);
    }
  }
}

//=================================================================================================

Standard_Boolean TopOpeBRepBuild_ShellFaceClassifier::CompareElement(const TopoShape& F)
{
#ifdef OCCT_DEBUG
//  const TopAbs_ShapeEnum t = F.ShapeType();
#endif
  Standard_Boolean bRet = Standard_True;
  //
  if (myFirstCompare)
  {
    Standard_Boolean found = myFaceShellMap.IsBound(F);
    if (!found)
    {
      // la face F est la premiere d'un bloc de faces
      // la face F d'un bloc de faces (F est supposee n'appartenir qu'a
      // un seul shell) n'a pas encore ete rencontree <=> le bloc de faces
      // dont F fait partie n'a pas encore ete shellise
      // on cree un shell et on l'attache a F
      myBuilder.MakeShell(myShell);
      myBuilder.Add(myShell, F);
      myFaceShellMap.Bind(F, myShell);
    }
    else
    {
      // la face F est la premiere d'un bloc de faces
      // on recupere le shell correspondant au bloc de faces (shellise)
      // dont F fait partie.
      TopoShape sbid = myFaceShellMap.Find(F);
      myShell           = TopoDS::Shell(sbid);
      bRet              = !bRet;
    }
    myFirstCompare = Standard_False;
  }
  else
  {
    // F n'est pas la premiere face d'un bloc.
    // myShell est necessairement defini et represente le bloc de faces
    // dont F fait partie
    myBuilder.Add(myShell, F);
  }
  return bRet;
}

#ifdef OCCT_DEBUG
  #include <BRepTools.hxx>
  #include <BRep_Builder.hxx>
  #include <TCollection_AsciiString.hxx>
static Standard_Integer STATIC_ishell = 0;
#endif

//=================================================================================================

TopAbs_State TopOpeBRepBuild_ShellFaceClassifier::State()
{
  TopAbs_State  state;
  Standard_Real tol3d = Precision::Confusion();

#ifdef OCCT_DEBUG
  if (TopOpeBRepBuild_GettraceCHK())
  {
    STATIC_ishell++;
    AsciiString1 home("/home/wb/mdl/gti/prod/TTOPOPE/src/test/data/");
    AsciiString1 sname("shell_");
    sname = home + sname + STATIC_ishell;
    AsciiString1 vname("vertex_");
    vname = home + vname + STATIC_ishell;
    ShapeBuilder  B;
    TopoVertex V;
    B.MakeVertex(V, myPoint3d, tol3d);
    std::cout << "TopOpeBRepBuild_ShellFaceClassifier : write shell " << sname;
    std::cout << " vertex " << vname << std::endl;
    BRepTools1::Write(myShell, sname.ToCString());
    BRepTools1::Write(V, vname.ToCString());
  }
#endif
  mySolidClassifier.Classify(myShell, myPoint3d, tol3d);
  state = mySolidClassifier.State();
  return state;
}
