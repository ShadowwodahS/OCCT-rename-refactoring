// Created on: 1999-06-18
// Created by: Sergei ZERTCHANINOV
// Copyright (c) 1999 Matra Datavision
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
#include <BRepBuilderAPI_Sewing.hxx>
#include <Geom2d_Curve.hxx>
#include <gp_Pnt.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_WireOrder.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_FaceConnect.hxx>
#include <ShapeFix_Wire.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeShape.hxx>
#include <TopTools_ListOfShape.hxx>

#ifdef OCCT_DEBUG
  #include <TopTools_MapOfShape.hxx>
#endif

//=======================================================================
// function : FaceConnect
//=======================================================================

FaceConnect::FaceConnect() {}

//=================================================================================================

Standard_Boolean FaceConnect::Add(const TopoFace& aFirst, const TopoFace& aSecond)
{
  if (!aFirst.IsNull() && !aSecond.IsNull())
  {
    // Process first face
    if (myConnected.IsBound(aFirst))
    {
      // Find list for the first face
      ShapeList& theFirstList = myConnected(aFirst);
      // Append second face to the first list
      TopTools_ListIteratorOfListOfShape theIter;
      for (theIter.Initialize(theFirstList); theIter.More(); theIter.Next())
        if (theIter.Value().IsSame(aSecond))
          return Standard_True;
      theFirstList.Append(aSecond);
    }
    else
    {
      // Append second face to the first list
      ShapeList theNewFirstList;
      theNewFirstList.Append(aSecond);
      myConnected.Bind(aFirst, theNewFirstList);
    }

    // Process second face if not same
    if (!aFirst.IsSame(aSecond))
    {
      if (myConnected.IsBound(aSecond))
      {
        // No need to iterate on faces - append first
        myConnected(aSecond).Append(aFirst);
      }
      else
      {
        // Append first face to the second list
        ShapeList theNewSecondList;
        theNewSecondList.Append(aFirst);
        myConnected.Bind(aSecond, theNewSecondList);
      }
    }

    return Standard_True;
  }

  return Standard_False;
}

//=================================================================================================

TopoShell FaceConnect::Build(const TopoShell& shell,
                                         const Standard_Real sewtoler,
                                         const Standard_Real fixtoler)
{
  TopoShell result = shell;

  /***************************************************************
  / INITIAL PREPARATIONS
  / Fill map of original free edges,
  / fill maps of resulting free and shared edges
  ***************************************************************/

  // Clear maps of free and shared edges
  myOriFreeEdges.Clear();
  myResFreeEdges.Clear();
  myResSharEdges.Clear();

  TopTools_DataMapOfShapeShape theFreeEdges;
  TopoShape                 theEdge, theFace;

  // Fill map of free edges / faces
  for (TopoDS_Iterator itf(result); itf.More(); itf.Next())
  {
    theFace = itf.Value();
    for (ShapeExplorer expe(theFace, TopAbs_EDGE); expe.More(); expe.Next())
    {
      theEdge = expe.Current();
      if (theFreeEdges.IsBound(theEdge))
        theFreeEdges.UnBind(theEdge);
      else
        theFreeEdges.Bind(theEdge, theFace);
    }
  }

  // Fill maps of original and resulting edges
  for (TopTools_DataMapIteratorOfDataMapOfShapeShape theFEIter(theFreeEdges); theFEIter.More();
       theFEIter.Next())
  {
    // Get pair (face / free edge)
    theEdge = theFEIter.Key1(), theFace = theFEIter.Value();
    // Process faces with bad connectivities only
    if (myConnected.IsBound(theFace) && !BRepInspector::Degenerated(TopoDS::Edge(theEdge)))
    {
      // Add to the map of original free edges
      if (myOriFreeEdges.IsBound(theFace))
      {
        // Append free edge to the existing list
        myOriFreeEdges(theFace).Append(theEdge);
      }
      else
      {
        // Append free edge to the new list
        ShapeList theNewList;
        theNewList.Append(theEdge);
        myOriFreeEdges.Bind(theFace, theNewList);
      }
      // Add to the maps of intermediate free and resulting edges
      if (!myResFreeEdges.IsBound(theEdge))
      {
        ShapeList theFree, theShared;
        theFree.Append(theEdge);
        myResFreeEdges.Bind(theEdge, theFree);
        myResSharEdges.Bind(theEdge, theShared);
      }
    }
  }

  // Clear the temporary map of free edges
  theFreeEdges.Clear();

#ifdef OCCT_DEBUG
  //-------------------------------
  // szv debug - preparation results
  //-------------------------------
  if (!myOriFreeEdges.IsEmpty())
  {
    std::cout << std::endl << "FACE CONNECT PREPARATION RESULTS:" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    Standard_Integer freenum = 0, facenum = 0;
    for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape theOFIter(myOriFreeEdges);
         theOFIter.More();
         theOFIter.Next())
    {
      freenum += theOFIter.Value().Extent();
      facenum++;
    }
    std::cout << "TOTAL: " << facenum << " faces containing " << freenum << " free edges"
              << std::endl;
  }
  //-------------------------------
