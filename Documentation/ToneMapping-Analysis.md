# 📝 Tone Mapping Analysis

## Overview
This document describes the Fidelity Engine's tone mapping stage, including the newly integrated ACES tone mapping operator and sRGB gamma correction. Tone mapping converts high dynamic range (HDR) values from the lighting and bloom passes into a low dynamic range (LDR) color for display.

## Pipeline Context
1. **Lighting Pass**: Produces HDR radiance with exposure applied in `PbrLighting.frag`.
2. **Bloom Pass**: Extracts and filters bright highlights into a bloom texture.
3. **Tone Mapping Pass**: Combines HDR and bloom, applies ACES tone mapping, then gamma corrects to sRGB.
4. **Final Composition**: Output written to the screen render target.

## Implementation Details
File: `Resources/Shaders/ToneMapping.frag`

### ACES Tone Mapping
```glsl
vec3 ACESFilm(vec3 x) {
    // ACES approximation constants
    const float a=2.51, b=0.03, c=2.43, d=0.59, e=0.14;
    return clamp((x*(a*x + b))/(x*(c*x + d) + e), 0.0, 1.0);
}
```
- **Purpose**: Preserves highlight detail and contrast more faithfully than simple Reinhard.
- **Quality**: Industry-standard (Unreal, ACES workflow).

### sRGB Gamma Correction
```glsl
vec3 GammaCorrect(vec3 color, float gamma) {
    return pow(color, vec3(1.0 / gamma));
}
```
- **Purpose**: Converts linear output from tone mapping into sRGB space for correct display on monitors.
- **Gamma**: Default `2.2`.

### Main Shader Flow
```glsl
// Combine HDR lighting + bloom
hdrSample = mix(hdrSample, bloomSample, Constants.BloomStrength);

// Tone mapping or clamp
mapped = Constants.ToneMappingEnabled
    ? ACESFilm(hdrSample)
    : min(hdrSample, vec3(1.0));

// Apply gamma correction
finalColor = GammaCorrect(mapped, 2.2);
FinalColour = vec4(finalColor, colourSample.a);
```

## Parameters & Defaults
| Parameter               | Default    | Description                                      |
|-------------------------|------------|--------------------------------------------------|
| `ToneMappingEnabled`    | `true`     | Toggle tone mapping on/off                       |
| `BloomStrength`         | `0.15`     | Blend factor between HDR and bloom outputs       |
| `Exposure`             | `1.0`      | Camera exposure applied upstream in lighting     |
| `Gamma`                | `2.2`      | sRGB gamma value for display correction          |
| `ACES Constants`       | a=2.51, b=0.03, c=2.43, d=0.59, e=0.14 | ACES film curve constants |