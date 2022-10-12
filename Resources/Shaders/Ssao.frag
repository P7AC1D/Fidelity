#version 410
const uint MaxKernelSize = 512;

layout(std140) uniform SsaoConstantsBuffer
{
  mat4 View;
  mat4 Projection;
  mat4 ProjectionInv;
  vec4 NoiseSamples[MaxKernelSize];
  uint KernelSize;
  float Radius;
  float Bias;
} SsaoConstants;

layout(location = 0) in vec2 TexCoord;

out float Occulsion;

uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2D NoiseMap;
uniform sampler2D PositionMap;

vec3 calculatePositionVS(vec2 screenCoords)
{
  vec2 windowDimensions = textureSize(NormalMap, 0);
  vec3 fragPos = vec3((gl_FragCoord.x / windowDimensions.x), (gl_FragCoord.y / windowDimensions.y), 0.0f);
  fragPos.z = texture(DepthMap, fragPos.xy).r;
  vec4 fragPosProjected = SsaoConstants.ProjectionInv * vec4(fragPos * 2.0f - 1.0f, 1.0f);
  fragPosProjected.xyz / fragPosProjected.w;
  return fragPosProjected.xyz;
}

void main()
{
  // Calculate fragment position in view-space from depth and fragment sceen-space position.
  vec3 positionVS = calculatePositionVS(gl_FragCoord.xy);

  // transform normal vector to range [-1,1]
  vec3 normalWS = normalize(texture(NormalMap, TexCoord).xyz * 2.0f - 1.0f);
  vec3 normalVS = (SsaoConstants.View * vec4(normalWS, 0.0f)).xyz;

  // tile noise texture over screen
  ivec2 screenSize = textureSize(DepthMap, 0);
  vec2 noiseScale = vec2(screenSize.x / 4.0, screenSize.y / 4.0); 
  vec3 randomVec = normalize(texture(NoiseMap, TexCoord * noiseScale).xyz);

  // Calc tangent -> view space transform
  vec3 tangent = normalize(randomVec - normalVS * dot(randomVec, normalVS));
  vec3 bitangent = cross(normalVS, tangent);
  mat3 transform = mat3(tangent, bitangent, normalVS);

  float occlusion = 0.0;
  for (uint i = 0; i < SsaoConstants.KernelSize; ++i)
  {
    vec3 samplePositionVs = transform * SsaoConstants.NoiseSamples[i].xyz;
    samplePositionVs = positionVS + samplePositionVs * SsaoConstants.Radius;

    // Transform view-space position to clip-space
    vec4 offset = vec4(samplePositionVs, 1.0);
    offset = SsaoConstants.Projection * offset;
    offset.xyz /= offset.w;
    offset.xyz  = offset.xyz * 0.5f + 0.5f;

    float sampleDepth = calculatePositionVS(offset.xy).z;

    float rangeCheck = smoothstep(0.0, 1.0, SsaoConstants.Radius / abs(positionVS.z - sampleDepth));
    occlusion += (sampleDepth >= samplePositionVs.z + SsaoConstants.Bias ? 1.0f : 0.0f) * rangeCheck;
  }
  occlusion = 1.0 - (occlusion / SsaoConstants.KernelSize);

  Occulsion = occlusion;
}
