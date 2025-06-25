# ⚙️ Fidelity Engine PBR Pipeline Analysis

**Document Version**: 1.0  
**Date**: June 24, 2025  
**Author**: Technical Analysis  
**Status**: Draft - Review Recommended

---

## 📋 Overview

This document analyzes the Fidelity Engine's **Physically Based Rendering (PBR)** pipeline, detailing its deferred shading approach, BRDF implementation, resource bindings, and performance considerations.

### Key Features

- **Deferred PBR Workflow**: Separation of G-buffer generation and lighting passes
- **Cook-Torrance BRDF**: Energy-conserving specular and diffuse terms
- **Multiple Light Types**: Directional and point lights with attenuation
- **Ambient & HDR Support**: Screen-space AO, tone mapping, and bloom integration

---

## 🏗️ Architecture & Data Flow

### Rendering Stages
```
Geometry Pass (G-buffer) → SSAO & Shadows → Lighting Pass (PBR) → Tone Mapping & Bloom → Composition
```  
- **Geometry Pass**: Writes albedo, normals, roughness/metalness, and depth
- **SSAO & Shadows**: Generates ambient occlusion and shadow maps
- **Lighting Pass**: Reconstructs world position and applies BRDF for each light
- **Tone Mapping & Bloom**: Converts HDR result to LDR and extracts bloom highlights

---

## 🔧 G-Buffer Layout & Bindings

| Attach | Semantic        | Format   | Binding Slot |
|--------|-----------------|----------|--------------|
| 0      | Albedo          | RGB8     | Texture unit 0 |
| 1      | Depth           | R32F     | Texture unit 1 |
| 2      | Normal          | RGB16F   | Texture unit 2 |
| 3      | Material (R/M)  | RG16F    | Texture unit 3 |
| 4      | Occlusion       | R8       | Texture unit 5 |

Uniform buffer **PerFrameBuffer** (std140) provides camera matrices, view position, directional light, exposure, and ambient parameters.

---

## 🔍 Core Lighting Algorithm

### Position Reconstruction
```glsl
vec3 reconstructPosition(vec2 uv) {
    float z = texture(DepthMap, uv).r;
    vec4 clip = vec4(uv * 2.0 - 1.0, z * 2.0 - 1.0, 1.0);
    vec4 world = Constants.ProjViewInv * clip;
    return world.xyz / world.w;
}
```

### Cook-Torrance BRDF Components

1. **Normal Distribution Function (GGX)**
2. **Geometry Function (Smith)**
3. **Fresnel (Schlick Approximation)**
4. **Specular and Diffuse Weighting**

---

### Lighting Evaluation
```glsl
vec3 evaluateLighting(...) {
    vec3 radiance = vec3(0.0);
    // Directional Light
    radiance += calcDirLight(...);
    // Point Lights
    for (int i = 0; i < Constants.LightCount; ++i) {
        radiance += calcPointLight(Constants.Lights[i], ...);
    }
    // Ambient + SSAO
    radiance += albedo * Constants.AmbientColour * Constants.AmbientIntensity * occlusion;
    return radiance;
}
```

---

## ⚙️ Performance & Quality Considerations

- **Branch Minimization**: Loop unrolling for light count limits
- **Precision**: Use `highp` for world-space operations
- **Batching**: Group draw calls by material to reduce state changes
- **Tuning**: Expose roughness/metalness sliders in ImGui for artists

---

## ✅ Checklist & Validation

- [ ] G-buffer format matches shader declarations
- [ ] Uniform buffer alignment (std140) verified
- [ ] Bloom & tone mapping thresholds correctly applied
- [ ] Light attenuation curves artist-approved
- [ ] RenderDoc captures show expected outputs

---

*End of PBR Pipeline Analysis*
