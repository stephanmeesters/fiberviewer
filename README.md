# Fiber Viewer

Compiling the Fiber Viewer
------------------------------

Note: Compile everything in 32 bit.

1) Install the Windows SDK for Windows 7 and .NET Framework 4
This will have the gl.h header you need. It should be inside the GL folder: C:\Program Files (x86)\Windows Kits\8.1\Include\um\GL\

2) Get wglext.h and glext.h from http://www.opengl.org/registry and add it to the GL folder

3) compile freeglut 3.0.0 -- http://freeglut.sourceforge.net/
Place its header files (glut.h, freeglut.h, ... ) in the GL folder
Place the compiled library (freeglut.lib) in C:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x86

4) compile glew 1.12.0  -- http://glew.sourceforge.net/ https://github.com/nigels-com/glew
Also place these header files (glew.h, glxew.h, wglew.h) into the GL folder
After compiling in 32 bits you should have a glew32s.lib file, add the path to the CMakeLists.txt

5) Install CMake 3.3.0 or higher. Run CMake with the Fiber Viewer src dir as source code directory. 
Build the binaries in another dir outside the source directory.
I used Visual Studio 12 2013 as generator (32 bits).

6) Open the created Visual Studio project file and compile in Release mode.