// Created on: 2017-06-13
// Created by: Alexander MALYSHEV
// Copyright (c) 2017 OPEN CASCADE SAS
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

#include <RWStl.hxx>

#include <Message_ProgressScope.hxx>
#include <NCollection_Vector.hxx>
#include <OSD_File.hxx>
#include <OSD_FileSystem.hxx>
#include <OSD_OpenFile.hxx>
#include <RWStl_Reader.hxx>

namespace
{

static const Standard_Integer THE_STL_SIZEOF_FACET = 50;
// clang-format off
  static const Standard_Integer IND_THRESHOLD = 1000; // increment the indicator every 1k triangles
// clang-format on
static const size_t THE_BUFFER_SIZE = 1024; // The length of buffer to read (in bytes)

//! Writing a Little Endian 32 bits integer
inline static void convertInteger(const Standard_Integer theValue, Standard_Character* theResult)
{
  union {
    Standard_Integer   i;
    Standard_Character c[4];
  } anUnion;

  anUnion.i = theValue;

  theResult[0] = anUnion.c[0];
  theResult[1] = anUnion.c[1];
  theResult[2] = anUnion.c[2];
  theResult[3] = anUnion.c[3];
}

//! Writing a Little Endian 32 bits float
inline static void convertDouble(const Standard_Real theValue, Standard_Character* theResult)
{
  union {
    Standard_ShortReal i;
    Standard_Character c[4];
  } anUnion;

  anUnion.i = (Standard_ShortReal)theValue;

  theResult[0] = anUnion.c[0];
  theResult[1] = anUnion.c[1];
  theResult[2] = anUnion.c[2];
  theResult[3] = anUnion.c[3];
}

class Reader : public Reader3
{
public:
  //! Add new node
  virtual Standard_Integer AddNode(const Coords3d& thePnt) Standard_OVERRIDE
  {
    myNodes.Append(thePnt);
    return myNodes.Size();
  }

  //! Add new triangle
  virtual void AddTriangle(Standard_Integer theNode1,
                           Standard_Integer theNode2,
                           Standard_Integer theNode3) Standard_OVERRIDE
  {
    myTriangles.Append(Triangle2(theNode1, theNode2, theNode3));
  }

  //! Creates MeshTriangulation from collected data
  Handle(MeshTriangulation) GetTriangulation()
  {
    if (myTriangles.IsEmpty())
      return Handle(MeshTriangulation)();

    Handle(MeshTriangulation) aPoly =
      new MeshTriangulation(myNodes.Length(), myTriangles.Length(), Standard_False);
    for (Standard_Integer aNodeIter = 0; aNodeIter < myNodes.Size(); ++aNodeIter)
    {
      aPoly->SetNode(aNodeIter + 1, myNodes[aNodeIter]);
    }

    for (Standard_Integer aTriIter = 0; aTriIter < myTriangles.Size(); ++aTriIter)
    {
      aPoly->SetTriangle(aTriIter + 1, myTriangles[aTriIter]);
    }

    return aPoly;
  }

protected:
  void Clear()
  {
    myNodes.Clear();
    myTriangles.Clear();
  }

private:
  NCollection_Vector<Coords3d>        myNodes;
  NCollection_Vector<Triangle2> myTriangles;
};

class MultiDomainReader : public Reader
{
public:
  //! Add new solid
  //! Add triangulation to triangulation list for multi-domain case
  virtual void AddSolid() Standard_OVERRIDE
  {
    if (Handle(MeshTriangulation) aCurrentTri = GetTriangulation())
    {
      myTriangulationList.Append(aCurrentTri);
    }
    Clear();
  }

  //! Returns triangulation list for multi-domain case
  NCollection_Sequence<Handle(MeshTriangulation)>& ChangeTriangulationList()
  {
    return myTriangulationList;
  }

private:
  NCollection_Sequence<Handle(MeshTriangulation)> myTriangulationList;
};

} // namespace

//=================================================================================================

Handle(MeshTriangulation) RWStl1::ReadFile(const Standard_CString       theFile,
                                           const Standard_Real          theMergeAngle,
                                           const Message_ProgressRange& theProgress)
{
  Reader aReader;
  aReader.SetMergeAngle(theMergeAngle);
  aReader.Read(theFile, theProgress);
  // note that returned bool value is ignored intentionally -- even if something went wrong,
  // but some data have been read, we at least will return these data
  return aReader.GetTriangulation();
}

//=================================================================================================

void RWStl1::ReadFile(const Standard_CString                            theFile,
                     const Standard_Real                               theMergeAngle,
                     NCollection_Sequence<Handle(MeshTriangulation)>& theTriangList,
                     const Message_ProgressRange&                      theProgress)
{
  MultiDomainReader aReader;
  aReader.SetMergeAngle(theMergeAngle);
  aReader.Read(theFile, theProgress);
  theTriangList.Clear();
  theTriangList.Append(aReader.ChangeTriangulationList());
}

//=================================================================================================

