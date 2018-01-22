#include "Matrix3.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

#include "Quaternion.hpp"
#include "Vector3.hpp"

Matrix3 Matrix3::Zero = Matrix3(0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0);

Matrix3 Matrix3::Identity = Matrix3(1.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f);

Matrix3::Matrix3()
{
}

Matrix3::Matrix3(const Matrix3& mat)
{
  std::memcpy(_m, mat._m, 9 * sizeof(float32));
}

Matrix3::Matrix3(float32 a, float32 b, float32 c,
                 float32 d, float32 e, float32 f,
                 float32 g, float32 h, float32 i)
{
  _m[0][0] = a; _m[0][1] = b; _m[0][2] = c;
  _m[1][0] = d; _m[1][1] = e; _m[1][2] = f;
  _m[2][0] = g; _m[2][1] = h; _m[2][2] = i;
}

Matrix3::Matrix3(const Quaternion& qat)
{
  float tx  = qat[0]+qat[0];
  float ty  = qat[1]+qat[1];
  float fTz  = qat[2]+qat[2];
  float twx = tx*qat[3];
  float twy = ty*qat[3];
  float twz = fTz*qat[3];
  float txx = tx*qat[0];
  float txy = ty*qat[0];
  float txz = fTz*qat[0];
  float tyy = ty*qat[1];
  float tyz = fTz*qat[1];
  float tzz = fTz*qat[2];
  
  _m[0][0] = 1.0f-(tyy+tzz);
  _m[0][1] = txy-twz;
  _m[0][2] = txz+twy;
  _m[1][0] = txy+twz;
  _m[1][1] = 1.0f-(txx+tzz);
  _m[1][2] = tyz-twx;
  _m[2][0] = txz-twy;
  _m[2][1] = tyz+twx;
  _m[2][2] = 1.0f-(txx+tyy);
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

Matrix3 Matrix3::operator+(const Matrix3& rhs) const
{
  return Matrix3(_m[0][0] + rhs._m[0][0], _m[0][1] + rhs._m[0][1], _m[0][2] + rhs._m[0][2],
                 _m[1][0] + rhs._m[1][0], _m[1][1] + rhs._m[1][1], _m[1][2] + rhs._m[1][2],
                 _m[2][0] + rhs._m[2][0], _m[2][1] + rhs._m[2][1], _m[2][2] + rhs._m[2][2]);
}

Matrix3 Matrix3::operator-(const Matrix3& rhs) const
{
  return Matrix3(_m[0][0] - rhs._m[0][0], _m[0][1] - rhs._m[0][1], _m[0][2] - rhs._m[0][2],
                 _m[1][0] - rhs._m[1][0], _m[1][1] - rhs._m[1][1], _m[1][2] - rhs._m[1][2],
                 _m[2][0] - rhs._m[2][0], _m[2][1] - rhs._m[2][1], _m[2][2] - rhs._m[2][2]);
}

Matrix3 Matrix3::operator*(const Matrix3& rhs) const
{
  //Matrix3 result;
  //for (uint32 row = 0; row < 3; ++row)
  //{
  //  for (uint32 col = 0; col < 3; ++col)
  //  {
  //    result._m[row][col] = _m[row][0] * rhs._m[0][col] + _m[row][1] * rhs._m[1][col] + _m[row][2] * rhs._m[2][col];
  //  }
  //}
  //return result;

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

Vector3 Matrix3::operator*(const Vector3& rhs) const
{
  return Vector3(rhs[0] * _m[0][0] + rhs[1] * _m[0][1] + rhs[2] * _m[0][2],
                 rhs[0] * _m[1][0] + rhs[1] * _m[1][1] + rhs[2] * _m[1][2],
                 rhs[0] * _m[2][0] + rhs[1] * _m[2][1] + rhs[2] * _m[2][2]);
}

Matrix3& Matrix3::operator+=(float32 rhs)
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

Matrix3& Matrix3::operator-=(float32 rhs)
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

Matrix3& Matrix3::operator*=(float32 rhs)
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

bool Matrix3::operator==(const Matrix3& rhs) const
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

bool Matrix3::operator!=(const Matrix3& rhs) const
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

Vector3& Matrix3::operator[](uint32 row)
{
  assert(row < 3);
  return *(Vector3*)_m[row];
}

const Vector3& Matrix3::operator[](uint32 row) const
{
  assert(row < 3);
  return *(Vector3*)_m[row];
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

Matrix3 operator+(float32 lhs, const Matrix3& rhs)
{
  return Matrix3(lhs + rhs._m[0][0], lhs + rhs._m[0][1], lhs + rhs._m[0][2],
                 lhs + rhs._m[1][0], lhs + rhs._m[1][1], lhs + rhs._m[1][2],
                 lhs + rhs._m[2][0], lhs + rhs._m[2][1], lhs + rhs._m[2][2]);
}

Matrix3 operator-(float32 lhs, const Matrix3& rhs)
{
  return Matrix3(lhs - rhs._m[0][0], lhs - rhs._m[0][1], lhs - rhs._m[0][2],
                 lhs - rhs._m[1][0], lhs - rhs._m[1][1], lhs - rhs._m[1][2],
                 lhs - rhs._m[2][0], lhs - rhs._m[2][1], lhs - rhs._m[2][2]);
}

Matrix3 operator*(float32 lhs, const Matrix3& rhs)
{
  return Matrix3(lhs * rhs._m[0][0], lhs * rhs._m[0][1], lhs * rhs._m[0][2],
                 lhs * rhs._m[1][0], lhs * rhs._m[1][1], lhs * rhs._m[1][2],
                 lhs * rhs._m[2][0], lhs * rhs._m[2][1], lhs * rhs._m[2][2]);
}
