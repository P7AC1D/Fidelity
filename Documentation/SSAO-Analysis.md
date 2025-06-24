# 🔍 Fidelity Engine SSAO System Analysis

**Document Version**: 1.0  
**Date**: June 24, 2025  
**Author**: Technical Analysis  
**Status**: Complete - Post Optimization

---

## 📋 Overview

This document provides an in-depth analysis of the Fidelity Engine's **Screen-Space Ambient Occlusion (SSAO)** implementation within a deferred rendering pipeline. We cover the generation of sample kernels, noise texture creation, dual-pass SSAO + blur strategy, and integration into the PBR lighting stage.

### Key Features

- **Hemispherical Sample Kernel**: Dynamic kernel of up to 512 samples, scaled for artistic control  
- **Noise Texture Tiling**: 4×4 rotation map to reduce patterning artifacts  
- **Two-Pass Approach**: Raw SSAO pass followed by blur pass for smooth occlusion  
- **Configurable Parameters**: Radius, bias, strength, sample count exposed via UI  
- **Deferred Integration**: AO modulates ambient/indirect lighting in the lighting shader

---

## 🏗️ Architecture & Data Flow

```
G-Buffer (Depth + Normal) → SSAO Shader → Raw AO Map → Blur Shader → Blurred AO Map → Lighting Shader
```  

| Stage            | Input Textures        | Output                  |
|------------------|-----------------------|-------------------------|
| SSAO Pass        | DepthMap, NormalMap   | Raw SSAO (R8)           |
| Blur Pass        | Raw SSAO (R8)         | Blurred SSAO (R8)       |
| Lighting Pass    | G-Buffer, Shadows, SSAO Blur | Final Lit Frame        |

---

## 🔧 Technical Implementation Details

### Constants & Sampling Patterns

```cpp
const uint32 SSAO_NOISE_TEXTURE_SIZE = 4;    // 4×4 rotation map
const uint32 SSAO_MAX_KERNEL_SIZE = 512;     // Allocate space for max samples
uint32 KernelSize;                           // Actual sample count in use
float  Radius;                               // Sampling radius
float  Bias;                                 // Depth bias to avoid self-occlusion
float  Strength;                             // AO intensity multiplier
Vector4 NoiseSamples[SSAO_MAX_KERNEL_SIZE];  // Sample vectors
```  

**Purpose**:  
- **KernelSize**: Controls the number of sample vectors used  
- **Radius/Bias**: Tune occlusion scale and artifact avoidance  
- **NoiseSamples**: 3D vectors for sample directions in view-space

### Uniform Buffer Structure

| Field         | Description                            |
|---------------|----------------------------------------|
| `KernelSize`  | Number of active sample vectors       |
| `Radius`      | Maximum reach of occlusion sampling   |
| `Bias`        | Small offset to prevent self-shadow   |
| `Strength`    | Occlusion strength multiplier         |
| `NoiseSamples`| Array of sample direction vectors     |

### Texture Inputs

| Texture      | Format  | Usage                                 |
|--------------|---------|---------------------------------------|
| `DepthMap`   | R32F    | Screen-space depth from G-buffer      |
| `NormalMap`  | RGB16F  | World-space normals from G-buffer     |
| `NoiseMap`   | RGB32F  | Tiled rotation vectors for sample randomization |

---

## ⚙️ Core Algorithm Breakdown

### Step 1: Kernel & Noise Generation

#### Kernel Generation (CPU)
```cpp
std::vector<Vector3> kernel;
kernel.reserve(KernelSize);
for (uint32 i = 0; i < KernelSize; ++i) {
  vec3 sample = randomHemisphereSample();
  float scale = lerp(0.1f, 1.0f, (float(i)/KernelSize)*(float(i)/KernelSize));
  kernel[i] = normalize(sample) * scale;
}
```
- Samples are distributed more densely near the origin for stable occlusion.

#### Noise Texture (CPU)
```cpp
std::vector<Vector3> noise;
noise.resize(SSAO_NOISE_TEXTURE_SIZE*SSAO_NOISE_TEXTURE_SIZE);
for (auto &n : noise) {
  n = vec3(rand()*2-1, rand()*2-1, 0);
}
_noiseTexture.write(noise);
```
- Tiled over the screen to rotate the sample kernel per-pixel.

### Step 2: SSAO Pass (Raw Occlusion)
```glsl
// In Ssao.frag
vec3 posVS = reconstructPosition(gl_FragCoord);
vec3 normalVS = normalize(texture(NormalMap, uv).xyz*2-1);
vec2 noiseUV = uv * windowSize / 4;
vec3 tangent = normalize(normalVS);
vec3 bitangent = normalize(cross(normalVS, tangent));
mat3 TBN = mat3(tangent, bitangent, normalVS);

float occlusion = 0.0;
for (int i=0; i<KernelSize; ++i) {
  vec3 sampleVS = TBN * kernel[i];
  vec3 samplePoint = posVS + sampleVS * Radius;
  float sampleDepth = projectDepth(samplePoint);
  occlusion += (sampleDepth >= samplePoint.z + Bias ? 1.0 : 0.0);
}
occlusion = 1.0 - (occlusion / KernelSize);
FragColor = occlusion;
```
- Projects each sample back into depth buffer to test for occluders.

### Step 3: Blur Pass
```glsl
// In SsaoBlur.frag
float result = 0.0;
vec2 texel = 1.0 / windowSize;
for(int x=-2; x<=2; ++x) {
  for(int y=-2; y<=2; ++y) {
    result += texture(SSAOMap, uv + vec2(x,y)*texel).r;
  }
}
FragColor = result / 25.0;
```
- 5×5 box blur for smooth occlusion with minimal cost.

### Step 4: Lighting Integration
```glsl
// In PbrLighting.frag
float ao = texture(OcculsionMap, uv).r;
if (!Constants.SsaoEnabled) ao = 1.0;

// Direct lighting unaffected
color += calcDirectLighting(...);
// Ambient (indirect) modulated by AO
color += ambientColor * ao;
```
- Only the ambient component is multiplied by the SSAO term to avoid darkening direct lights.

---

## ✅ Conclusion

Your SSAO implementation—dual-pass generation, noise tiling, and ambient-only blending—is aligned with modern deferred PBR pipelines. For further enhancements, consider:  
- Bilinear upsampling for half-resolution SSAO  
- Temporal filtering or denoising for stability  
- Bent normals for improved IBL occlusion  

By following these practices, you ensure high-quality ambient occlusion without compromising direct lighting fidelity.
