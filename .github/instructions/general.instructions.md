---
applyTo: "**/*.cpp **/*.h **/*.hpp **/*.c **/*.cc **/*.cxx **/*.inl"
---

Provide project context and coding guidelines that AI should follow when generating code, answering questions, or reviewing changes.

# General Instructions for Code Generation and Review

## Project Context

This project is a C++ application focused on high-performance computing. It utilizes modern C++ standards (C++17 and above) and emphasizes clean, maintainable code. The project is structured to facilitate modular development and testing.

## Coding Guidelines

1. **C++ Standards**: Use C++17 or later features. Avoid deprecated features from earlier standards.
2. **Code Style**: Follow the Google C++ Style Guide for formatting and naming conventions
   - Use 2 spaces for indentation.
   - Use `camelCase` for variable and function names.
   - Use `PascalCase` for class names.
   - Use `UPPER_CASE` for constants.
3. **Comments**: Write clear and concise comments. Use Doxygen-style comments for public APIs.
4. **Error Handling**: Use exceptions for error handling. Avoid using `errno` or return codes.
5. **Memory Management**: Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers. Avoid manual memory management unless absolutely necessary.
6. **Performance**: Optimize for performance but prioritize readability and maintainability.
7. **Testing**: Write unit tests for all new features and bug fixes. Use a testing framework like Google Test or Catch2. Ensure tests are comprehensive and cover edge cases.
8. **Dependencies**: Use modern C++ libraries where applicable (e.g., STL, Boost). Avoid unnecessary dependencies.
9. **Version Control**: Use meaningful commit messages. Follow the project's branching strategy (e.g., feature branches, pull requests).
10. **Documentation**: Maintain up-to-date documentation using Doxygen for public APIs and complex algorithms. Use Markdown for README files and other documentation.
11. **Architecture**: Follow a modular architecture. Keep related code together and separate concerns clearly. Use namespaces to avoid name collisions.
