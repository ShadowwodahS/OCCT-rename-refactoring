// Created on: 1993-06-14
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

#ifndef _TopOpeBRepBuild_Builder_HeaderFile
#define _TopOpeBRepBuild_Builder_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopAbs_State.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRepDS_BuildTool.hxx>
#include <TopTools_HArray1OfShape.hxx>
#include <TopTools_DataMapOfIntegerListOfShape.hxx>
#include <TopTools_HArray1OfListOfShape.hxx>
#include <TopOpeBRepDS_DataMapOfShapeListOfShapeOn1State.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <Standard_Integer.hxx>
#include <TopOpeBRepTool_ShapeClassifier.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>
#include <TopTools_IndexedMapOfOrientedShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopTools_DataMapOfIntegerShape.hxx>
#include <TopOpeBRepDS_Config.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
class TopOpeBRepDS_HDataStructure;
class TopOpeBRepTool_ShapeExplorer;
class TopOpeBRepBuild_ShapeSet;
class TopOpeBRepBuild_EdgeBuilder;
class TopOpeBRepBuild_FaceBuilder;
class TopOpeBRepBuild_SolidBuilder;
class TopOpeBRepBuild_WireEdgeSet;
class PointIterator;
class TopOpeBRepBuild_PaveSet;
class GTopologyClassifier;
class TopOpeBRepBuild_ShellFaceSet;
class SurfaceIterator;
class CurveIterator;
class TopoVertex;
class Point3d;

// resolve name collisions with X11 headers
#ifdef FillSolid
  #undef FillSolid
#endif

