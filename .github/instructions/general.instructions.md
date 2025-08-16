---
applyTo: "**/*.cpp **/*.h **/*.hpp **/*.c **/*.cc **/*.cxx **/*.inl"
---

## Project Context

This project is a C++ application focused on high-performance computing. It utilizes modern C++ standards (C++20 and above) and emphasizes clean, maintainable code. The project is structured to facilitate modular development and testing.

🔧 Core Practices
Modular Design: Split into clear modules (e.g., Renderer, Scene, Assets, Platform).
Smart Pointers & RAII: Use std::unique_ptr/shared_ptr to manage memory safely.
Consistent Naming: Use PascalCase for classes, camelCase for variables, and namespaces for grouping.
🧱 Design Principles
SOLID: Follow object-oriented best practices for clean, extensible code.
Component-Based Architecture: For scene objects (e.g., Transform, Mesh, Camera).
Data-Oriented Design: Optimize performance-critical paths using SoA and cache-friendly layouts.
🧪 Testing & Debugging
Unit Tests: Use Google Test or Catch2 for math and utility modules.
Debug Tools: Integrate RenderDoc, ImGui, and logging for runtime inspection.
📦 Build & Style
Cross-Platform: Use CMake and abstract platform-specific code.
Code Style: Enforce with clang-format and clang-tidy.
Documentation: Use Doxygen and maintain a dev wiki.
