// Copyright (c) 1995-1999 Matra Datavision
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

// JCV 08/01/91 Modif introduction des classes Mat2d et XY dans gp1

#define No_Standard_OutOfRange

#include <gp_Trsf2d.hxx>

#include <gp.hxx>
#include <gp_Ax2d.hxx>
#include <gp_GTrsf2d.hxx>
#include <gp_Mat2d.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec2d.hxx>
#include <gp_XY.hxx>
#include <Standard_ConstructionError.hxx>

void Transform2d::SetMirror(const gp_Ax2d& A)
{
  shape              = gp_Ax1Mirror;
  scale              = -1.0;
  const gp_Dir2d& V  = A.Direction();
  const gp_Pnt2d& P  = A.Location();
  Standard_Real   VX = V.X();
  Standard_Real   VY = V.Y();
  Standard_Real   X0 = P.X();
  Standard_Real   Y0 = P.Y();
  matrix.SetCol(1, Coords2d(1.0 - 2.0 * VX * VX, -2.0 * VX * VY));
  matrix.SetCol(2, Coords2d(-2.0 * VX * VY, 1.0 - 2.0 * VY * VY));

  loc.SetCoord(-2.0 * ((VX * VX - 1.0) * X0 + (VX * VY * Y0)),
               -2.0 * ((VX * VY * X0) + (VY * VY - 1.0) * Y0));
}

void Transform2d::SetTransformation(const gp_Ax2d& FromA1, const gp_Ax2d& ToA2)
{
  shape = gp_CompoundTrsf;
  scale = 1.0;
  // matrix from XOY to A2 :
  const Coords2d& V1 = ToA2.Direction().XY();
  Coords2d        V2(-V1.Y(), V1.X());
  matrix.SetCol(1, V1);
  matrix.SetCol(2, V2);
  loc = ToA2.Location().XY();
  matrix.Transpose();
  loc.Multiply(matrix);
  loc.Reverse();
  // matrix FromA1 to XOY
  const Coords2d& V3 = FromA1.Direction().XY();
  Coords2d        V4(-V3.Y(), V3.X());
  Matrix2d     MA1(V3, V4);
  Coords2d        MA1loc = FromA1.Location().XY();
  // matrix * MA1 => FromA1 ToA2
  MA1loc.Multiply(matrix);
  loc.Add(MA1loc);
  matrix.Multiply(MA1);
}

void Transform2d::SetTransformation(const gp_Ax2d& A)
{
  shape           = gp_CompoundTrsf;
  scale           = 1.0;
  const Coords2d& V1 = A.Direction().XY();
  Coords2d        V2(-V1.Y(), V1.X());
  matrix.SetCol(1, V1);
  matrix.SetCol(2, V2);
  loc = A.Location().XY();
  matrix.Transpose();
  loc.Multiply(matrix);
  loc.Reverse();
}

void Transform2d::SetTranslationPart(const gp_Vec2d& V)
{
  loc             = V.XY();
  Standard_Real X = loc.X();
  if (X < 0)
    X = -X;
  Standard_Real Y = loc.Y();
  if (Y < 0)
    Y = -Y;
  if (X <= gp1::Resolution() && Y <= gp1::Resolution())
  {
    if (shape == gp_Identity || shape == gp_PntMirror || shape == gp_Scale || shape == gp_Rotation
        || shape == gp_Ax1Mirror)
    {
    }
    else if (shape == gp_Translation)
    {
      shape = gp_Identity;
    }
    else
    {
      shape = gp_CompoundTrsf;
    }
  }
  else
  {
    if (shape == gp_Translation || shape == gp_Scale || shape == gp_PntMirror)
    {
    }
    else if (shape == gp_Identity)
    {
      shape = gp_Translation;
    }
    else
    {
      shape = gp_CompoundTrsf;
    }
  }
}

