#version 410

layout(location = 0) in vec3 aPosition;

layout(std140) uniform ObjectBuffer
{
  mat4 Model;
  mat4 ModelView;
  mat4 ModelViewProjection;
} Object;

layout (std140) uniform TransformsBuffer
{
    mat4 LightSpace;
} Transforms;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  gl_Position = Transforms.LightSpace * Object.Model * vec4(aPosition, 1.0f);
}