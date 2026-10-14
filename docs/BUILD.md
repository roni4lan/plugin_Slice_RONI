# Entorno de compilación verificado

- Windows, MSYS2 MINGW64 existente en el equipo.
- GCC 16.1.0-5, make 4.4.1-3.
- openFrameworks 0.12.1, paquete oficial MSYS2 MINGW64.
- ofxImGui, archivo de la rama develop descargado durante este desarrollo.

SHA256 de los archivos descargados:

```text
of_v0.12.1_msys2_mingw64_release.zip
C3405AD4401BC0E305902BA89879CA8362FA467616414B846A6DCBF1075CB26B

ofxImGui-develop.zip
854BFAB6AE2C09F8361E53D2D3A18E2298347C1C147E361DE44D0931F1D3EE1F
```

Los archivos de dependencias permanecen en vendor/, fuera de Git. El paquete
oF contiene un archivo de pkg-config de Emscripten que no pudo extraerse con
bsdtar en Windows; no forma parte de la compilación MINGW64 utilizada.

Dependencias MSYS2 usadas (prefijo mingw-w64-x86_64-): cairo, curl, freeimage,
glew, glfw, glm, fmt, harfbuzz, libsndfile, mpg123, nlohmann-json, openal,
pugixml, rtaudio, uriparser, utf8cpp, pkgconf y ntldd, más make de MSYS.
Los nombres freeimage y mpg123 son los del registro actual; el instalador
incluido en oF 0.12.1 usa FreeImage para el primero.

El alias de unidad temporal solo existe mientras corre scripts/build.ps1.
Todas las fuentes y dependencias permanecen en el workspace. No se modifica
el PATH permanente del sistema. La compilación y copy_dlls usan el entorno
MINGW64 dentro del proceso bash de construcción.

copy_dlls identifica las bibliotecas necesarias con ntldd y las copia a bin/;
no copia las DLL del sistema operativo. Conservar las licencias de las
dependencias al distribuir una futura versión portable.

Salidas: Spout2 master, SHA256 de spout.zip
0CD717C8D14DBF21A8DF5C16C0C64317DBFB2F9EE256138D7FBEB8F7EBFC53F7.
Se compila SpoutLibrary con scripts/build-spout.ps1, CMake 4.3.3 y Ninja 1.13.2.
Usar ese DLL y el header fuente correspondiente; la DLL precompilada incluida
en el archivo descargado produjo un fallo nativo durante el envío.

ofxNDI master, SHA256 de ndi.zip
24BBCAEA3926232C2BD12E7FCA14CA3E416D6E8F04C36B720AD557471CDA362C.
Se usan sus headers NDI y el runtime x64 incluido, mediante carga dinámica.
Ver OUTPUT.md para plataforma y formato de alfa de cada backend.
