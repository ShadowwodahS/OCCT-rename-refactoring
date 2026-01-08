// Author: Kirill Gavrilov
// Copyright (c) 2016-2019 OPEN CASCADE SAS
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

#ifndef _RWGltf_GltfJsonParser_HeaderFile
#define _RWGltf_GltfJsonParser_HeaderFile

#include <Message_Gravity.hxx>
#include <Message_ProgressScope.hxx>
#include <RWGltf_GltfPrimArrayData.hxx>
#include <RWGltf_GltfLatePrimitiveArray.hxx>
#include <RWGltf_GltfBufferView.hxx>
#include <RWGltf_GltfRootElement.hxx>
#include <RWGltf_MaterialCommon.hxx>
#include <RWGltf_MaterialMetallicRoughness.hxx>
#include <RWMesh_CoordinateSystemConverter.hxx>
#include <RWMesh_NodeAttributes.hxx>
#include <TColStd_IndexedDataMapOfStringString.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_SequenceOfShape.hxx>

// workaround name collisions with XLib
#ifdef None
  #undef None
#endif
#ifdef Bool
  #undef Bool
#endif

#ifdef HAVE_RAPIDJSON
  // #define RAPIDJSON_ASSERT
  #include <Standard_WarningsDisable.hxx>
  #include <rapidjson/document.h>
  #include <rapidjson/prettywriter.h>
  #include <rapidjson/stringbuffer.h>
  #include <rapidjson/istreamwrapper.h>
  #include <rapidjson/ostreamwrapper.h>
  #include <Standard_WarningsRestore.hxx>

typedef rapidjson::Document::ValueType RWGltf_JsonValue;
#endif

//! INTERNAL tool for parsing glTF document (JSON structure).
class GLTFJsonParser
#ifdef HAVE_RAPIDJSON
    : public rapidjson::Document
