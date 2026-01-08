// Created on: 1996-09-05
// Created by: Yves FRICAUD
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

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRep_TVertex.hxx>
#include <BRepAlgo_AsDes.hxx>
#include <BRepAlgo_Loop.hxx>
#include <BRepOffset_Analyse.hxx>
#include <BRepOffset_MakeLoops.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_MapOfShape.hxx>

#include <stdio.h>
#ifdef DRAW
  #include <DBRep.hxx>
Standard_Integer        NbF    = 1;
static Standard_Boolean Affich = Standard_False;
#endif

MakeLoops1::MakeLoops1() {}

//=================================================================================================

void MakeLoops1::Build(const ShapeList&   LF,
                                 const Handle(BRepAlgo_AsDes)& AsDes,
                                 ShapeImage&               Image,
                                 ShapeImage&               theImageVV,
                                 const Message_ProgressRange&  theRange)
{
  TopTools_ListIteratorOfListOfShape it(LF);
  TopTools_ListIteratorOfListOfShape itl, itLCE;
  BRepAlgo_Loop                      Loops;
  Loops.VerticesForSubstitute(myVerVerMap);
  Loops.SetImageVV(theImageVV);
  Message_ProgressScope aPSOuter(theRange, NULL, 2);
  Message_ProgressScope aPS1(aPSOuter.Next(), "Init loops", LF.Size());
  for (; it.More(); it.Next(), aPS1.Next())
  {
    if (!aPS1.More())
    {
      return;
    }
    const TopoFace& F = TopoDS::Face(it.Value());
    //---------------------------
    // Initialization of Loops.
    //---------------------------
    Loops.Init(F);
    //-----------------------------
    // return edges of F.
    //-----------------------------
    const ShapeList& LE = AsDes->Descendant(F);
    ShapeList        AddedEdges;

    for (itl.Initialize(LE); itl.More(); itl.Next())
    {
      TopoEdge E = TopoDS::Edge(itl.Value());
      if (Image.HasImage(E))
      {
        //-------------------------------------------
        // E was already cut in another face.
        // Return the cut edges reorientate them as E.
        // See pb for the edges that have disappeared?
        //-------------------------------------------
        const ShapeList& LCE = Image.Image(E);
        for (itLCE.Initialize(LCE); itLCE.More(); itLCE.Next())
        {
          TopoShape CE = itLCE.Value().Oriented(E.Orientation());
          Loops.AddConstEdge(TopoDS::Edge(CE));
        }
      }
      else
      {
        Loops.AddEdge(E, AsDes->Descendant(E));
        AddedEdges.Append(E);
      }
    }
    //------------------------
    // Unwind.
    //------------------------
    Loops.Perform();
    Loops.WiresToFaces();
    //------------------------
    // MAJ SD.
    //------------------------
    const ShapeList& NF = Loops.NewFaces();
    //-----------------------
    // F => New faces;
    //-----------------------
    Image.Bind(F, NF);

    TopTools_ListIteratorOfListOfShape itAdded;
    for (itAdded.Initialize(AddedEdges); itAdded.More(); itAdded.Next())
    {
      const TopoEdge& E = TopoDS::Edge(itAdded.Value());
      //-----------------------
      //  E => New edges;
      //-----------------------
      const ShapeList& LoopNE = Loops.NewEdges(E);
      if (Image.HasImage(E))
      {
        Image.Add(E, LoopNE);
      }
      else
      {
        Image.Bind(E, LoopNE);
      }
    }
  }
  Loops.GetVerticesForSubstitute(myVerVerMap);
  if (myVerVerMap.IsEmpty())
    return;
  ShapeBuilder          BB;
  Message_ProgressScope aPS2(aPSOuter.Next(), "Building loops", LF.Size());
  for (it.Initialize(LF); it.More(); it.Next(), aPS2.Next())
  {
    if (!aPS2.More())
    {
      return;
    }
    TopoShape         F = it.Value();
    ShapeList LIF;
    Image.LastImage(F, LIF);
    for (itl.Initialize(LIF); itl.More(); itl.Next())
    {
      const TopoShape& IF = itl.Value();
      ShapeExplorer     EdExp(IF, TopAbs_EDGE);
      for (; EdExp.More(); EdExp.Next())
      {
        TopoShape         E = EdExp.Current();
        ShapeList VList;
        TopoDS_Iterator      VerExp(E);
        for (; VerExp.More(); VerExp.Next())
          VList.Append(VerExp.Value());
        TopTools_ListIteratorOfListOfShape itlv(VList);
        for (; itlv.More(); itlv.Next())
        {
          const TopoShape& V = itlv.Value();
          if (myVerVerMap.IsBound(V))
          {
            TopoShape NewV = myVerVerMap(V);
            E.Free(Standard_True);
            NewV.Orientation(V.Orientation());
            Handle(BRep_TVertex)& TV    = *((Handle(BRep_TVertex)*)&V.TShape());
            Handle(BRep_TVertex)& NewTV = *((Handle(BRep_TVertex)*)&NewV.TShape());
            if (TV->Tolerance() > NewTV->Tolerance())
              NewTV->Tolerance(TV->Tolerance());
            NewTV->ChangePoints().Append(TV->ChangePoints());
            AsDes->Replace(V, NewV);
            BB.Remove(E, V);
            BB.Add(E, NewV);
          }
        }
      }
    }
  }
}

