// Created on: 1999-01-04
// Created by: Xuan PHAM PHU
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

#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopOpeBRepTool_CLASSI.hxx>
#include <TopOpeBRepTool_EXPORT.hxx>
#include <TopOpeBRepTool_REGUS.hxx>
#include <TopOpeBRepTool_REGUW.hxx>
#include <TopOpeBRepTool_TOOL.hxx>

#ifdef DRAW
  #include <TopOpeBRepTool_DRAW.hxx>
#endif

#define M_FORWARD(ori) (ori == TopAbs_FORWARD)
#define M_REVERSED(ori) (ori == TopAbs_REVERSED)
#define M_INTERNAL(ori) (ori == TopAbs_INTERNAL)
#define M_EXTERNAL(ori) (ori == TopAbs_EXTERNAL)

#define FORWARD (1)
#define REVERSED (2)
#define INTERNAL (3)
#define EXTERNAL (4)
#define CLOSING (5)

#ifdef OCCT_DEBUG
extern Standard_Boolean           TopOpeBRepTool_GettraceREGUSO();
static TopTools_IndexedMapOfShape STATIC_mape, STATIC_mapf, STATIC_mapw, STATIC_mapsh;

static Standard_Integer FUN_adds(const TopoShape& s)
{
  TopAbs_ShapeEnum        typ = s.ShapeType();
  AsciiString1 aa;
  Standard_Integer        is = 0;
  if (typ == TopAbs_SHELL)
  {
    aa = AsciiString1("s");
    is = STATIC_mapsh.Add(s);
  }
  if (typ == TopAbs_WIRE)
  {
    aa = AsciiString1("w");
    is = STATIC_mapw.Add(s);
  }
  if (typ == TopAbs_FACE)
  {
    aa = AsciiString1("f");
    is = STATIC_mapf.Add(s);
  }
  if (typ == TopAbs_EDGE)
  {
    aa = AsciiString1("e");
    is = STATIC_mape.Add(s);
  }
  #ifdef DRAW
  Standard_Boolean trc = TopOpeBRepTool_GettraceREGUSO();
  if (trc)
    FUN_tool_draw(aa, s, is);
  #endif
  return is;
}
#endif

static void FUN_Raise()
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = TopOpeBRepTool_GettraceREGUSO();
  if (trc)
    std::cout << "***** Failure in REGUS **********" << std::endl;
//  throw ExceptionBase("REGUS");
#endif
}

//=================================================================================================

TopOpeBRepTool_REGUS::TopOpeBRepTool_REGUS()
{
  hasnewsplits = Standard_False;
  mynF = myoldnF = 0;

  myFsplits.Clear();
  myOshNsh.Clear();

  myS.Nullify();
  mymapeFs.Clear();
  mymapeFsstatic.Clear();
  mymapemult.Clear();

  myedstoconnect.Clear();
}

//=================================================================================================

void TopOpeBRepTool_REGUS::Init(const TopoShape& S)
{
  hasnewsplits = Standard_False;

  mynF = myoldnF = 0;
  myS            = S;

  mymapeFs.Clear();
  mymapeFsstatic.Clear();
  mymapemult.Clear();
  myedstoconnect.Clear();
}

//=================================================================================================

const TopoShape& TopOpeBRepTool_REGUS::S() const
{
  return myS;
}

//=================================================================================================

void TopOpeBRepTool_REGUS::SetFsplits(TopTools_DataMapOfShapeListOfShape& Fsplits)
{
  myFsplits = Fsplits;
}

//=================================================================================================

void TopOpeBRepTool_REGUS::GetFsplits(TopTools_DataMapOfShapeListOfShape& Fsplits) const
{
  Fsplits = myFsplits;
}

//=================================================================================================

void TopOpeBRepTool_REGUS::SetOshNsh(TopTools_DataMapOfShapeListOfShape& OshNsh)
{
  myOshNsh = OshNsh;
}

//=================================================================================================

