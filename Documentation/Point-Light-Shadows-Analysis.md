# 🌟 Fidelity Engine Point Light Shadow System Analysis

## 📋 Overview

This document provides an in-depth analysis of the Fidelity Engine's **Omnidirectional Point Light Shadow Mapping** implementation using OpenGL's **TextureCubeArray**. The system efficiently renders shadows for multiple point lights simultaneously, supporting up to 8 shadow-casting point lights with soft shadow filtering integrated into the deferred PBR pipeline.

### Key Features

- **TextureCubeArray Storage**: Efficient GPU memory layout for multiple point light shadow maps
- **Geometry Shader Optimization**: Single-pass rendering to all 6 cubemap faces per light
- **Multi-Light Support**: Up to 8 simultaneous shadow-casting point lights
- **4D Texture Sampling**: `samplerCubeArray` with `vec4(direction, lightIndex)` coordinates
- **Soft Shadow Filtering**: PCF sampling with configurable quality settings

---

## 🏗️ Architecture & Data Flow

```
Scene Geometry → Point Light Depth Pass → TextureCubeArray → Lighting Pass → Shadowed Point Lights
      ↓                    ↓                     ↓              ↓              ↓
- Static Meshes    - 6 View Matrices     - Cube Array     - PBR Lighting   - Soft Shadows
- Transforms       - Geometry Shader     - Depth Storage  - Shadow Sampling - Final Image
```

| Stage                | Input                    | Output                        |
|----------------------|--------------------------|-------------------------------|
| Point Depth Pass     | Scene Geometry           | TextureCubeArray (D32F)       |
| Lighting Pass        | G-Buffer, Point Shadows  | Shadowed Point Light Contrib  |

---

## 🔧 Technical Implementation Details

### Constants & Configuration

```cpp
const uint32 MAX_POINT_LIGHT_SHADOW_CASTERS = 8;    // Maximum shadow-casting lights
const uint32 POINT_LIGHT_SHADOW_RESOLUTION = 1024;  // Cubemap face resolution
```

**Purpose**:
- **MAX_POINT_LIGHT_SHADOW_CASTERS**: Limits GPU memory usage and performance cost
- **POINT_LIGHT_SHADOW_RESOLUTION**: Balances shadow quality vs memory/performance

### TextureCubeArray Structure

```cpp
TextureDesc shadowMapDesc;
shadowMapDesc.Width = 1024;                           // Shadow map resolution
shadowMapDesc.Height = 1024;                          // Square cubemap faces
shadowMapDesc.Usage = TextureUsage::Depth;            // Depth-only storage
shadowMapDesc.Type = TextureType::TextureCubeArray;   // Cubemap array type
shadowMapDesc.Format = TextureFormat::D32F;           // 32-bit float depth
shadowMapDesc.Count = MAX_POINT_LIGHT_SHADOW_CASTERS; // 8 cubemaps maximum
```

**Memory Layout**:
- **Total Layers**: `6 * MAX_POINT_LIGHT_SHADOW_CASTERS` (48 layers for 8 lights)
- **Layer Indexing**: Light 0 uses layers 0-5, Light 1 uses layers 6-11, etc.
- **Face Ordering**: +X, -X, +Y, -Y, +Z, -Z (OpenGL standard)

### Point Light Buffer Structure

| Field              | Description                           |
|--------------------|---------------------------------------|
| `Position`         | Light world position                  |
| `FarPlane`         | Light radius/range                    |
| `shadowMatrices[6]`| 6 view-projection matrices for faces  |
| `LightIndex`       | Index in global light array          |

### Shader Inputs

| Texture/Buffer       | Format        | Usage                                 |
|----------------------|---------------|---------------------------------------|
| `PointShadowMaps`    | TextureCubeArray D32F | Point light shadow depth storage |
| `PointLightBuffer`   | Uniform Buffer | Light position, matrices, index      |

---

## ⚙️ Core Algorithm Breakdown

### Step 1: Shadow Matrix Generation (CPU)

```cpp
// Standard cubemap face directions and up vectors
const std::array<Vector3,6> dirs = {{
    Vector3( 1, 0, 0), Vector3(-1, 0, 0),  // +X, -X
    Vector3( 0, 1, 0), Vector3( 0,-1, 0),  // +Y, -Y  
    Vector3( 0, 0, 1), Vector3( 0, 0,-1)   // +Z, -Z
}};

// Generate 90° FOV perspective projection
Matrix4 proj = Matrix4::Perspective(Degree(90.0f), 1.0f, nearPlane, farPlane);

// Create view matrices for each face
for (int i = 0; i < 6; ++i) {
    Matrix4 view = Matrix4::LookAt(lightPos, lightPos + dirs[i], ups[i]);
    shadowMatrices[i] = proj * view;
}
```

### Step 2: Geometry Shader (Point Shadow Depth Pass)

```glsl
// PointShadowMap.geom
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 shadowMatrices[6];
uniform int lightIndex;

out vec4 FragPos;

void main() {
    for(int face = 0; face < 6; ++face) {
        gl_Layer = lightIndex * 6 + face;  // Target specific cubemap face
        
        for(int i = 0; i < 3; ++i) {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}
```

