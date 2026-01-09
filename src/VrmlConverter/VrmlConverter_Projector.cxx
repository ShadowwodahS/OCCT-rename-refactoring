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

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Ax3.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <Standard_Type.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfVec.hxx>
#include <Vrml_Instancing.hxx>
#include <Vrml_TransformSeparator.hxx>
#include <VrmlConverter_Projector.hxx>

IMPLEMENT_STANDARD_RTTIEXT(VrmlConverter_Projector, RefObject)

VrmlConverter_Projector::VrmlConverter_Projector(const TopTools_Array1OfShape&    Shapes,
                                                 const Standard_Real              Focus,
                                                 const Standard_Real              DX,
                                                 const Standard_Real              DY,
                                                 const Standard_Real              DZ,
                                                 const Standard_Real              XUp,
                                                 const Standard_Real              YUp,
                                                 const Standard_Real              ZUp,
                                                 const VrmlConverter_TypeOfCamera Camera,
                                                 const VrmlConverter_TypeOfLight  Light)

{

  myTypeOfCamera = Camera;
  myTypeOfLight  = Light;

  Standard_Integer i;
  Box2          box;
  Standard_Real    Xmin, Xmax, Ymin, Ymax, Zmin, Zmax, diagonal;
  Standard_Real    Xtarget, Ytarget, Ztarget, Angle, MaxAngle, Height, MaxHeight;

  for (i = Shapes.Lower(); i <= Shapes.Upper(); i++)
  {
    BRepBndLib1::AddClose(Shapes.Value(i), box);
  }

  Standard_Real DistMax = 500000;
  Standard_Real TolMin  = 0.000001;

  box.Enlarge(TolMin);
  box.Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);

  if (box.IsOpenXmin())
    Xmin = -DistMax;
  if (box.IsOpenXmax())
    Xmax = DistMax;
  if (box.IsOpenYmin())
    Ymin = -DistMax;
  if (box.IsOpenYmax())
    Ymax = DistMax;
  if (box.IsOpenZmin())
    Zmin = -DistMax;
  if (box.IsOpenZmax())
    Zmax = DistMax;

  Standard_Real xx = (Xmax - Xmin);
  Standard_Real yy = (Ymax - Ymin);
  Standard_Real zz = (Zmax - Zmin);

  Xtarget = (Xmin + Xmax) / 2;
  Ytarget = (Ymin + Ymax) / 2;
  Ztarget = (Zmin + Zmax) / 2;

  //  std::cout << " target: " << std::endl;
  //  std::cout << " X: " << Xtarget << " Y: " << Ytarget  << " Z: " << Ztarget  <<  std::endl;

  //  Point3d Target(Xtarget, Ytarget, Ztarget);
  //  Vector3d VTarget(Target.X(),Target.Y(),Target.Z());

  Dir3d Zpers(DX, DY, DZ);
  Vector3d V(Zpers);

  diagonal = Sqrt(xx * xx + yy * yy + zz * zz);

  Vector3d aVec = V.Multiplied(0.5 * diagonal + TolMin + Focus);

  Point3d Source;
  Source.SetX(Xtarget + aVec.X());
  Source.SetY(Ytarget + aVec.Y());
  Source.SetZ(Ztarget + aVec.Z());

  //  std::cout << " source: " << std::endl;
  //  std::cout << " X: " << Source.X() << " Y: " << Source.Y() << " Z: " << Source.Z()  <<
  //  std::endl;

  Vector3d VSource(Source.X(), Source.Y(), Source.Z());

  //  Vector3d Proj(Source,Target);
  //  std::cout << " Vec(source-target): " << std::endl;
  //  std::cout << " X: " << Proj.X() << " Y: " << Proj.Y() << " Z: " << Proj.Z()  <<  std::endl;

  Dir3d Ypers(XUp, YUp, ZUp);

  if (Ypers.IsParallel(Zpers, Precision1::Angular()))
  {
    throw ExceptionBase("Projection Vector is Parallel to High Point Direction");
  }
  Dir3d Xpers = Ypers.Crossed(Zpers);

  //  std::cout << " Dir(Zpers): " << std::endl;
  //  std::cout << " X: " << Zpers.X() << " Y: " << Zpers.Y() << " Z: " << Zpers.Z()  <<  std::endl;
  //  std::cout << " Dir(Xpers): " << std::endl;
  //  std::cout << " X: " << Xpers.X() << " Y: " << Xpers.Y() << " Z: " << Xpers.Z()  <<  std::endl;

  Ax3 Axe(Source, Zpers, Xpers);

  Transform3d T;

  //  Makes the transformation allowing passage from the basic
  //  coordinate system
  //  {P(0.,0.,0.), VX (1.,0.,0.), VY (0.,1.,0.), VZ (0., 0. ,1.) }
  //  to the local coordinate system defined with the Ax3 ToSystem.
  //  Same utilisation as the previous method. FromSystem1 is
  //  defaulted to the absolute coordinate system.
  T.SetTransformation(Axe);

  Standard_Boolean Pers = Standard_False;
  if (Camera == VrmlConverter_PerspectiveCamera)
    Pers = Standard_True;

  // build a Projector with automatic minmax directions
  myProjector = HLRAlgoProjector(T, Pers, Focus);

  Transform3d T3;
  T3 = T.Inverted();
  //  T3.SetTranslationPart(Vector3d (0,0,0));

  myMatrixTransform.SetMatrix(T3);

  // For VRweb1.3
  //  Transform3d T1 = T;
  //  T1.SetTranslationPart(Vector3d (0,0,0));
  //  myMatrixTransform.SetMatrix(T1);

  //
  //== definitions cameras and lights
  //

  if (Light == VrmlConverter_DirectionLight)
  {
    myDirectionalLight.SetDirection(Zpers.Reversed());
  }

  if (Light == VrmlConverter_PointLight)
  {
    myPointLight.SetLocation(VSource);
  }

  if (Light == VrmlConverter_SpotLight || Camera != VrmlConverter_NoCamera)
  {

    /*
  Dir3d Zmain (0,0,1);
  Dir3d Xmain (1,0,0);


  Dir3d Dturn;
  Standard_Real AngleTurn;

  if( Zmain.IsParallel(Zpers,Precision1::Angular()) )
    {
      if ( Zmain.IsOpposite(Zpers,Precision1::Angular()) )
    {
      Dturn = Zpers;
      AngleTurn = - Xmain.Angle(Xpers);
    }
      Dturn = Zpers;
      AngleTurn = Xmain.Angle(Xpers);
    }
  else
    {
      Dturn = Zmain.Crossed(Zpers);
      AngleTurn = Zmain.Angle(Zpers);
    }
*/

    Point3d             CurP;
    TColgp_Array1OfPnt ArrP(1, 8);

    CurP.SetCoord(Xmin, Ymin, Zmin);
    ArrP.SetValue(1, CurP);
    CurP.SetCoord(Xmin + xx, Ymin, Zmin);
    ArrP.SetValue(2, CurP);
    CurP.SetCoord(Xmin + xx, Ymin + yy, Zmin);
    ArrP.SetValue(3, CurP);
    CurP.SetCoord(Xmin, Ymin + yy, Zmin);
    ArrP.SetValue(4, CurP);

    CurP.SetCoord(Xmin, Ymin, Zmax);
    ArrP.SetValue(5, CurP);
    CurP.SetCoord(Xmin + xx, Ymin, Zmax);
    ArrP.SetValue(6, CurP);
    CurP.SetCoord(Xmin + xx, Ymin + yy, Zmax);
    ArrP.SetValue(7, CurP);
    CurP.SetCoord(Xmin, Ymin + yy, Zmax);
    ArrP.SetValue(8, CurP);

    //
    Vector3d V1, V2;
    Point3d P1, P2;

    MaxHeight = TolMin;
    MaxAngle  = TolMin;

    for (i = ArrP.Lower(); i <= ArrP.Upper(); i++)
    {
      P1 = ArrP.Value(i);
      P2 = P1.Transformed(T);

      V1.SetX(P2.X());
      V1.SetY(P2.Y());
      V1.SetZ(P2.Z());

      V2.SetX(P2.X());
      V2.SetY(0);
      V2.SetZ(P2.Z());

      //  std::cout << " Angle: " << V1.Angle(V2) << std::endl;
      //  std::cout << " ****************** " << std::endl;
      if (Abs(V1.Angle(V2)) > Abs(MaxAngle))
        MaxAngle = Abs(V1.Angle(V2));

      V2.SetX(0);
      V2.SetY(P2.Y());
      V2.SetZ(P2.Z());

      //  std::cout << " Angle: " << V1.Angle(V2) << std::endl;
      //  std::cout << " ****************** " << std::endl;
      if (Abs(V1.Angle(V2)) > Abs(MaxAngle))
        MaxAngle = Abs(V1.Angle(V2));

      if (Abs(P2.Y()) > Abs(MaxHeight))
      {
        //  std::cout << " Height Y: " << P2.Y() << std::endl;
        //  std::cout << " ****************** " << std::endl;
        MaxHeight = Abs(P2.Y());
      }

      if (Abs(P2.X()) > Abs(MaxHeight))
      {
        //  std::cout << " Height X: " << P2.X() << std::endl;
        //  std::cout << " ****************** " << std::endl;
        MaxHeight = Abs(P2.X());
      }
    }
    Height = MaxHeight;
    //  std::cout << " MaxHeight: " << Height << std::endl;
    //  std::cout << " ****************** " << std::endl;

    Angle = MaxAngle;
    //  std::cout << " MaxAngle: " << Angle << std::endl;
    //  std::cout << " ****************** " << std::endl;

    if (Light == VrmlConverter_SpotLight)
    {
      mySpotLight.SetLocation(VSource);
      mySpotLight.SetDirection(Zpers.Reversed());
      mySpotLight.SetCutOffAngle(2 * Angle);
    }

    if (Camera == VrmlConverter_PerspectiveCamera)
    {
      //    myPerspectiveCamera.SetPosition(VSource);
      //    myPerspectiveCamera.SetOrientation(SFRotation
      //    (Dturn.X(),Dturn.Y(),Dturn.Z(),AngleTurn));
      myPerspectiveCamera.SetFocalDistance(Focus);
      myPerspectiveCamera.SetAngle(2 * Angle);
    }

    if (Camera == VrmlConverter_OrthographicCamera)
    {
      //  myOrthographicCamera.SetPosition(VSource);
      //  myOrthographicCamera.SetOrientation(SFRotation
      //  (Dturn.X(),Dturn.Y(),Dturn.Z(),AngleTurn));
      myOrthographicCamera.SetFocalDistance(Focus);
      myOrthographicCamera.SetHeight(2 * Height);
    }
  }
}

