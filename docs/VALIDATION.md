# Verificación por fase

## Fase 0

XML real de Arena 7.19.1 revisión 32983: 1 screen, 3 slices. Estructura,
coordenadas, homografías identidad y esquinas de grilla comprobadas.
Las variantes sin ejemplos permanecen documentadas en FORMAT.md.

## Fase 1

Release compilada y ejecutada. Vista del fixture comprobada mediante captura
de la ventana: nombres Slice 3, Slice 2, Slice 1 y sus posiciones correctas.
Selección por clic y zoom comprobados en la interfaz. Apertura XML, round-trip
JSON de bindings/salida/OSC, resolución alternativa 2560×1440 independiente
del screen de 1920×1080 y rechazo de coordenada inválida pasaron el self-test.

## Fase 2

Release compilada y ejecutada. Grosor y radio de esquina cambiados en vivo
en la ventana a 60 FPS. Se comprobaron longitud/remuestreo, vértices finitos
en miter/round/bevel, caché sin regeneración por frame, herencia por campo,
fondo transparente y alfa premultiplicado al 50%.

Alpha, screen y multiply se compararon contra colores RGBA esperados para
dos bordes superpuestos. Se verificó la orientación usando un rectángulo
asimétrico en Y: la fila de textura cero corresponde a la entrada y=0 con
la configuración FBO utilizada por openFrameworks.

Benchmark final de 120 frames, GL finish por frame, 50 rectángulos, FBO
1920×1080: 489.35 FPS del renderer. No representa rendimiento con Spout,
glow bloom o interfaz de Arena; esos casos pertenecen a fases posteriores.
La aplicación presenta a 60 FPS y había otra ventana de prueba activa.

Los resultados de self-test se generan en bin/data/self-test-report.json.
El estado running se escribe antes de la prueba para impedir confundir un
informe anterior con una ejecución que falla. Las capturas de readback se
generan en bin/data/phase2-check.png.

## Fase 3

Release compilada y ejecutada. Cada uno de los ocho presets se renderizó con
una geometría idéntica a dos tiempos distintos. Se obtuvieron ocho hashes
de imagen distintos y siete presets que cambian con el tiempo; Solid conserva
su imagen inicial, con animación opcional desde el panel.

Los readbacks de Neon, Chase, DoubleLine y Bloom se inspeccionaron visualmente.
El test de bloom comprueba halo fuera del borde y transparencia lejos del slice.
Las pruebas de alfa, mezcla, orientación y herencia de Fase 2 siguen pasando.

Benchmark de 120 frames con GL finish, 50 slices, Neon con falloff y FBO
1920×1080: 236.29 FPS del renderer; las meshes no se regeneraron durante
la animación. La interfaz presenta a 60 FPS. No se ha medido aún la salida
con Resolume ni el benchmark de 50 slices con bloom.

## Fase 4 — Windows

Spout: ida y vuelta de una textura RGBA semitransparente y asimétrica mediante
un segundo receptor Spout. Todos los canales coinciden dentro de 2/255,
incluyendo orientación y dimensiones. Se compiló SpoutLibrary desde el mismo
árbol de fuentes y headers con GCC: la DLL precompilada descargada fallaba
en la llamada nativa. El wrapper usa únicamente la interfaz compatible de
SpoutLibrary, sin transmitir objetos std::string/vector entre compiladores.

NDI: receptor local RGBA con resolución 640×360. Orientación y alfa verificados;
máxima variación de alfa observada 5/255, con tolerancia de 8/255 por compresión.
El receptor se libera después de la prueba y el emisor deja de anunciarse.

Arena 7.19.1 detectó RONI_RESO_TEST. Se observó en su Advanced Output el mapping
ejemplo_advanced con Virtual Output 1920×1080 y los bordes cian sobre una
capa Solid Color roja: el fondo y los interiores se conservan transparentes,
y los bordes coinciden con los límites de los slices. El usuario participó
directamente en esta comprobación usando los controles de la app y Arena.

Syphon permanece condicionado a macOS y ofxSyphon; no se ha compilado ni
ejecutado en macOS. La aceptación y el empaquetado actuales corresponden a Windows.

La prueba manual detectó un límite inferior que el formato de un slider
redondeaba a cero (noiseScale). Se desactivó el redondeo del control y se
conservó mayor precisión; los mensajes de render se limpian al corregir el valor.

## Fase 5

Release compilada y ejecutada. Suite completa aprobada: ocho mensajes OSC
entregados por UDP local cambian estilo, grosor, velocidad, color, grupo,
BPM, salida y fuente de modulación. Paquete inválido rechazado. Pulso BPM
comprobado leyendo píxeles del FBO en dos fases opuestas. FFT detecta un
seno grave de 93.75 Hz; un parámetro reacciona a su nivel. Entrada real:
un dispositivo disponible, 1024 muestras recibidas por callback.
LFO, AD/ADSR, tap tempo, suma antes del clamp y persistencia comprobados.
Benchmark con otros programas abiertos: 206.85 FPS con 50 slices y falloff.

## Fase 6

Release compilada y suite completa aprobada. Exportación XML simulada sobre
una copia del fixture: cambio 1920×1080 a 2560×1440 y desplazamiento de un
contorno 100 px. El nuevo borde se verificó leyendo el FBO; Neon y grosor 23
se conservaron. ID reemplazado: recuperación por nombre único aprobada.
Exportación incompleta: se conservó el último mapping. Debounce 350 ms probado.
Undo/redo agrupa un gesto de edición. Shader deliberadamente inválido rechazado
mediante GL_LINK_STATUS; la imagen anterior permaneció idéntica byte por byte;
restaurar el shader permitió recargar. El mensaje de compilación inválida en
el log forma parte de esta prueba negativa. Falloff: 197.66 FPS con 50 slices.

Selección múltiple, grupos, atajos y modo performance implementados. La
carpeta portable incluye ejecutable, DLL, shaders, presets, fixture, documentos
y avisos de licencias. La aceptación actual cubre Windows; Mac permanece pendiente.

Portable verificada desde dist/Pluguin Slice RONI RESO, con código de salida 0
y passed=true. Benchmark portable: 192.52 FPS con 50 slices y falloff. Interfaz
observada a 60 FPS mediante la skill computer-use; F12 oculta/restaura controles
y el contador de Spout continúa aumentando en performance. Se dejó la versión
portable abierta con el fixture y Spout RONI RESO activo.

Temporización Spout corregida: el envío avanza por fecha programada, evitando
acumular el jitter de la interfaz. La última compilación portable volvió a
pasar la suite completa. Prueba manual de envío: interfaz 60 FPS y emisor
aproximadamente 58–60 FPS, frente a 43–49 con el temporizador anterior.