**Innovation**: Single geometry pass renders to all 6 cubemap faces simultaneously using `gl_Layer` targeting.

### Step 3: Fragment Shader (Depth Writing)

```glsl
// PointShadowMap.frag
in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main() {
    // Calculate distance from light to fragment
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // Normalize to [0,1] range for depth storage
    lightDistance = lightDistance / farPlane;
    
    // Write normalized depth
    gl_FragDepth = lightDistance;
}
```

**Process**: Converts world-space distance to normalized depth value for storage.

### Step 4: Shadow Sampling (Lighting Pass)

```glsl
// PbrLighting.frag
uniform samplerCubeArray PointShadowMaps;

float getPointShadowPcf(Light light, int lightIndex, vec3 fragPos, float bias) {
    vec3 fragToLight = fragPos - light.Position;
    float currentDepth = length(fragToLight);
    
    float shadow = 0.0f;
    float diskRadius = 0.05f;  // PCF sampling radius
    
    // Multi-sample PCF for soft shadows
    for (int i = 0; i < Constants.ShadowSampleCount; i++) {
        vec3 sampleOffset = sampleOffsetDirections[i] * diskRadius;
        
        // 4D texture coordinate: (direction, layer)
        vec4 sampleCoord = vec4(fragToLight + sampleOffset, float(lightIndex));
        float closestDepth = texture(PointShadowMaps, sampleCoord).r;
        
        // Convert back to world space distance
        closestDepth *= light.Radius;
        
        if (currentDepth - bias >= closestDepth) {
            shadow += 1.0f;
        }
    }
    
    return shadow / float(Constants.ShadowSampleCount);
}
```

**Key Features**:
- **4D Sampling**: `vec4(direction, lightIndex)` for cubemap array access
- **Distance-based Depth**: Uses actual world distance rather than projected depth
- **PCF Filtering**: Multiple samples for soft shadow edges

### Step 5: Multi-Light Integration

```glsl
// Point light contributions with shadows
for (int i = 0; i < Constants.LightCount; i++) {
    Light currentLight = Constants.Lights[i];
    
    // Calculate lighting contribution
    vec3 lightContrib = calcPointLight(currentLight, normal, position, viewDir, 
                                      albedo, roughness, metalness, F0);
    
    // Apply shadows if within shadow-casting limit
    float shadowFactor = 1.0f;
    if (i < Constants.MaxPointLightShadowCasters) {
        vec3 toLight = position - currentLight.Position;
        vec3 dir = normalize(toLight);
        float nl = max(dot(normal, dir), 0.0);
        float bias = max(0.001, (1.0 - nl) * 0.01);
        
        shadowFactor = getPointShadowPcf(currentLight, i, position, bias);
    }
    
    // Accumulate shadowed lighting
    totalRadiance += lightContrib * (1.0 - shadowFactor);
}
```

---

## 🎨 Visual Impact & Quality

### Shadow Quality Features

1. **Omnidirectional Coverage**: Complete 360° shadow casting from point lights
2. **Soft Shadow Edges**: PCF sampling creates natural penumbra effects
3. **Distance-based Attenuation**: Shadows fade naturally with light range
4. **Multi-light Support**: Up to 8 lights can cast shadows simultaneously
5. **Consistent Quality**: Uniform shadow resolution across all light directions

### Performance Characteristics

| Aspect | Specification | Performance Impact |
|--------|---------------|-------------------|
| **Shadow Resolution** | 1024x1024 per face | 6MB per light (D32F) |
| **Maximum Lights** | 8 shadow casters | 48MB total VRAM |
| **Geometry Passes** | 1 pass per light | 8 geometry passes max |
| **Fragment Samples** | 16-32 PCF samples | Configurable quality |

### Artifact Prevention

| Artifact | Cause | Solution Implemented |
|----------|-------|---------------------|
| **Shadow acne** | Depth precision issues | Surface-normal-based bias |
| **Light bleeding** | Insufficient bias | Distance-based bias calculation |
| **Temporal aliasing** | Static sampling patterns | PCF with offset sampling |
| **Memory bandwidth** | Large texture access | Efficient cubemap array layout |

---

## � Performance Analysis

### GPU Memory Usage
```
Single Point Light Shadow:
- Cubemap: 1024² × 6 faces × 4 bytes = 24MB
- Total for 8 lights: 192MB

Optimized Configuration:
- Resolution: 1024² (good quality/performance balance)
- Format: D32F (sufficient precision)
```

### Quality vs Performance Presets
```cpp
struct ShadowQuality {
    uint32 resolution;
    uint32 sampleCount;
    uint32 maxLights;
};

const ShadowQuality LOW    = {512,  8,  4};  // 12MB VRAM
const ShadowQuality MEDIUM = {1024, 16, 6};  // 36MB VRAM  
const ShadowQuality HIGH   = {2048, 32, 8};  // 192MB VRAM
