// Created on: 1994-12-23
// Created by: Remi LEQUETTE
// Copyright (c) 1994-1999 Matra Datavision
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

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools_Quilt.hxx>
#include <Geom2d_Curve.hxx>
#include <Standard_NoSuchObject.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>
#include <TopTools_MapOfShape.hxx>

//=================================================================================================

ShapeQuilt::ShapeQuilt()
    : hasCopy(Standard_False)
{
}

//=================================================================================================

static Standard_Boolean NeedCopied(const TopoShape&                        theShape,
                                   const TopTools_IndexedDataMapOfShapeShape& myBounds)
{
  // test if the shape must be copied
  // i.e. it contains a bound subshape
  Standard_Boolean IsCopied = Standard_False;
  TopoDS_Iterator  itv(theShape);
  for (; itv.More(); itv.Next())
  {
    if (myBounds.Contains(itv.Value()))
    {
      IsCopied = Standard_True;
      break;
    }
  }
  return IsCopied;
}

static void CopyShape(const TopoEdge& E, TopTools_IndexedDataMapOfShapeShape& myBounds)
{
  TopoEdge NE = E;
  NE.EmptyCopy();
  NE.Orientation(TopAbs_FORWARD);
  ShapeBuilder B;
  // add the edges
  TopoDS_Iterator itv;
  itv.Initialize(E, Standard_False); // TCollection_DataMap
  for (; itv.More(); itv.Next())
  {
    const TopoShape& V = itv.Value();
    if (myBounds.Contains(V))
    {
      B.Add(NE, myBounds.FindFromKey(V).Oriented(V.Orientation()));
    }
    else
    {
      B.Add(NE, V);
    }
  }
  // set the 3d range
  Standard_Real f, l;
  BRepInspector::Range(E, f, l);
  B.Range(NE, f, l);
  myBounds.Add(E, NE.Oriented(TopAbs_FORWARD));
}

