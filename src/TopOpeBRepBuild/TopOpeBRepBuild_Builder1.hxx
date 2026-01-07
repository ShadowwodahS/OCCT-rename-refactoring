// Created on: 1999-09-29
// Created by: Maxim ZVEREV
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

#ifndef _TopOpeBRepBuild_Builder1_HeaderFile
#define _TopOpeBRepBuild_Builder1_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapOfOrientedShapeInteger.hxx>
#include <TopOpeBRepBuild_Builder.hxx>
#include <TopAbs_State.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopOpeBRepDS_DataMapOfShapeState.hxx>
#include <Standard_Integer.hxx>
#include <TopTools_SequenceOfShape.hxx>
class TopOpeBRepDS_BuildTool;
class TopOpeBRepDS_HDataStructure;
class GTopologyClassifier;
class TopOpeBRepBuild_ShellFaceSet;
class TopOpeBRepBuild_WireEdgeSet;
class TopOpeBRepBuild_PaveSet;
class TopoEdge;
class TopoFace;

//! extension  of  the  class  TopOpeBRepBuild_Builder  dedicated
//! to  avoid  bugs  in  "Rebuilding Result" algorithm  for  the  case  of  SOLID/SOLID  Boolean
//! Operations
class TopOpeBRepBuild_Builder1 : public TopOpeBRepBuild_Builder
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRepBuild_Builder1(const TopOpeBRepDS_BuildTool& BT);

  Standard_EXPORT virtual ~TopOpeBRepBuild_Builder1();

  //! Removes all splits and merges already performed.
  //! Does NOT clear the handled DS  (except  ShapeWithStatesMaps).
  Standard_EXPORT virtual void Clear() Standard_OVERRIDE;

  Standard_EXPORT virtual void Perform(const Handle(TopOpeBRepDS_HDataStructure)& HDS)
    Standard_OVERRIDE;

  Standard_EXPORT virtual void Perform(const Handle(TopOpeBRepDS_HDataStructure)& HDS,
                                       const TopoShape&                        S1,
                                       const TopoShape& S2) Standard_OVERRIDE;

  Standard_EXPORT virtual void MergeKPart() Standard_OVERRIDE;

  Standard_EXPORT virtual void MergeKPart(const TopAbs_State TB1,
                                          const TopAbs_State TB2) Standard_OVERRIDE;

  Standard_EXPORT virtual void GFillSolidSFS(const TopoShape&           SO1,
                                             const ShapeList&   LSO2,
                                             const GTopologyClassifier&  G,
                                             TopOpeBRepBuild_ShellFaceSet& SFS) Standard_OVERRIDE;

  Standard_EXPORT virtual void GFillShellSFS(const TopoShape&           SH1,
                                             const ShapeList&   LSO2,
                                             const GTopologyClassifier&  G,
                                             TopOpeBRepBuild_ShellFaceSet& SFS) Standard_OVERRIDE;

  Standard_EXPORT virtual void GWESMakeFaces(const TopoShape&          FF,
                                             TopOpeBRepBuild_WireEdgeSet& WES,
                                             ShapeList&        LOF) Standard_OVERRIDE;

  Standard_EXPORT void GFillSplitsPVS(const TopoShape&          anEdge,
                                      const GTopologyClassifier& G1,
                                      TopOpeBRepBuild_PaveSet&     PVS);

  Standard_EXPORT void GFillFaceNotSameDomSFS(const TopoShape&           F1,
                                              const ShapeList&   LSO2,
                                              const GTopologyClassifier&  G,
                                              TopOpeBRepBuild_ShellFaceSet& SFS);

  Standard_EXPORT void GFillFaceNotSameDomWES(const TopoShape&          F1,
                                              const ShapeList&  LSO2,
                                              const GTopologyClassifier& G,
                                              TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GFillWireNotSameDomWES(const TopoShape&          W1,
                                              const ShapeList&  LSO2,
                                              const GTopologyClassifier& G,
                                              TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GFillEdgeNotSameDomWES(const TopoShape&          E1,
                                              const ShapeList&  LSO2,
                                              const GTopologyClassifier& G,
                                              TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GFillFaceSameDomSFS(const TopoShape&           F1,
                                           const ShapeList&   LSO2,
                                           const GTopologyClassifier&  G,
                                           TopOpeBRepBuild_ShellFaceSet& SFS);

  Standard_EXPORT void GFillFaceSameDomWES(const TopoShape&          F1,
                                           const ShapeList&  LSO2,
                                           const GTopologyClassifier& G,
                                           TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GFillWireSameDomWES(const TopoShape&          W1,
                                           const ShapeList&  LSO2,
                                           const GTopologyClassifier& G,
                                           TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GFillEdgeSameDomWES(const TopoShape&          E1,
                                           const ShapeList&  LSO2,
                                           const GTopologyClassifier& G,
                                           TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void PerformONParts(const TopoShape&               F,
                                      const TopTools_IndexedMapOfShape& SDfaces,
                                      const GTopologyClassifier&      G,
                                      TopOpeBRepBuild_WireEdgeSet&      WES);

  Standard_EXPORT void PerformPieceIn2D(const TopoEdge&           aPieceToPerform,
                                        const TopoEdge&           aOriginalEdge,
                                        const TopoFace&           edgeFace,
                                        const TopoFace&           toFace,
                                        const GTopologyClassifier& G,
                                        Standard_Boolean&            keep);

  Standard_EXPORT Standard_Integer PerformPieceOn2D(const TopoShape&   aPieceObj,
                                                    const TopoShape&   aFaceObj,
                                                    const TopoShape&   aEdgeObj,
                                                    ShapeList& aListOfPieces,
                                                    ShapeList& aListOfFaces,
                                                    ShapeList& aListOfPiecesOut2d);

  Standard_EXPORT Standard_Integer TwoPiecesON(const TopTools_SequenceOfShape& aSeq,
                                               ShapeList&           aListOfPieces,
                                               ShapeList&           aListOfFaces,
                                               ShapeList&           aListOfPiecesOut2d);

  Standard_EXPORT Standard_Integer CorrectResult2d(TopoShape& aResult);

  friend class TopOpeBRepBuild_HBuilder;

protected:
  Standard_EXPORT void PerformShapeWithStates();

  Standard_EXPORT void PerformShapeWithStates(const TopoShape& anObj, const TopoShape& aTool);

  Standard_EXPORT void StatusEdgesToSplit(const TopoShape&               anObj,
                                          const TopTools_IndexedMapOfShape& anEdgesToSplitMap,
                                          const TopTools_IndexedMapOfShape& anEdgesToRestMap);

  Standard_EXPORT void SplitEdge(const TopoShape&               anEdge,
                                 ShapeList&             aLNew,
                                 TopOpeBRepDS_DataMapOfShapeState& aDataMapOfShapeState);

  Standard_EXPORT void PerformFacesWithStates(const TopoShape&               anObj,
                                              const TopTools_IndexedMapOfShape& aFaces,
                                              TopOpeBRepDS_DataMapOfShapeState& aSplF);

  Standard_EXPORT Standard_Integer IsSame2d(const TopTools_SequenceOfShape& aSeq,
                                            ShapeList&           aListOfPiecesOut2d);

  Standard_EXPORT void OrientateEdgeOnFace(TopoEdge&                 EdgeToPerform,
                                           const TopoFace&           baseFace,
                                           const TopoFace&           edgeFace,
                                           const GTopologyClassifier& G1,
                                           Standard_Boolean&            stateOfFaceOri) const;

  TopTools_DataMapOfShapeListOfShape myFSplits;
  TopTools_DataMapOfShapeListOfShape myESplits;

private:
  TopTools_IndexedMapOfShape                          mySameDomMap;
  TopoShape                                        mySDFaceToFill;
  TopoShape                                        myBaseFaceToFill;
  TopTools_IndexedDataMapOfShapeListOfShape           myMapOfEdgeFaces;
  NCollection_DataMap<TopoShape, Standard_Boolean> myMapOfEdgeWithFaceState;
  TopTools_IndexedMapOfShape                          myProcessedPartsOut2d;
  TopTools_IndexedMapOfShape                          myProcessedPartsON2d;
  TopTools_IndexedMapOfShape                          mySplitsONtoKeep;
  TopTools_IndexedMapOfOrientedShape                  mySourceShapes;
  TopTools_IndexedDataMapOfShapeShape                 myMapOfCorrect2dEdges;
};

#endif // _TopOpeBRepBuild_Builder1_HeaderFile