void TopOpeBRepTool_REGUS::GetOshNsh(TopTools_DataMapOfShapeListOfShape& OshNsh) const
{
  OshNsh = myOshNsh;
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_REGUS::MapS()
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = TopOpeBRepTool_GettraceREGUSO();
  Standard_Integer ish = FUN_adds(S());
  if (trc)
    std::cout << "**    MAPPING    ** shape" << ish << std::endl;
#endif

  // mymapeFs, myoldnF :
  myoldnF = 0;
  ShapeExplorer exf(myS, TopAbs_FACE);
  for (; exf.More(); exf.Next())
  {
    const TopoShape& f = exf.Current();
    myoldnF++;

    ShapeExplorer exe(f, TopAbs_EDGE);
    for (; exe.More(); exe.Next())
    {
      const TopoShape& e   = exe.Current();
      Standard_Boolean    isb = mymapeFs.IsBound(e);
      if (isb)
      {
        mymapeFs.ChangeFind(e).Append(f);
        mymapeFsstatic.ChangeFind(e).Append(f);
      }
      else
      {
        ShapeList lof;
        lof.Append(f);
        mymapeFs.Bind(e, lof);
        mymapeFsstatic.Bind(e, lof);
      }
    } // exe
  } // exf
  mynF = myoldnF;

  // mymapemult :
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itm(mymapeFs);
  for (; itm.More(); itm.Next())
  {
    const TopoShape&         e   = itm.Key();
    const ShapeList& lof = itm.Value();
    Standard_Integer            nf  = lof.Extent();
    if (nf > 2)
      mymapemult.Add(e);
#ifdef OCCT_DEBUG
    if (trc)
    {
      std::cout << "co(e" << FUN_adds(e) << ")= ";
      TopTools_ListIteratorOfListOfShape it(lof);
      for (; it.More(); it.Next())
        std::cout << " f" << FUN_adds(it.Value());
      std::cout << std::endl;
    }
#endif
  } // itm(mymapeFs)
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_REGUS::WireToFace(const TopoFace&          Fanc,
                                                  const ShapeList& nWs,
                                                  ShapeList&       nFs)
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = TopOpeBRepTool_GettraceREGUSO();
#endif
  nFs.Clear();
  TopTools_DataMapOfShapeListOfShape mapWlow;
  TopoShape                       aLocalShape = Fanc.Oriented(TopAbs_FORWARD);
  TopoFace                        aFace       = TopoDS::Face(aLocalShape);
  //  TopoFace aFace = TopoDS::Face(Fanc.Oriented(TopAbs_FORWARD));
  TopOpeBRepTool_CLASSI classi;
  classi.Init2d(aFace);

  Standard_Boolean classifok = classi.Classilist(nWs, mapWlow);
  if (!classifok)
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << "** classif fails" << std::endl;
#endif
    return Standard_False;
  }

  Standard_Boolean facesbuilt = TOOL1::WireToFace(Fanc, mapWlow, nFs);
  if (!facesbuilt)
  {
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << "** facesbuilt fails" << std::endl;
#endif
    return Standard_False;
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_REGUS::SplitF(const TopoFace&    Fanc,
                                              ShapeList& FSplits)
{
  // prequesitory : All edges have already been split, there is no
  //                internal vertex on edge, except for internal edge.
  TopAbs_Orientation oAnc                = Fanc.Orientation();
  TopoShape       aLocalShapeFromFace = Fanc.Oriented(TopAbs_FORWARD);
  TopoFace        aFace               = TopoDS::Face(aLocalShapeFromFace);
  //  TopoFace aFace = TopoDS::Face(Fanc.Oriented(TopAbs_FORWARD));

  FSplits.Clear();

  TopOpeBRepTool_REGUW REGUW(aFace);

  ShapeList nWs;
  Standard_Boolean     hassp = Standard_False;
  ShapeExplorer      exw(aFace, TopAbs_WIRE);
  for (; exw.More(); exw.Next())
  {
    const TopoShape& w = exw.Current();
    REGUW.Init(w);
    REGUW.MapS();

    ShapeList eIs;
    // --------
    ShapeExplorer exe(w, TopAbs_EDGE);
    for (; exe.More(); exe.Next())
    {
      const TopoShape& e = exe.Current();
      if (M_INTERNAL(e.Orientation()))
        eIs.Append(e);
    } // exe

    TopTools_ListIteratorOfListOfShape ite(eIs);
    //    if (!ite.More()) {nWs.Append(w); continue;}

    while (ite.More())
    {
      const TopoEdge& eI = TopoDS::Edge(ite.Value());
      TopoVertex      vf, vl;
      TopoShape       aLocalShape = eI.Oriented(TopAbs_FORWARD);
      TopExp1::Vertices(TopoDS::Edge(aLocalShape), vf, vl);
      //      TopExp1::Vertices(TopoDS::Edge(eI.Oriented(TopAbs_FORWARD)),vf,vl);
      TopOpeBRepTool_connexity cof;
      REGUW.Connexity(vf, cof);
      ShapeList     lef;
      Standard_Integer         nef = cof.AllItems(lef);
      TopOpeBRepTool_connexity col;
      REGUW.Connexity(vl, col);
      ShapeList lel;
      Standard_Integer     nel = col.AllItems(lel);
      if ((nef <= 1) || (nel <= 1))
      {
        eIs.Remove(ite);
        continue;
      }

      // prequesitory : we do not have internal vertices in edges oriented FOR
      aLocalShape    = eI.Oriented(TopAbs_REVERSED);
      TopoEdge eR = TopoDS::Edge(aLocalShape);
      aLocalShape    = eI.Oriented(TopAbs_FORWARD);
      TopoEdge eF = TopoDS::Edge(aLocalShape);
      //      TopoEdge eR = TopoDS::Edge(eI.Oriented(TopAbs_REVERSED));
      //      TopoEdge eF = TopoDS::Edge(eI.Oriented(TopAbs_FORWARD));

      ShapeExplorer exv(eI, TopAbs_VERTEX);
      for (; exv.More(); exv.Next())
      {
        const TopoVertex& v  = TopoDS::Vertex(exv.Current());
        Standard_Boolean     ok = REGUW.RemoveOldConnexity(v, INTERNAL, eI);
        if (!ok)
          return Standard_False;
        Standard_Integer ivF = TOOL1::OriinSor(v, eF);
        ok                   = REGUW.AddNewConnexity(v, ivF, eF);
        if (!ok)
          return Standard_False;
        Standard_Integer ivR = TOOL1::OriinSor(v, eR);
        ok                   = REGUW.AddNewConnexity(v, ivR, eR);
        if (!ok)
          return Standard_False;
        ok = REGUW.UpdateMultiple(v);
        if (!ok)
          return Standard_False;
      } // exv
      ite.Next();
    } // ite(eIs)

    // now all edges of <eIs> are INTERNAL edges of <w>
    // their 2 bounds are of connexity > 1.
    //    if (eIs.IsEmpty()) {nWs.Append(w); continue;}

    ShapeList spW;
    // --------
    Standard_Boolean spok = REGUW.REGU(); // only first step
    if (!spok)
    {
      FUN_Raise();
      return Standard_False;
    }
    REGUW.GetSplits(spW);
    if (!spW.IsEmpty())
    {
      nWs.Append(spW);
      hassp = Standard_True;
    }
  } // exw

  if (!hassp)
    return Standard_False;
  ShapeList nFs;
  Standard_Boolean     ok = TopOpeBRepTool_REGUS::WireToFace(aFace, nWs, nFs);
  if (!ok)
  {
    FUN_Raise();
    return Standard_False;
  }

  TopTools_ListIteratorOfListOfShape itf(nFs);
  for (; itf.More(); itf.Next())
    FSplits.Append(itf.Value().Oriented(oAnc));
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_REGUS::SplitFaces()
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = TopOpeBRepTool_GettraceREGUSO();
  Standard_Integer ish = FUN_adds(S());
  if (trc)
    std::cout << "**    SPLITTING FACES    ** shape" << ish << std::endl;
#endif
  ShapeExplorer exf(myS, TopAbs_FACE);
  for (; exf.More(); exf.Next())
  {

    // splitting face :
    const TopoFace&   f = TopoDS::Face(exf.Current());
    ShapeList lfsp;
    Standard_Boolean     issp = TopOpeBRepTool_REGUS::SplitF(f, lfsp);

    if (!issp)
      continue;

    myFsplits.Bind(f, lfsp);

    // updating the map of connexity :
    // f -> lfsp = {fsp}
    mynF--;
    TopTools_ListIteratorOfListOfShape itf(lfsp);
    for (; itf.More(); itf.Next())
    {
      const TopoShape& fsp = itf.Value();
      mynF++;

      ShapeExplorer exe(fsp, TopAbs_EDGE);
      for (; exe.More(); exe.Next())
      {
        // fsp -> {e}
        const TopoShape& e   = exe.Current();
        Standard_Boolean    isb = mymapeFs.IsBound(e);
        if (!isb)
        {
          FUN_Raise();
          return Standard_False;
        }

        // <mymapeFs>
        ShapeList& lof = mymapeFs.ChangeFind(e);
        TOOL1::Remove(lof, f);
        lof.Append(fsp);

        // <mymapemult>
        Standard_Integer nf = lof.Extent();
        if (nf > 2)
          mymapemult.Add(e);
      } // exe(fsp)
    } // itf(lfsp)

#ifdef OCCT_DEBUG
    if (trc)
    {
      std::cout << "split(f" << FUN_adds(f) << ")= ";
      TopTools_ListIteratorOfListOfShape it(lfsp);
      for (; it.More(); it.Next())
        std::cout << " f" << FUN_adds(it.Value());
      std::cout << std::endl;
    }
#endif
  } // exf(myS)
  return Standard_True;
}

static void FUN_update(const TopoShape& fcur, TopTools_MapOfShape& edstoconnect)
// purpose : <e> edge of <fcur>
//  1. <e> is INTERNAL or EXTERNAL   -> nothing is done
//  2. <e> is closing edge of <fcur> -> nothing is done
//  3. <e> is already bound in <edstoconnect> -> remove it from the map
//     (then has 2 ancestor faces stored in the current Block)
//  4. elsewhere, add it in the map.
//
// !! if <fcur> is INTERNAL/EXTERNAL -> nothing is done
{
  TopAbs_Orientation ofcur = fcur.Orientation();
  if (M_INTERNAL(ofcur) || M_EXTERNAL(ofcur))
    return;

  ShapeExplorer exe(fcur, TopAbs_EDGE);
  for (; exe.More(); exe.Next())
  {
    const TopoShape& e  = exe.Current();
    TopAbs_Orientation  oe = e.Orientation();
    if (M_INTERNAL(oe) || M_EXTERNAL(oe))
      continue;

    Standard_Boolean isclo = TOOL1::IsClosingE(TopoDS::Edge(e), TopoDS::Face(fcur));
    if (isclo)
      continue;

    Standard_Boolean isb = edstoconnect.Contains(e);
    if (isb)
      edstoconnect.Remove(e);
    else
      edstoconnect.Add(e);
  } // exe
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_REGUS::REGU()
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc  = TopOpeBRepTool_GettraceREGUSO();
  Standard_Integer ishe = FUN_adds(myS);
  if (trc)
    std::cout << "**    REGU    **" << ishe << std::endl;
#endif
  ShapeList Splits;
  Standard_Boolean     toregu = !mymapemult.IsEmpty() || (mynF != myoldnF);
  if (!toregu)
    return Standard_False;

  // purpose : myS -> {Blocks},
  //           a Block is a closed shell with "valid" edges.
  //           - a valid edge in a Block has at most two ancestor faces -
  //
  // Give us the starting couple (<ei>, <fi>) :
  // * If <ei> has only one untouched ancestor face <fj> left, fj+1 <- fj
  //   Else among the untouched ancestors faces, we choose the one for which
  //   angle (<veci>, <vecj>) is the smallest; providing face <fj> reduces
  //   the matter described by <fi>.
  // * update <mymapeFs> for <ei> (<fj> as touched).
  // * Update <mymapemult> for <fi>'s bound edges :
  //   - if bound edge is not in the map, add it.
  //   - else if bound edge has two ancestor faces in current list <mylFinBlock>,
  //     delete it form the map.
  //
  //  ShapeList lFinBlock; // <lFinBlock> describes a valid closed shell when
  //  <myedstoconnect> is emptied.
  mylFinBlock.Clear();
  Standard_Integer nite = 0;
  while (nite <= mynF)
  {
    Standard_Boolean startBlock = mylFinBlock.IsEmpty();
    Standard_Boolean endBlock   = myedstoconnect.IsEmpty() && (!startBlock);

#ifdef OCCT_DEBUG
    Standard_Boolean tr = Standard_False;
    if (tr)
    {
      TopTools_MapIteratorOfMapOfShape it(myedstoconnect);
      std::cout << "still to connect : ";
      for (; it.More(); it.Next())
        std::cout << " e" << FUN_adds(it.Key());
      std::cout << std::endl;
    }
#endif

    //* endBlock
    // ---------
    if (endBlock)
    {
      // building up shell on <mylFinBlock>
      Standard_Integer nFcur     = mylFinBlock.Extent();
      Standard_Boolean unchanged = (nFcur == myoldnF) && (mynF == myoldnF);
      if (unchanged)
      {
#ifdef OCCT_DEBUG
        if (trc)
          std::cout << "#** shell" << ishe << " valid\n";
#endif
        return Standard_False; // nyi analysis if we should raise or not
      }
      else
      {
        TopoShell newShe;
        TOOL1::MkShell(mylFinBlock, newShe);
        Splits.Append(newShe);
#ifdef OCCT_DEBUG
        if (trc)
        {
          std::cout << "#** shell " << ishe << " gives new shell " << FUN_adds(newShe) << std::endl;
          for (TopTools_ListIteratorOfListOfShape it(mylFinBlock); it.More(); it.Next())
            std::cout << ";dins f" << FUN_adds(it.Value());
          std::cout << std::endl << std::endl;
        }
#endif
        mylFinBlock.Clear();
        startBlock = Standard_True;
      }
    } // endBlock

    //* all faces touched
    // ------------------
    Standard_Boolean FINI = (nite == mynF);
    if (FINI)
      break;

    Standard_Integer advance = Standard_False;
    //* initializing a new Block
    // -------------------------
    if (startBlock || endBlock)
    {
      advance = InitBlock();
      if (!advance)
        return Standard_False;
    } // startBlock||endBlock

    //* choosing next face
    // -------------------
    else
    {
      advance = NextinBlock();
    }

    // ** updating connexity
    ::FUN_update(myf, myedstoconnect);

    if (!advance)
    {
      endBlock = myedstoconnect.IsEmpty() && (!startBlock);
      if (!endBlock)
        return Standard_False;
      else
        continue;
    }

    ShapeExplorer exe(myf, TopAbs_EDGE);
    for (; exe.More(); exe.Next())
    {
      const TopoShape& e   = exe.Current();
      Standard_Boolean    isb = mymapeFs.IsBound(e);
      if (!isb)
        continue; // ancestors faces of <e> are stored in Blocks
      TOOL1::Remove(mymapeFs.ChangeFind(e), myf);
    } // exe

    mylFinBlock.Append(myf);
    nite++;
  } // nite <= mynF

  myOshNsh.Bind(S(), Splits);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_REGUS::InitBlock()
{
  Standard_Integer nec = myedstoconnect.Extent();
  if (nec != 0)
    return Standard_False; // should be empty

  ShapeList                                eds;
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itm(mymapeFs);
  for (; itm.More(); itm.Next())
    eds.Append(itm.Key());

  TopTools_ListIteratorOfListOfShape ite(eds);
  for (; ite.More(); ite.Next())
  {
    const TopoShape&         e   = ite.Value();
    const ShapeList& lof = mymapeFs.Find(e);
    if (lof.IsEmpty())
    {
      mymapeFs.UnBind(e);
      continue;
    }
    myf = lof.First();
#ifdef OCCT_DEBUG
    Standard_Boolean trc = TopOpeBRepTool_GettraceREGUSO();
    if (trc)
      std::cout << "* Block : first face = f" << FUN_adds(myf) << std::endl;
#endif
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_REGUS::NextinBlock()
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = TopOpeBRepTool_GettraceREGUSO();
#endif
  // we try to connect first edge of <myf> bound in <myedstoconnect>
  ShapeList eds;
  ShapeExplorer      exe(myf, TopAbs_EDGE);
  for (; exe.More(); exe.Next())
  {
    const TopoShape& e   = exe.Current();
    Standard_Boolean    isb = myedstoconnect.Contains(e);
    if (isb)
      eds.Append(e);
  } // exe
  Standard_Boolean alleftouched = eds.IsEmpty();
  if (alleftouched)
  {
    TopTools_MapIteratorOfMapOfShape itc(myedstoconnect);
    for (; itc.More(); itc.Next())
    {
      const TopoShape& e       = itc.Key();
      Standard_Boolean    isBound = mymapeFs.IsBound(e);
      // all ancestor faces of <e> have been stored
      if (!isBound)
      {
        myedstoconnect.Remove(e);
        continue;
      }

      const ShapeList& lof = mymapeFs.Find(e);
      Standard_Integer            nf  = lof.Extent();
      if (nf == 0)
      {
        myedstoconnect.Remove(e);
        mymapeFs.UnBind(e);
        continue;
      }

      //      myf = lof.First(); 130499
      if (lof.Extent() == 1)
        myf = lof.First();
      else
      {
        // looking for first face stored in the current block
        // connexed to e

        TopTools_ListIteratorOfListOfShape itff(mylFinBlock);
        TopTools_MapOfShape                mapf;
        for (; itff.More(); itff.Next())
          mapf.Add(itff.Value());
        // lofc : the list of faces connexed to e in <myS>
        // lof  : the list of untouched faces connexed to e in <myS>
        const ShapeList& lofc = mymapeFsstatic.Find(e);

        itff.Initialize(lofc);
        TopoFace fref;
        for (; itff.More(); itff.Next())
        {
          const TopoFace& fc  = TopoDS::Face(itff.Value());
          Standard_Boolean   isb = mapf.Contains(fc);
          if (isb)
          {
            fref = fc;
            break;
          }
        } // itff(lofc)
        if (fref.IsNull())
        {
          return Standard_False; // !!!!!!!!!! a revoir 130499
        }
        else
        {
          myf = fref;
          TopoFace      ffound;
          Standard_Boolean ok = NearestF(TopoDS::Edge(e), lof, ffound);
          if (!ok)
            return Standard_False;
          myf = ffound;
        }
      }

      return Standard_True;
    }
    return Standard_False;
  }

  TopTools_ListIteratorOfListOfShape ite(eds);
  for (; ite.More(); ite.Next())
  {
    const TopoShape& e   = ite.Value();
    Standard_Boolean    isb = mymapeFs.IsBound(e);
    // all ancestor faces of <e> have been stored
    if (!isb)
    {
      myedstoconnect.Remove(e);
      continue;
    }

    const ShapeList& lof = mymapeFs.Find(e);
    Standard_Integer            nf  = lof.Extent();
    if (nf == 0)
    {
      myedstoconnect.Remove(e);
      mymapeFs.UnBind(e);
      continue;
    }
#ifdef OCCT_DEBUG
    if (trc)
    {
      std::cout << "e" << FUN_adds(e) << " on " << nf << " untouched f:" << std::endl;
    }
#endif
    if (nf == 1)
      myf = lof.First();
    else
    {
      TopoFace      ffound;
      Standard_Boolean ok = NearestF(TopoDS::Edge(e), lof, ffound);
      if (!ok)
        return Standard_False;
      myf = ffound;
    }
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << "->myf = f" << FUN_adds(myf) << std::endl;
#endif
    return Standard_True;
  } // itm(myedstoconnect)
  return Standard_False;
}

static Standard_Boolean FUN_vectors(const TopoFace&     f,
                                    const TopoEdge&     e,
                                    const Standard_Real    pare,
                                    Dir3d&                nt,
                                    Dir3d&                xx,
                                    const Standard_Real    tola,
                                    const Standard_Boolean approx)
{
  // <nt> :
  if (approx)
  {
    Standard_Boolean ok = TOOL1::tryNgApp(pare, e, f, tola, nt);
    if (!ok)
      return Standard_False;
  }
  else
  {
    Vector3d           tmp;
    Standard_Boolean ok = FUN_tool_nggeomF(pare, e, f, tmp);
    if (!ok)
      return Standard_False;
    nt = Dir3d(tmp);
  }
  if (M_REVERSED(f.Orientation()))
    nt.Reverse();
  // <xx> :
  Standard_Boolean ok = FUN_tool_getxx(f, e, pare, xx);
  if (!ok)
    return Standard_False;
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_REGUS::NearestF(const TopoEdge&          e,
                                                const ShapeList& lof,
                                                TopoFace&                ffound) const
// prequesitory : <e> is shared by <myf> and faces of <lof>.
//
// NYIXPU!!!!!!!! if (xx1 tg xx2) -> use curvatures
//
{
#ifdef OCCT_DEBUG
  Standard_Boolean trc = TopOpeBRepTool_GettraceREGUSO();
#endif
  ffound.Nullify();
  TopoFace fref = TopoDS::Face(myf);

  // Give us edge <e>, and a reference face <fref> (= <myf>)
  // - parameter on <e> = <pare>.
  // - xxi = tangent fo face fi at pnt(e,pare) oriented INSIDE 2d(fi)
  //        normal to tge = tg(e,pare).
  // purpose : looking for ffound /
  //  MatterAng(xxref, xxfound) = Min{ MatterAng(xxref, xxi), xxi for fi in <lof>
  //                                 providing fi reduces 3d(fref) }

  // <parone> :
  Standard_Real f, l;
  FUN_tool_bounds(e, f, l);
  Standard_Real eps  = 0.45678;
  Standard_Real pare = (1 - eps) * f + eps * l;

  // RONd (x,y,z) = (xxref,ntref,x^y)
  // clang-format off
  Standard_Real tola = Precision::Angular()*1.e3; //Dir3d xapp,yapp; Standard_Boolean refapp = Standard_False;
  // clang-format on
  Dir3d           x, y;
  Standard_Boolean ok = ::FUN_vectors(fref, e, pare, y, x, tola, Standard_False);
  if (!ok)
  {
    FUN_Raise();
    return Standard_False;
  }

  // initializing
  // ------------
  Standard_Real                      angfound = 0;
  TopTools_ListIteratorOfListOfShape itf(lof);
  for (; itf.More(); itf.Next())
  {
    ffound = TopoDS::Face(itf.Value());
    Dir3d ntfound, xxfound;
    ok = ::FUN_vectors(ffound, e, pare, ntfound, xxfound, tola, Standard_False);
    if (!ok)
    {
      FUN_Raise();
      return Standard_False;
    }

    Standard_Boolean oppo = TOOL1::Matter(x, y, xxfound, ntfound, tola, angfound);
#ifdef OCCT_DEBUG
    if (trc && !oppo)
      std::cout << "   f" << FUN_adds(fref) << ",f" << FUN_adds(ffound) << " not oppo" << std::endl;
#endif
    if (!oppo)
    {
      ffound.Nullify();
      continue;
    }

    if (angfound < tola)
    {
      //      refapp = Standard_True; ::FUN_vectors(fref,e,pare,yapp,xapp,tola,Standard_True);
      //      ::FUN_vectors(ffound,e,pare,ntfound,xxfound,tola,Standard_True);
      //      TOOL1::Matter(xapp,yapp,xxfound,ntfound,tola, angfound);
      ok = TOOL1::MatterKPtg(fref, ffound, e, angfound);
      if (!ok)
      {
        FUN_Raise();
        return Standard_False;
      }
    }
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << "   ang(f" << FUN_adds(fref) << ",f" << FUN_adds(ffound) << ")=" << angfound
                << std::endl;
#endif
    break;
  }
  if (ffound.IsNull())
  {
    FUN_Raise();
    return Standard_False;
  }
  if (itf.More())
    itf.Next();
  else
    return Standard_True;

  // selecting nearest face
  // ----------------------
  for (; itf.More(); itf.Next())
  {
    Dir3d             nti, xxi;
    const TopoFace& fi = TopoDS::Face(itf.Value());
    ok                    = ::FUN_vectors(fi, e, pare, nti, xxi, tola, Standard_False);
    if (!ok)
    {
      FUN_Raise();
      return Standard_False;
    }

    Standard_Real    angi = 0;
    Standard_Boolean oppo = TOOL1::Matter(x, y, xxi, nti, tola, angi);
#ifdef OCCT_DEBUG
    if (trc && !oppo)
      std::cout << "   f" << FUN_adds(fref) << ",f" << FUN_adds(fi) << " not oppo" << std::endl;
#endif
    if (!oppo)
      continue;

    if (angi < tola)
    {
      //      if (!refapp) ::FUN_vectors(fref,e,pare,yapp,xapp,tola,Standard_True);
      //      ::FUN_vectors(fi,e,pare,nti,xxi,tola,Standard_True);
      //      TOOL1::Matter(xapp,yapp,xxi,nti,tola, angi);
      ok = TOOL1::MatterKPtg(fref, fi, e, angi);
      if (!ok)
      {
        FUN_Raise();
        return Standard_False;
      }
    }
#ifdef OCCT_DEBUG
    if (trc)
      std::cout << "   ang(f" << FUN_adds(fref) << ",f" << FUN_adds(fi) << ")=" << angi
                << std::endl;
#endif
    if (angi > angfound)
      continue;
    angfound = angi;
    ffound   = fi;
  }
  return Standard_True;
}
