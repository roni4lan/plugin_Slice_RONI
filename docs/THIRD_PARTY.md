# Dependencias del paquete local

El paquete contiene bibliotecas dinámicas MSYS2 MINGW64, openFrameworks,
ofxImGui/Dear ImGui, ofxOsc/oscpack, Spout y el runtime NDI x64 usado en las
pruebas. Las licencias disponibles están copiadas en licenses; los avisos de
los headers NDI aplican a los headers y remiten a la licencia del SDK/runtime.

Fuentes de dependencias y avisos:
- openFrameworks: https://github.com/openframeworks/openFrameworks/tree/0.12.1
- ofxImGui: https://github.com/jvcleave/ofxImGui/tree/develop
- Spout: https://github.com/leadedge/Spout2
- ofxNDI y runtime de prueba: https://github.com/leadedge/ofxNDI
- Paquetes MSYS2: https://packages.msys2.org/
- Licencia SDK NDI: http://ndi.link/ndisdk_license

Este paquete se prepara para uso local del proyecto. Antes de una publicación
pública, revisar las condiciones del runtime NDI y los requisitos de distribución
y acceso a fuentes de las bibliotecas LGPL. No se publicó un instalador ni un
repositorio remoto. Los DLL se pueden sustituir junto con sus versiones compatibles.