void Transform2d::SetScaleFactor(const Standard_Real S)
{
  if (S == 1.0)
  {
    Standard_Real X = loc.X();
    if (X < 0)
      X = -X;
    Standard_Real Y = loc.Y();
    if (Y < 0)
      Y = -Y;
    if (X <= gp1::Resolution() && Y <= gp1::Resolution())
    {
      if (shape == gp_Identity || shape == gp_Rotation)
      {
      }
      else if (shape == gp_Scale)
      {
        shape = gp_Identity;
      }
      else if (shape == gp_PntMirror)
      {
        shape = gp_Translation;
      }
      else
      {
        shape = gp_CompoundTrsf;
      }
    }
    else
    {
      if (shape == gp_Identity || shape == gp_Rotation || shape == gp_Scale)
      {
      }
      else if (shape == gp_PntMirror)
      {
        shape = gp_Translation;
      }
      else
      {
        shape = gp_CompoundTrsf;
      }
    }
  }
  else if (S == -1)
  {
    if (shape == gp_PntMirror || shape == gp_Ax1Mirror)
    {
    }
    else if (shape == gp_Identity || shape == gp_Scale)
    {
      shape = gp_PntMirror;
    }
    else
    {
      shape = gp_CompoundTrsf;
    }
  }
  else
  {
    if (shape == gp_Scale)
    {
    }
    else if (shape == gp_Identity || shape == gp_Translation || shape == gp_PntMirror)
    {
      shape = gp_Scale;
    }
    else
    {
      shape = gp_CompoundTrsf;
    }
  }
  scale = S;
}

Matrix2d Transform2d::VectorialPart() const
{
  if (scale == 1.0)
    return matrix;
  Matrix2d M = matrix;
  if (shape == gp_Scale || shape == gp_PntMirror)
    M.SetDiagonal(matrix.Value(1, 1) * scale, matrix.Value(2, 2) * scale);
  else
    M.Multiply(scale);
  return M;
}

Standard_Real Transform2d::RotationPart() const
{
  return ATan2(matrix.Value(2, 1), matrix.Value(1, 1));
}

void Transform2d::Invert()
{
  //                                    -1
  //  X' = scale * R * X + T  =>  X = (R  / scale)  * ( X' - T)
  //
  // Pour les Transform2d puisque le scale est extrait de la matrice R
  // on a toujours determinant (R) = 1 et R-1 = R transposee.
  if (shape == gp_Identity)
  {
  }
  else if (shape == gp_Translation || shape == gp_PntMirror)
  {
    loc.Reverse();
  }
  else if (shape == gp_Scale)
  {
    Standard_Real As = scale;
    if (As < 0)
      As = -As;
    Standard_ConstructionError_Raise_if(As <= gp1::Resolution(),
                                        "Transform2d::Invert() - transformation has zero scale");
    scale = 1.0 / scale;
    loc.Multiply(-scale);
  }
  else
  {
    Standard_Real As = scale;
    if (As < 0)
      As = -As;
    Standard_ConstructionError_Raise_if(As <= gp1::Resolution(),
                                        "Transform2d::Invert() - transformation has zero scale");
    scale = 1.0 / scale;
    matrix.Transpose();
    loc.Multiply(matrix);
    loc.Multiply(-scale);
  }
}

