#include "Matrix4.hpp"

#include <cassert>
#include <cstring>

#include "Math.hpp"
#include "Matrix3.hpp"
#include "Quaternion.hpp"

static float32 Minor(const Matrix4 &_m, const uint32 r0, const uint32 r1, const uint32 r2,
                     const uint32 c0, const uint32 c1, const uint32 c2)
{
  return _m[r0][c0] * (_m[r1][c1] * _m[r2][c2] - _m[r2][c1] * _m[r1][c2]) - _m[r0][c1] * (_m[r1][c0] * _m[r2][c2] - _m[r2][c0] * _m[r1][c2]) + _m[r0][c2] * (_m[r1][c0] * _m[r2][c1] - _m[r2][c0] * _m[r1][c1]);
}

Matrix4 Matrix4::Zero = Matrix4(0.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 0.0f);

Matrix4 Matrix4::Identity = Matrix4(1.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f);

Matrix4 Matrix4::Translation(const Vector3 &translation)
{
  Matrix4 result(Matrix4::Identity);
  result[3] = result[0] * translation[0] + result[1] * translation[1] + result[2] * translation[2] + result[3];
  return result;
}

Matrix4 Matrix4::Scaling(const Vector3 &scale)
{
  Matrix4 result(Matrix4::Identity);
  result[0] = result[0] * scale[0];
  result[1] = result[1] * scale[1];
  result[2] = result[2] * scale[2];
  result[3] = result[3];
  return result;
}

Matrix4 Matrix4::Rotation(const Quaternion &rotation)
{
  Matrix4 result(rotation);
  return result;
}

Matrix4 Matrix4::LookAt(const Vector3 &eye, const Vector3 &center, const Vector3 &up)
{
  Vector3 f(Vector3::Normalize(center - eye));
  Vector3 s(Vector3::Normalize(Vector3::Cross(f, up)));
  Vector3 u(Vector3::Cross(s, f));

  Matrix4 result(Matrix4::Identity);
  result[0][0] = s[0];
  result[1][0] = s[1];
  result[2][0] = s[2];
  result[0][1] = u[0];
  result[1][1] = u[1];
  result[2][1] = u[2];
  result[0][2] = -f[0];
  result[1][2] = -f[1];
  result[2][2] = -f[2];
  result[3][0] = -Vector3::Dot(s, eye);
  result[3][1] = -Vector3::Dot(u, eye);
  result[3][2] = Vector3::Dot(f, eye);
  return result;
}