Handle(MeshTriangulation) RWStl1::ReadFile(const SystemPath&              theFile,
                                           const Message_ProgressRange& theProgress)
{
  SystemFile aFile(theFile);
  if (!aFile.Exists())
  {
    return Handle(MeshTriangulation)();
  }

  AsciiString1 aPath;
  theFile.SystemName(aPath);
  return ReadFile(aPath.ToCString(), theProgress);
}

//=================================================================================================

Handle(MeshTriangulation) RWStl1::ReadBinary(const SystemPath&              theFile,
                                             const Message_ProgressRange& theProgress)
{
  AsciiString1 aPath;
  theFile.SystemName(aPath);

  const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();
  std::shared_ptr<std::istream> aStream =
    aFileSystem->OpenIStream(aPath, std::ios::in | std::ios::binary);
  if (aStream.get() == NULL)
  {
    return Handle(MeshTriangulation)();
  }

  Reader aReader;
  if (!aReader.ReadBinary(*aStream, theProgress))
  {
    return Handle(MeshTriangulation)();
  }

  return aReader.GetTriangulation();
}

//=================================================================================================

Handle(MeshTriangulation) RWStl1::ReadAscii(const SystemPath&              theFile,
                                            const Message_ProgressRange& theProgress)
{
  AsciiString1 aPath;
  theFile.SystemName(aPath);

  const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();
  std::shared_ptr<std::istream> aStream =
    aFileSystem->OpenIStream(aPath, std::ios::in | std::ios::binary);
  if (aStream.get() == NULL)
  {
    return Handle(MeshTriangulation)();
  }

  // get length of file to feed progress indicator
  aStream->seekg(0, aStream->end);
  std::streampos theEnd = aStream->tellg();
  aStream->seekg(0, aStream->beg);

  Reader                  aReader;
  ReadLineBuffer aBuffer(THE_BUFFER_SIZE);
  if (!aReader.ReadAscii(*aStream, aBuffer, theEnd, theProgress))
  {
    return Handle(MeshTriangulation)();
  }

  return aReader.GetTriangulation();
}

//=================================================================================================

Standard_Boolean RWStl1::WriteBinary(const Handle(MeshTriangulation)& theMesh,
                                    const SystemPath&                   thePath,
                                    const Message_ProgressRange&      theProgress)
{
  if (theMesh.IsNull() || theMesh->NbTriangles() <= 0)
  {
    return Standard_False;
  }

  AsciiString1 aPath;
  thePath.SystemName(aPath);

  FILE* aFile = OSD_OpenFile(aPath, "wb");
  if (aFile == NULL)
  {
    return Standard_False;
  }

  Standard_Boolean isOK = writeBinary(theMesh, aFile, theProgress);

  fclose(aFile);
  return isOK;
}

//=================================================================================================

Standard_Boolean RWStl1::WriteAscii(const Handle(MeshTriangulation)& theMesh,
                                   const SystemPath&                   thePath,
                                   const Message_ProgressRange&      theProgress)
{
  if (theMesh.IsNull() || theMesh->NbTriangles() <= 0)
  {
    return Standard_False;
  }

  AsciiString1 aPath;
  thePath.SystemName(aPath);

  FILE* aFile = OSD_OpenFile(aPath, "w");
  if (aFile == NULL)
  {
    return Standard_False;
  }

  Standard_Boolean isOK = writeASCII(theMesh, aFile, theProgress);
  fclose(aFile);
  return isOK;
}

//=================================================================================================