#endif

  /***************************************************************
  / APPLY SEWING ON CONNECTED FACES
  / Change maps of original free edges and resulting shared edges
  ***************************************************************/

  if (!myOriFreeEdges.IsEmpty())
  {

    // Allocate array of faces to be sewed
    TopoShape           theFirstFace, theSecondFace;
    TopTools_Array1OfShape theFacesToSew(1, 2);
    Standard_Integer       theNumOfFacesToSew = 0;
    Standard_Boolean       skip_pair          = Standard_False;

    TopTools_ListIteratorOfListOfShape theOriginalIter, theResultsIter;
    TopoShape                       theAuxE, theOrigE, theAuxF;

    ShapeBuilder theBuilder;

    TopTools_DataMapOfShapeListOfShape theProcessed;

    for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape theConnectedIter(myConnected);
         theConnectedIter.More();
         theConnectedIter.Next())
    {
      // Process first face only if it is in the map of faces / free edges
      theFirstFace = theConnectedIter.Key1();
      if (myOriFreeEdges.IsBound(theFirstFace))
      {

        // Place first face into the array
        theFacesToSew.SetValue(1, theFirstFace);
        theNumOfFacesToSew = 1;
        // Create the list of processed faces
        ShapeList theProcessedList;

        // Explore the list of connected faces
        const ShapeList&        theConnectedList = theConnectedIter.Value();
        TopTools_ListIteratorOfListOfShape theConnectedListIter;
        for (theConnectedListIter.Initialize(theConnectedList); theConnectedListIter.More();
             theConnectedListIter.Next())
        {
          // Process second face only if it is in the map of faces / free edges
          theSecondFace = theConnectedListIter.Value();
          if (myOriFreeEdges.IsBound(theSecondFace))
          {

            // Place second face into the array
            theFacesToSew.SetValue(2, theSecondFace);
            // Add second face to the list of processed faces
            theProcessedList.Append(theSecondFace);

            // Skip the pair if already processed
            skip_pair = Standard_False;
            if (theProcessed.IsBound(theSecondFace))
            {
              ShapeList&              theProcCnxList = theProcessed(theSecondFace);
              TopTools_ListIteratorOfListOfShape theProcCnxListIter;
              for (theProcCnxListIter.Initialize(theProcCnxList);
                   theProcCnxListIter.More() && !skip_pair;
                   theProcCnxListIter.Next())
                if (theFirstFace.IsSame(theProcCnxListIter.Value()))
                  skip_pair = Standard_True;
            }
            if (!skip_pair)
            {

              // Process second face for the pair of different faces only
              if (theFirstFace.IsSame(theSecondFace))
              {
#ifdef OCCT_DEBUG
                std::cout << "Warning: FaceConnect::Build: Self-connected face"
                          << std::endl;
#endif
              }
              else
                theNumOfFacesToSew = 2;

              TopTools_DataMapOfShapeShape theSewerWires;
              BRepBuilderAPI_Sewing        theSewer(sewtoler);

              // Prepare set of faces containing free edges
              Standard_Integer i = 1;
              for (i = 1; i <= theNumOfFacesToSew; i++)
              {
                // Prepare empty face to fill with free edges
                TopoShape theFaceToSew = theFacesToSew(i);
                theAuxF                   = theFaceToSew.EmptyCopied();
                // Fill empty face with free edges
                for (theOriginalIter.Initialize(myOriFreeEdges(theFaceToSew));
                     theOriginalIter.More();
                     theOriginalIter.Next())
                {
                  for (theResultsIter.Initialize(myResFreeEdges(theOriginalIter.Value()));
                       theResultsIter.More();
                       theResultsIter.Next())
                  {
                    // Bind free edge to wire to find results later
                    theAuxE = theResultsIter.Value();
                    TopoWire theAuxW;
                    theBuilder.MakeWire(theAuxW);
                    theBuilder.Add(theAuxW, theAuxE);
                    theBuilder.Add(theAuxF, theAuxW);
                    theSewerWires.Bind(theAuxE, theAuxW);
                    theSewer.Add(theAuxW);
                  }
                }
                // Add constructed face to sewer
                theSewer.Add(theAuxF);
              }

              // Perform sewing on the list of free edges
              Standard_Boolean sewing_ok = Standard_True;
              {
                try
                {
                  OCC_CATCH_SIGNALS theSewer.Perform();
                }
                catch (ExceptionBase const&)
                {
                  sewing_ok = Standard_False;
                }
              }
              if (sewing_ok)
                if (theSewer.SewedShape().IsNull())
                  sewing_ok = Standard_False;

              if (sewing_ok)
              {
                TopTools_DataMapOfShapeShape theResultEdges;

                // Find modified edges for the faces
                for (i = 1; i <= theNumOfFacesToSew; i++)
                {
                  for (theOriginalIter.Initialize(myOriFreeEdges(theFacesToSew(i)));
                       theOriginalIter.More();
                       theOriginalIter.Next())
                  {
                    // Get original free edge
                    theOrigE                             = theOriginalIter.Value();
                    ShapeList& theOldFreeList = myResFreeEdges(theOrigE);
                    theResultsIter.Initialize(theOldFreeList);
                    while (theResultsIter.More())
                    {
                      theAuxE = theSewerWires(theResultsIter.Value());
                      // Process modified edges
                      if (theSewer.IsModified(theAuxE))
                      {
                        // Fill map of result edges
                        for (ShapeExplorer expe(theSewer.Modified(theAuxE), TopAbs_EDGE);
                             expe.More();
                             expe.Next())
                        {
                          theAuxE = expe.Current();
                          // Check edge for being shared
                          if (theResultEdges.IsBound(theAuxE))
                          {
                            // Edge was shared - move in results list
                            myResSharEdges(theResultEdges(theAuxE)).Append(theAuxE);
                            myResSharEdges(theOrigE).Append(theAuxE);
                            theResultEdges.UnBind(theAuxE);
                          }
                          else
                            theResultEdges.Bind(theAuxE, theOrigE);
                        }
                        // Remove modified free edge from the list
                        theOldFreeList.Remove(theResultsIter);
                      }
                      else
                        theResultsIter.Next();
                    }
                  }
                }

                // Put free edges back to the lists of results
                for (TopTools_DataMapIteratorOfDataMapOfShapeShape theResIter(theResultEdges);
                     theResIter.More();
                     theResIter.Next())
                {
                  theAuxE = theResIter.Key1();
                  myResFreeEdges(theResIter.Value()).Append(theAuxE);
                }
              }
            }
          }
        }

        // Bind the list of processed faces to the processed face
        theProcessed.Bind(theFirstFace, theProcessedList);
      }
    }

    // Clear the temporary map of processed faces
    theProcessed.Clear();

