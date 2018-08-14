#pragma once
#include <string>

#include "../Core/Types.hpp"
#include "../Maths/Colour.hpp"
#include "../Maths/Vector3.hpp"

enum LightType
{
  Directional,
  Point,
  Spot,
};

class Light
{
public:
  Light(LightType lightType, const std::string& name) :
    _name(name),
    _radius(1.0f),
    _intensity(1.0f),
    _colour(Colour::White),
    _position(Vector3::Zero),
    _direction(-Vector3::Normalize(Vector3::Identity)),
    _lightType(lightType)
  { }

  inline void SetColour(const Colour& colour) { _colour = colour; }
  inline void SetPosition(const Vector3& position) { _position = position; }
	inline void SetDirection(const Vector3& direction) { _direction = Vector3::Normalize(direction); }
  inline void SetRadius(float32 radius) { _radius = radius; }
	inline void SetIntensity(float32 intensity) { _intensity = intensity; }

  inline Colour GetColour() const { return _colour; }
  inline Vector3 GetPosition() const { return _position; }
  inline Vector3 GetDirection() const { return _direction; }
  inline float32 GetRadius() const { return _radius; }
	inline float32 GetIntensity() const { return _intensity;  }
  inline LightType GetType() const { return _lightType; }

private:
  std::string _name;
  float32 _radius;
	float32 _intensity;
  Colour _colour;
  Vector3 _position;
  Vector3 _direction;
  LightType _lightType;
};
