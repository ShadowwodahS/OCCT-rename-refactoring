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

#include <BRepCheck_Analyzer.hxx>
#include <Standard_ProgramError.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRepBuild_define.hxx>
#include <TopOpeBRepBuild_ShapeSet.hxx>

#ifdef DRAW
  #include <DBRep.hxx>
  #include <DBRep_DrawableShape.hxx>
  #include <TopoDS_Iterator.hxx>
  #include <TopTools_IndexedMapOfShape.hxx>
static AsciiString1 PRODINS("dins ");

static void ShapeEnumToString(const TopAbs_ShapeEnum T, AsciiString1& N)
{
  if (T == TopAbs_SHAPE)
    N = "s";
  else if (T == TopAbs_COMPOUND)
    N = "co";
  else if (T == TopAbs_COMPSOLID)
    N = "cs";
  else if (T == TopAbs_SOLID)
    N = "so";
  else if (T == TopAbs_SHELL)
    N = "sh";
  else if (T == TopAbs_FACE)
    N = "f";
  else if (T == TopAbs_WIRE)
    N = "w";
  else if (T == TopAbs_EDGE)
    N = "e";
  else if (T == TopAbs_VERTEX)
    N = "v";
}
#endif

#ifdef OCCT_DEBUG
extern Standard_Boolean TopOpeBRepBuild_GettraceCHK();
extern Standard_Boolean TopOpeBRepBuild_GettraceCHKOK();
extern Standard_Boolean TopOpeBRepBuild_GettraceCHKNOK();

Standard_EXPORT void debaddss() {}

Standard_IMPORT TopOpeBRepBuild_Builder* LOCAL_PBUILDER_DEB;
#endif

//=================================================================================================

TopOpeBRepBuild_ShapeSet::TopOpeBRepBuild_ShapeSet(const TopAbs_ShapeEnum SubShapeType,
                                                   const Standard_Boolean checkshape)
    : mySubShapeType(SubShapeType),
      myCheckShape(checkshape)
{
  if (SubShapeType == TopAbs_EDGE)
    myShapeType = TopAbs_FACE;
  else if (SubShapeType == TopAbs_VERTEX)
    myShapeType = TopAbs_EDGE;
  else
    throw Standard_ProgramError("ShapeSet : bad ShapeType");
  myDEBNumber = 0;

  myCheckShape = Standard_False; // temporary NYI
}

