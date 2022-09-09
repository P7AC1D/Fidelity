#version 410

layout(std140) uniform PointLightBuffer
{
  mat4 Model;
  mat4 ModelView;
  mat4 ModelViewProjection;
  vec4 Colour;
  vec3 Position;
  float Radius;
  vec3 Direction;
  int LightType; // 0 - Point; 1 - Directional
} PointLight;

layout(location = 0) in vec3 aPosition;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  gl_Position = PointLight.ModelViewProjection * vec4(aPosition, 1.0f);
}