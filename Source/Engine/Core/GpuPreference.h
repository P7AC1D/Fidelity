#pragma once

/**
 * GPU Preference Exports
 * 
 * This file contains exports that tell Windows and graphics drivers
 * that this application prefers high-performance dedicated graphics
 * over integrated graphics for better performance.
 */

#ifdef _WIN32

// NVIDIA Optimus: Request high-performance GPU
// This export tells NVIDIA drivers to prefer the dedicated GPU
// over integrated graphics for this application
extern "C" {
    extern __declspec(dllexport) unsigned long NvOptimusEnablement;
}

// AMD PowerXpress: Request high-performance GPU
// This export tells AMD drivers to prefer the dedicated GPU
// over integrated graphics for this application
extern "C" {
    extern __declspec(dllexport) int AmdPowerXpressRequestHighPerformance;
}

#endif // _WIN32

/**
 * Additional GPU preference utilities
 */
namespace GpuPreference {
    
    /**
     * Get the current GPU vendor string from OpenGL
     * Should be called after OpenGL context is created
     */
    const char* getGpuVendor();
    
    /**
     * Get the current GPU renderer string from OpenGL
     * Should be called after OpenGL context is created
     */
    const char* getGpuRenderer();
    
    /**
     * Check if we're running on a dedicated GPU
     * Returns true if using NVIDIA, AMD, or other dedicated GPU
     * Returns false if using Intel integrated graphics
     */
    bool isUsingDedicatedGpu();
    
    /**
     * Log GPU information to console
     * Useful for debugging GPU selection issues
     */
    void logGpuInfo();
}
