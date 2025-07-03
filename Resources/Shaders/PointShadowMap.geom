#version 410

// specify input primitive type
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(std140) uniform PointLightBuffer {
  vec3 Position;
  float FarPlane;
  mat4 shadowMatrices[6];
  int lightIndex;
  float _padding[3]; // Alignment padding
} PLB;

in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[];

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

out vec4 FragPos;

void main()
{
  for (int face = 0; face < 6; ++face)
  {
    // Calculate layer for cube array: lightIndex * 6 + face
    gl_Layer = PLB.lightIndex * 6 + face;
    
    for (int i = 0; i < 3; ++i)
    {
      FragPos = gl_in[i].gl_Position;
      gl_Position = PLB.shadowMatrices[face] * FragPos;
      EmitVertex();
    }
    EndPrimitive();
  }
}