//=================================================================================================

static Standard_Boolean IsBetweenCorks(const TopoShape&           E,
                                       const Handle(BRepAlgo_AsDes)& AsDes,
                                       const ShapeList&   LContext)
{
  if (!AsDes->HasAscendant(E))
    return 1;
  const ShapeList&        LF = AsDes->Ascendant(E);
  TopTools_ListIteratorOfListOfShape it;
  for (it.Initialize(LF); it.More(); it.Next())
  {
    const TopoShape&                S     = it.Value();
    Standard_Boolean                   found = 0;
    TopTools_ListIteratorOfListOfShape it2;
    for (it2.Initialize(LContext); it2.More(); it2.Next())
    {
      if (S.IsSame(it2.Value()))
      {
        found = 1;
        break;
      }
    }
    if (!found)
      return 0;
  }
  return 1;
}

//=================================================================================================

void MakeLoops1::BuildOnContext(const ShapeList&   LContext,
                                          const BRepOffset_Analyse&     Analyse,
                                          const Handle(BRepAlgo_AsDes)& AsDes,
                                          ShapeImage&               Image,
                                          const Standard_Boolean        InSide,
                                          const Message_ProgressRange&  theRange)
{
  //-----------------------------------------
  // unwinding of caps.
  //-----------------------------------------
  TopTools_ListIteratorOfListOfShape it(LContext);
  TopTools_ListIteratorOfListOfShape itl, itLCE;
  BRepAlgo_Loop                      Loops;
  Loops.VerticesForSubstitute(myVerVerMap);
  ShapeExplorer     exp;
  TopTools_MapOfShape MapExtent;

  Message_ProgressScope aPS(theRange, "Building deepening faces", LContext.Extent());
  for (; it.More(); it.Next(), aPS.Next())
  {
    if (!aPS.More())
    {
      return;
    }
    const TopoFace&  F = TopoDS::Face(it.Value());
    TopTools_MapOfShape MBound;
    //-----------------------------------------------
    // Initialisation of Loops.
    // F is reversed it will be added in myOffC.
    // and myOffC will be reversed in the final result.
    //-----------------------------------------------
    TopoShape aReversedF = F.Reversed();
    if (InSide)
      Loops.Init(TopoDS::Face(aReversedF));
    //    if (InSide) Loops.Init(TopoDS::Face(F.Reversed()));
    else
      Loops.Init(F);
    //--------------------------------------------------------
    // return edges of F not modified by definition.
    //--------------------------------------------------------
    for (exp.Init(F.Oriented(TopAbs_FORWARD), TopAbs_EDGE); exp.More(); exp.Next())
    {
      TopoEdge CE = TopoDS::Edge(exp.Current());
      MBound.Add(CE);
      if (Analyse.HasAncestor(CE))
      {
        // the stop of cups except for the connectivity stops between caps.
        //      if (!AsDes->HasAscendant(CE)) {
        TopoShape aReversedE = CE.Reversed();
        if (InSide)
          Loops.AddConstEdge(CE);
        else
          Loops.AddConstEdge(TopoDS::Edge(aReversedE));
        //	else        Loops.AddConstEdge(TopoDS::Edge(CE.Reversed()));
      }
    }
    //------------------------------------------------------
    // Trace of offsets + connectivity edge between caps.
    //------------------------------------------------------
    const ShapeList& LE = AsDes->Descendant(F);
    ShapeList        AddedEdges;

    for (itl.Initialize(LE); itl.More(); itl.Next())
    {
      TopoEdge E = TopoDS::Edge(itl.Value());
      if (Image.HasImage(E))
      {
        //-------------------------------------------
        // E was already cut in another face.
        // Return cut edges and orientate them as E.
        // See pb for the edges that have disappeared?
        //-------------------------------------------
        const ShapeList& LCE = Image.Image(E);
        for (itLCE.Initialize(LCE); itLCE.More(); itLCE.Next())
        {
          TopoShape CE = itLCE.Value().Oriented(E.Orientation());
          if (MapExtent.Contains(E))
          {
            Loops.AddConstEdge(TopoDS::Edge(CE));
            continue;
          }
          if (!MBound.Contains(E))
            CE.Reverse();
          if (InSide)
            Loops.AddConstEdge(TopoDS::Edge(CE));
          else
          {
            TopoShape aReversedE = CE.Reversed();
            Loops.AddConstEdge(TopoDS::Edge(aReversedE));
          }
          //	  else        Loops.AddConstEdge(TopoDS::Edge(CE.Reversed()));
        }
      }
      else
      {
        if (IsBetweenCorks(E, AsDes, LContext) && AsDes->HasDescendant(E))
        {
          // connection between 2 caps
          MapExtent.Add(E);
          ShapeList LV;
          if (InSide)
          {
            for (itLCE.Initialize(AsDes->Descendant(E)); itLCE.More(); itLCE.Next())
            {
              LV.Append(itLCE.Value().Reversed());
            }
            Loops.AddEdge(E, LV);
          }
          else
          {
            Loops.AddEdge(E, AsDes->Descendant(E));
          }
          AddedEdges.Append(E);
        }
        else if (IsBetweenCorks(E, AsDes, LContext))
        {
          TopoShape aLocalShape = E.Reversed();
          if (InSide)
            Loops.AddConstEdge(E);
          else
            Loops.AddConstEdge(TopoDS::Edge(aLocalShape));
          //	  if (InSide) Loops.AddConstEdge(TopoDS::Edge(E));
          //	  else        Loops.AddConstEdge(TopoDS::Edge(E.Reversed()));
        }
        else
        {
          TopoShape aLocalShape = E.Reversed();
          if (InSide)
            Loops.AddConstEdge(TopoDS::Edge(aLocalShape));
          else
            Loops.AddConstEdge(E);
          //	  if (InSide) Loops.AddConstEdge(TopoDS::Edge(E.Reversed()));
          //	  else        Loops.AddConstEdge(TopoDS::Edge(E));
        }
      }
    }
    //------------------------
    // Unwind.
    //------------------------
    Loops.Perform();
    Loops.WiresToFaces();
    //------------------------
    // MAJ SD.
    //------------------------
    const ShapeList& NF = Loops.NewFaces();
    //-----------------------
    // F => New faces;
    //-----------------------
    Image.Bind(F, NF);

    TopTools_ListIteratorOfListOfShape itAdded;
    for (itAdded.Initialize(AddedEdges); itAdded.More(); itAdded.Next())
    {
      const TopoEdge& E = TopoDS::Edge(itAdded.Value());
      //-----------------------
      //  E => New edges;
      //-----------------------
      if (Image.HasImage(E))
      {
        Image.Add(E, Loops.NewEdges(E));
      }
      else
      {
        Image.Bind(E, Loops.NewEdges(E));
      }
    }
  }
  Loops.GetVerticesForSubstitute(myVerVerMap);
  if (myVerVerMap.IsEmpty())
    return;
  ShapeBuilder BB;
  for (it.Initialize(LContext); it.More(); it.Next())
  {
    TopoShape         F = it.Value();
    ShapeList LIF;
    Image.LastImage(F, LIF);
    for (itl.Initialize(LIF); itl.More(); itl.Next())
    {
      const TopoShape& IF = itl.Value();
      ShapeExplorer     EdExp(IF, TopAbs_EDGE);
      for (; EdExp.More(); EdExp.Next())
      {
        TopoShape         E = EdExp.Current();
        ShapeList VList;
        TopoDS_Iterator      VerExp(E);
        for (; VerExp.More(); VerExp.Next())
          VList.Append(VerExp.Value());
        TopTools_ListIteratorOfListOfShape itlv(VList);
        for (; itlv.More(); itlv.Next())
        {
          const TopoShape& V = itlv.Value();
          if (myVerVerMap.IsBound(V))
          {
            TopoShape NewV = myVerVerMap(V);
            E.Free(Standard_True);
            NewV.Orientation(V.Orientation());
            Handle(BRep_TVertex)& TV    = *((Handle(BRep_TVertex)*)&V.TShape());
            Handle(BRep_TVertex)& NewTV = *((Handle(BRep_TVertex)*)&NewV.TShape());
            if (TV->Tolerance() > NewTV->Tolerance())
              NewTV->Tolerance(TV->Tolerance());
            NewTV->ChangePoints().Append(TV->ChangePoints());
            AsDes->Replace(V, NewV);
            BB.Remove(E, V);
            BB.Add(E, NewV);
          }
        }
      }
    }
  }
}

