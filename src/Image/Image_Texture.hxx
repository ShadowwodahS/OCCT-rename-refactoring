// Author: Kirill Gavrilov
// Copyright (c) 2015-2019 OPEN CASCADE SAS
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

#ifndef _Image_Texture_HeaderFile
#define _Image_Texture_HeaderFile

#include <NCollection_Buffer.hxx>
#include <TCollection_AsciiString.hxx>

class Image_CompressedPixMap;
class Image_SupportedFormats;
class Image_PixMap;

//! Texture image definition.
//! The image can be stored as path to image file, as file path with the given offset and as a data
//! buffer of encoded image.
class Image_Texture : public RefObject
{
  DEFINE_STANDARD_RTTIEXT(Image_Texture, RefObject)
public:
  //! Constructor pointing to file location.
  Standard_EXPORT explicit Image_Texture(const AsciiString1& theFileName);

  //! Constructor pointing to file part.
  Standard_EXPORT explicit Image_Texture(const AsciiString1& theFileName,
                                         int64_t                        theOffset,
                                         int64_t                        theLength);

  //! Constructor pointing to buffer.
  Standard_EXPORT explicit Image_Texture(const Handle(NCollection_Buffer)& theBuffer,
                                         const AsciiString1&    theId);

  //! Return generated texture id.
  const AsciiString1& TextureId() const { return myTextureId; }

  //! Return image file path.
  const AsciiString1& FilePath() const { return myImagePath; }

  //! Return offset within file.
  int64_t FileOffset() const { return myOffset; }

  //! Return length of image data within the file after offset.
  int64_t FileLength() const { return myLength; }

  //! Return buffer holding encoded image content.
  const Handle(NCollection_Buffer)& DataBuffer() const { return myBuffer; }

  //! Return mime-type of image file based on ProbeImageFileFormat().
  Standard_EXPORT AsciiString1 MimeType() const;

  //! Return image file format.
  Standard_EXPORT AsciiString1 ProbeImageFileFormat() const;

  //! Image reader without decoding data for formats supported natively by GPUs.
  Standard_EXPORT virtual Handle(Image_CompressedPixMap) ReadCompressedImage(
    const Handle(Image_SupportedFormats)& theSupported) const;

  //! Image reader.
  Standard_EXPORT virtual Handle(Image_PixMap) ReadImage(
    const Handle(Image_SupportedFormats)& theSupported) const;

  //! Write image to specified file without decoding data.
  Standard_EXPORT virtual Standard_Boolean WriteImage(const AsciiString1& theFile);

  //! Write image to specified stream without decoding data.
  Standard_EXPORT virtual Standard_Boolean WriteImage(std::ostream&                  theStream,
                                                      const AsciiString1& theFile);

public: //! @name hasher interface
  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth = -1) const;

protected:
  //! Read image from normal image file.
  Standard_EXPORT virtual Handle(Image_PixMap) loadImageFile(
    const AsciiString1& thePath) const;

  //! Read image from file with some offset.
  Standard_EXPORT virtual Handle(Image_PixMap) loadImageOffset(
    const AsciiString1& thePath,
    int64_t                        theOffset,
    int64_t                        theLength) const;

  //! Read image from buffer.
  Standard_EXPORT virtual Handle(Image_PixMap) loadImageBuffer(
    const Handle(NCollection_Buffer)& theBuffer,
    const AsciiString1&    theId) const;

protected:
  AsciiString1    myTextureId; //!< generated texture id
  AsciiString1    myImagePath; //!< image file path
  Handle(NCollection_Buffer) myBuffer;    //!< image buffer
  int64_t                    myOffset;    //!< offset within file
  int64_t                    myLength;    //!< length within file
};

namespace std
{
template <>
struct equal_to<Handle(Image_Texture)>
{
  bool operator()(const Handle(Image_Texture)& theTex1, const Handle(Image_Texture)& theTex2) const
  {
    if (theTex1.IsNull() != theTex2.IsNull())
    {
      return Standard_False;
    }
    else if (theTex1.IsNull())
    {
      return Standard_True;
    }
    return theTex1->TextureId().IsEqual(theTex2->TextureId());
  }
};

template <>
struct hash<Handle(Image_Texture)>
{
  size_t operator()(const Handle(Image_Texture)& theTexture) const noexcept
  {
    return !theTexture.IsNull() ? std::hash<AsciiString1>{}(theTexture->TextureId()) : 0;
  }
};
} // namespace std

#endif // _Image_Texture_HeaderFile
