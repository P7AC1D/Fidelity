#version 410
    
layout(triangles, invocations = 4) in;
layout(triangle_strip, max_vertices = 3) out;
    
layout (std140) uniform TransformsBuffer
{
    mat4 lightSpaceMatrices[4];
} Transforms;

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
    
void main()
{          
  for (int i = 0; i < 3; ++i)
  {
    gl_Position = Transforms.lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
    gl_Layer = gl_InvocationID;
    EmitVertex();
  }
  EndPrimitive();
}  