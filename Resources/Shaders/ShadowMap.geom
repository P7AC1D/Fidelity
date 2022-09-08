#version 410
    
layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;
    
layout (std140) uniform TransformsBuffer
{
    mat4 lightSpaceMatrices[16];
} Transforms;
    
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