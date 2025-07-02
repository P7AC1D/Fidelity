#version 410
in vec4 FragPos;

layout(std140) uniform PointLightBuffer {
  vec3 Position;
  float FarPlane;
  mat4 shadowMatrices[6];
} PLB;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - PLB.Position);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / PLB.FarPlane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}  