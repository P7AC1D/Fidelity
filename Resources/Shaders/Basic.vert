#version 410

layout(std140) uniform PerObjectBuffer
{
  mat4 Model;
  mat4 ModelView;
  mat4 ModelViewProjection;
  bool DiffuseEnabled;
  bool NormalEnabled;
  bool SpecularEnabled;
  bool OpacityEnabled;
  vec4 AmbientColour;
  vec4 DiffuseColour;
  vec4 SpecularColour;
  float SpecularExponent;
} Object;

layout(location = 0) in vec3 aPosition;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  gl_Position = Object.ModelViewProjection * vec4(aPosition, 1.0f);
}