Standard_Boolean RWStl1::writeASCII(const Handle(MeshTriangulation)& theMesh,
                                   FILE*                             theFile,
                                   const Message_ProgressRange&      theProgress)
{
  // note that space after 'solid' is necessary for many systems
  if (fwrite("solid \n", 1, 7, theFile) != 7)
  {
    return Standard_False;
  }

  char aBuffer[512];
  memset(aBuffer, 0, sizeof(aBuffer));

  const Standard_Integer NBTriangles = theMesh->NbTriangles();
  Message_ProgressScope  aPS(theProgress, "Triangles", NBTriangles);

  Standard_Integer anElem[3] = {0, 0, 0};
  for (Standard_Integer aTriIter = 1; aTriIter <= NBTriangles; ++aTriIter)
  {
    const Triangle2 aTriangle = theMesh->Triangle1(aTriIter);
    aTriangle.Get(anElem[0], anElem[1], anElem[2]);

    const Point3d aP1 = theMesh->Node(anElem[0]);
    const Point3d aP2 = theMesh->Node(anElem[1]);
    const Point3d aP3 = theMesh->Node(anElem[2]);

    const Vector3d aVec1(aP1, aP2);
    const Vector3d aVec2(aP1, aP3);
    Vector3d       aVNorm = aVec1.Crossed(aVec2);
    if (aVNorm.SquareMagnitude() > gp1::Resolution())
    {
      aVNorm.Normalize();
    }
    else
    {
      aVNorm.SetCoord(0.0, 0.0, 0.0);
    }

    Sprintf(aBuffer,
            " facet normal % 12e % 12e % 12e\n"
            "   outer loop\n"
            "     vertex % 12e % 12e % 12e\n"
            "     vertex % 12e % 12e % 12e\n"
            "     vertex % 12e % 12e % 12e\n"
            "   endloop\n"
            " endfacet\n",
            aVNorm.X(),
            aVNorm.Y(),
            aVNorm.Z(),
            aP1.X(),
            aP1.Y(),
            aP1.Z(),
            aP2.X(),
            aP2.Y(),
            aP2.Z(),
            aP3.X(),
            aP3.Y(),
            aP3.Z());

    if (fprintf(theFile, "%s", aBuffer) < 0)
    {
      return Standard_False;
    }

    // update progress only per 1k triangles
    if ((aTriIter % IND_THRESHOLD) == 0)
    {
      if (!aPS.More())
        return Standard_False;
      aPS.Next(IND_THRESHOLD);
    }
  }

  if (fwrite("endsolid\n", 1, 9, theFile) != 9)
  {
    return Standard_False;
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean RWStl1::writeBinary(const Handle(MeshTriangulation)& theMesh,
                                    FILE*                             theFile,
                                    const Message_ProgressRange&      theProgress)
{
  char aHeader[80] = "STL Exported by Open CASCADE Technology [dev.opencascade.org]";
  if (fwrite(aHeader, 1, 80, theFile) != 80)
  {
    return Standard_False;
  }

  const Standard_Integer aNBTriangles = theMesh->NbTriangles();
  Message_ProgressScope  aPS(theProgress, "Triangles", aNBTriangles);

  const Standard_Size                    aNbChunkTriangles = 4096;
  const Standard_Size                    aChunkSize = aNbChunkTriangles * THE_STL_SIZEOF_FACET;
  NCollection_Array1<Standard_Character> aData(1, aChunkSize);
  Standard_Character*                    aDataChunk = &aData.ChangeFirst();

  Standard_Character aConv[4];
  convertInteger(aNBTriangles, aConv);
  if (fwrite(aConv, 1, 4, theFile) != 4)
  {
    return Standard_False;
  }

  Standard_Size aByteCount = 0;
  for (Standard_Integer aTriIter = 1; aTriIter <= aNBTriangles; ++aTriIter)
  {
    Standard_Integer    id[3];
    const Triangle2 aTriangle = theMesh->Triangle1(aTriIter);
    aTriangle.Get(id[0], id[1], id[2]);

    const Point3d aP1 = theMesh->Node(id[0]);
    const Point3d aP2 = theMesh->Node(id[1]);
    const Point3d aP3 = theMesh->Node(id[2]);

    Vector3d aVec1(aP1, aP2);
    Vector3d aVec2(aP1, aP3);
    Vector3d aVNorm = aVec1.Crossed(aVec2);
    if (aVNorm.SquareMagnitude() > gp1::Resolution())
    {
      aVNorm.Normalize();
    }
    else
    {
      aVNorm.SetCoord(0.0, 0.0, 0.0);
    }

    convertDouble(aVNorm.X(), &aDataChunk[aByteCount]);
    aByteCount += 4;
    convertDouble(aVNorm.Y(), &aDataChunk[aByteCount]);
    aByteCount += 4;
    convertDouble(aVNorm.Z(), &aDataChunk[aByteCount]);
    aByteCount += 4;

    convertDouble(aP1.X(), &aDataChunk[aByteCount]);
    aByteCount += 4;
    convertDouble(aP1.Y(), &aDataChunk[aByteCount]);
    aByteCount += 4;
    convertDouble(aP1.Z(), &aDataChunk[aByteCount]);
    aByteCount += 4;

    convertDouble(aP2.X(), &aDataChunk[aByteCount]);
    aByteCount += 4;
    convertDouble(aP2.Y(), &aDataChunk[aByteCount]);
    aByteCount += 4;
    convertDouble(aP2.Z(), &aDataChunk[aByteCount]);
    aByteCount += 4;

    convertDouble(aP3.X(), &aDataChunk[aByteCount]);
    aByteCount += 4;
    convertDouble(aP3.Y(), &aDataChunk[aByteCount]);
    aByteCount += 4;
    convertDouble(aP3.Z(), &aDataChunk[aByteCount]);
    aByteCount += 4;

    aDataChunk[aByteCount] = 0;
    aByteCount += 1;
    aDataChunk[aByteCount] = 0;
    aByteCount += 1;

    // Chunk is filled. Dump it to the file.
    if (aByteCount == aChunkSize)
    {
      if (fwrite(aDataChunk, 1, aChunkSize, theFile) != aChunkSize)
      {
        return Standard_False;
      }

      aByteCount = 0;
    }

    // update progress only per 1k triangles
    if ((aTriIter % IND_THRESHOLD) == 0)
    {
      if (!aPS.More())
        return Standard_False;
      aPS.Next(IND_THRESHOLD);
    }
  }

  // Write last part if necessary.
  if (aByteCount != aChunkSize)
  {
    if (fwrite(aDataChunk, 1, aByteCount, theFile) != aByteCount)
    {
      return Standard_False;
    }
  }

  return Standard_True;
}
