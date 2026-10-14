# Salida a Resolume

## Windows y Spout

Se usa la interfaz SpoutLibrary del SDK Spout2 en lugar de enlazar las librerías
MSVC de un addon. Es una adaptación del stack para MSYS2/GCC. La biblioteca
se compila desde vendor/Spout2-master mediante scripts/build-spout.ps1 y se
coloca junto al ejecutable como SpoutLibrary.dll. La aplicación incluye el
header de esa misma fuente. No mezclar con el header de Binaries: difiere del
header actual del árbol descargado.

La salida preserva RGBA y permite elegir alfa premultiplicado o directo.
El valor inicial es premultiplied. En el clip receptor de Arena, elegir el
Alpha Type que corresponda al modo enviado. Los nombres y guías de la GUI
no forman parte de la textura enviada.

## NDI

Los headers y el runtime proceden de ofxNDI. Se usa la API dinámica del SDK,
sin enlazar import libraries de otro compilador. Windows busca
Processing.NDI.Lib.x64.dll junto al ejecutable. macOS busca libndi.dylib.
La interfaz de carga v5 utiliza exclusivamente funciones de su prefijo
compatible; se envía RGBA con alfa directo, según la especificación NDI.

NDI hace readback de la textura a CPU y comprime el vídeo. El rendimiento de
esta ruta se mide por separado del renderer y de Spout.

## Syphon

La implementación está condicionada a TARGET_OSX y a la presencia de
ofxSyphon. Para compilar en macOS, agregar ofxSyphon con el generador de
proyectos de openFrameworks y colocar el runtime NDI de macOS junto al
ejecutable si se va a usar esa salida. Syphon no se puede ejecutar ni
verificar en el entorno Windows actual; el selector devuelve un mensaje
explícito de plataforma no disponible.

## Resolución y frecuencia

La salida usa inicialmente la resolución del XML. Resolucion personalizada
reescalará la textura terminada a las dimensiones elegidas; esto cambia la
escala de sus bordes y debe corresponder al ajuste del clip receptor.
FPS salida controla el ritmo de envío entre 1 y 120; la interfaz conserva un
mínimo de 60 FPS de presentación. El contador muestra el ritmo realmente
enviado y el número total de frames.

Cambiar backend, nombre, resolución o FPS recrea el emisor. Reintentar salida
permite volver a inicializarlo tras una biblioteca faltante o un fallo de GPU.
Desactivar libera los recursos y elimina el emisor.

## Fuentes oficiales

- SpoutLibrary: https://github.com/leadedge/Spout2/tree/master/SPOUTSDK/SpoutLibrary
- ofxNDI: https://github.com/leadedge/ofxNDI
- Formato RGBA de NDI: https://docs.ndi.video/all/developing-with-ndi/sdk/frame-types
- Carga dinámica de NDI: https://docs.ndi.video/all/developing-with-ndi/sdk/dynamic-loading-of-ndi-libraries
- ofxSyphon: https://github.com/astellato/ofxSyphon
- Recepción en Arena: https://www.resolume.com/support/syphonspout