/*static void CopyShape(const TopoWire& W,TopTools_DataMapOfShapeShape& myBounds)
{
  TopoWire NW;
  B.MakeWire(NW);
  TopoDS_Iterator ite(W,Standard_False);
  for ( ; ite.More(); ite.Next()){
    const TopoEdge& E = TopoDS::Edge(ite.Value());
    TopAbs_Orientation OE = E.Orientation();
    if (myBounds.IsBound(E)) {
      TopoEdge& NE = TopoDS::Edge(myBounds(E));
      B.Add(NW,NE.Oriented(OE));
    }
    else
      B.Add(NW,E);
  }
  NW.Orientation(W.Orientation());
  myBounds.Bind(W,NW);
}*/
void ShapeQuilt::Add(const TopoShape& S)
{
  // std::cout <<" Version of sewing with free edges"<<std::endl;
  //  Binds all the faces of S
  //   - to the face itself if it is not copied
  //   - to the copy if it is copied
  if (myBounds.Contains(S))
  {
    return;
  }

  ShapeBuilder B;
  for (ShapeExplorer wex(S, TopAbs_WIRE, TopAbs_FACE); wex.More(); wex.Next())
  {
    myBounds.Add(wex.Current(), wex.Current());
  }

  for (ShapeExplorer eex(S, TopAbs_EDGE, TopAbs_WIRE); eex.More(); eex.Next())
  {
    myBounds.Add(eex.Current(), eex.Current());
  }

  for (ShapeExplorer vex(S, TopAbs_VERTEX, TopAbs_EDGE); vex.More(); vex.Next())
  {
    myBounds.Add(vex.Current(), vex.Current());
  }

  // explore the faces
  for (ShapeExplorer fex(S, TopAbs_FACE); fex.More(); fex.Next())
  {

    // explore the edges of the face and try to copy them
    // if one edge is bound the face must be copied

    Standard_Boolean   copyFace = Standard_False;
    const TopoFace& F        = TopoDS::Face(fex.Current());

    if (hasCopy)
    { // if their is no binding, do not test for copy

      for (ShapeExplorer fed(F, TopAbs_EDGE); fed.More(); fed.Next())
      {

        if (myBounds.Contains(fed.Current()))
        {
          copyFace = Standard_True;
        }
        else
        {
          // test if the edge must be copied
          // i.e. it contains a bound vertex

          Standard_Boolean copyEdge = NeedCopied(fed.Current(), myBounds);
          // Standard_Boolean copyEdge = Standard_False;
          const TopoEdge& E = TopoDS::Edge(fed.Current());

          // TopoDS_Iterator itv(E) ;
          // for ( ; itv.More(); itv.Next()) {
          // if (myBounds.IsBound(itv.Value())) {
          //	copyEdge = Standard_True;
          // break;
          //     }
          //  }

          if (copyEdge)
          {

            // copy of an edge

            copyFace = Standard_True;
            CopyShape(E, myBounds);
            // TopoEdge NE = E; //gka version for free edges
            // NE.EmptyCopy();

            // NE.Orientation(TopAbs_FORWARD);
            //  add the edges
            // itv.Initialize(E,Standard_False) ;
            // for ( ; itv.More(); itv.Next()) {
            // const TopoShape& V = itv.Value();
            // if (myBounds.IsBound(V)) {
            //  B.Add(NE,myBounds(V).Oriented(V.Orientation()));
            //}
            // else {
            // B.Add(NE,V);
            //}
            //}
            // set the 3d range
            // Standard_Real f,l;
            // BRepInspector::Range(E,f,l);
            // B.Range(NE,f,l);

            // myBounds.Bind(E,NE.Oriented(TopAbs_FORWARD));
          }
        }
      }
    }

    // NF will be the copy of F or F itself
    TopoFace NF = F;

    if (copyFace)
    {

      // copy of a face

      NF.EmptyCopy();
      NF.Orientation(TopAbs_FORWARD);

      for (TopoDS_Iterator itw(F, Standard_False); itw.More(); itw.Next())
      {
        const TopoWire& W = TopoDS::Wire(itw.Value());

        TopoWire NW;
        B.MakeWire(NW);
        TopoDS_Iterator ite(W, Standard_False);
        Standard_Real   UFirst, ULast;

        // Reconstruction des wires.

        for (; ite.More(); ite.Next())
        {
          const TopoEdge& E  = TopoDS::Edge(ite.Value());
          TopAbs_Orientation OE = E.Orientation();
          if (myBounds.Contains(E))
          {
            const TopoEdge& NE = TopoDS::Edge(myBounds.FindFromKey(E));
            // pcurve.
            if (NE.Orientation() == TopAbs_FORWARD)
            {
              B.UpdateEdge(NE,
                           BRepInspector::CurveOnSurface(E, F, UFirst, ULast),
                           F,
                           BRepInspector::Tolerance(E));
            }
            else
            {
              // Si NE est REVERSED
              // => les curve3d n ont pas la meme orientation.
              // ( C est une convention cf ShapeQuilt.cdl. et la methode
              // Bind.)
              // => la PCurve de E sur F doit etre inversee.

              OE                       = TopAbs1::Reverse(OE);
              Handle(GeomCurve2d) CE  = BRepInspector::CurveOnSurface(E, F, UFirst, ULast);
              Handle(GeomCurve2d) NCE = CE->Reversed();
              B.UpdateEdge(NE, NCE, F, BRepInspector::Tolerance(E));
              Standard_Real tmp = UFirst;
              UFirst            = CE->ReversedParameter(ULast);
              ULast             = CE->ReversedParameter(tmp);
            }
            // pcurve range
            B.Range(NE, F, UFirst, ULast);

            B.Add(NW, NE.Oriented(OE));
          }
          else
          {
            B.Add(NW, E);
          }
        }
        NW.Orientation(W.Orientation());
        B.Add(NF, NW);
      }
      NF.Orientation(F.Orientation());
    }

    // binds the face to itself or its copy
    myBounds.Add(F, NF);
  }
}

//=================================================================================================

void ShapeQuilt::Bind(const TopoVertex& Vold, const TopoVertex& Vnew)
{
  if (!myBounds.Contains(Vold))
  {
    myBounds.Add(Vold, Vnew);
  }
}

//=================================================================================================

