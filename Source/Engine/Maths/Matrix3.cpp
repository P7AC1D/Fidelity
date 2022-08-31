#include "Matrix3.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

#include "Matrix4.hpp"
#include "Quaternion.hpp"
#include "Vector3.hpp"

Matrix3 Matrix3::Zero = Matrix3(0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0);

Matrix3 Matrix3::Identity = Matrix3(1.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f);

Matrix3 Matrix3::LookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up)
{
  Vector3 camDir = Vector3::Normalize(target - eye);
  if (std::abs(Vector3::Dot(camDir, up)))
  {
    camDir += 0.00001f;
  }
  Vector3 camRight = Vector3::Normalize(Vector3::Cross(camDir, up));
  Vector3 camUp = Vector3::Normalize(Vector3::Cross(camRight, camDir));

  Matrix3 view(Matrix3::Identity);
  view[0][0] = camRight.X;
  view[1][0] = camRight.Y;
  view[2][0] = camRight.Z;
  view[0][1] = camUp.X;
  view[1][1] = camUp.Y;
  view[2][1] = camUp.Z;
  view[0][2] = -camDir.X;
  view[1][2] = -camDir.Y;
  view[2][2] = -camDir.Z;
  return view;
}

Matrix3::Matrix3()
{
}

Matrix3::Matrix3(const Matrix3 &mat)
{
  std::memcpy(_m, mat._m, 9 * sizeof(float32));
}

Matrix3::Matrix3(const Matrix4 &mat)
{
  _m[0][0] = mat[0][0];
  _m[0][1] = mat[0][1];
  _m[0][2] = mat[0][2];
  _m[1][0] = mat[1][0];
  _m[1][1] = mat[1][1];
  _m[1][2] = mat[1][2];
  _m[2][0] = mat[2][0];
  _m[2][1] = mat[2][1];
  _m[2][2] = mat[2][2];
}

Matrix3::Matrix3(float32 a, float32 b, float32 c,
                 float32 d, float32 e, float32 f,
                 float32 g, float32 h, float32 i)
{
  _m[0][0] = a;
  _m[0][1] = b;
  _m[0][2] = c;
  _m[1][0] = d;
  _m[1][1] = e;
  _m[1][2] = f;
  _m[2][0] = g;
  _m[2][1] = h;
  _m[2][2] = i;
}

Matrix3::Matrix3(const Quaternion &q)
{
  Matrix3 result(Matrix3::Identity);
  float32 qxx(q.X * q.X);
  float32 qyy(q.Y * q.Y);
  float32 qzz(q.Z * q.Z);
  float32 qxz(q.X * q.Z);
  float32 qxy(q.X * q.Y);
  float32 qyz(q.Y * q.Z);
  float32 qwx(q.W * q.X);
  float32 qwy(q.W * q.Y);
  float32 qwz(q.W * q.Z);

  result[0][0] = 1.0f - 2.0f * (qyy + qzz);
  result[0][1] = 2.0f * (qxy + qwz);
  result[0][2] = 2.0f * (qxz - qwy);

  result[1][0] = 2.0f * (qxy - qwz);
  result[1][1] = 1.0f - 2.0f * (qxx + qzz);
  result[1][2] = 2.0f * (qyz + qwx);

  result[2][0] = 2.0f * (qxz + qwy);
  result[2][1] = 2.0f * (qyz - qwx);
  result[2][2] = 1.0f - 2.0f * (qxx + qyy);
  *this = result;
}

Matrix3 Matrix3::operator+(float32 rhs) const
{
  return Matrix3(_m[0][0] + rhs, _m[0][1] + rhs, _m[0][2] + rhs,
                 _m[1][0] + rhs, _m[1][1] + rhs, _m[1][2] + rhs,
                 _m[2][0] + rhs, _m[2][1] + rhs, _m[2][2] + rhs);
}

Matrix3 Matrix3::operator-(float32 rhs) const
{
  return Matrix3(_m[0][0] - rhs, _m[0][1] - rhs, _m[0][2] - rhs,
                 _m[1][0] - rhs, _m[1][1] - rhs, _m[1][2] - rhs,
                 _m[2][0] - rhs, _m[2][1] - rhs, _m[2][2] - rhs);
}

Matrix3 Matrix3::operator*(float32 rhs) const
{
  return Matrix3(_m[0][0] * rhs, _m[0][1] * rhs, _m[0][2] * rhs,
                 _m[1][0] * rhs, _m[1][1] * rhs, _m[1][2] * rhs,
                 _m[2][0] * rhs, _m[2][1] * rhs, _m[2][2] * rhs);
}