#ifdef OCCT_DEBUG
    //-------------------------------
    // szv debug - sewing results
    //-------------------------------
    std::cout << std::endl << "FACE CONNECT SEWING RESULTS:" << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "Sewing tolerance was set to " << sewtoler << std::endl;
    Standard_Integer totfree = 0, totshared = 0;
    for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape theOF2Iter(myOriFreeEdges);
         theOF2Iter.More();
         theOF2Iter.Next())
    {
      TopTools_ListIteratorOfListOfShape theOFL2Iter;
      for (theOFL2Iter.Initialize(theOF2Iter.Value()); theOFL2Iter.More(); theOFL2Iter.Next())
      {
        totfree += myResFreeEdges(theOFL2Iter.Value()).Extent();
        totshared += myResSharEdges(theOFL2Iter.Value()).Extent();
      }
    }
    std::cout << "TOTAL: " << totfree << " free, " << totshared << " shared edges" << std::endl;
    //-------------------------------
#endif

    /***************************************************************
    / PERFORM EDGES REPLACEMENT
    ***************************************************************/

    TopTools_DataMapOfShapeShape       theRepEdges;
    TopTools_DataMapOfShapeListOfShape theRepVertices;
    TopTools_DataMapOfShapeShape       theOldVertices;
    TopTools_DataMapOfShapeListOfShape theNewVertices;

    // Replace old edges by resulting ones
    TopoWire   theNewW;
    TopoVertex theOldV1, theOldV2, theNewV1, theNewV2, theNewV;
    Point3d        theOldP1, theOldP2;
    Standard_Real dist1, dist2, curdist1, curdist2;
    for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape theOEIter(myOriFreeEdges);
         theOEIter.More();
         theOEIter.Next())
    {
      // Iterate on original free edges
      for (theOriginalIter.Initialize(theOEIter.Value()); theOriginalIter.More();
           theOriginalIter.Next())
      {
        TopoEdge theOldE = TopoDS::Edge(theOriginalIter.Value());

        // Prepare empty wire to add new edges for reshape
        theBuilder.MakeWire(theNewW);

        // Explore new edges and vertices
        Standard_Boolean emptywire = Standard_True;
        for (Standard_Integer i = 1; i <= 2; i++)
        {
          // Select list of free or shared edges
          if (i == 1)
            theResultsIter.Initialize(myResFreeEdges(theOldE));
          else
            theResultsIter.Initialize(myResSharEdges(theOldE));
          // Iterate on new edges
          for (; theResultsIter.More(); theResultsIter.Next())
          {
            theAuxE = theResultsIter.Value();
            if (!theAuxE.IsSame(theOldE))
            {
              // Add new edge to the wire
              theBuilder.Add(theNewW, theAuxE);
              emptywire = Standard_False;
            }
          }
        }

        if (!emptywire)
        {

          // Get vertices on old and new edges
          TopExp1::Vertices(theOldE, theOldV1, theOldV2);
          theOldP1 = BRepInspector::Pnt(theOldV1);
          theOldP2 = BRepInspector::Pnt(theOldV2);

          // Process vertices for replacing
          dist1 = -1.;
          dist2 = -1.;
          for (ShapeExplorer expv(theNewW, TopAbs_VERTEX); expv.More(); expv.Next())
          {
            TopoVertex theNewVtx = TopoDS::Vertex(expv.Current());
            Point3d        theNewPt  = BRepInspector::Pnt(theNewVtx);
            curdist1                = theOldP1.Distance(theNewPt);
            curdist2                = theOldP2.Distance(theNewPt);
            if (dist1 < 0 || curdist1 < dist1)
            {
              dist1    = curdist1;
              theNewV1 = theNewVtx;
            }
            if (dist2 < 0 || curdist2 < dist2)
            {
              dist2    = curdist2;
              theNewV2 = theNewVtx;
            }
          }

          // Place results in map for replacing
          if (!theOldV1.IsSame(theNewV1))
          {
            if (theRepVertices.IsBound(theOldV1))
            {
              ShapeList&              theList1 = theRepVertices(theOldV1);
              TopTools_ListIteratorOfListOfShape theIter1;
              Standard_Boolean                   found = Standard_False;
              for (theIter1.Initialize(theList1); theIter1.More(); theIter1.Next())
                if (theIter1.Value().IsSame(theNewV1))
                {
                  found = Standard_True;
                  break;
                }
              if (!found)
                theList1.Append(theNewV1);
            }
            else
            {
              ShapeList theNewList1;
              theNewList1.Append(theNewV1);
              theRepVertices.Bind(theOldV1, theNewList1);
            }
          }
          if (!theOldV2.IsSame(theNewV2))
          {
            if (theRepVertices.IsBound(theOldV2))
            {
              ShapeList&              theList2 = theRepVertices(theOldV2);
              TopTools_ListIteratorOfListOfShape theIter2;
              Standard_Boolean                   found = Standard_False;
              for (theIter2.Initialize(theList2); theIter2.More(); theIter2.Next())
                if (theIter2.Value().IsSame(theNewV2))
                {
                  found = Standard_True;
                  break;
                }
              if (!found)
                theList2.Append(theNewV2);
            }
            else
            {
              ShapeList theNewList2;
              theNewList2.Append(theNewV2);
              theRepVertices.Bind(theOldV2, theNewList2);
            }
          }

          // Bind edge to replace
          theRepEdges.Bind(theOldE, theNewW);
        }
      }
    }

    if (!theRepEdges.IsEmpty())
    {

      Handle(ShapeBuild_ReShape) theReShape = new ShapeBuild_ReShape;

      // Replace edges
      for (TopTools_DataMapIteratorOfDataMapOfShapeShape theREIter(theRepEdges); theREIter.More();
           theREIter.Next())
      {
        theReShape->Replace(theREIter.Key1() /*.Oriented(TopAbs_FORWARD)*/,
                            theREIter.Value() /*.Oriented(TopAbs_FORWARD)*/);
      }
      // smh#8
      TopoShape tmpReShape = theReShape->Apply(result);
      result                  = TopoDS::Shell(tmpReShape);
      if (theReShape->Status(ShapeExtend_OK))
      {
#ifdef OCCT_DEBUG
        std::cout << "Warning: FaceConnect::Build: Edges not replaced by ReShape"
                  << std::endl;
#endif
      }
      else if (theReShape->Status(ShapeExtend_FAIL1))
      {
#ifdef OCCT_DEBUG
        std::cout << "Error: FaceConnect::Build: ReShape failed on edges" << std::endl;
#endif
      }
      else
      {

        Handle(WireHealer)        SFW = new WireHealer;
        Handle(ShapeFix_Face)        SFF = new ShapeFix_Face;
        Edge1           SAE;
        Standard_Real                f, l;
        Handle(GeomCurve2d)         c2d;
        Handle(ShapeExtend_WireData) sewd;

        // Perform necessary fixes on subshapes
        // smh#8
        TopoShape emptyCopiedShell = result.EmptyCopied();
        TopoShell theShell         = TopoDS::Shell(emptyCopiedShell);
        for (TopoDS_Iterator itf1(result); itf1.More(); itf1.Next())
        {
          TopoFace newface = TopoDS::Face(itf1.Value());
          // smh#8
          TopoShape emptyCopiedFace = newface.EmptyCopied();
          TopoFace  EmpFace         = TopoDS::Face(emptyCopiedFace);
          for (TopoDS_Iterator itw(newface); itw.More(); itw.Next())
          {
            if (itw.Value().ShapeType() != TopAbs_WIRE)
              continue;
            TopoWire theWire = TopoDS::Wire(itw.Value());

            sewd = new ShapeExtend_WireData(theWire);
            ShapeAnalysis_WireOrder SAWO(Standard_False, 0);
            for (Standard_Integer i = 1; i <= sewd->NbEdges(); i++)
            {

              // smh#8
              TopoShape tmpFace = EmpFace.Oriented(TopAbs_FORWARD);
              if (!SAE.PCurve(sewd->Edge(i), TopoDS::Face(tmpFace), c2d, f, l))
                continue;
              SAWO.Add(c2d->Value(f).XY(), c2d->Value(l).XY());
            }
            SAWO.Perform();

            SFW->Load(sewd);
            SFW->FixReorder(SAWO);
            SFW->FixReorder();

            SFW->SetFace(EmpFace);
            SFW->SetPrecision(fixtoler);
            SFW->SetMaxTolerance(sewtoler);

            SFW->FixEdgeCurves();
            SFW->FixSelfIntersection();
            theWire = SFW->Wire();
            theBuilder.Add(EmpFace, theWire);
          }
          // #ifdef AIX  CKY : applies to all platforms
          SFF->Init(EmpFace);
          //	  SFF->Init(TopoDS::Face(EmpFace));

          TopTools_DataMapOfShapeListOfShape MapWires;
          MapWires.Clear();
          if (SFF->FixOrientation(MapWires))
            EmpFace = SFF->Face();
          theBuilder.Add(theShell, EmpFace);
        }
        theShell.Closed(BRepInspector::IsClosed(theShell));
        result = theShell;

        if (!theRepVertices.IsEmpty())
        {

          // Prepare vertices to replace
          TopoShape theOld, theNew, theRep, theAux;
          for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape theRV1Iter(theRepVertices);
               theRV1Iter.More();
               theRV1Iter.Next())
          {
            // Get the old vertex, create empty list of replaced vertices
            theOld = theRV1Iter.Key1();
            ShapeList theNewList;
            // Explore the list of new vertices
            TopTools_ListIteratorOfListOfShape theN1Iter;
            for (theN1Iter.Initialize(theRV1Iter.Value()); theN1Iter.More(); theN1Iter.Next())
            {
              theNew = theN1Iter.Value();
              if (theOldVertices.IsBound(theNew))
              {
                // Vertex has a replacing vertex in the map
                theRep = theOldVertices(theNew);
                if (!theRep.IsSame(theOld))
                {
                  // Vertex is not in current list
                  theOldVertices.Bind(theRep, theOld);
                  theNewList.Append(theRep);
                  TopTools_ListIteratorOfListOfShape theN3Iter;
                  for (theN3Iter.Initialize(theNewVertices(theRep)); theN3Iter.More();
                       theN3Iter.Next())
                  {
                    theAux                 = theN3Iter.Value();
                    theOldVertices(theAux) = theOld;
                    theNewList.Append(theAux);
                  }
                  theNewVertices.UnBind(theRep);
                }
              }
              else
              {
                theOldVertices.Bind(theNew, theOld);
                theNewList.Append(theNew);
              }
            }
            theNewVertices.Bind(theOld, theNewList);
          }

          // Update vertices positions and tolerances
          TopoVertex theNewVert, theOldVert;
          for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape theRV2Iter(theNewVertices);
               theRV2Iter.More();
               theRV2Iter.Next())
          {
            theNewVert = TopoDS::Vertex(theRV2Iter.Key1());
            // Calculate the vertex position
            Point3d theLBound, theRBound, thePosition;
            theLBound = theRBound = BRepInspector::Pnt(theNewVert);
            TopTools_ListIteratorOfListOfShape theN2Iter;
            for (theN2Iter.Initialize(theRV2Iter.Value()); theN2Iter.More(); theN2Iter.Next())
            {
              thePosition       = BRepInspector::Pnt(TopoDS::Vertex(theN2Iter.Value()));
              Standard_Real val = thePosition.X();
              if (val < theLBound.X())
                theLBound.SetX(val);
              else if (val > theRBound.X())
                theRBound.SetX(val);
              val = thePosition.Y();
              if (val < theLBound.Y())
                theLBound.SetY(val);
              else if (val > theRBound.Y())
                theRBound.SetY(val);
              val = thePosition.Z();
              if (val < theLBound.Z())
                theLBound.SetZ(val);
              else if (val > theRBound.Z())
                theRBound.SetZ(val);
            }
            thePosition                = Point3d((theLBound.XYZ() + theRBound.XYZ()) / 2.);
            Standard_Real theTolerance = 0., curtoler;
            // Calculate the vertex tolerance
            for (theN2Iter.Initialize(theRV2Iter.Value()); theN2Iter.More(); theN2Iter.Next())
            {
              theOldVert = TopoDS::Vertex(theN2Iter.Value());
              curtoler =
                thePosition.Distance(BRepInspector::Pnt(theOldVert)) + BRepInspector::Tolerance(theOldVert);
              if (curtoler > theTolerance)
                theTolerance = curtoler;
            }
            curtoler =
              thePosition.Distance(BRepInspector::Pnt(theNewVert)) + BRepInspector::Tolerance(theNewVert);
            if (curtoler > theTolerance)
              theTolerance = curtoler;
            theBuilder.UpdateVertex(theNewVert, thePosition, theTolerance);
          }

          // Replace vertices
          theReShape->Clear();
          for (TopTools_DataMapIteratorOfDataMapOfShapeShape theNVIter(theOldVertices);
               theNVIter.More();
               theNVIter.Next())
            theReShape->Replace(theNVIter.Key1().Oriented(TopAbs_FORWARD),
                                theNVIter.Value().Oriented(TopAbs_FORWARD));
          // smh#8
          TopoShape tmpshape = theReShape->Apply(result);
          result                = TopoDS::Shell(tmpshape);

          if (theReShape->Status(ShapeExtend_FAIL1))
          {
#ifdef OCCT_DEBUG
            std::cout << "Error: FaceConnect::Build: ReShape failed on vertices"
                      << std::endl;
#endif
          }
        }