void VrmlConverter_Projector::Add(Standard_OStream& anOStream) const
{
  switch (myTypeOfCamera)
  {
    case VrmlConverter_NoCamera:
      break;
    case VrmlConverter_PerspectiveCamera: {
      TransformSeparator TS;
      TS.Print(anOStream);
      myMatrixTransform.Print(anOStream);
      Instancing I1("Perspective Camera");
      I1.DEF(anOStream);
      myPerspectiveCamera.Print(anOStream);
      TS.Print(anOStream);
    }
    break;
    case VrmlConverter_OrthographicCamera: {
      TransformSeparator TS;
      TS.Print(anOStream);
      myMatrixTransform.Print(anOStream);
      Instancing I2("Orthographic Camera");
      I2.DEF(anOStream);
      myOrthographicCamera.Print(anOStream);
      TS.Print(anOStream);
    }
    break;
  }

  switch (myTypeOfLight)
  {
    case VrmlConverter_NoLight:
      break;
    case VrmlConverter_DirectionLight: {
      myDirectionalLight.Print(anOStream);
    }
    break;
    case VrmlConverter_PointLight: {
      myPointLight.Print(anOStream);
    }
    break;
    case VrmlConverter_SpotLight: {
      mySpotLight.Print(anOStream);
    }
    break;
  }
}

void VrmlConverter_Projector::SetCamera(const VrmlConverter_TypeOfCamera aCamera)
{
  myTypeOfCamera = aCamera;
}

VrmlConverter_TypeOfCamera VrmlConverter_Projector::Camera() const
{
  return myTypeOfCamera;
}

void VrmlConverter_Projector::SetLight(const VrmlConverter_TypeOfLight aLight)
{
  myTypeOfLight = aLight;
}

VrmlConverter_TypeOfLight VrmlConverter_Projector::Light() const
{
  return myTypeOfLight;
}

HLRAlgoProjector VrmlConverter_Projector::Projector() const
{
  return myProjector;
}