Matrix4 Matrix4::Perspective(const Radian &yFov, float32 aspect, float32 zNear, float32 zFar)
{
  float32 tanHalfFov = Math::Tan(0.5f * yFov);
  Matrix4 result(Matrix4::Zero);
  result[0][0] = 1.0f / (aspect * tanHalfFov);
  result[1][1] = 1.0f / (tanHalfFov);
  result[2][2] = -(zFar + zNear) / (zFar - zNear);
  result[2][3] = -1.0f;
  result[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
  return result;
}

Matrix4 Matrix4::Orthographic(float32 left, float32 right, float32 bottom, float32 top, float32 zNear, float32 zFar)
{
  Matrix4 result(Matrix4::Identity);
  result[0][0] = 2.0f / (right - left);
  result[1][1] = 2.0f / (top - bottom);
  result[2][2] = -2.0f / (zFar - zNear);
  result[3][0] = -(right + left) / (right - left);
  result[3][1] = -(top + bottom) / (top - bottom);
  result[3][2] = -(zFar + zNear) / (zFar - zNear);
  return result;
}

Matrix3 Matrix4::ToMatrix3(const Matrix4 &mat)
{
  return Matrix3(mat);
}

Matrix4::Matrix4()
{
}

Matrix4::Matrix4(const Matrix4 &mat)
{
  std::memcpy(_m, mat._m, 16 * sizeof(float32));
}

Matrix4::Matrix4(const Matrix3 &mat)
{
  _m[0][0] = mat[0][0];
  _m[0][1] = mat[0][1];
  _m[0][2] = mat[0][2];
  _m[0][3] = 0.0f;
  _m[1][0] = mat[1][0];
  _m[1][1] = mat[1][1];
  _m[1][2] = mat[1][2];
  _m[1][3] = 0.0f;
  _m[2][0] = mat[2][0];
  _m[2][1] = mat[2][1];
  _m[2][2] = mat[2][2];
  _m[2][3] = 0.0f;
  _m[3][0] = 0.0f;
  _m[3][1] = 0.0f;
  _m[3][2] = 0.0f;
  _m[3][3] = 1.0f;
}

Matrix4::Matrix4(float32 a, float32 b, float32 c, float32 d,
                 float32 e, float32 f, float32 g, float32 h,
                 float32 i, float32 j, float32 k, float32 l,
                 float32 m, float32 n, float32 o, float32 p)
{
  _m[0][0] = a;
  _m[0][1] = b;
  _m[0][2] = c;
  _m[0][3] = d;
  _m[1][0] = e;
  _m[1][1] = f;
  _m[1][2] = g;
  _m[1][3] = h;
  _m[2][0] = i;
  _m[2][1] = j;
  _m[2][2] = k;
  _m[2][3] = l;
  _m[3][0] = m;
  _m[3][1] = n;
  _m[3][2] = o;
  _m[3][3] = p;
}

Matrix4::Matrix4(const Quaternion &qat)
{
  *this = Matrix4(Matrix3(qat));
}

Matrix4 Matrix4::operator+(float32 rhs) const
{
  Matrix4 mat;
  for (uint32 row = 0; row < 4; ++row)
  {
    for (uint32 col = 0; col < 4; ++col)
    {
      mat[row][col] = _m[row][col] + rhs;
    }
  }
  return mat;
}

Matrix4 Matrix4::operator-(float32 rhs) const
{
  Matrix4 mat;
  for (uint32 row = 0; row < 4; ++row)
  {
    for (uint32 col = 0; col < 4; ++col)
    {
      mat[row][col] = _m[row][col] - rhs;
    }
  }
  return mat;
}

Matrix4 Matrix4::operator*(float32 rhs) const
{
  Matrix4 mat;
  for (uint32 row = 0; row < 4; ++row)
  {
    for (uint32 col = 0; col < 4; ++col)
    {
      mat[row][col] = _m[row][col] * rhs;
    }
  }
  return mat;
}

Matrix4 Matrix4::operator+(const Matrix4 &rhs) const
{
  Matrix4 mat;
  for (uint32 row = 0; row < 4; ++row)
  {
    for (uint32 col = 0; col < 4; ++col)
    {
      mat[row][col] = _m[row][col] + rhs[row][col];
    }
  }
  return mat;
}

Matrix4 Matrix4::operator-(const Matrix4 &rhs) const
{
  Matrix4 mat;
  for (uint32 row = 0; row < 4; ++row)
  {
    for (uint32 col = 0; col < 4; ++col)
    {
      mat[row][col] = _m[row][col] - rhs[row][col];
    }
  }
  return mat;
}

Matrix4 Matrix4::operator*(const Matrix4 &rhs) const
{
  Matrix4 result;
  result[0] = _m[0] * rhs._m[0][0] + _m[1] * rhs._m[0][1] + _m[2] * rhs._m[0][2] + _m[3] * rhs._m[0][3];
  result[1] = _m[0] * rhs._m[1][0] + _m[1] * rhs._m[1][1] + _m[2] * rhs._m[1][2] + _m[3] * rhs._m[1][3];
  result[2] = _m[0] * rhs._m[2][0] + _m[1] * rhs._m[2][1] + _m[2] * rhs._m[2][2] + _m[3] * rhs._m[2][3];
  result[3] = _m[0] * rhs._m[3][0] + _m[1] * rhs._m[3][1] + _m[2] * rhs._m[3][2] + _m[3] * rhs._m[3][3];
  return result;
}

Vector4 Matrix4::operator*(const Vector4 &rhs) const
{
  return Vector4(_m[0][0] * rhs[0] + _m[1][0] * rhs[1] + _m[2][0] * rhs[2] + _m[3][0] * rhs[3],
                 _m[0][1] * rhs[0] + _m[1][1] * rhs[1] + _m[2][1] * rhs[2] + _m[3][1] * rhs[3],
                 _m[0][2] * rhs[0] + _m[1][2] * rhs[1] + _m[2][2] * rhs[2] + _m[3][2] * rhs[3],
                 _m[0][3] * rhs[0] + _m[1][3] * rhs[1] + _m[2][3] * rhs[2] + _m[3][3] * rhs[3]);
}

// Matrix4& Matrix4::operator+=(float32 rhs)
//{
//   Matrix4 mat;
//   for (uint32 row = 0; row < 4; ++row)
//   {
//     for (uint32 col = 0; col < 4; ++col)
//     {
//       _m[row][col] += rhs;
//     }
//   }
//   return *this;
// }

// Matrix4& Matrix4::operator-=(float32 rhs)
//{
//   Matrix4 mat;
//   for (uint32 row = 0; row < 4; ++row)
//   {
//     for (uint32 col = 0; col < 4; ++col)
//     {
//       _m[row][col] -= rhs;
//     }
//   }
//   return *this;
// }

// Matrix4& Matrix4::operator*=(float32 rhs)
//{
//   Matrix4 mat;
//   for (uint32 row = 0; row < 4; ++row)
//   {
//     for (uint32 col = 0; col < 4; ++col)
//     {
//       _m[row][col] *= rhs;
//     }
//   }
//   return *this;
// }

bool Matrix4::operator==(const Matrix4 &rhs) const
{
  for (uint32 row = 0; row < 4; ++row)
  {
    for (uint32 col = 0; col < 4; ++col)
    {
      if (_m[row][col] != rhs._m[row][col])
      {
        return false;
      }
    }
  }
  return true;
}

bool Matrix4::operator!=(const Matrix4 &rhs) const
{
  uint32 equalityCount = 0;
  for (uint32 row = 0; row < 4; ++row)
  {
    for (uint32 col = 0; col < 4; ++col)
    {
      if (_m[row][col] == rhs._m[row][col])
      {
        ++equalityCount;
      }
    }
  }
  return equalityCount < 16;
}

Vector4 &Matrix4::operator[](uint32 row)
{
  assert(row < 4);
  return _m[row];
}

const Vector4 &Matrix4::operator[](uint32 row) const
{
  assert(row < 4);
  return _m[row];
}

float32 Matrix4::Determinate() const
{
  return _m[0][0] * Minor(*this, 1, 2, 3, 1, 2, 3) -
         _m[0][1] * Minor(*this, 1, 2, 3, 0, 2, 3) +
         _m[0][2] * Minor(*this, 1, 2, 3, 0, 1, 3) -
         _m[0][3] * Minor(*this, 1, 2, 3, 0, 1, 2);
}

Matrix4 Matrix4::Inverse() const
{
  float32 m00 = _m[0][0], m01 = _m[0][1], m02 = _m[0][2], m03 = _m[0][3];
  float32 m10 = _m[1][0], m11 = _m[1][1], m12 = _m[1][2], m13 = _m[1][3];
  float32 m20 = _m[2][0], m21 = _m[2][1], m22 = _m[2][2], m23 = _m[2][3];
  float32 m30 = _m[3][0], m31 = _m[3][1], m32 = _m[3][2], m33 = _m[3][3];

  float32 v0 = m20 * m31 - m21 * m30;
  float32 v1 = m20 * m32 - m22 * m30;
  float32 v2 = m20 * m33 - m23 * m30;
  float32 v3 = m21 * m32 - m22 * m31;
  float32 v4 = m21 * m33 - m23 * m31;
  float32 v5 = m22 * m33 - m23 * m32;

  float32 t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
  float32 t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
  float32 t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
  float32 t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

  float32 invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

  float32 d00 = t00 * invDet;
  float32 d10 = t10 * invDet;
  float32 d20 = t20 * invDet;
  float32 d30 = t30 * invDet;

  float32 d01 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
  float32 d11 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
  float32 d21 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
  float32 d31 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

  v0 = m10 * m31 - m11 * m30;
  v1 = m10 * m32 - m12 * m30;
  v2 = m10 * m33 - m13 * m30;
  v3 = m11 * m32 - m12 * m31;
  v4 = m11 * m33 - m13 * m31;
  v5 = m12 * m33 - m13 * m32;

  float32 d02 = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
  float32 d12 = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
  float32 d22 = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
  float32 d32 = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

  v0 = m21 * m10 - m20 * m11;
  v1 = m22 * m10 - m20 * m12;
  v2 = m23 * m10 - m20 * m13;
  v3 = m22 * m11 - m21 * m12;
  v4 = m23 * m11 - m21 * m13;
  v5 = m23 * m12 - m22 * m13;

  float32 d03 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
  float32 d13 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
  float32 d23 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
  float32 d33 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

  return Matrix4(d00, d01, d02, d03,
                 d10, d11, d12, d13,
                 d20, d21, d22, d23,
                 d30, d31, d32, d33);
}

Matrix4 Matrix4::Transpose() const
{
  return Matrix4(_m[0][0], _m[1][0], _m[2][0], _m[3][0],
                 _m[0][1], _m[1][1], _m[2][1], _m[3][1],
                 _m[0][2], _m[1][2], _m[2][2], _m[3][2],
                 _m[0][3], _m[1][3], _m[2][3], _m[3][3]);
}

Matrix4 operator+(float32 lhs, const Matrix4 &rhs)
{
  Matrix4 mat;
  for (uint32 row = 0; row < 4; ++row)
  {
    for (uint32 col = 0; col < 4; ++col)
    {
      mat[row][col] = lhs + rhs[row][col];
    }
  }
  return mat;
}

Matrix4 operator-(float32 lhs, const Matrix4 &rhs)
{
  Matrix4 mat;
  for (uint32 row = 0; row < 4; ++row)
  {
    for (uint32 col = 0; col < 4; ++col)
    {
      mat[row][col] = lhs - rhs[row][col];
    }
  }
  return mat;
}

Matrix4 operator*(float32 lhs, const Matrix4 &rhs)
{
  Matrix4 mat;
  for (uint32 row = 0; row < 4; ++row)
  {
    for (uint32 col = 0; col < 4; ++col)
    {
      mat[row][col] = lhs * rhs[row][col];
    }
  }
  return mat;
}