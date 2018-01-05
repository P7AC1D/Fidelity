#pragma once
#include "../Core/Types.hpp"

class Vector2;
class Vector3;

class Vector4
{
public:
  static Vector4 One;
  
  static float32 Dot(const Vector4& lhs, const Vector4& rhs);
  static Vector4 Normalize(const Vector4& vec);
  
  Vector4();
  Vector4(float32 k);
  Vector4(float32 a, float32 b, float32 c, float32 d);
  Vector4(const Vector2& vec, float32 c = 0.0f, float32 d = 0.0f);
  Vector4(const Vector3& vec, float32 d = 0.0f);
  Vector4(const Vector4& vec);
  
  Vector4& operator=(const Vector4& rhs);
  Vector4 operator+(const Vector4& rhs) const;
  Vector4 operator-(const Vector4& rhs) const;
  Vector4 operator*(const Vector4& rhs) const;
  
  Vector4 operator+(float32 rhs) const;
  Vector4 operator-(float32 rhs) const;
  Vector4 operator*(float32 rhs) const;
  
  Vector4 operator+=(const Vector4& rhs);
  Vector4 operator-=(const Vector4& rhs);
  
  Vector4& operator+=(float32 rhs);
  Vector4& operator-=(float32 rhs);
  Vector4& operator*=(float32 rhs);
  
  bool operator==(const Vector4& rhs) const;
  bool operator!=(const Vector4& rhs) const;
  
  float32& operator[](uint32 i);
  const float32& operator[](uint32 i) const;
  
  float32 Length() const;
  void Normalize();

  const float32* Ptr() const;
  
  friend Vector4 operator+(float32 lhs, const Vector4& rhs);
  friend Vector4 operator-(float32 lhs, const Vector4& rhs);
  friend Vector4 operator*(float32 lhs, const Vector4& rhs);
  
private:
  float32 _x;
  float32 _y;
  float32 _z;
  float32 _w;
};