#ifdef OCCT_DEBUG
        //-------------------------------
        // szv debug - reshape results
        //-------------------------------
        std::cout << std::endl << "FACE CONNECT REPLACEMENT RESULTS:" << std::endl;
        std::cout << "---------------------------------" << std::endl;
        TopTools_MapOfShape theTmpMap;
        Standard_Integer    toteold = 0, totenew = 0;
        for (TopTools_DataMapIteratorOfDataMapOfShapeShape theR1Iter(theRepEdges); theR1Iter.More();
             theR1Iter.Next())
        {
          toteold++;
          if (!theTmpMap.Contains(theR1Iter.Value()))
          {
            theTmpMap.Add(theR1Iter.Value());
            for (TopoDS_Iterator itw(TopoDS::Wire(theR1Iter.Value())); itw.More(); itw.Next())
              totenew++;
          }
        }
        Standard_Integer totvold = 0, totvnew = 0;
        for (TopTools_DataMapIteratorOfDataMapOfShapeShape theR2Iter(theOldVertices);
             theR2Iter.More();
             theR2Iter.Next())
        {
          totvold++;
          if (!theTmpMap.Contains(theR2Iter.Value()))
          {
            theTmpMap.Add(theR2Iter.Value());
            totvnew++;
          }
        }
        std::cout << "TOTAL: " << toteold << " edges, " << totvold << " vertices replaced by "
                  << totenew << " edges, " << totvnew << " vertices" << std::endl
                  << std::endl;
        //-------------------------------
#endif
      }
    }
  }

  return result;
}

//=================================================================================================

void FaceConnect::Clear()
{
  myConnected.Clear();
  myOriFreeEdges.Clear();
  myResFreeEdges.Clear();
  myResSharEdges.Clear();
}