void ShapeQuilt::Bind(const TopoEdge& Eold, const TopoEdge& Enew)
{
  if (!myBounds.Contains(Eold))
  {
    TopoEdge ENew = Enew;
    if (IsCopied(Enew))
    {
      ENew = TopoDS::Edge(Copy(Enew));
      ENew.Orientation(Enew.Orientation());
    }

    if (Eold.Orientation() != ENew.Orientation())
    {
      myBounds.Add(Eold.Oriented(TopAbs_FORWARD), ENew.Oriented(TopAbs_REVERSED));
    }
    else
    {
      myBounds.Add(Eold.Oriented(TopAbs_FORWARD), ENew.Oriented(TopAbs_FORWARD));
    }
    // if new binding bind also the vertices
    TopoDS_Iterator itold(Eold);
    while (itold.More())
    {
      if (!myBounds.Contains(itold.Value()))
      {
        // find the vertex of Enew with same orientation
        TopAbs_Orientation anOrien = itold.Value().Orientation();
        TopoDS_Iterator    itnew(ENew);
        while (itnew.More())
        {
          if (itnew.Value().Orientation() == anOrien)
          {
            TopoVertex VNew = TopoDS::Vertex(itnew.Value());
            if (IsCopied(VNew))
            {
              // if VNew has been copied take the copy
              VNew = TopoDS::Vertex(Copy(VNew));
            }
            myBounds.Add(itold.Value(), VNew);
            break;
          }
          itnew.Next();
        }
      }
      itold.Next();
    }
    hasCopy = Standard_True;
  }
}

//=================================================================================================

Standard_Boolean ShapeQuilt::IsCopied(const TopoShape& S) const
{
  if (myBounds.Contains(S))
  {
    return !S.IsSame(myBounds.FindFromKey(S));
  }
  else
    return Standard_False;
}

//=================================================================================================

const TopoShape& ShapeQuilt::Copy(const TopoShape& S) const
{
  Standard_NoSuchObject_Raise_if(!IsCopied(S), "ShapeQuilt::Copy");
  return myBounds.FindFromKey(S);
}

//=================================================================================================

