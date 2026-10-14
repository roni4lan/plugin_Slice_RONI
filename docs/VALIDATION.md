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