//! The Builder  algorithm    constructs   topological
//! objects  from   an    existing  topology  and  new
//! geometries attached to the topology. It is used to
//! construct the result of a topological operation;
//! the existing  topologies are the parts involved in
//! the  topological  operation and the new geometries
//! are the intersection lines and points.
class TopOpeBRepBuild_Builder
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRepBuild_Builder(const TopOpeBRepDS_BuildTool& BT);

  Standard_EXPORT virtual ~TopOpeBRepBuild_Builder();

  Standard_EXPORT TopOpeBRepDS_BuildTool& ChangeBuildTool();

  Standard_EXPORT const TopOpeBRepDS_BuildTool& BuildTool() const;

  //! Stores the data structure <HDS>,
  //! Create shapes from the new geometries.
  Standard_EXPORT virtual void Perform(const Handle(TopOpeBRepDS_HDataStructure)& HDS);

  //! Stores the data structure <HDS>,
  //! Create shapes from the new geometries,
  //! Evaluates if an operation performed on shapes S1,S2
  //! is a particular case.
  Standard_EXPORT virtual void Perform(const Handle(TopOpeBRepDS_HDataStructure)& HDS,
                                       const TopoShape&                        S1,
                                       const TopoShape&                        S2);

  //! returns the DS handled by this builder
  Standard_EXPORT Handle(TopOpeBRepDS_HDataStructure) DataStructure() const;

  //! Removes all splits and merges already performed.
  //! Does NOT clear the handled DS.
  Standard_EXPORT virtual void Clear();

  //! Merges  the two edges <S1> and <S2> keeping the
  //! parts in each edge of states <TB1> and <TB2>.
  //! Booleans onA, onB, onAB indicate whether parts of edges
  //! found as state ON respectively on first, second, and both
  //! shapes must be (or not) built.
  Standard_EXPORT void MergeEdges(const ShapeList& L1,
                                  const TopAbs_State          TB1,
                                  const ShapeList& L2,
                                  const TopAbs_State          TB2,
                                  const Standard_Boolean      onA  = Standard_False,
                                  const Standard_Boolean      onB  = Standard_False,
                                  const Standard_Boolean      onAB = Standard_False);

  //! Merges  the two faces <S1>   and <S2> keeping the
  //! parts in each face of states <TB1> and <TB2>.
  Standard_EXPORT void MergeFaces(const ShapeList& S1,
                                  const TopAbs_State          TB1,
                                  const ShapeList& S2,
                                  const TopAbs_State          TB2,
                                  const Standard_Boolean      onA  = Standard_False,
                                  const Standard_Boolean      onB  = Standard_False,
                                  const Standard_Boolean      onAB = Standard_False);

  //! Merges  the two solids <S1>   and <S2> keeping the
  //! parts in each solid of states <TB1> and <TB2>.
  Standard_EXPORT void MergeSolids(const TopoShape& S1,
                                   const TopAbs_State  TB1,
                                   const TopoShape& S2,
                                   const TopAbs_State  TB2);

  //! Merges the two shapes <S1> and <S2> keeping the
  //! parts of states <TB1>,<TB2> in <S1>,<S2>.
  Standard_EXPORT void MergeShapes(const TopoShape& S1,
                                   const TopAbs_State  TB1,
                                   const TopoShape& S2,
                                   const TopAbs_State  TB2);

  Standard_EXPORT void End();

  Standard_EXPORT Standard_Boolean Classify() const;

  Standard_EXPORT void ChangeClassify(const Standard_Boolean B);

  //! Merges the solid <S>  keeping the
  //! parts of state <TB>.
  Standard_EXPORT void MergeSolid(const TopoShape& S, const TopAbs_State TB);

  //! Returns the vertex created on point <I>.
  Standard_EXPORT const TopoShape& NewVertex(const Standard_Integer I) const;

  //! Returns the edges created on curve <I>.
  Standard_EXPORT const ShapeList& NewEdges(const Standard_Integer I) const;

  //! Returns the faces created on surface <I>.
  Standard_EXPORT const ShapeList& NewFaces(const Standard_Integer I) const;

  //! Returns True if the shape <S> has been split.
  Standard_EXPORT Standard_Boolean IsSplit(const TopoShape& S, const TopAbs_State TB) const;

  //! Returns the split parts <TB> of shape <S>.
  Standard_EXPORT const ShapeList& Splits(const TopoShape& S,
                                                     const TopAbs_State  TB) const;

  //! Returns True if the shape <S> has been merged.
  Standard_EXPORT Standard_Boolean IsMerged(const TopoShape& S, const TopAbs_State TB) const;

  //! Returns the merged parts <TB> of shape <S>.
  Standard_EXPORT const ShapeList& Merged(const TopoShape& S,
                                                     const TopAbs_State  TB) const;

  Standard_EXPORT void InitSection();

  //! create parts ON solid of section edges
  Standard_EXPORT void SplitSectionEdges();

  //! create parts ON solid of section edges
  Standard_EXPORT virtual void SplitSectionEdge(const TopoShape& E);

  //! return the section edges built on new curves.
  Standard_EXPORT void SectionCurves(ShapeList& L);

  //! return the parts of edges found ON the boundary
  //! of the two arguments S1,S2 of Perform()
  Standard_EXPORT void SectionEdges(ShapeList& L);

  //! Fills anAncMap with pairs (edge,ancestor edge) for each
  //! split from the map aMapON for the shape object identified
  //! by ShapeRank
  Standard_EXPORT void FillSecEdgeAncestorMap(const Standard_Integer        aShapeRank,
                                              const TopTools_MapOfShape&    aMapON,
                                              TopTools_DataMapOfShapeShape& anAncMap) const;

  //! return all section edges.
  Standard_EXPORT void Section(ShapeList& L);

  Standard_EXPORT const ShapeList& Section();

  //! update the DS by creating new geometries.
  //! create vertices on DS points.
  Standard_EXPORT void BuildVertices(const Handle(TopOpeBRepDS_HDataStructure)& DS);

  //! update the DS by creating new geometries.
  //! create shapes from the new geometries.
  Standard_EXPORT void BuildEdges(const Handle(TopOpeBRepDS_HDataStructure)& DS);

  Standard_EXPORT const TopOpeBRepDS_DataMapOfShapeListOfShapeOn1State& MSplit(
    const TopAbs_State s) const;

  Standard_EXPORT TopOpeBRepDS_DataMapOfShapeListOfShapeOn1State& ChangeMSplit(
    const TopAbs_State s);

  Standard_EXPORT void MakeEdges(const TopoShape&          E,
                                 TopOpeBRepBuild_EdgeBuilder& B,
                                 ShapeList&        L);

  Standard_EXPORT void MakeFaces(const TopoShape&          F,
                                 TopOpeBRepBuild_FaceBuilder& B,
                                 ShapeList&        L);

  Standard_EXPORT void MakeSolids(TopOpeBRepBuild_SolidBuilder& B, ShapeList& L);

  Standard_EXPORT void MakeShells(TopOpeBRepBuild_SolidBuilder& B, ShapeList& L);

  //! Returns a ref.on the list of shapes connected to <S> as
  //! <TB> split parts of <S>.
  //! Mark <S> as split in <TB> parts.
  Standard_EXPORT ShapeList& ChangeSplit(const TopoShape& S, const TopAbs_State TB);

  Standard_EXPORT Standard_Boolean Opec12() const;

  Standard_EXPORT Standard_Boolean Opec21() const;

  Standard_EXPORT Standard_Boolean Opecom() const;

  Standard_EXPORT Standard_Boolean Opefus() const;

  Standard_EXPORT TopAbs_State ShapePosition(const TopoShape& S, const ShapeList& LS);

  Standard_EXPORT Standard_Boolean KeepShape(const TopoShape&         S,
                                             const ShapeList& LS,
                                             const TopAbs_State          T);

  Standard_EXPORT static TopAbs_ShapeEnum TopType(const TopoShape& S);

  Standard_EXPORT static Standard_Boolean Reverse(const TopAbs_State T1, const TopAbs_State T2);

  Standard_EXPORT static TopAbs_Orientation Orient(const TopAbs_Orientation O,
                                                   const Standard_Boolean   R);

  Standard_EXPORT void FindSameDomain(ShapeList& L1, ShapeList& L2) const;

  Standard_EXPORT void FindSameDomainSameOrientation(ShapeList& LSO,
                                                     ShapeList& LDO) const;

  Standard_EXPORT void MapShapes(const TopoShape& S1, const TopoShape& S2);

  Standard_EXPORT void ClearMaps();

  Standard_EXPORT void FindSameRank(const ShapeList& L1,
                                    const Standard_Integer      R,
                                    ShapeList&       L2) const;

  Standard_EXPORT Standard_Integer ShapeRank(const TopoShape& S) const;

  Standard_EXPORT Standard_Boolean IsShapeOf(const TopoShape&    S,
                                             const Standard_Integer I12) const;

  Standard_EXPORT static Standard_Boolean Contains(const TopoShape&         S,
                                                   const ShapeList& L);

  Standard_EXPORT Standard_Integer FindIsKPart();

  Standard_EXPORT Standard_Integer IsKPart() const;

  Standard_EXPORT virtual void MergeKPart();

  Standard_EXPORT virtual void MergeKPart(const TopAbs_State TB1, const TopAbs_State TB2);

  Standard_EXPORT void MergeKPartiskole();

  Standard_EXPORT void MergeKPartiskoletge();

  Standard_EXPORT void MergeKPartisdisj();

  Standard_EXPORT void MergeKPartisfafa();

  Standard_EXPORT void MergeKPartissoso();

  Standard_EXPORT Standard_Integer KPiskole();

  Standard_EXPORT Standard_Integer KPiskoletge();

  Standard_EXPORT Standard_Integer KPisdisj();

  Standard_EXPORT Standard_Integer KPisfafa();

  Standard_EXPORT Standard_Integer KPissoso();

  Standard_EXPORT void KPClearMaps();

  Standard_EXPORT Standard_Integer KPlhg(const TopoShape&    S,
                                         const TopAbs_ShapeEnum T,
                                         ShapeList&  L) const;

  Standard_EXPORT Standard_Integer KPlhg(const TopoShape& S, const TopAbs_ShapeEnum T) const;

  Standard_EXPORT Standard_Integer KPlhsd(const TopoShape&    S,
                                          const TopAbs_ShapeEnum T,
                                          ShapeList&  L) const;

  Standard_EXPORT Standard_Integer KPlhsd(const TopoShape& S, const TopAbs_ShapeEnum T) const;

  Standard_EXPORT TopAbs_State KPclasSS(const TopoShape&         S1,
                                        const ShapeList& exceptLS1,
                                        const TopoShape&         S2);

  Standard_EXPORT TopAbs_State KPclasSS(const TopoShape& S1,
                                        const TopoShape& exceptS1,
                                        const TopoShape& S2);

  Standard_EXPORT TopAbs_State KPclasSS(const TopoShape& S1, const TopoShape& S2);

  Standard_EXPORT Standard_Boolean KPiskolesh(const TopoShape&   S,
                                              ShapeList& LS,
                                              ShapeList& LF) const;

  Standard_EXPORT Standard_Boolean KPiskoletgesh(const TopoShape&   S,
                                                 ShapeList& LS,
                                                 ShapeList& LF) const;

  Standard_EXPORT void KPSameDomain(ShapeList& L1, ShapeList& L2) const;

  Standard_EXPORT Standard_Integer KPisdisjsh(const TopoShape& S) const;

  Standard_EXPORT Standard_Integer KPisfafash(const TopoShape& S) const;

  Standard_EXPORT Standard_Integer KPissososh(const TopoShape& S) const;

  Standard_EXPORT void KPiskoleanalyse(const TopAbs_State FT1,
                                       const TopAbs_State FT2,
                                       const TopAbs_State ST1,
                                       const TopAbs_State ST2,
                                       Standard_Integer&  I,
                                       Standard_Integer&  I1,
                                       Standard_Integer&  I2) const;

  Standard_EXPORT void KPiskoletgeanalyse(const TopOpeBRepDS_Config Conf,
                                          const TopAbs_State        ST1,
                                          const TopAbs_State        ST2,
                                          Standard_Integer&         I) const;

  Standard_EXPORT void KPisdisjanalyse(const TopAbs_State ST1,
                                       const TopAbs_State ST2,
                                       Standard_Integer&  I,
                                       Standard_Integer&  IC1,
                                       Standard_Integer&  IC2) const;

  Standard_EXPORT static Standard_Integer KPls(const TopoShape&    S,
                                               const TopAbs_ShapeEnum T,
                                               ShapeList&  L);

  Standard_EXPORT static Standard_Integer KPls(const TopoShape& S, const TopAbs_ShapeEnum T);

  Standard_EXPORT TopAbs_State KPclassF(const TopoShape& F1, const TopoShape& F2);

  Standard_EXPORT void KPclassFF(const TopoShape& F1,
                                 const TopoShape& F2,
                                 TopAbs_State&       T1,
                                 TopAbs_State&       T2);

  Standard_EXPORT Standard_Boolean KPiskoleFF(const TopoShape& F1,
                                              const TopoShape& F2,
                                              TopAbs_State&       T1,
                                              TopAbs_State&       T2);

  Standard_EXPORT static Standard_Boolean KPContains(const TopoShape&         S,
                                                     const ShapeList& L);

  Standard_EXPORT TopoShape KPmakeface(const TopoShape&         F1,
                                          const ShapeList& LF2,
                                          const TopAbs_State          T1,
                                          const TopAbs_State          T2,
                                          const Standard_Boolean      R1,
                                          const Standard_Boolean      R2);

  Standard_EXPORT static Standard_Integer KPreturn(const Standard_Integer KP);

  Standard_EXPORT void SplitEvisoONperiodicF();

  Standard_EXPORT void GMergeSolids(const ShapeList&  LSO1,
                                    const ShapeList&  LSO2,
                                    const GTopologyClassifier& G);

  Standard_EXPORT void GFillSolidsSFS(const ShapeList&   LSO1,
                                      const ShapeList&   LSO2,
                                      const GTopologyClassifier&  G,
                                      TopOpeBRepBuild_ShellFaceSet& SFS);

  Standard_EXPORT virtual void GFillSolidSFS(const TopoShape&           SO1,
                                             const ShapeList&   LSO2,
                                             const GTopologyClassifier&  G,
                                             TopOpeBRepBuild_ShellFaceSet& SFS);

  Standard_EXPORT void GFillSurfaceTopologySFS(const TopoShape&           SO1,
                                               const GTopologyClassifier&  G,
                                               TopOpeBRepBuild_ShellFaceSet& SFS);

  Standard_EXPORT void GFillSurfaceTopologySFS(const SurfaceIterator& IT,
                                               const GTopologyClassifier&        G,
                                               TopOpeBRepBuild_ShellFaceSet&       SFS) const;

  Standard_EXPORT virtual void GFillShellSFS(const TopoShape&           SH1,
                                             const ShapeList&   LSO2,
                                             const GTopologyClassifier&  G,
                                             TopOpeBRepBuild_ShellFaceSet& SFS);

  Standard_EXPORT void GFillFaceSFS(const TopoShape&           F1,
                                    const ShapeList&   LSO2,
                                    const GTopologyClassifier&  G,
                                    TopOpeBRepBuild_ShellFaceSet& SFS);

  Standard_EXPORT void GSplitFaceSFS(const TopoShape&           F1,
                                     const ShapeList&   LSclass,
                                     const GTopologyClassifier&  G,
                                     TopOpeBRepBuild_ShellFaceSet& SFS);

  Standard_EXPORT void GMergeFaceSFS(const TopoShape&           F,
                                     const GTopologyClassifier&  G,
                                     TopOpeBRepBuild_ShellFaceSet& SFS);

  Standard_EXPORT void GSplitFace(const TopoShape&          F,
                                  const GTopologyClassifier& G,
                                  const ShapeList&  LSclass);

  Standard_EXPORT void AddONPatchesSFS(const GTopologyClassifier&  G,
                                       TopOpeBRepBuild_ShellFaceSet& SFS);

  Standard_EXPORT void FillOnPatches(const ShapeList&               anEdgesON,
                                     const TopoShape&                       aBaseFace,
                                     const TopTools_IndexedMapOfOrientedShape& avoidMap);

  Standard_EXPORT void FindFacesTouchingEdge(const TopoShape&    aFace,
                                             const TopoShape&    anEdge,
                                             const Standard_Integer aShRank,
                                             ShapeList&  aFaces) const;

  Standard_EXPORT void GMergeFaces(const ShapeList&  LF1,
                                   const ShapeList&  LF2,
                                   const GTopologyClassifier& G);

  Standard_EXPORT void GFillFacesWES(const ShapeList&  LF1,
                                     const ShapeList&  LF2,
                                     const GTopologyClassifier& G,
                                     TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GFillFacesWESK(const ShapeList&  LF1,
                                      const ShapeList&  LF2,
                                      const GTopologyClassifier& G,
                                      TopOpeBRepBuild_WireEdgeSet& WES,
                                      const Standard_Integer       K);

  Standard_EXPORT void GFillFacesWESMakeFaces(const ShapeList&  LF1,
                                              const ShapeList&  LF2,
                                              const ShapeList&  LSO,
                                              const GTopologyClassifier& G);

  Standard_EXPORT void GFillFaceWES(const TopoShape&          F,
                                    const ShapeList&  LF2,
                                    const GTopologyClassifier& G,
                                    TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GFillCurveTopologyWES(const TopoShape&          F,
                                             const GTopologyClassifier& G,
                                             TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GFillCurveTopologyWES(const CurveIterator& IT,
                                             const GTopologyClassifier&      G,
                                             TopOpeBRepBuild_WireEdgeSet&      WES) const;

  Standard_EXPORT void GFillONPartsWES(const TopoShape&          F,
                                       const GTopologyClassifier& G,
                                       const ShapeList&  LSclass,
                                       TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GFillWireWES(const TopoShape&          W,
                                    const ShapeList&  LF2,
                                    const GTopologyClassifier& G,
                                    TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GFillEdgeWES(const TopoShape&          E,
                                    const ShapeList&  LF2,
                                    const GTopologyClassifier& G,
                                    TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GSplitEdgeWES(const TopoShape&          E,
                                     const ShapeList&  LF2,
                                     const GTopologyClassifier& G,
                                     TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GMergeEdgeWES(const TopoShape&          E,
                                     const GTopologyClassifier& G,
                                     TopOpeBRepBuild_WireEdgeSet& WES);

  Standard_EXPORT void GSplitEdge(const TopoShape&          E,
                                  const GTopologyClassifier& G,
                                  const ShapeList&  LSclass);

  Standard_EXPORT void GMergeEdges(const ShapeList&  LE1,
                                   const ShapeList&  LE2,
                                   const GTopologyClassifier& G);

  Standard_EXPORT void GFillEdgesPVS(const ShapeList&  LE1,
                                     const ShapeList&  LE2,
                                     const GTopologyClassifier& G,
                                     TopOpeBRepBuild_PaveSet&     PVS);

  Standard_EXPORT void GFillEdgePVS(const TopoShape&          E,
                                    const ShapeList&  LE2,
                                    const GTopologyClassifier& G,
                                    TopOpeBRepBuild_PaveSet&     PVS);

  Standard_EXPORT void GFillPointTopologyPVS(const TopoShape&          E,
                                             const GTopologyClassifier& G,
                                             TopOpeBRepBuild_PaveSet&     PVS);

  Standard_EXPORT void GFillPointTopologyPVS(const TopoShape&               E,
                                             const PointIterator& IT,
                                             const GTopologyClassifier&      G,
                                             TopOpeBRepBuild_PaveSet&          PVS) const;

  Standard_EXPORT Standard_Boolean GParamOnReference(const TopoVertex& V,
                                                     const TopoEdge&   E,
                                                     Standard_Real&       P) const;

  Standard_EXPORT Standard_Boolean GKeepShape(const TopoShape&         S,
                                              const ShapeList& Lref,
                                              const TopAbs_State          T);

  //! return True if S is classified <T> / Lref shapes
  Standard_EXPORT Standard_Boolean GKeepShape1(const TopoShape&         S,
                                               const ShapeList& Lref,
                                               const TopAbs_State          T,
                                               TopAbs_State&               pos);

  //! add to Lou the shapes of Lin classified <T> / Lref shapes.
  //! Lou is not cleared. (S is a dummy trace argument)
  Standard_EXPORT void GKeepShapes(const TopoShape&         S,
                                   const ShapeList& Lref,
                                   const TopAbs_State          T,
                                   const ShapeList& Lin,
                                   ShapeList&       Lou);

  Standard_EXPORT void GSFSMakeSolids(const TopoShape&           SOF,
                                      TopOpeBRepBuild_ShellFaceSet& SFS,
                                      ShapeList&         LOSO);

  Standard_EXPORT void GSOBUMakeSolids(const TopoShape&           SOF,
                                       TopOpeBRepBuild_SolidBuilder& SOBU,
                                       ShapeList&         LOSO);

  Standard_EXPORT virtual void GWESMakeFaces(const TopoShape&          FF,
                                             TopOpeBRepBuild_WireEdgeSet& WES,
                                             ShapeList&        LOF);

  Standard_EXPORT void GFABUMakeFaces(const TopoShape&             FF,
                                      TopOpeBRepBuild_FaceBuilder&    FABU,
                                      ShapeList&           LOF,
                                      TopTools_DataMapOfShapeInteger& MWisOld);

  Standard_EXPORT void RegularizeFaces(const TopoShape&         FF,
                                       const ShapeList& lnewFace,
                                       ShapeList&       LOF);

  Standard_EXPORT void RegularizeFace(const TopoShape&   FF,
                                      const TopoShape&   newFace,
                                      ShapeList& LOF);

  Standard_EXPORT void RegularizeSolids(const TopoShape&         SS,
                                        const ShapeList& lnewSolid,
                                        ShapeList&       LOS);

  Standard_EXPORT void RegularizeSolid(const TopoShape&   SS,
                                       const TopoShape&   newSolid,
                                       ShapeList& LOS);

  Standard_EXPORT void GPVSMakeEdges(const TopoShape&      EF,
                                     TopOpeBRepBuild_PaveSet& PVS,
                                     ShapeList&    LOE) const;

  Standard_EXPORT void GEDBUMakeEdges(const TopoShape&          EF,
                                      TopOpeBRepBuild_EdgeBuilder& EDBU,
                                      ShapeList&        LOE) const;

  Standard_EXPORT Standard_Boolean GToSplit(const TopoShape& S, const TopAbs_State TB) const;

  Standard_EXPORT Standard_Boolean GToMerge(const TopoShape& S) const;

  Standard_EXPORT static Standard_Boolean GTakeCommonOfSame(const GTopologyClassifier& G);

  Standard_EXPORT static Standard_Boolean GTakeCommonOfDiff(const GTopologyClassifier& G);

  Standard_EXPORT void GFindSamDom(const TopoShape&   S,
                                   ShapeList& L1,
                                   ShapeList& L2) const;

  Standard_EXPORT void GFindSamDom(ShapeList& L1, ShapeList& L2) const;

  Standard_EXPORT void GFindSamDomSODO(const TopoShape&   S,
                                       ShapeList& LSO,
                                       ShapeList& LDO) const;

  Standard_EXPORT void GFindSamDomSODO(ShapeList& LSO, ShapeList& LDO) const;

  Standard_EXPORT void GMapShapes(const TopoShape& S1, const TopoShape& S2);

  Standard_EXPORT void GClearMaps();

  Standard_EXPORT void GFindSameRank(const ShapeList& L1,
                                     const Standard_Integer      R,
                                     ShapeList&       L2) const;

  Standard_EXPORT Standard_Integer GShapeRank(const TopoShape& S) const;

  Standard_EXPORT Standard_Boolean GIsShapeOf(const TopoShape&    S,
                                              const Standard_Integer I12) const;

  Standard_EXPORT static Standard_Boolean GContains(const TopoShape&         S,
                                                    const ShapeList& L);

  Standard_EXPORT static void GCopyList(const ShapeList& Lin,
                                        const Standard_Integer      i1,
                                        const Standard_Integer      i2,
                                        ShapeList&       Lou);

  Standard_EXPORT static void GCopyList(const ShapeList& Lin, ShapeList& Lou);

  Standard_EXPORT void GdumpLS(const ShapeList& L) const;

  Standard_EXPORT static void GdumpPNT(const Point3d& P);

  Standard_EXPORT static void GdumpORIPARPNT(const TopAbs_Orientation o,
                                             const Standard_Real      p,
                                             const Point3d&            Pnt);

  Standard_EXPORT void GdumpSHA(const TopoShape& S, const Standard_Address str = NULL) const;

  Standard_EXPORT void GdumpSHAORI(const TopoShape& S, const Standard_Address str = NULL) const;

  Standard_EXPORT void GdumpSHAORIGEO(const TopoShape&    S,
                                      const Standard_Address str = NULL) const;

  Standard_EXPORT void GdumpSHASTA(const Standard_Integer         iS,
                                   const TopAbs_State             T,
                                   const AsciiString1& a = "",
                                   const AsciiString1& b = "") const;

  Standard_EXPORT void GdumpSHASTA(const TopoShape&            S,
                                   const TopAbs_State             T,
                                   const AsciiString1& a = "",
                                   const AsciiString1& b = "") const;

  Standard_EXPORT void GdumpSHASTA(const Standard_Integer          iS,
                                   const TopAbs_State              T,
                                   const TopOpeBRepBuild_ShapeSet& SS,
                                   const AsciiString1&  a = "",
                                   const AsciiString1&  b = "",
                                   const AsciiString1&  c = "\n") const;

  Standard_EXPORT void GdumpEDG(const TopoShape& S, const Standard_Address str = NULL) const;

  Standard_EXPORT void GdumpEDGVER(const TopoShape&    E,
                                   const TopoShape&    V,
                                   const Standard_Address str = NULL) const;

  Standard_EXPORT void GdumpSAMDOM(const ShapeList& L,
                                   const Standard_Address      str = NULL) const;

  Standard_EXPORT void GdumpEXP(const TopOpeBRepTool_ShapeExplorer& E) const;

  Standard_EXPORT void GdumpSOBU(TopOpeBRepBuild_SolidBuilder& SB) const;

  Standard_EXPORT void GdumpFABU(TopOpeBRepBuild_FaceBuilder& FB) const;

  Standard_EXPORT void GdumpEDBU(TopOpeBRepBuild_EdgeBuilder& EB) const;

  Standard_EXPORT Standard_Boolean GtraceSPS(const Standard_Integer iS) const;

  Standard_EXPORT Standard_Boolean GtraceSPS(const Standard_Integer iS,
                                             const Standard_Integer jS) const;

  Standard_EXPORT Standard_Boolean GtraceSPS(const TopoShape& S) const;

  Standard_EXPORT Standard_Boolean GtraceSPS(const TopoShape& S, Standard_Integer& IS) const;

  Standard_EXPORT void GdumpSHASETreset();

  Standard_EXPORT Standard_Integer GdumpSHASETindex();

  Standard_EXPORT static void PrintGeo(const TopoShape& S);

  Standard_EXPORT static void PrintSur(const TopoFace& F);

  Standard_EXPORT static void PrintCur(const TopoEdge& E);

  Standard_EXPORT static void PrintPnt(const TopoVertex& V);

  Standard_EXPORT static void PrintOri(const TopoShape& S);

  Standard_EXPORT static AsciiString1 StringState(const TopAbs_State S);

  Standard_EXPORT static Standard_Boolean GcheckNBOUNDS(const TopoShape& E);

  friend class TopOpeBRepBuild_HBuilder;

protected:
  //! update the DS by creating new geometries.
  //! create edges on the new curve <Icurv>.
  Standard_EXPORT void BuildEdges(const Standard_Integer                     iC,
                                  const Handle(TopOpeBRepDS_HDataStructure)& DS);

  //! update the DS by creating new geometries.
  //! create faces on the new surface <ISurf>.
  Standard_EXPORT void BuildFaces(const Standard_Integer                     iS,
                                  const Handle(TopOpeBRepDS_HDataStructure)& DS);

  //! update the DS by creating new geometries.
  //! create shapes from the new geometries.
  Standard_EXPORT void BuildFaces(const Handle(TopOpeBRepDS_HDataStructure)& DS);

  //! Split <E1> keeping the parts of state <TB1>.
  Standard_EXPORT void SplitEdge(const TopoShape& E1,
                                 const TopAbs_State  TB1,
                                 const TopAbs_State  TB2);

  //! Split <E1> keeping the parts of state <TB1>.
  Standard_EXPORT void SplitEdge1(const TopoShape& E1,
                                  const TopAbs_State  TB1,
                                  const TopAbs_State  TB2);

  //! Split <E1> keeping the parts of state <TB1>.
  Standard_EXPORT void SplitEdge2(const TopoShape& E1,
                                  const TopAbs_State  TB1,
                                  const TopAbs_State  TB2);

  //! Split <F1> keeping the  parts of state  <TB1>.
  //! Merge faces with same domain, keeping parts  of
  //! state <TB2>.
  Standard_EXPORT void SplitFace(const TopoShape& F1,
                                 const TopAbs_State  TB1,
                                 const TopAbs_State  TB2);

  Standard_EXPORT void SplitFace1(const TopoShape& F1,
                                  const TopAbs_State  TB1,
                                  const TopAbs_State  TB2);

  Standard_EXPORT void SplitFace2(const TopoShape& F1,
                                  const TopAbs_State  TB1,
                                  const TopAbs_State  TB2);

  //! Split <S1> keeping the parts of state <TB1>.
  Standard_EXPORT void SplitSolid(const TopoShape& S1,
                                  const TopAbs_State  TB1,
                                  const TopAbs_State  TB2);

  //! Explore shapes of given  by explorer <Ex> to split them.
  //! Store  new shapes in the set <SS>.
  //! According to RevOri, reverse or not their orientation.
  Standard_EXPORT void SplitShapes(TopOpeBRepTool_ShapeExplorer& Ex,
                                   const TopAbs_State            TB1,
                                   const TopAbs_State            TB2,
                                   TopOpeBRepBuild_ShapeSet&     SS,
                                   const Standard_Boolean        RevOri);

  //! Split edges of <F1> and store  wires and edges in
  //! the set <WES>. According to RevOri, reverse (or not) orientation.
  Standard_EXPORT void FillFace(const TopoShape&          F1,
                                const TopAbs_State           TB1,
                                const ShapeList&  LF2,
                                const TopAbs_State           TB2,
                                TopOpeBRepBuild_WireEdgeSet& WES,
                                const Standard_Boolean       RevOri);

  //! Split faces of <S1> and store shells  and faces in
  //! the set <SS>. According to RevOri, reverse (or not) orientation.
  Standard_EXPORT void FillSolid(const TopoShape&         S1,
                                 const TopAbs_State          TB1,
                                 const ShapeList& LS2,
                                 const TopAbs_State          TB2,
                                 TopOpeBRepBuild_ShapeSet&   SS,
                                 const Standard_Boolean      RevOri);

  //! Split subshapes of <S1> and store subshapes in
  //! the set <SS>. According to RevOri, reverse (or not) orientation.
  Standard_EXPORT void FillShape(const TopoShape&         S1,
                                 const TopAbs_State          TB1,
                                 const ShapeList& LS2,
                                 const TopAbs_State          TB2,
                                 TopOpeBRepBuild_ShapeSet&   SS,
                                 const Standard_Boolean      RevOri);

  //! fills the vertex set PVS with the point iterator IT.
  //! IT accesses a list of interferences which geometry is a point or a vertex.
  //! TB indicates the orientation to give to the geometries
  //! found in interference list accessed by IT.
  Standard_EXPORT void FillVertexSet(PointIterator& IT,
                                     const TopAbs_State          TB,
                                     TopOpeBRepBuild_PaveSet&    PVS) const;

  //! fills vertex set PVS with the current value of IT.
  //! I geometry is a point or a vertex.
  //! TB  indicates the orientation to give to geometries found I
  Standard_EXPORT void FillVertexSetOnValue(const PointIterator& IT,
                                            const TopAbs_State                TB,
                                            TopOpeBRepBuild_PaveSet&          PVS) const;

  //! Returns True if the shape <S> has not already been split
  Standard_EXPORT Standard_Boolean ToSplit(const TopoShape& S, const TopAbs_State TB) const;

  //! add the shape <S> to the map of split shapes.
  //! mark <S> as split/not split on <state>, according to B value.
  Standard_EXPORT void MarkSplit(const TopoShape&    S,
                                 const TopAbs_State     TB,
                                 const Standard_Boolean B = Standard_True);

  //! Returns a ref. on the list of shapes connected to <S> as
  //! <TB> merged parts of <S>.
  Standard_EXPORT ShapeList& ChangeMerged(const TopoShape& S, const TopAbs_State TB);

  //! Returns a ref. on the vertex created on point <I>.
  Standard_EXPORT TopoShape& ChangeNewVertex(const Standard_Integer I);

  //! Returns a ref. on the list of edges created on curve <I>.
  Standard_EXPORT ShapeList& ChangeNewEdges(const Standard_Integer I);

  //! Returns a ref. on the list of faces created on surface <I>.
  Standard_EXPORT ShapeList& ChangeNewFaces(const Standard_Integer I);

  Standard_EXPORT void AddIntersectionEdges(TopoShape&             F,
                                            const TopAbs_State        TB,
                                            const Standard_Boolean    RevOri,
                                            TopOpeBRepBuild_ShapeSet& ES) const;

  Standard_EXPORT void UpdateSplitAndMerged(const TopTools_DataMapOfIntegerListOfShape& mle,
                                            const TopTools_DataMapOfIntegerShape&       mre,
                                            const TopTools_DataMapOfShapeShape&         mlf,
                                            const TopAbs_State                          state);

  TopAbs_State                                   myState1;
  TopAbs_State                                   myState2;
  TopoShape                                   myShape1;
  TopoShape                                   myShape2;
  Handle(TopOpeBRepDS_HDataStructure)            myDataStructure;
  TopOpeBRepDS_BuildTool                         myBuildTool;
  Handle(HArray1OfShape)                myNewVertices;
  TopTools_DataMapOfIntegerListOfShape           myNewEdges;
  Handle(HArray1OfListOfShape)          myNewFaces;
  TopOpeBRepDS_DataMapOfShapeListOfShapeOn1State mySplitIN;
  TopOpeBRepDS_DataMapOfShapeListOfShapeOn1State mySplitON;
  TopOpeBRepDS_DataMapOfShapeListOfShapeOn1State mySplitOUT;
  TopOpeBRepDS_DataMapOfShapeListOfShapeOn1State myMergedIN;
  TopOpeBRepDS_DataMapOfShapeListOfShapeOn1State myMergedON;
  TopOpeBRepDS_DataMapOfShapeListOfShapeOn1State myMergedOUT;
  ShapeList                           myEmptyShapeList;
  ShapeList                           myListOfSolid;
  ShapeList                           myListOfFace;
  ShapeList                           myListOfEdge;
  TopTools_DataMapOfShapeListOfShape             myFSplits;
  TopTools_DataMapOfShapeListOfShape             myESplits;
  Standard_Boolean                               mySectionDone;
  Standard_Boolean                               mySplitSectionEdgesDone;
  ShapeList                           mySection;
  TopoSolid                                   mySolidReference;
  TopoSolid                                   mySolidToFill;
  ShapeList                           myFaceAvoid;
  TopoFace                                    myFaceReference;
  TopoFace                                    myFaceToFill;
  ShapeList                           myEdgeAvoid;
  TopoEdge                                    myEdgeReference;
  TopoEdge                                    myEdgeToFill;
  ShapeList                           myVertexAvoid;
  TopTools_IndexedMapOfShape                     myMAP1;
  TopTools_IndexedMapOfShape                     myMAP2;
  Standard_Integer                               myIsKPart;
  TopTools_DataMapOfShapeListOfShape             myKPMAPf1f2;
  Standard_Integer                               mySHASETindex;
  Standard_Boolean                               myClassifyDef;
  Standard_Boolean                               myClassifyVal;
  TopOpeBRepTool_ShapeClassifier                 myShapeClassifier;
  TopTools_MapOfShape                            myMemoSplit;
  AsciiString1                        myEmptyAS;
  Standard_Boolean                               myProcessON;
  TopTools_IndexedDataMapOfShapeShape            myONFacesMap;
  TopTools_IndexedMapOfOrientedShape             myONElemMap;

private:
};

#endif // _TopOpeBRepBuild_Builder_HeaderFile
