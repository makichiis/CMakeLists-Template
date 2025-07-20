# CMakeLists-Template

A basic CMakeLists template which attempts to support AddressSanitizer with a compiler-agnostic and generator-agnostic configuration. 

> This is still an immature CMakeLists and more work is needed to make it sufficient for most projects. Works as a bare minimum way to successfully link with AddressSanitizer whether you're on Windows or Linux.

# About

This is a composition of my research on writing a cross-platform CMakeLists which uses proper debugging information and
does its best to support linkage with the AddressSanitizer library. I intend to use this in all of my important projects 
and will iteratively refactor and build on top of it as I see need. The template is also heavily annotated in order to
explain and justify each part of the configuration.

If you notice any issues with the template, please
feel free to open an issue and give your diagnostics and feedback. Please star this repo if you like this template! 

> NOTE to novice developers: While this CMakeLists *is* usable from the command line, I recommend that you know what you are doing 
before doing so. That being said, if you want to know how you would run these configurations from the command line, you may look  
at the cmake arguments that cmake-tools for Visual Studio Code spits out when selecting certain kits and variants. If using CMake 
from the command line on a Linux or BSD (Mac, etc.) system, invoking cmake *should* be straightforward, and this cmakelists 
attempts to respect your development environment. 

If building on Windows from command line with MSVC back-end: 
https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-160#run-cmake-from-the-command-line-or-a-ci-pipeline


If using `cmake-tools` for Visual Studio Code, and you want to use an undetected build generator (like Ninja):
https://github.com/microsoft/vscode-cmake-tools/blob/015abf52297c5319fb3e81b598f4b11dd82479b0/docs/cmake-presets.md

# Platform Support
- [x] MSVC/Visual C++ 
- [x] MSVC/Clang (with ClangCL) 
- [x] MSVC/Clang with Ninja
- [x] GNU/Clang for Linux 
- [x] GNU/GCC for Linux 
- [ ] GNU/Clang for Windows (untested)
- [ ] GNU/GCC for Windows (untested)

> If you are using MSVC, AddressSanitizer can be installed through the Visual Studio installer.

# TODO
- [ ] Unit testing suite 
- [ ] Boilerplate dependency management 
- [ ] Driver and library target variants 
- [ ] Template repository configuration through GitHub
- [ ] Multi-platform deployment testing through workflows 

# References

- Evaluating configuration type(s): https://stackoverflow.com/questions/31661264/how-to-check-if-generator-is-a-multi-config-generator-in-a-cmakelists-txt
- CRT: https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170
- CL args: https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category?view=msvc-170
- Easy-to-read ClangCL argument overview: https://clang.llvm.org/docs/UsersManual.html#windows
- AddressSanitizer with Visual Studio: https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/
- Many gnu/msvc arguments from https://nullprogram.com/blog/2023/04/29/
- The (informally) ugliest color in the word: https://en.wikipedia.org/wiki/Pantone_448_C
- Some good articles on CMake:
    - https://ecrafter.wordpress.com/2012/04/24/cmake-tutorial-part-2/
    - https://web.archive.org/web/20190722182127/http://voices.canonical.com/jussi.pakkanen/2013/03/26/a-list-of-common-cmake-antipatterns/