void Transform2d::Multiply(const Transform2d& T)
{
  if (T.shape == gp_Identity)
  {
  }
  else if (shape == gp_Identity)
  {
    shape  = T.shape;
    scale  = T.scale;
    loc    = T.loc;
    matrix = T.matrix;
  }
  else if (shape == gp_Rotation && T.shape == gp_Rotation)
  {
    if (loc.X() != 0.0 || loc.Y() != 0.0)
    {
      loc.Add(T.loc.Multiplied(matrix));
    }
    matrix.Multiply(T.matrix);
  }
  else if (shape == gp_Translation && T.shape == gp_Translation)
  {
    loc.Add(T.loc);
  }
  else if (shape == gp_Scale && T.shape == gp_Scale)
  {
    loc.Add(T.loc.Multiplied(scale));
    scale = scale * T.scale;
  }
  else if (shape == gp_PntMirror && T.shape == gp_PntMirror)
  {
    scale = 1.0;
    shape = gp_Translation;
    loc.Add(T.loc.Reversed());
  }
  else if (shape == gp_Ax1Mirror && T.shape == gp_Ax1Mirror)
  {
    shape = gp_Rotation;
    Coords2d Tloc(T.loc);
    Tloc.Multiply(matrix);
    Tloc.Multiply(scale);
    scale = scale * T.scale;
    loc.Add(Tloc);
    matrix.Multiply(T.matrix);
  }
  else if ((shape == gp_CompoundTrsf || shape == gp_Rotation || shape == gp_Ax1Mirror)
           && T.shape == gp_Translation)
  {
    Coords2d Tloc(T.loc);
    Tloc.Multiply(matrix);
    if (scale != 1.0)
      Tloc.Multiply(scale);
    loc.Add(Tloc);
  }
  else if ((shape == gp_Scale || shape == gp_PntMirror) && T.shape == gp_Translation)
  {
    Coords2d Tloc(T.loc);
    Tloc.Multiply(scale);
    loc.Add(Tloc);
  }
  else if (shape == gp_Translation
           && (T.shape == gp_CompoundTrsf || T.shape == gp_Rotation || T.shape == gp_Ax1Mirror))
  {
    shape = gp_CompoundTrsf;
    scale = T.scale;
    loc.Add(T.loc);
    matrix = T.matrix;
  }
  else if (shape == gp_Translation && (T.shape == gp_Scale || T.shape == gp_PntMirror))
  {
    shape = T.shape;
    loc.Add(T.loc);
    scale = T.scale;
  }
  else if ((shape == gp_PntMirror || shape == gp_Scale)
           && (T.shape == gp_PntMirror || T.shape == gp_Scale))
  {
    shape = gp_CompoundTrsf;
    Coords2d Tloc(T.loc);
    Tloc.Multiply(scale);
    loc.Add(Tloc);
    scale = scale * T.scale;
  }
  else if ((shape == gp_CompoundTrsf || shape == gp_Rotation || shape == gp_Ax1Mirror)
           && (T.shape == gp_Scale || T.shape == gp_PntMirror))
  {
    shape = gp_CompoundTrsf;
    Coords2d Tloc(T.loc);
    Tloc.Multiply(matrix);
    if (scale == 1.0)
      scale = T.scale;
    else
    {
      Tloc.Multiply(scale);
      scale = scale * T.scale;
    }
    loc.Add(Tloc);
  }
  else if ((T.shape == gp_CompoundTrsf || T.shape == gp_Rotation || T.shape == gp_Ax1Mirror)
           && (shape == gp_Scale || shape == gp_PntMirror))
  {
    shape = gp_CompoundTrsf;
    Coords2d Tloc(T.loc);
    Tloc.Multiply(scale);
    scale = scale * T.scale;
    loc.Add(Tloc);
    matrix = T.matrix;
  }
  else
  {
    shape = gp_CompoundTrsf;
    Coords2d Tloc(T.loc);
    Tloc.Multiply(matrix);
    if (scale != 1.0)
    {
      Tloc.Multiply(scale);
      scale = scale * T.scale;
    }
    else
    {
      scale = T.scale;
    }
    loc.Add(Tloc);
    matrix.Multiply(T.matrix);
  }
}

