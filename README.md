# CMakeLists-Template

A basic CMakeLists template which attempts to support AddressSanitizer with a compiler-agnostic and generator-agnostic configuration. 

> This is still an immature CMakeLists and more work is needed to make it sufficient for most projects. Works as a bare minimum way to successfully link with AddressSanitizer whether you're on Windows or Linux.

# Platform Support
- [x] MSVC/Visual C++ 
- [x] MSVC/Clang (with ClangCL) 
- [x] MSVC/Clang with Ninja
- [x] GNU/Clang for Linux 
- [x] GNU/GCC for Linux 
- [ ] GNU/Clang for Windows (untested)
- [ ] GNU/GCC for Windows (untested)

> If you are using MSVC, AddressSanitizer can be installed through the Visual Studio installer.