TopOpeBRepBuild_ShapeSet::~TopOpeBRepBuild_ShapeSet() {}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::AddShape(const TopoShape& S)
{
  Standard_Boolean chk = CheckShape(S);
#ifdef OCCT_DEBUG
  DumpCheck(std::cout, " AddShape", S, chk);
#endif

  if (!chk)
    return;
  ProcessAddShape(S);
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::AddStartElement(const TopoShape& S)
{
  Standard_Boolean chk = CheckShape(S);
#ifdef OCCT_DEBUG
  DumpCheck(std::cout, " AddStartElement", S, chk);
#endif

  if (!chk)
    return;
  ProcessAddStartElement(S);
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::AddElement(const TopoShape& S)
{
  Standard_Boolean chk = CheckShape(S);
#ifdef OCCT_DEBUG
  DumpCheck(std::cout, " AddElement", S, chk);
#endif

  if (!chk)
    return;
  ProcessAddElement(S);
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::ProcessAddShape(const TopoShape& S)
{
  if (!myOMSH.Contains(S))
  {
    myOMSH.Add(S);
    myShapes.Append(S);
  }
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::ProcessAddStartElement(const TopoShape& S)
{
  if (!myOMSS.Contains(S))
  {
    myOMSS.Add(S);
    myStartShapes.Append(S);
    ProcessAddElement(S);
  }
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::ProcessAddElement(const TopoShape& S)
{
  if (!myOMES.Contains(S))
  {
    myOMES.Add(S);
    ShapeList         Lemp;
    TopOpeBRepTool_ShapeExplorer Ex(S, mySubShapeType);
    for (; Ex.More(); Ex.Next())
    {
      const TopoShape& subshape = Ex.Current();
      Standard_Boolean    b        = (!mySubShapeMap.Contains(subshape));
      if (b)
        mySubShapeMap.Add(subshape, Lemp);
      mySubShapeMap.ChangeFromKey(subshape).Append(S);
    }
  }
}

//=================================================================================================

const ShapeList& TopOpeBRepBuild_ShapeSet::StartElements() const
{
  return myStartShapes;
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::InitShapes()
{
  myShapesIter.Initialize(myShapes);
}

//=================================================================================================

Standard_Boolean TopOpeBRepBuild_ShapeSet::MoreShapes() const
{
  Standard_Boolean b = myShapesIter.More();
  return b;
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::NextShape()
{
  myShapesIter.Next();
}

//=================================================================================================

const TopoShape& TopOpeBRepBuild_ShapeSet::Shape() const
{
  const TopoShape& S = myShapesIter.Value();
  return S;
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::InitStartElements()
{
  myStartShapesIter.Initialize(myStartShapes);
}

//=================================================================================================

Standard_Boolean TopOpeBRepBuild_ShapeSet::MoreStartElements() const
{
  Standard_Boolean b = myStartShapesIter.More();
  return b;
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::NextStartElement()
{
  myStartShapesIter.Next();
}

//=================================================================================================

const TopoShape& TopOpeBRepBuild_ShapeSet::StartElement() const
{
  const TopoShape& S = myStartShapesIter.Value();
  return S;
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::InitNeighbours(const TopoShape& S)
{
  mySubShapeExplorer.Init(S, mySubShapeType);
  myCurrentShape = S;
  FindNeighbours();
}

//=================================================================================================

Standard_Boolean TopOpeBRepBuild_ShapeSet::MoreNeighbours()
{
  Standard_Boolean b = myIncidentShapesIter.More();
  return b;
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::NextNeighbour()
{
  myIncidentShapesIter.Next();
  Standard_Boolean noisimore = !myIncidentShapesIter.More();
  if (noisimore)
  {
    Standard_Boolean ssemore = mySubShapeExplorer.More();
    if (ssemore)
    {
      mySubShapeExplorer.Next();
      FindNeighbours();
    }
  }
}

//=================================================================================================

const TopoShape& TopOpeBRepBuild_ShapeSet::Neighbour() const
{
  const TopoShape& S = myIncidentShapesIter.Value();
  return S;
}

//=================================================================================================

ShapeList& TopOpeBRepBuild_ShapeSet::ChangeStartShapes()
{
  return myStartShapes;
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::FindNeighbours()
{
  while (mySubShapeExplorer.More())
  {

    // l = list of edges neighbour of edge myCurrentShape through
    // the vertex mySubShapeExplorer.Current(), which is a vertex of the
    // edge myCurrentShape.
    const TopoShape&         V = mySubShapeExplorer.Current();
    const ShapeList& l = MakeNeighboursList(myCurrentShape, V);

    // myIncidentShapesIter iterates on the neighbour edges of the edge
    // given as InitNeighbours() argument (this edge has been stored
    // in the field myCurrentShape).

    myIncidentShapesIter.Initialize(l);
    if (myIncidentShapesIter.More())
      break;
    else
      mySubShapeExplorer.Next();
  }
}

//=======================================================================
// function : MakeNeighboursList
// purpose  : // (Earg = Edge, Varg = Vertex) to find connected to Earg by Varg
//=======================================================================
const ShapeList& TopOpeBRepBuild_ShapeSet::MakeNeighboursList(
  const TopoShape& /*Earg*/,
  const TopoShape& Varg)
{
  const ShapeList& l = mySubShapeMap.FindFromKey(Varg);
  return l;
}

//=================================================================================================

Standard_Integer TopOpeBRepBuild_ShapeSet::MaxNumberSubShape(const TopoShape& Shape)
{
  Standard_Integer                   i, m = 0;
  TopOpeBRepTool_ShapeExplorer       SE(Shape, mySubShapeType);
  TopTools_ListIteratorOfListOfShape LI;
  while (SE.More())
  {
    const TopoShape& SubShape = SE.Current();
    if (!mySubShapeMap.Contains(SubShape))
    {
      SE.Next();
      continue;
    }
    const ShapeList& l = mySubShapeMap.FindFromKey(SubShape);
    LI.Initialize(l);
    for (i = 0; LI.More(); LI.Next(), i++)
    {
    }
    m = Max(m, i);
    SE.Next();
  }
  return m;
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::CheckShape(const Standard_Boolean checkshape)
{
  myCheckShape = checkshape;

#ifdef OCCT_DEBUG
  if (TopOpeBRepBuild_GettraceCHK() && !myCheckShape)
  {
    DumpName(std::cout, "no checkshape set on ");
    std::cout << std::endl;
  }
#endif
}

//=================================================================================================

Standard_Boolean TopOpeBRepBuild_ShapeSet::CheckShape() const
{
  return myCheckShape;
}

//=================================================================================================

Standard_Boolean TopOpeBRepBuild_ShapeSet::CheckShape(const TopoShape&    S,
                                                      const Standard_Boolean checkgeom)
{
  if (!myCheckShape)
    return Standard_True;

  BRepCheck_Analyzer ana(S, checkgeom);
  Standard_Boolean   val = ana.IsValid();
  if (val)
  {
    return Standard_True;
  }
  else
  {
    return Standard_False;
  }
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::DumpName(Standard_OStream&              OS,
                                        const AsciiString1& str) const
{
  OS << str << "(" << myDEBName << "," << myDEBNumber << ")";
}

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_ShapeSet::DumpCheck(Standard_OStream&              OS,
                                         const AsciiString1& str,
                                         const TopoShape&            S,
                                         const Standard_Boolean         chk
#else
void TopOpeBRepBuild_ShapeSet::DumpCheck(Standard_OStream&,
                                         const AsciiString1&,
                                         const TopoShape&,
                                         const Standard_Boolean
#endif
) const
{
  if (!myCheckShape)
    return;

#ifdef OCCT_DEBUG
  TopAbs_ShapeEnum t = S.ShapeType();
  if (!chk)
  {
    if (TopOpeBRepBuild_GettraceCHK() || TopOpeBRepBuild_GettraceCHKNOK())
    {
      DumpName(OS, "*********************** ");
      OS << str << " ";
      TopAbs1::Print(t, OS);
      OS << " : incorrect" << std::endl;
    }
  }
  else
  {
    if (TopOpeBRepBuild_GettraceCHK() || TopOpeBRepBuild_GettraceCHKOK())
    {
      DumpName(OS, "");
      OS << str << " ";
      TopAbs1::Print(t, OS);
      OS << " : correct" << std::endl;
    }
  }
  if (!chk)
    debaddss();
#endif
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::DumpSS()
{
#ifdef DRAW
  DumpName(std::cout, "\nDumpSS start ");
  TopTools_ListIteratorOfListOfShape it;
  Standard_Integer                   i, j, ne;
  AsciiString1            s1("   ");
  InitShapes();
  std::cout << std::endl << "#Shapes : ";
  if (!MoreShapes())
    std::cout << 0;
  std::cout << std::endl;
  for (i = 1; MoreShapes(); NextShape(), i++)
  {
    AsciiString1 ns = SNameori(Shape());
    std::cout << PRODINS << ns << "; # " << i << " draw" << std::endl;
    DBRep1::Set(ns.ToCString(), Shape());
  }

  InitStartElements();
  std::cout << "#StartElements : ";
  if (!MoreStartElements())
    std::cout << 0;
  std::cout << std::endl;
  for (i = 1; MoreStartElements(); NextStartElement(), i++)
  {
    std::cout << PRODINS << SNameori(StartElement()) << "; # " << i << " draw" << std::endl;
  }

  InitStartElements();
  std::cout << "#Neighbours of StartElements : ";
  if (!MoreStartElements())
    std::cout << 0;
  std::cout << std::endl;
  for (i = 1; MoreStartElements(); NextStartElement(), i++)
  {
    const TopoShape&     e    = StartElement();
    AsciiString1 enam = SNameori(e);
    InitNeighbours(e);
    if (MoreNeighbours())
    {
      AsciiString1 sne("clear; ");
      sne = sne + PRODINS + enam;
      for (ne = 1; MoreNeighbours(); NextNeighbour(), ne++)
      {
        const TopoShape& N = Neighbour();
        sne                   = sne + " " + SNameori(N);
      }
      sne = sne + "; wclick; #draw";
      std::cout << sne << std::endl;
    }
  }

  Standard_Integer ism, nsm = mySubShapeMap.Extent();
  std::cout << "#Incident shapes : ";
  if (!nsm)
    std::cout << 0;
  std::cout << std::endl;
  for (i = 1, ism = 1; ism <= nsm; ism++, i++)
  {
    const TopoShape&                v   = mySubShapeMap.FindKey(ism);
    const ShapeList&        lsv = mySubShapeMap.FindFromIndex(ism);
    TopTools_ListIteratorOfListOfShape itle(lsv);
    if (itle.More())
    {
      AsciiString1 vnam = SName(v);
      AsciiString1 sle("clear; ");
      sle = sle + PRODINS + vnam;
      for (j = 1; itle.More(); itle.Next(), j++)
      {
        const TopoShape& e = itle.Value();
        sle                   = sle + " " + SNameori(e);
      }
      sle = sle + "; wclick; #draw";
      std::cout << sle << std::endl;
    }
  }
  DumpName(std::cout, "DumpSS end ");
#endif
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::DumpBB()
{
#ifdef DRAW
  DumpName(std::cout, "\nDumpBB ");
  TopTools_ListIteratorOfListOfShape it;
  Standard_Integer                   i, j, ne = 0, nb = 1, curr, currloc;
  AsciiString1            s1("   "), stt, enam, nnam, vnam;
  InitShapes();

  std::cout << std::endl << "#Shapes : (block old) ";
  if (!MoreShapes())
    std::cout << 0;
  std::cout << std::endl;
  for (i = 1; MoreShapes(); NextShape(), i++, nb++)
  {
    std::cout << "Block1 number" << nb << " (old)." << std::endl;
    const TopoShape& e = Shape();
    TopoDS_Iterator     ShapIter(e);
    for (ne = 1; ShapIter.More(); ShapIter.Next(), ne++)
    {
      const TopoShape& subsha = ShapIter.Value();
      ShapeEnumToString(subsha.ShapeType(), stt);
      enam = stt + ne + "ShaB" + nb;
      DBRep1::Set(enam.ToCString(), subsha);
      std::cout << "clear; " << PRODINS << enam << "; #draw" << std::endl;
    }
  }

  InitStartElements();
  TopTools_IndexedMapOfShape mos;
  std::cout << "#Elements : (block new) : ";
  if (!MoreStartElements())
    std::cout << 0;
  std::cout << std::endl;
  mos.Clear();
  for (; MoreStartElements(); NextStartElement())
  {
    const TopoShape& e = StartElement();
    curr                  = mos.Extent();
    if (mos.Add(e) > curr)
    {
      std::cout << "#Block1 number" << nb << " (new)." << std::endl;
      nb++;
      ne++;
      enam = "";
      enam = enam + "ste" + ne + "newB" + nb;
      DBRep1::Set(enam.ToCString(), e);

      while (curr < mos.Extent())
      {
        curr    = mos.Extent();
        currloc = curr;
        InitNeighbours(mos.FindKey(curr));
        for (; MoreNeighbours(); NextNeighbour())
        {
          const TopoShape& N = Neighbour();
          if (mos.Add(N) > currloc)
          {
            currloc++;
            ne++;
            // to know if ste or ele is displayed; start
            const ShapeList& LSE = StartElements();
            it.Initialize(LSE);
            while (it.More())
              if (it.Value() == N)
                break;
              else
                it.Next();
            enam = "";
            if (it.More())
            {
              enam = enam + "ste" + ne + "newB" + nb;
              DBRep1::Set(enam.ToCString(), N);
            }
            else
            {
              enam = enam + "ele" + ne + "newB" + nb;
              DBRep1::Set(enam.ToCString(), N);
              std::cout << PRODINS << enam << "; #draw" << std::endl;
            }
          }
        }
      }
    }
  }
#endif
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::DEBName(const AsciiString1& N)
{
  myDEBName = N;
}

//=================================================================================================

const AsciiString1& TopOpeBRepBuild_ShapeSet::DEBName() const
{
  return myDEBName;
}

//=================================================================================================

void TopOpeBRepBuild_ShapeSet::DEBNumber(const Standard_Integer I)
{
  myDEBNumber = I;
}

//=================================================================================================

Standard_Integer TopOpeBRepBuild_ShapeSet::DEBNumber() const
{
  return myDEBNumber;
}

//=================================================================================================

#ifdef DRAW
AsciiString1 TopOpeBRepBuild_ShapeSet::SName(const TopoShape& /*S*/,
                                                        const AsciiString1& sb,
                                                        const AsciiString1& sa) const
{
  AsciiString1 str;

  str                          = sb;
  AsciiString1 WESi = myDEBName.SubString(1, 1) + myDEBNumber;
  str                          = str + WESi;
  TopAbs_ShapeEnum        t    = S.ShapeType();
  AsciiString1 sts;
  ShapeEnumToString(t, sts);
  sts.UpperCase();
  str                   = str + sts.SubString(1, 1);
  Standard_Integer isub = mySubShapeMap.FindIndex(S);
  Standard_Integer ista = myOMSS.FindIndex(S);
  Standard_Integer iele = myOMES.FindIndex(S);
  Standard_Integer isha = myOMSH.FindIndex(S);
  if (isub)
    str = str + "sub" + isub;
  if (ista)
    str = str + "sta" + ista;
  else if (iele)
    str = str + "ele" + iele;
  if (isha)
    str = str + "sha" + isha;
  str = str + sa;

  return str;
}
#else
AsciiString1 TopOpeBRepBuild_ShapeSet::SName(const TopoShape&,
                                                        const AsciiString1&,
                                                        const AsciiString1&) const
{
  AsciiString1 str;
  return str;
}
#endif

//=================================================================================================

#ifdef DRAW
AsciiString1 TopOpeBRepBuild_ShapeSet::SNameori(const TopoShape&            S,
                                                           const AsciiString1& sb,
                                                           const AsciiString1& sa) const
{
  AsciiString1 str;
  str                         = sb + SName(S);
  AsciiString1 sto = TopAbs1::ShapeOrientationToString(S.Orientation());
  str                         = str + sto.SubString(1, 1);
  str                         = str + sa;
  return str;
}
#else
AsciiString1 TopOpeBRepBuild_ShapeSet::SNameori(const TopoShape&,
                                                           const AsciiString1&,
                                                           const AsciiString1&) const
{
  AsciiString1 str;
  return str;
}
#endif

//=================================================================================================

#ifdef DRAW
AsciiString1 TopOpeBRepBuild_ShapeSet::SName(const ShapeList&    L,
                                                        const AsciiString1& sb,
                                                        const AsciiString1& /*sa*/) const
{
  AsciiString1 str;
  for (TopTools_ListIteratorOfListOfShape it(L); it.More(); it.Next())
    str = str + sb + SName(it.Value()) + sa + " ";
  return str;
}
#else
AsciiString1 TopOpeBRepBuild_ShapeSet::SName(const ShapeList&,
                                                        const AsciiString1&,
                                                        const AsciiString1&) const
{
  AsciiString1 str;
  return str;
}
#endif

//=================================================================================================

AsciiString1 TopOpeBRepBuild_ShapeSet::SNameori(
  const ShapeList& /*L*/,
  const AsciiString1& /*sb*/,
  const AsciiString1& /*sa*/) const
{
  AsciiString1 str;
#ifdef DRAW
  for (TopTools_ListIteratorOfListOfShape it(L); it.More(); it.Next())
    str = str + sb + SNameori(it.Value()) + sa + " ";
#endif
  return str;
}