void Transform2d::Power(const Standard_Integer N)
{
  if (shape == gp_Identity)
  {
  }
  else
  {
    if (N == 0)
    {
      scale = 1.0;
      shape = gp_Identity;
      matrix.SetIdentity();
      loc = Coords2d(0.0, 0.0);
    }
    else if (N == 1)
    {
    }
    else if (N == -1)
      Invert();
    else
    {
      if (N < 0)
        Invert();
      if (shape == gp_Translation)
      {
        Standard_Integer Npower = N;
        if (Npower < 0)
          Npower = -Npower;
        Npower--;
        Coords2d Temploc = loc;
        for (;;)
        {
          if (IsOdd(Npower))
            loc.Add(Temploc);
          if (Npower == 1)
            break;
          Temploc.Add(Temploc);
          Npower = Npower / 2;
        }
      }
      else if (shape == gp_Scale)
      {
        Standard_Integer Npower = N;
        if (Npower < 0)
          Npower = -Npower;
        Npower--;
        Coords2d         Temploc   = loc;
        Standard_Real Tempscale = scale;
        for (;;)
        {
          if (IsOdd(Npower))
          {
            loc.Add(Temploc.Multiplied(scale));
            scale = scale * Tempscale;
          }
          if (Npower == 1)
            break;
          Temploc.Add(Temploc.Multiplied(Tempscale));
          Tempscale = Tempscale * Tempscale;
          Npower    = Npower / 2;
        }
      }
      else if (shape == gp_Rotation)
      {
        Standard_Integer Npower = N;
        if (Npower < 0)
          Npower = -Npower;
        Npower--;
        Matrix2d Tempmatrix(matrix);
        if (loc.X() == 0.0 && loc.Y() == 0.0)
        {
          for (;;)
          {
            if (IsOdd(Npower))
              matrix.Multiply(Tempmatrix);
            if (Npower == 1)
              break;
            Tempmatrix.Multiply(Tempmatrix);
            Npower = Npower / 2;
          }
        }
        else
        {
          Coords2d Temploc = loc;
          for (;;)
          {
            if (IsOdd(Npower))
            {
              loc.Add(Temploc.Multiplied(matrix));
              matrix.Multiply(Tempmatrix);
            }
            if (Npower == 1)
              break;
            Temploc.Add(Temploc.Multiplied(Tempmatrix));
            Tempmatrix.Multiply(Tempmatrix);
            Npower = Npower / 2;
          }
        }
      }
      else if (shape == gp_PntMirror || shape == gp_Ax1Mirror)
      {
        if (IsEven(N))
        {
          shape = gp_Identity;
          scale = 1.0;
          matrix.SetIdentity();
          loc.SetCoord(0.0, 0.0);
        }
      }
      else
      {
        shape                   = gp_CompoundTrsf;
        Standard_Integer Npower = N;
        if (Npower < 0)
          Npower = -Npower;
        Npower--;
        matrix.SetDiagonal(scale * matrix.Value(1, 1), scale * matrix.Value(2, 2));
        Coords2d         Temploc   = loc;
        Standard_Real Tempscale = scale;
        Matrix2d      Tempmatrix(matrix);
        for (;;)
        {
          if (IsOdd(Npower))
          {
            loc.Add((Temploc.Multiplied(matrix)).Multiplied(scale));
            scale = scale * Tempscale;
            matrix.Multiply(Tempmatrix);
          }
          if (Npower == 1)
            break;
          Tempscale = Tempscale * Tempscale;
          Temploc.Add((Temploc.Multiplied(Tempmatrix)).Multiplied(Tempscale));
          Tempmatrix.Multiply(Tempmatrix);
          Npower = Npower / 2;
        }
      }
    }
  }
}