#endif
{
public:
#ifdef HAVE_RAPIDJSON
  //! Auxiliary method for formatting error code.
  Standard_EXPORT static const char* FormatParseError(rapidjson::ParseErrorCode theCode);
#endif

public:
  //! Empty constructor.
  Standard_EXPORT GLTFJsonParser(TopTools_SequenceOfShape& theRootShapes);

  //! Set file path.
  Standard_EXPORT void SetFilePath(const AsciiString1& theFilePath);

  //! Set flag for probing file without complete reading.
  void SetProbeHeader(bool theToProbe) { myToProbeHeader = theToProbe; }

  //! Return prefix for reporting issues.
  const AsciiString1& ErrorPrefix() const { return myErrorPrefix; }

  //! Set prefix for reporting issues.
  void SetErrorPrefix(const AsciiString1& theErrPrefix) { myErrorPrefix = theErrPrefix; }

  //! Set map for storing node attributes.
  void SetAttributeMap(RWMesh_NodeAttributeMap& theAttribMap) { myAttribMap = &theAttribMap; }

  //! Set list for storing external files.
  void SetExternalFiles(NCollection_IndexedMap<AsciiString1>& theExternalFiles)
  {
    myExternalFiles = &theExternalFiles;
  }

  //! Set metadata map.
  void SetMetadata(TColStd_IndexedDataMapOfStringString& theMetadata) { myMetadata = &theMetadata; }

  //! Set flag to translate asset.extras into metadata.
  void SetReadAssetExtras(bool theToRead) { myToReadAssetExtras = theToRead; }

  //! Return transformation from glTF to OCCT coordinate system.
  const RWMesh_CoordinateSystemConverter& CoordinateSystemConverter() const { return myCSTrsf; }

  //! Set transformation from glTF to OCCT coordinate system.
  void SetCoordinateSystemConverter(const RWMesh_CoordinateSystemConverter& theConverter)
  {
    myCSTrsf = theConverter;
  }

  //! Initialize binary format.
  void SetBinaryFormat(int64_t theBinBodyOffset, int64_t theBinBodyLen)
  {
    myIsBinary      = true;
    myBinBodyOffset = theBinBodyOffset;
    myBinBodyLen    = theBinBodyLen;
  }

  //! Set flag to ignore nodes without Geometry1, TRUE by default.
  void SetSkipEmptyNodes(bool theToSkip) { myToSkipEmptyNodes = theToSkip; }

  //! Set flag to flag to load all scenes in the document, FALSE by default which means only main
  //! (default) scene will be loaded.
  void SetLoadAllScenes(bool theToLoadAll) { myToLoadAllScenes = theToLoadAll; }

  //! Set flag to use Mesh1 name in case if Node name is empty, TRUE by default.
  void SetMeshNameAsFallback(bool theToFallback) { myUseMeshNameAsFallback = theToFallback; }

  //! Parse glTF document.
  Standard_EXPORT bool Parse(const Message_ProgressRange& theProgress);

  //! Return face list for loading triangulation.
  NCollection_Vector<TopoFace>& FaceList() { return myFaceList; }

protected:
#ifdef HAVE_RAPIDJSON
  //! Search mandatory root elements in the document.
  //! Return FALSE if some mandatory element is missing.
  Standard_EXPORT bool gltfParseRoots();

  //! Parse default scene.
  Standard_EXPORT bool gltfParseScene(const Message_ProgressRange& theProgress);

  //! Parse document metadata.
  Standard_EXPORT void gltfParseAsset();

protected:
  //! Parse materials defined in the document.
  Standard_EXPORT void gltfParseMaterials();

  //! Parse standard material.
  Standard_EXPORT bool gltfParseStdMaterial(Handle(RWGltf_MaterialCommon)& theMat,
                                            const RWGltf_JsonValue&        theMatNode);

  //! Parse pbrMetallicRoughness material.
  Standard_EXPORT bool gltfParsePbrMaterial(Handle(RWGltf_MaterialMetallicRoughness)& theMat,
                                            const RWGltf_JsonValue&                   theMatNode);

  //! Parse common material (KHR_materials_common extension).
  Standard_EXPORT bool gltfParseCommonMaterial(Handle(RWGltf_MaterialCommon)& theMat,
                                               const RWGltf_JsonValue&        theMatNode);

  //! Parse texture definition.
  Standard_EXPORT bool gltfParseTexture(Handle(Image_Texture)&  theTexture,
                                        const RWGltf_JsonValue* theTextureId);

  //! Parse texture definition in binary buffer of GLB file.
  Standard_EXPORT bool gltfParseTexturInGlbBuffer(Handle(Image_Texture)&         theTexture,
                                                  const RWGltf_JsonValue&        theBinVal,
                                                  const AsciiString1& theBufferViewId,
                                                  const RWGltf_JsonValue&        theBufferViewName);

  //! Parse texture definition in binary buffer of glTF file.
  Standard_EXPORT bool gltfParseTextureInBufferView(Handle(Image_Texture)&         theTexture,
                                                    const AsciiString1& theSourceId,
                                                    const AsciiString1& theBufferViewhId,
                                                    const RWGltf_JsonValue&        theBufferView);

  //! Bind material definition to the map.
  Standard_EXPORT void gltfBindMaterial(const Handle(RWGltf_MaterialMetallicRoughness)& theMatPbr,
                                        const Handle(RWGltf_MaterialCommon)& theMatCommon);

protected:
  //! Parse scene array of nodes recursively.
  Standard_EXPORT bool gltfParseSceneNodes(TopTools_SequenceOfShape&    theShapeSeq,
                                           const RWGltf_JsonValue&      theSceneNodes,
                                           const Message_ProgressRange& theProgress);

  //! Parse scene node recursively.
  Standard_EXPORT bool gltfParseSceneNode(TopoShape&                  theNodeShape,
                                          const AsciiString1& theSceneNodeId,
                                          const RWGltf_JsonValue&        theSceneNode,
                                          const Message_ProgressRange&   theProgress);

  //! Parse mesh element.
  Standard_EXPORT bool gltfParseMesh(TopoShape&                  theMeshShape,
                                     const AsciiString1& theMeshId,
                                     const RWGltf_JsonValue&        theMesh);

  //! Parse primitive array.
  Standard_EXPORT bool gltfParsePrimArray(TopoShape&                  thePrimArrayShape,
                                          const AsciiString1& theMeshId,
                                          const AsciiString1& theMeshName,
                                          const RWGltf_JsonValue&        thePrimArray);

  //! Parse accessor.
  Standard_EXPORT bool gltfParseAccessor(const Handle(RWGltf_GltfLatePrimitiveArray)& theMeshData,
                                         const AsciiString1&               theName,
                                         const RWGltf_JsonValue&                      theAccessor,
                                         const RWGltf_GltfArrayType                   theType,
                                         const RWGltf_JsonValue* theCompBuffView);

  //! Parse buffer view.
  Standard_EXPORT bool gltfParseBufferView(const Handle(RWGltf_GltfLatePrimitiveArray)& theMeshData,
                                           const AsciiString1&               theName,
                                           const RWGltf_JsonValue&    theBufferView,
                                           const GltfAccessor& theAccessor,
                                           const RWGltf_GltfArrayType theType);

  //! Parse buffer.
  Standard_EXPORT bool gltfParseBuffer(const Handle(RWGltf_GltfLatePrimitiveArray)& theMeshData,
                                       const AsciiString1&               theName,
                                       const RWGltf_JsonValue&                      theBuffer,
                                       const GltfAccessor&                   theAccessor,
                                       const GltfBufferView&                 theView,
                                       const RWGltf_GltfArrayType                   theType);

protected:
  //! Read vec4 from specified item.
  static bool gltfReadVec4(Graphic3d_Vec4d& theVec4, const RWGltf_JsonValue* theVal)
  {
    if (theVal == NULL || !theVal->IsArray() || theVal->Size() != 4)
    {
      return false;
    }

    for (int aCompIter = 0; aCompIter < 4; ++aCompIter)
    {
      const RWGltf_JsonValue& aGenVal = (*theVal)[aCompIter];
      if (!aGenVal.IsNumber())
      {
        return false;
      }
      theVec4[aCompIter] = aGenVal.GetDouble();
    }
    return true;
  }

  //! Validate color
  static bool validateColor4(const Graphic3d_Vec4d& theVec)
  {
    return theVec.r() >= 0.0 && theVec.r() <= 1.0 && theVec.g() >= 0.0 && theVec.g() <= 1.0
           && theVec.b() >= 0.0 && theVec.b() <= 1.0 && theVec.a() >= 0.0 && theVec.a() <= 1.0;
  }

  //! Read vec3 from specified item.
  static bool gltfReadVec3(Graphic3d_Vec3d& theVec3, const RWGltf_JsonValue* theVal)
  {
    if (theVal == NULL || !theVal->IsArray() || theVal->Size() != 3)
    {
      return false;
    }

    for (int aCompIter = 0; aCompIter < 3; ++aCompIter)
    {
      const RWGltf_JsonValue& aGenVal = (*theVal)[aCompIter];
      if (!aGenVal.IsNumber())
      {
        return false;
      }
      theVec3[aCompIter] = aGenVal.GetDouble();
    }
    return true;
  }

  //! Validate color
  static bool validateColor3(const Graphic3d_Vec3d& theVec)
  {
    return theVec.r() >= 0.0 && theVec.r() <= 1.0 && theVec.g() >= 0.0 && theVec.g() <= 1.0
           && theVec.b() >= 0.0 && theVec.b() <= 1.0;
  }

protected:
  //! Groups for re-using shapes.
  enum ShapeMapGroup
  {
    ShapeMapGroup_Nodes,     //!< nodes
    ShapeMapGroup_Meshes,    //!< meshes
    ShapeMapGroup_PrimArray, //!< primitive array
  };

  //! Bind name attribute.
  void bindNodeShape(TopoShape&                     theShape,
                     const TopLoc_Location&            theLoc,
                     const AsciiString1&    theNodeId,
                     const RWGltf_JsonValue*           theUserName,
                     const Handle(TDataStd_NamedData)& theExtras)
  {
    bindNamedShape(theShape, ShapeMapGroup_Nodes, theLoc, theNodeId, theUserName, theExtras);
  }

  //! Bind name attribute.
  void bindMeshShape(TopoShape&                     theShape,
                     const AsciiString1&    theMeshId,
                     const RWGltf_JsonValue*           theUserName,
                     const Handle(TDataStd_NamedData)& theExtras)
  {
    bindNamedShape(theShape,
                   ShapeMapGroup_Meshes,
                   TopLoc_Location(),
                   theMeshId,
                   theUserName,
                   theExtras);
  }

  //! Find named shape.
  bool findNodeShape(TopoShape& theShape, const AsciiString1& theNodeId) const
  {
    return findNamedShape(theShape, ShapeMapGroup_Nodes, theNodeId);
  }

  //! Find named shape.
  bool findMeshShape(TopoShape& theShape, const AsciiString1& theMeshId) const
  {
    return findNamedShape(theShape, ShapeMapGroup_Meshes, theMeshId);
  }

  //! Bind name attribute.
  Standard_EXPORT void bindNamedShape(TopoShape&                     theShape,
                                      ShapeMapGroup                     theGroup,
                                      const TopLoc_Location&            theLoc,
                                      const AsciiString1&    theId,
                                      const RWGltf_JsonValue*           theUserName,
                                      const Handle(TDataStd_NamedData)& theExtras);

  //! Find named shape.
  bool findNamedShape(TopoShape&                  theShape,
                      ShapeMapGroup                  theGroup,
                      const AsciiString1& theId) const
  {
    return myShapeMap[theGroup].Find(theId, theShape);
  }

  //! Return the string representation of the key.
  static AsciiString1 getKeyString(const RWGltf_JsonValue& theValue)
  {
    if (theValue.IsString())
    {
      return AsciiString1(theValue.GetString());
    }
    else if (theValue.IsInt())
    {
      return AsciiString1(theValue.GetInt());
    }
    return AsciiString1();
  }

protected:
  //! Auxiliary structure for fast look-up of document sub-nodes of specified node.
  class GltfElementMap
  {
  public:
    //! Empty constructor.
    GltfElementMap()
        : myRoot(NULL)
    {
    }

    //! Return TRUE if this element is NULL.
    bool IsNull() const { return myRoot == NULL; }

    //! Access this node.
    const RWGltf_JsonValue* Root() const { return myRoot; }

    //! Find the child node with specified key.
    const RWGltf_JsonValue* FindChild(const AsciiString1& theKey)
    {
      const RWGltf_JsonValue* aNode = NULL;
      return myChildren.Find(theKey, aNode) ? aNode : NULL;
    }

    //! Find the child node with specified key.
    const RWGltf_JsonValue* FindChild(const RWGltf_JsonValue& theKey)
    {
      const AsciiString1 aKey = getKeyString(theKey);
      if (aKey.IsEmpty())
      {
        return NULL;
      }

      const RWGltf_JsonValue* aNode = NULL;
      return myChildren.Find(aKey, aNode) ? aNode : NULL;
    }

    //! Initialize the element.
    void Init(const AsciiString1& theRootName, const RWGltf_JsonValue* theRoot);

  private:
    NCollection_DataMap<AsciiString1, const RWGltf_JsonValue*> myChildren;
    const RWGltf_JsonValue*                                               myRoot;
  };

private:
  //! Parse transformation matrix of the node.
  //! @param theSceneNodeId Name of the node. Used only for printing messages.
  //! @param theMatrixVal Json value containing transformation matrix.
  //! @param theResult TopLoc_Location object where result of parsing will be written.
  //! @param If true - parsing was successful, transformation is written into @p theResult.
  //!        If true - failed to parse, @p theResult is unchanged.
  bool parseTransformationMatrix(const AsciiString1& theSceneNodeId,
                                 const RWGltf_JsonValue&        theMatrixVal,
                                 TopLoc_Location&               theResult) const;

  //! Parse transformation components of the node.
  //! @param theSceneNodeId Name of the node. Used only for printing messages.
  //! @param theRotationVal Json value containing rotation component of transformation.
  //!        May be null in which case it is ignored.
  //! @param theScaleVal Json value containing scale component of transformation.
  //!        May be null in which case it is ignored.
  //! @param theTranslationVal Json value containing translation component of transformation.
  //!        May be null in which case it is ignored.
  //! @param theResult TopLoc_Location object where result of parsing will be written.
  //! @param If true - parsing was successful, transformation is written into @p theResult.
  //!        If true - failed to parse, @p theResult is unchanged.
  bool parseTransformationComponents(const AsciiString1& theSceneNodeId,
                                     const RWGltf_JsonValue*        theRotationVal,
                                     const RWGltf_JsonValue*        theScaleVal,
                                     const RWGltf_JsonValue*        theTranslationVal,
                                     TopLoc_Location&               theResult) const;
#endif
protected:
  //! Print message about invalid glTF syntax.
  void reportGltfSyntaxProblem(const AsciiString1& theMsg,
                               Message_Gravity                theGravity) const;

protected:
  TopTools_SequenceOfShape* myRootShapes; //!< sequence of result root shapes
  RWMesh_NodeAttributeMap*  myAttribMap;  //!< shape attributes
  NCollection_IndexedMap<AsciiString1>*
    myExternalFiles;                                //!< list of external file references
                                                    // clang-format off
  RWMesh_CoordinateSystemConverter myCSTrsf;        //!< transformation from glTF to OCCT coordinate system
                                                    // clang-format on
  TColStd_IndexedDataMapOfStringString* myMetadata; //!< file metadata

  NCollection_DataMap<AsciiString1, Handle(RWGltf_MaterialMetallicRoughness)>
                                                                              myMaterialsPbr;
  NCollection_DataMap<AsciiString1, Handle(RWGltf_MaterialCommon)> myMaterialsCommon;
  NCollection_DataMap<AsciiString1, Handle(XCAFDoc_VisMaterial)>   myMaterials;
  NCollection_DataMap<AsciiString1, TopoShape>                  myShapeMap[3];

  NCollection_DataMap<AsciiString1, bool>                       myProbedFiles;
  NCollection_DataMap<AsciiString1, Handle(NCollection_Buffer)> myDecodedBuffers;
  NCollection_Vector<TopoFace> myFaceList; //!< face list for loading triangulation

  AsciiString1 myFilePath;         //!< file path
  AsciiString1 myFolder;           //!< folder
  AsciiString1 myErrorPrefix;      //!< invalid syntax error prefix
  int64_t                 myBinBodyOffset;    //!< offset to binary body
  int64_t                 myBinBodyLen;       //!< binary body length
  bool                    myIsBinary;         //!< binary document
  bool                    myIsGltf1;          //!< obsolete glTF 1.0 version format
  bool                    myToSkipEmptyNodes; //!< ignore nodes without Geometry1
  // clang-format off
  bool                      myToLoadAllScenes;  //!< flag to load all scenes in the document, FALSE by default
  bool                      myUseMeshNameAsFallback; //!< flag to use Mesh1 name in case if Node name is empty, TRUE by default
  bool                      myToProbeHeader;  //!< flag to probe header without full reading, FALSE by default
  bool                      myToReadAssetExtras; //!< flag to translate asset.extras into metadata, TRUE by default
  // clang-format on

#ifdef HAVE_RAPIDJSON
  GltfElementMap myGltfRoots[RWGltf_GltfRootElement_NB]; //!< glTF format root elements
#endif
};

#endif // _RWGltf_GltfJsonParser_HeaderFile