//=================================================================================================

void MakeLoops1::BuildFaces(const ShapeList&   LF,
                                      const Handle(BRepAlgo_AsDes)& AsDes,
                                      ShapeImage&               Image,
                                      const Message_ProgressRange&  theRange)
{
  TopTools_ListIteratorOfListOfShape itr, itl, itLCE;
  Standard_Boolean                   ToRebuild;
  BRepAlgo_Loop                      Loops;
  Loops.VerticesForSubstitute(myVerVerMap);
  ShapeBuilder B;

  //----------------------------------
  // Loop on all faces //.
  //----------------------------------
  Message_ProgressScope aPS(theRange, "Building faces", LF.Size());
  for (itr.Initialize(LF); itr.More(); itr.Next(), aPS.Next())
  {
    if (!aPS.More())
    {
      return;
    }
    TopoFace F = TopoDS::Face(itr.Value());
    Loops.Init(F);
    ToRebuild = Standard_False;
    ShapeList AddedEdges;

    if (!Image.HasImage(F))
    {
      //----------------------------------
      // Face F not yet reconstructed.
      //----------------------------------
      const ShapeList& LE = AsDes->Descendant(F);
      //----------------------------------------------------------------
      // first loop to find if the edges of the face were reconstructed.
      // - maj on map MONV. Some vertices on reconstructed edges
      // coincide geometrically with old but are not IsSame.
      //----------------------------------------------------------------
      TopTools_DataMapOfShapeShape MONV;
      TopoVertex                OV1, OV2, NV1, NV2;

      for (itl.Initialize(LE); itl.More(); itl.Next())
      {
        TopoEdge E = TopoDS::Edge(itl.Value());
        if (Image.HasImage(E))
        {
          const ShapeList& LCE = Image.Image(E);
          if (LCE.Extent() == 1 && LCE.First().IsSame(E))
          {
            TopoShape aLocalShape = LCE.First().Oriented(E.Orientation());
            TopoEdge  CE          = TopoDS::Edge(aLocalShape);
            //	    TopoEdge CE = TopoDS::Edge(LCE.First().Oriented(E.Orientation()));
            Loops.AddConstEdge(CE);
            continue;
          }
          //----------------------------------
          // F should be reconstructed.
          //----------------------------------
          ToRebuild = Standard_True;
          for (itLCE.Initialize(LCE); itLCE.More(); itLCE.Next())
          {
            TopoShape aLocalShape = itLCE.Value().Oriented(E.Orientation());
            TopoEdge  CE          = TopoDS::Edge(aLocalShape);
            //	    TopoEdge CE = TopoDS::Edge(itLCE.Value().Oriented(E.Orientation()));
            TopExp1::Vertices(E, OV1, OV2);
            TopExp1::Vertices(CE, NV1, NV2);
            if (!OV1.IsSame(NV1))
              MONV.Bind(OV1, NV1);
            if (!OV2.IsSame(NV2))
              MONV.Bind(OV2, NV2);
            Loops.AddConstEdge(CE);
          }
        }
      }
      if (ToRebuild)
      {
#ifdef DRAW
        if (Affich)
        {
          char name[256];
          sprintf(name, "CF_%d", NbF++);
          DBRep1::Set(name, F);
        }
#endif

        //-----------------------------------------------------------
        // Non-reconstructed edges on other faces are added.
        // If their vertices were reconstructed they are reconstructed.
        //-----------------------------------------------------------
        for (itl.Initialize(LE); itl.More(); itl.Next())
        {
          Standard_Real f, l;
          TopoEdge   E = TopoDS::Edge(itl.Value());
          BRepInspector::Range(E, f, l);
          if (!Image.HasImage(E))
          {
            TopExp1::Vertices(E, OV1, OV2);
            ShapeList LV;
            if (MONV.IsBound(OV1))
            {
              TopoVertex VV = TopoDS::Vertex(MONV(OV1));
              VV.Orientation(TopAbs_FORWARD);
              LV.Append(VV);
              TopoShape aLocalShape = VV.Oriented(TopAbs_INTERNAL);
              B.UpdateVertex(TopoDS::Vertex(aLocalShape), f, E, BRepInspector::Tolerance(VV));
            }
            if (MONV.IsBound(OV2))
            {
              TopoVertex VV = TopoDS::Vertex(MONV(OV2));
              VV.Orientation(TopAbs_REVERSED);
              LV.Append(VV);
              TopoShape aLocalShape = VV.Oriented(TopAbs_INTERNAL);
              B.UpdateVertex(TopoDS::Vertex(aLocalShape), l, E, BRepInspector::Tolerance(VV));
              //	      B.UpdateVertex(TopoDS::Vertex(VV.Oriented(TopAbs_INTERNAL)),
              //			     l,E,BRepInspector::Tolerance(VV));
            }
            if (LV.IsEmpty())
              Loops.AddConstEdge(E);
            else
            {
              Loops.AddEdge(E, LV);
              AddedEdges.Append(E);
            }
          }
        }
      }
    }
    if (ToRebuild)
    {
      //------------------------
      // Reconstruction.
      //------------------------
      Loops.Perform();
      Loops.WiresToFaces();
      //------------------------
      // MAJ SD.
      //------------------------
      const ShapeList& NF = Loops.NewFaces();
      //-----------------------
      // F => New faces;
      //-----------------------
      Image.Bind(F, NF);

      TopTools_ListIteratorOfListOfShape itAdded;
      for (itAdded.Initialize(AddedEdges); itAdded.More(); itAdded.Next())
      {
        const TopoEdge& E = TopoDS::Edge(itAdded.Value());
        //-----------------------
        //  E => New edges;
        //-----------------------
        if (Image.HasImage(E))
        {
          Image.Add(E, Loops.NewEdges(E));
        }
        else
        {
          Image.Bind(E, Loops.NewEdges(E));
        }
      }
    }
  }
  Loops.GetVerticesForSubstitute(myVerVerMap);
  if (myVerVerMap.IsEmpty())
    return;
  ShapeBuilder BB;
  for (itr.Initialize(LF); itr.More(); itr.Next())
  {
    TopoShape         F = itr.Value();
    ShapeList LIF;
    Image.LastImage(F, LIF);
    for (itl.Initialize(LIF); itl.More(); itl.Next())
    {
      const TopoShape& IF = itl.Value();
      ShapeExplorer     EdExp(IF, TopAbs_EDGE);
      for (; EdExp.More(); EdExp.Next())
      {
        TopoShape         E = EdExp.Current();
        ShapeList VList;
        TopoDS_Iterator      VerExp(E);
        for (; VerExp.More(); VerExp.Next())
          VList.Append(VerExp.Value());
        TopTools_ListIteratorOfListOfShape itlv(VList);
        for (; itlv.More(); itlv.Next())
        {
          const TopoShape& V = itlv.Value();
          if (myVerVerMap.IsBound(V))
          {
            TopoShape NewV = myVerVerMap(V);
            E.Free(Standard_True);
            NewV.Orientation(V.Orientation());
            Handle(BRep_TVertex)& TV    = *((Handle(BRep_TVertex)*)&V.TShape());
            Handle(BRep_TVertex)& NewTV = *((Handle(BRep_TVertex)*)&NewV.TShape());
            if (TV->Tolerance() > NewTV->Tolerance())
              NewTV->Tolerance(TV->Tolerance());
            NewTV->ChangePoints().Append(TV->ChangePoints());
            AsDes->Replace(V, NewV);
            BB.Remove(E, V);
            BB.Add(E, NewV);
          }
        }
      }
    }
  }
}