void Transform2d::PreMultiply(const Transform2d& T)
{
  if (T.shape == gp_Identity)
  {
  }
  else if (shape == gp_Identity)
  {
    shape  = T.shape;
    scale  = T.scale;
    loc    = T.loc;
    matrix = T.matrix;
  }
  else if (shape == gp_Rotation && T.shape == gp_Rotation)
  {
    loc.Multiply(T.matrix);
    loc.Add(T.loc);
    matrix.PreMultiply(T.matrix);
  }
  else if (shape == gp_Translation && T.shape == gp_Translation)
  {
    loc.Add(T.loc);
  }
  else if (shape == gp_Scale && T.shape == gp_Scale)
  {
    loc.Multiply(T.scale);
    loc.Add(T.loc);
    scale = scale * T.scale;
  }
  else if (shape == gp_PntMirror && T.shape == gp_PntMirror)
  {
    scale = 1.0;
    shape = gp_Translation;
    loc.Reverse();
    loc.Add(T.loc);
  }
  else if (shape == gp_Ax1Mirror && T.shape == gp_Ax1Mirror)
  {
    shape = gp_Rotation;
    loc.Multiply(T.matrix);
    loc.Multiply(T.scale);
    scale = scale * T.scale;
    loc.Add(T.loc);
    matrix.PreMultiply(T.matrix);
  }
  else if ((shape == gp_CompoundTrsf || shape == gp_Rotation || shape == gp_Ax1Mirror)
           && T.shape == gp_Translation)
  {
    loc.Add(T.loc);
  }
  else if ((shape == gp_Scale || shape == gp_PntMirror) && T.shape == gp_Translation)
  {
    loc.Add(T.loc);
  }
  else if (shape == gp_Translation
           && (T.shape == gp_CompoundTrsf || T.shape == gp_Rotation || T.shape == gp_Ax1Mirror))
  {
    shape  = gp_CompoundTrsf;
    matrix = T.matrix;
    if (T.scale == 1.0)
      loc.Multiply(T.matrix);
    else
    {
      scale = T.scale;
      loc.Multiply(matrix);
      loc.Multiply(scale);
    }
    loc.Add(T.loc);
  }
  else if ((T.shape == gp_Scale || T.shape == gp_PntMirror) && shape == gp_Translation)
  {
    loc.Multiply(T.scale);
    loc.Add(T.loc);
    scale = T.scale;
    shape = T.shape;
  }
  else if ((shape == gp_PntMirror || shape == gp_Scale)
           && (T.shape == gp_PntMirror || T.shape == gp_Scale))
  {
    shape = gp_CompoundTrsf;
    loc.Multiply(T.scale);
    loc.Add(T.loc);
    scale = scale * T.scale;
  }
  else if ((shape == gp_CompoundTrsf || shape == gp_Rotation || shape == gp_Ax1Mirror)
           && (T.shape == gp_Scale || T.shape == gp_PntMirror))
  {
    shape = gp_CompoundTrsf;
    loc.Multiply(T.scale);
    loc.Add(T.loc);
    scale = scale * T.scale;
  }
  else if ((T.shape == gp_CompoundTrsf || T.shape == gp_Rotation || T.shape == gp_Ax1Mirror)
           && (shape == gp_Scale || shape == gp_PntMirror))
  {
    shape  = gp_CompoundTrsf;
    matrix = T.matrix;
    if (T.scale == 1.0)
      loc.Multiply(T.matrix);
    else
    {
      loc.Multiply(matrix);
      loc.Multiply(T.scale);
      scale = T.scale * scale;
    }
    loc.Add(T.loc);
  }
  else
  {
    shape = gp_CompoundTrsf;
    loc.Multiply(T.matrix);
    if (T.scale != 1.0)
    {
      loc.Multiply(T.scale);
      scale = scale * T.scale;
    }
    loc.Add(T.loc);
    matrix.PreMultiply(T.matrix);
  }
}

//=================================================================================================

void Transform2d::SetValues(const Standard_Real a11,
                          const Standard_Real a12,
                          const Standard_Real a13,
                          const Standard_Real a21,
                          const Standard_Real a22,
                          const Standard_Real a23)
{
  Coords2d col1(a11, a21);
  Coords2d col2(a12, a22);
  Coords2d col3(a13, a23);
  // compute the determinant
  Matrix2d      M(col1, col2);
  Standard_Real s  = M.Determinant();
  Standard_Real As = s;
  if (As < 0)
    As = -As;
  Standard_ConstructionError_Raise_if(As < gp1::Resolution(),
                                      "Transform2d::SetValues, null determinant");

  if (s > 0)
    s = sqrt(s);
  else
    s = sqrt(-s);

  M.Divide(s);

  scale = s;
  shape = gp_CompoundTrsf;

  matrix = M;
  Orthogonalize();

  loc = col3;
}

//=======================================================================
// function : Orthogonalize
// purpose  :
// ATTENTION!!!
//      Orthogonalization is not equivalent transformation.Therefore, transformation with
//        source matrix and with orthogonalized matrix can lead to different results for
//        one shape. Consequently, source matrix must be close to orthogonalized
//        matrix for reducing these differences.
//=======================================================================
void Transform2d::Orthogonalize()
{
  // See correspond comment in Transform3d::Orthogonalize() method in order to make this
  // algorithm clear.

  Matrix2d aTM(matrix);

  Coords2d aV1 = aTM.Column(1);
  Coords2d aV2 = aTM.Column(2);

  aV1.Normalize();

  aV2 -= aV1 * (aV2.Dot(aV1));
  aV2.Normalize();

  aTM.SetCols(aV1, aV2);

  aV1 = aTM.Row(1);
  aV2 = aTM.Row(2);

  aV1.Normalize();

  aV2 -= aV1 * (aV2.Dot(aV1));
  aV2.Normalize();

  aTM.SetRows(aV1, aV2);

  matrix = aTM;
}