Matrix3 Matrix3::operator+(const Matrix3 &rhs) const
{
  return Matrix3(_m[0][0] + rhs._m[0][0], _m[0][1] + rhs._m[0][1], _m[0][2] + rhs._m[0][2],
                 _m[1][0] + rhs._m[1][0], _m[1][1] + rhs._m[1][1], _m[1][2] + rhs._m[1][2],
                 _m[2][0] + rhs._m[2][0], _m[2][1] + rhs._m[2][1], _m[2][2] + rhs._m[2][2]);
}

Matrix3 Matrix3::operator-(const Matrix3 &rhs) const
{
  return Matrix3(_m[0][0] - rhs._m[0][0], _m[0][1] - rhs._m[0][1], _m[0][2] - rhs._m[0][2],
                 _m[1][0] - rhs._m[1][0], _m[1][1] - rhs._m[1][1], _m[1][2] - rhs._m[1][2],
                 _m[2][0] - rhs._m[2][0], _m[2][1] - rhs._m[2][1], _m[2][2] - rhs._m[2][2]);
}

Matrix3 Matrix3::operator*(const Matrix3 &rhs) const
{
  // Matrix3 result;
  // for (uint32 row = 0; row < 3; ++row)
  //{
  //   for (uint32 col = 0; col < 3; ++col)
  //   {
  //     result._m[row][col] = _m[row][0] * rhs._m[0][col] + _m[row][1] * rhs._m[1][col] + _m[row][2] * rhs._m[2][col];
  //   }
  // }
  // return result;

  Matrix3 result;
  result[0][0] = _m[0][0] * rhs._m[0][0] + _m[1][0] * rhs._m[0][1] + _m[2][0] * rhs._m[0][2];
  result[0][1] = _m[0][1] * rhs._m[0][0] + _m[1][1] * rhs._m[0][1] + _m[2][1] * rhs._m[0][2];
  result[0][2] = _m[0][2] * rhs._m[0][0] + _m[1][2] * rhs._m[0][1] + _m[2][2] * rhs._m[0][2];
  result[1][0] = _m[0][0] * rhs._m[1][0] + _m[1][0] * rhs._m[1][1] + _m[2][0] * rhs._m[1][2];
  result[1][1] = _m[0][1] * rhs._m[1][0] + _m[1][1] * rhs._m[1][1] + _m[2][1] * rhs._m[1][2];
  result[1][2] = _m[0][2] * rhs._m[1][0] + _m[1][2] * rhs._m[1][1] + _m[2][2] * rhs._m[1][2];
  result[2][0] = _m[0][0] * rhs._m[2][0] + _m[1][0] * rhs._m[2][1] + _m[2][0] * rhs._m[2][2];
  result[2][1] = _m[0][1] * rhs._m[2][0] + _m[1][1] * rhs._m[2][1] + _m[2][1] * rhs._m[2][2];
  result[2][2] = _m[0][2] * rhs._m[2][0] + _m[1][2] * rhs._m[2][1] + _m[2][2] * rhs._m[2][2];
  return result;
}

Vector3 Matrix3::operator*(const Vector3 &rhs) const
{
  return Vector3(rhs[0] * _m[0][0] + rhs[1] * _m[0][1] + rhs[2] * _m[0][2],
                 rhs[0] * _m[1][0] + rhs[1] * _m[1][1] + rhs[2] * _m[1][2],
                 rhs[0] * _m[2][0] + rhs[1] * _m[2][1] + rhs[2] * _m[2][2]);
}

Matrix3 &Matrix3::operator+=(float32 rhs)
{
  _m[0][0] += rhs;
  _m[0][1] += rhs;
  _m[0][2] += rhs;
  _m[1][0] += rhs;
  _m[1][1] += rhs;
  _m[1][2] += rhs;
  _m[2][0] += rhs;
  _m[2][1] += rhs;
  _m[2][2] += rhs;
  return *this;
}

Matrix3 &Matrix3::operator-=(float32 rhs)
{
  _m[0][0] -= rhs;
  _m[0][1] -= rhs;
  _m[0][2] -= rhs;
  _m[1][0] -= rhs;
  _m[1][1] -= rhs;
  _m[1][2] -= rhs;
  _m[2][0] -= rhs;
  _m[2][1] -= rhs;
  _m[2][2] -= rhs;
  return *this;
}

Matrix3 &Matrix3::operator*=(float32 rhs)
{
  _m[0][0] *= rhs;
  _m[0][1] *= rhs;
  _m[0][2] *= rhs;
  _m[1][0] *= rhs;
  _m[1][1] *= rhs;
  _m[1][2] *= rhs;
  _m[2][0] *= rhs;
  _m[2][1] *= rhs;
  _m[2][2] *= rhs;
  return *this;
}