TopoShape ShapeQuilt::Shells() const
{
  // Outline of the algorithm
  //
  // In the map M we bind the free edges to their shells
  // We explore all the faces in myBounds
  // For each one we search the edges in the map and either :
  //
  // - Start a new shell if no edge is a free edge.
  // - Add the face to an existing shell
  // - Connect other shells if the face touch more than one shell

  // In the Map M the Shell is bound with the relative orientation of E
  // in the shell
  // In the map MF we binb the face to its shell.
  // In the Map MF the Shell is bound with the relative orientation of F
  // in the shell

  TopTools_DataMapOfShapeShape M, MF;
  ShapeBuilder                 B;
  TopoCompound              result;

  B.MakeCompound(result);

  TopTools_MapOfShape MapOtherShape; // gka
  TopTools_MapOfShape EdgesFaces;

  // loop on the face in myBounds
  // TopTools_DataMapIteratorOfDataMapOfShapeShape it(myBounds);

  // while (it.More())
  for (Standard_Integer ii = 1; ii <= myBounds.Extent(); ii++)
  {
    const TopoShape& Shape = myBounds.FindFromIndex(ii); // it.Value();
    if (Shape.ShapeType() == TopAbs_FACE)
    {
      for (ShapeExplorer aExpEdg(Shape, TopAbs_EDGE); aExpEdg.More(); aExpEdg.Next()) // gka
        EdgesFaces.Add(aExpEdg.Current());

      TopoShell       SH;
      TopAbs_Orientation NewO;

      ShapeExplorer itf1(Shape, TopAbs_EDGE);
      for (; itf1.More(); itf1.Next())
      {
        const TopoShape& E = itf1.Current();
        if (M.IsBound(E))
        {
          SH = TopoDS::Shell(M(E));
          if (SH.Orientation() == E.Orientation())
            NewO = TopAbs1::Reverse(Shape.Orientation());
          else
            NewO = Shape.Orientation();

          MF.Bind(Shape, SH.Oriented(NewO));
          break;
        }
      }

      if (SH.IsNull())
      {
        // Create a new shell, closed. Add it to the result.
        B.MakeShell(SH);
        SH.Closed(Standard_True);
        B.Add(result, SH);
        MF.Bind(Shape, SH.Oriented(Shape.Orientation()));
      }

      // Add the face to the shell
      SH.Free(Standard_True);
      //      B.Add(SH.Oriented(TopAbs_FORWARD), F .Oriented(MF(F).Orientation()));
      TopoShape arefShape = SH.Oriented(TopAbs_FORWARD);
      B.Add(arefShape, Shape.Oriented(MF(Shape).Orientation()));

      ShapeExplorer itf(Shape.Oriented(TopAbs_FORWARD), TopAbs_EDGE);

      for (; itf.More(); itf.Next())
      {
        const TopoEdge& E = TopoDS::Edge(itf.Current());

        if (M.IsBound(E))
        {
          const TopoShape oldShell = M(E);
          if (!oldShell.IsSame(SH))
          {
            // Fuse the old shell with the new one
            // Compare the orientation of E in SH and in oldshell.
            TopAbs_Orientation anOrien = E.Orientation();
            if (MF(Shape).Orientation() == TopAbs_REVERSED)
              anOrien = TopAbs1::Reverse(anOrien);

            Standard_Boolean Rev = (anOrien == oldShell.Orientation());
            // if rev = True oldShell has to be reversed.

            // Add the faces of oldShell in SH.
            for (TopoDS_Iterator its(oldShell); its.More(); its.Next())
            {
              const TopoFace  Fo = TopoDS::Face(its.Value());
              TopAbs_Orientation NewOFo;
              // update the orientation of Fo in SH.
              if (Rev)
                NewOFo = TopAbs1::Reverse(MF(Fo).Orientation());
              else
                NewOFo = MF(Fo).Orientation();

              MF.Bind(Fo, SH.Oriented(NewOFo));
              //	      B.Add  (SH.Oriented(TopAbs_FORWARD),Fo.Oriented(NewO));
              TopoShape arefShapeFo = SH.Oriented(TopAbs_FORWARD);
              B.Add(arefShapeFo, Fo.Oriented(NewOFo));
            }
            // Rebind the free edges of the old shell to the new shell
            // gka BUG 6491
            ShapeExplorer aexp(SH, TopAbs_EDGE);
            for (; aexp.More(); aexp.Next())
            {
              // for (TopTools_DataMapIteratorOfDataMapOfShapeShape itm(M);
              //		 itm.More(); ) {
              if (!M.IsBound(aexp.Current()))
                continue;
              const TopoShape& ae = aexp.Current();
              TopoShape        as = M.Find(ae);
              if (as.IsSame(oldShell))
              {
                // update the orientation of free edges in SH.
                if (Rev)
                  NewO = TopAbs1::Reverse(as.Orientation());
                else
                  NewO = as.Orientation();

                M.Bind(ae, SH.Oriented(NewO));
              }
            }
            // remove the old shell from the result
            B.Remove(result, oldShell.Oriented(TopAbs_FORWARD));
          }
          // Test if SH is always orientable.
          TopAbs_Orientation anOrien = E.Orientation();
          if (MF(Shape).Orientation() == TopAbs_REVERSED)
            anOrien = TopAbs1::Reverse(anOrien);

          if (M(E).Orientation() == anOrien)
            SH.Orientable(Standard_False);

          // remove the edge from M (no more a free edge)
          M.UnBind(E);
        }
        else
        {
          NewO = E.Orientation();
          if (MF(Shape).Orientation() == TopAbs_REVERSED)
            NewO = TopAbs1::Reverse(NewO);
          if (!E.IsNull())
            M.Bind(E, SH.Oriented(NewO));
          else
            continue;
        }
      }

      // freeze the shell
      SH.Free(Standard_False);
    }
    else
      MapOtherShape.Add(Shape);

    // it.Next();
  }

  // Unclose all shells having free edges
  for (TopTools_DataMapIteratorOfDataMapOfShapeShape it(M); it.More(); it.Next())
  {
    TopoShape S = it.Value();
    S.Closed(Standard_False);
  }

  TopTools_MapIteratorOfMapOfShape itother(MapOtherShape); // gka version for free edges
  for (; itother.More(); itother.Next())
  {
    if (!EdgesFaces.Contains(itother.Key1()) && myBounds.Contains(itother.Key1()))
    {
      TopoShape aSh = myBounds.FindFromKey(itother.Key1());
      B.Add(result, aSh);
    }
  }
  return result;
}
