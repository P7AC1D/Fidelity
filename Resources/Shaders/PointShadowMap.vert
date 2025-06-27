#version 410

layout(location = 0) in vec3 aPosition;

layout(std140) uniform PerObjectBuffer
{
  mat4 Model;
  mat4 ModelView;
  mat4 ModelViewProjection;
  vec4 DiffuseColour;
  bool DiffuseEnabled;
  bool NormalEnabled;
  bool OcclusionEnabled;
  bool OpacityEnabled;
} Object;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  gl_Position = Object.Model * vec4(aPosition, 1.0f);
}