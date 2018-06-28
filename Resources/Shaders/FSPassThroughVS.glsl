#version 410

layout(location = 0) in vec2 aPosition;
layout(location = 2) in vec2 aTexCoord;

out VsOut
{
  mat3 TbnMtx;
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
  gl_Position = vec4(aPosition, 0.0f, 1.0f);
}
