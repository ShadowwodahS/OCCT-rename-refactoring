// Copyright (c) 2021 OPEN CASCADE SAS
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

#include <RWMesh_TriangulationReader.hxx>

#include <Message.hxx>
#include <RWMesh_TriangulationSource.hxx>

IMPLEMENT_STANDARD_RTTIEXT(TriangulationReader, RefObject)

namespace
{
//! Forms string with loading statistic.
static AsciiString1 loadingStatistic(const AsciiString1& thePrefix,
                                                const Standard_Integer         theExpectedNodesNb,
                                                const Standard_Integer         theLoadedNodesNb,
                                                const Standard_Integer theExpectedTrianglesNb,
                                                const Standard_Integer theDegeneratedTrianglesNb,
                                                const Standard_Integer theLoadedTrianglesNb)
{
  AsciiString1 aNodesInfo;
  if (theExpectedNodesNb != theLoadedNodesNb)
  {
    aNodesInfo = AsciiString1("Nodes: ") + theExpectedNodesNb + " expected / ";
    aNodesInfo += AsciiString1(theLoadedNodesNb) + " loaded.";
  }
  AsciiString1 aTrianglesInfo;
  if (theExpectedTrianglesNb != theLoadedTrianglesNb)
  {
    if (!aNodesInfo.IsEmpty())
    {
      aNodesInfo += " ";
    }
    aTrianglesInfo =
      AsciiString1("Triangles: ") + theExpectedTrianglesNb + " expected / ";
    if (theDegeneratedTrianglesNb != 0)
    {
      aTrianglesInfo +=
        AsciiString1(theDegeneratedTrianglesNb) + " skipped degenerated / ";
    }
    aTrianglesInfo += AsciiString1(theLoadedTrianglesNb) + " loaded.";
  }
  if (aNodesInfo.IsEmpty() && aTrianglesInfo.IsEmpty())
  {
    return AsciiString1();
  }
  return thePrefix
         + ("Disconformity of the expected number of nodes/triangles for deferred mesh to the "
            "loaded amount. ")
         + aNodesInfo + aTrianglesInfo;
}
} // namespace

//=================================================================================================

void TriangulationReader::LoadingStatistic1::PrintStatistic(
  const AsciiString1& thePrefix) const
{
  AsciiString1 aStatisticInfo = loadingStatistic(thePrefix,
                                                            ExpectedNodesNb,
                                                            LoadedNodesNb,
                                                            ExpectedTrianglesNb,
                                                            DegeneratedTrianglesNb,
                                                            LoadedTrianglesNb);
  if (!aStatisticInfo.IsEmpty())
  {
    Message1::SendWarning(aStatisticInfo);
  }
}

//=================================================================================================

TriangulationReader::TriangulationReader()
    : myLoadingStatistic(NULL),
      myIsDoublePrecision(false),
      myToSkipDegenerateTris(false),
      myToPrintDebugMessages(false)
{
}

//=================================================================================================

TriangulationReader::~TriangulationReader()
{
  delete myLoadingStatistic;
}

//=================================================================================================

bool TriangulationReader::Load(const Handle(RWMesh_TriangulationSource)& theSourceMesh,
                                      const Handle(MeshTriangulation)&         theDestMesh,
                                      const Handle(OSD_FileSystem)&             theFileSystem) const
{
  Standard_ASSERT_RETURN(!theDestMesh.IsNull(),
                         "The destination mesh should be initialized before loading data to it",
                         false);
  theDestMesh->Clear();
  theDestMesh->SetDoublePrecision(myIsDoublePrecision);

  if (!load(theSourceMesh, theDestMesh, theFileSystem))
  {
    theDestMesh->Clear();
    return false;
  }
  if (!finalizeLoading(theSourceMesh, theDestMesh))
  {
    theDestMesh->Clear();
    return false;
  }
  return true;
}

//=================================================================================================

bool TriangulationReader::finalizeLoading(
  const Handle(RWMesh_TriangulationSource)& theSourceMesh,
  const Handle(MeshTriangulation)&         theDestMesh) const
{
  if (!theSourceMesh->CachedMinMax().IsVoid())
  {
    theDestMesh->SetCachedMinMax(theSourceMesh->CachedMinMax());
  }
  if (myLoadingStatistic)
  {
    Standard_Mutex::Sentry aLock(myMutex);
    myLoadingStatistic->ExpectedNodesNb += theSourceMesh->NbDeferredNodes();
    myLoadingStatistic->ExpectedTrianglesNb += theSourceMesh->NbDeferredTriangles();
    myLoadingStatistic->DegeneratedTrianglesNb += theSourceMesh->DegeneratedTriNb();
    myLoadingStatistic->LoadedNodesNb += theDestMesh->NbNodes();
    myLoadingStatistic->LoadedTrianglesNb += theDestMesh->NbTriangles();
  }
  else if (myToPrintDebugMessages)
  {
    AsciiString1 aStatisticInfo =
      loadingStatistic(AsciiString1("[Mesh1 reader. File '") + myFileName + "']. ",
                       theSourceMesh->NbDeferredNodes(),
                       theDestMesh->NbNodes(),
                       theSourceMesh->NbDeferredTriangles(),
                       theSourceMesh->DegeneratedTriNb(),
                       theDestMesh->NbTriangles());
    Message1::SendTrace(aStatisticInfo);
  }
  return true;
}

//=================================================================================================

bool TriangulationReader::setNbEdges(const Handle(MeshTriangulation)& theMesh,
                                            const Standard_Integer            theNbTris,
                                            const Standard_Boolean            theToCopyData) const
{
  Handle(RWMesh_TriangulationSource) aMesh = Handle(RWMesh_TriangulationSource)::DownCast(theMesh);
  if (aMesh.IsNull())
  {
    Message1::SendWarning("Only triangulation loading is supported.");
    return false;
  }
  if (theNbTris >= 1)
  {
    aMesh->ResizeEdges(theNbTris, theToCopyData);
    return true;
  }
  return false;
}

//=================================================================================================

Standard_Integer TriangulationReader::setEdge(const Handle(MeshTriangulation)& theMesh,
                                                     const Standard_Integer            theIndex,
                                                     const Standard_Integer theEdge) const
{
  Handle(RWMesh_TriangulationSource) aMesh = Handle(RWMesh_TriangulationSource)::DownCast(theMesh);
  if (aMesh.IsNull())
  {
    Message1::SendWarning("Only triangulation loading is supported.");
    return 0;
  }
  if (theEdge < 1 || theEdge > theMesh->NbNodes())
  {
    return 0;
  }
  aMesh->SetEdge(theIndex, theEdge);
  return 1;
}