bool Matrix3::operator==(const Matrix3 &rhs) const
{
  for (uint32 row = 0; row < 3; ++row)
  {
    for (uint32 col = 0; col < 3; ++col)
    {
      if (_m[row][col] != rhs._m[row][col])
      {
        return false;
      }
    }
  }
  return true;
}

bool Matrix3::operator!=(const Matrix3 &rhs) const
{
  uint32 equalityCount = 0;
  for (uint32 row = 0; row < 3; ++row)
  {
    for (uint32 col = 0; col < 3; ++col)
    {
      if (_m[row][col] == rhs._m[row][col])
      {
        ++equalityCount;
      }
    }
  }
  return equalityCount < 9;
}

Vector3 &Matrix3::operator[](uint32 row)
{
  assert(row < 3);
  return *(Vector3 *)_m[row];
}

const Vector3 &Matrix3::operator[](uint32 row) const
{
  assert(row < 3);
  return *(Vector3 *)_m[row];
}

float32 Matrix3::Determinate() const
{
  float32 cofactor00 = _m[1][1] * _m[2][2] - _m[1][2] * _m[2][1];
  float32 cofactor10 = _m[1][2] * _m[2][0] - _m[1][0] * _m[2][2];
  float32 cofactor20 = _m[1][0] * _m[2][1] - _m[1][1] * _m[2][0];

  return _m[0][0] * cofactor00 + _m[0][1] * cofactor10 + _m[0][2] * cofactor20;
}

Matrix3 Matrix3::Inverse() const
{
  Matrix3 invMat;
  invMat._m[0][0] = _m[1][1] * _m[2][2] - _m[1][2] * _m[2][1];
  invMat._m[0][1] = _m[0][2] * _m[2][1] - _m[0][1] * _m[2][2];
  invMat._m[0][2] = _m[0][1] * _m[1][2] - _m[0][2] * _m[1][1];
  invMat._m[1][0] = _m[1][2] * _m[2][0] - _m[1][0] * _m[2][2];
  invMat._m[1][1] = _m[0][0] * _m[2][2] - _m[0][2] * _m[2][0];
  invMat._m[1][2] = _m[0][2] * _m[1][0] - _m[0][0] * _m[1][2];
  invMat._m[2][0] = _m[1][0] * _m[2][1] - _m[1][1] * _m[2][0];
  invMat._m[2][1] = _m[0][1] * _m[2][0] - _m[0][0] * _m[2][1];
  invMat._m[2][2] = _m[0][0] * _m[1][1] - _m[0][1] * _m[1][0];

  float32 det = _m[0][0] * invMat._m[0][0] + _m[0][1] * invMat._m[1][0] + _m[0][2] * invMat._m[2][0];
  if (std::abs(det) <= 1e-06f)
  {
    return Matrix3::Zero;
  }

  float32 invDet = 1.0f / det;
  for (uint32 row = 0; row < 3; ++row)
  {
    for (uint32 col = 0; col < 3; ++col)
    {
      invMat._m[row][col] *= invDet;
    }
  }
  return invMat;
}

Matrix3 Matrix3::Transpose() const
{
  return Matrix3(_m[0][0], _m[1][0], _m[2][0],
                 _m[0][1], _m[1][1], _m[2][1],
                 _m[0][2], _m[1][2], _m[2][2]);
}

Matrix3 operator+(float32 lhs, const Matrix3 &rhs)
{
  return Matrix3(lhs + rhs._m[0][0], lhs + rhs._m[0][1], lhs + rhs._m[0][2],
                 lhs + rhs._m[1][0], lhs + rhs._m[1][1], lhs + rhs._m[1][2],
                 lhs + rhs._m[2][0], lhs + rhs._m[2][1], lhs + rhs._m[2][2]);
}

Matrix3 operator-(float32 lhs, const Matrix3 &rhs)
{
  return Matrix3(lhs - rhs._m[0][0], lhs - rhs._m[0][1], lhs - rhs._m[0][2],
                 lhs - rhs._m[1][0], lhs - rhs._m[1][1], lhs - rhs._m[1][2],
                 lhs - rhs._m[2][0], lhs - rhs._m[2][1], lhs - rhs._m[2][2]);
}

Matrix3 operator*(float32 lhs, const Matrix3 &rhs)
{
  return Matrix3(lhs * rhs._m[0][0], lhs * rhs._m[0][1], lhs * rhs._m[0][2],
                 lhs * rhs._m[1][0], lhs * rhs._m[1][1], lhs * rhs._m[1][2],
                 lhs * rhs._m[2][0], lhs * rhs._m[2][1], lhs * rhs._m[2][2]);
}
