#include "GpuPreference.h"
#include <glad/glad.h>
#include <iostream>
#include <string>
#include <algorithm>

#ifdef _WIN32
// Define the GPU preference exports here to avoid multiple definitions
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace GpuPreference {

const char* getGpuVendor() {
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    return vendor ? vendor : "Unknown";
}

const char* getGpuRenderer() {
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    return renderer ? renderer : "Unknown";
}

bool isUsingDedicatedGpu() {
    const char* vendor = getGpuVendor();
    const char* renderer = getGpuRenderer();
    
    if (!vendor || !renderer) {
        return false;
    }
    
    // Convert to lowercase for case-insensitive comparison
    std::string vendorStr(vendor);
    std::string rendererStr(renderer);
    std::transform(vendorStr.begin(), vendorStr.end(), vendorStr.begin(), ::tolower);
    std::transform(rendererStr.begin(), rendererStr.end(), rendererStr.begin(), ::tolower);
    
    // Check for dedicated GPU vendors
    bool isNvidia = vendorStr.find("nvidia") != std::string::npos;
    bool isAmd = vendorStr.find("amd") != std::string::npos || 
                 vendorStr.find("ati") != std::string::npos ||
                 rendererStr.find("radeon") != std::string::npos;
    
    // Check for Intel integrated graphics (usually indicates not using dedicated GPU)
    bool isIntelIntegrated = vendorStr.find("intel") != std::string::npos &&
                            (rendererStr.find("hd graphics") != std::string::npos ||
                             rendererStr.find("iris") != std::string::npos ||
                             rendererStr.find("uhd graphics") != std::string::npos ||
                             rendererStr.find("integrated") != std::string::npos);
    
    // Return true if we detect a dedicated GPU vendor and it's not Intel integrated
    return (isNvidia || isAmd) && !isIntelIntegrated;
}

void logGpuInfo() {
    const char* vendor = getGpuVendor();
    const char* renderer = getGpuRenderer();
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    std::cout << "=== GPU Information ===" << std::endl;
    std::cout << "Vendor: " << vendor << std::endl;
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL Version: " << (version ? version : "Unknown") << std::endl;
    std::cout << "GLSL Version: " << (glslVersion ? glslVersion : "Unknown") << std::endl;
    
    if (isUsingDedicatedGpu()) {
        std::cout << "Status: Using dedicated GPU ✓" << std::endl;
    } else {
        std::cout << "Status: Using integrated GPU (may impact performance) ⚠" << std::endl;
        std::cout << "Note: If you have a dedicated GPU, check your graphics settings" << std::endl;
    }
    std::cout << "======================" << std::endl;
}

} // namespace GpuPreference
