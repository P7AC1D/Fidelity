#version 410 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

layout (std140) uniform ObjectBuffer
{
  mat4 Model;
};

layout (std140) uniform CameraBuffer
{
  mat4 Projection;
  mat4 View;
  vec3 ViewDir;
};

out VsOut
{
  mat4 TbnMtx;
  vec4 Position;
  vec4 Normal;
  vec2 TexCoord;
  vec3 PositionTS;
  vec3 ViewDirTS;
} vsOut;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  vsOut.TexCoord = aTexCoord;
  gl_Position = Projection * View * vec4(aPosition, 1.0f);
}
