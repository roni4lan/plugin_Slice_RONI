# Estilos y parámetros

## Sólido — Fase 2

El shader border_solid.frag usa una tira de triángulos con distancia acumulada
u en píxeles y posición transversal v de -1 (exterior) a +1 (interior).
La coordenada normalizada del perímetro se obtiene dividiendo u por su longitud.

Geometría: space, thickness, offset, alignment, cornerRadius, joinStyle,
miterLimit, capStyle, closePath, subdivision, segmentsMode y edgeMask.
Offset positivo desplaza hacia el interior. En selectedEdges, los bits 1, 2,
4 y 8 corresponden al orden de aristas del XML; en el fixture son arriba,
derecha, abajo e izquierda. Con esquinas redondeadas, cada arista subdividida
se asigna al lado original más cercano. perEdge separa los lados con sus caps.

Color: colorA, opacity, saturation, brightness, hueShiftSpeed, blendMode,
innerFillOpacity e innerFillColor. Mezcla alpha y add usa alfa premultiplicado;
screen y multiply usan composición que contempla el alfa de ambos operandos.

Perfil: hard, soft y gaussian, con softness, coreWidth y coreColor.
En Fase 3 se agregan doubleLine y tripleLine: lineGap mide la distancia entre
centros y lineCount admite hasta seis líneas.

## Gradiente de perímetro — Fase 2

Usa la misma geometría y perfiles que Sólido. Seleccionar colorMode igual a
gradientPerimeter. Usa colorA, colorB, gradientOffset, gradientRepeat y
gradientRotation, o hasta ocho gradientStops ordenados de 0 a 1. Cada parada
incluye pos y color RGBA. El gradiente se repite con fract sobre el perímetro.

## Herencia

Los parámetros se resuelven por campo: defaults → global → grupo → slice.
El panel permite elegir el nivel editado. Desmarcar Propio elimina el override,
de modo que vuelve a aplicar el valor del nivel superior. Los grupos y sus
asignaciones se conservan dentro del proyecto JSON.

La textura de salida conserva alfa premultiplicado. La textura de vista previa
se convierte a alfa directo para la mezcla de ImGui. Las guías de selección
y los nombres pertenecen a la interfaz y no se dibujan en la textura de salida.

Los bordes que exceden los límites de la composición se recortan en su FBO;
usar alignment=inside para mantener visible el grosor sobre los límites.
space=output es una vista geométrica experimental: no corrige el doble warp
que ocurriría al volver a pasar esas coordenadas por el mismo mapping de Arena.

## Presets iniciales — Fase 3

| ID | Comportamiento inicial | Parámetros distintivos |
|---|---|---|
| Solid | Línea continua | pattern, profile |
| Dashed | Trazos que se desplazan | dashLength, gapLength, dashSpeed, dashRoundCaps |
| Chase | Cabeza y estela por el perímetro | animation, chaseLength, chaseTail, speed |
| Neon | Núcleo blanco, halo y flicker | profile, softness, coreWidth, glowRadius, glowIntensity, flicker |
| Gradient | Gradiente que gira | colorMode, gradientRotation, hueShiftSpeed |
| DoubleLine | Líneas paralelas con gradiente animado | profile, lineCount, lineGap, gradientRotation |
| Glitch | Ruido, cortes y temblor | noiseEnabled, noiseAffects, noiseAmount, glitchAmount, glitchRate, jitter |
| Pulse | Pulso de brillo y halo | animation, pulseRate, pulseDepth, glowEnabled |

Todos comparten la geometría, color y perfiles de la base y aceptan parámetros
editados en vivo. El selector aplica valores de efecto y mantiene los overrides
de geometría existentes; un preset JSON completo puede restaurar también la
geometría. Guardar y cargar presets se hace desde el panel, con archivos JSON
en bin/data/presets/ como ubicación inicial.

Patrones: solid, dashed, dotted, segments (trazos repetidos), ticks, zigzag,
wave y chevrons. Las ondas deforman la geometría en el vertex shader; los
contornos se subdividen conservando las esquinas para permitir deformaciones
a lo largo de cada lado, sin regenerar la mesh en cada frame.

Animaciones: none, pulse, blink, strobe, chase, multiChase, breathing, sweep,
trail, ripple, noiseFlow y random. pulseShape controla seno, triángulo,
cuadrado, diente de sierra y caída exponencial. Las direcciones cw y ccw
invierten el recorrido; pingPong alterna el avance; outwards/inwards dirigen
el desplazamiento normal de ondas/ruido y de la expansión animada.

phaseSpread reparte un ciclo entre slices. El orden se puede elegir por índice,
X, Y, distancia al centro o una ordenación determinista basada en ID y seed.

Ruido: perlin, simplex, value y worley, con noiseScale y noiseSpeed.
noiseAffects usa bits 1=posición, 2=grosor, 4=opacidad y 8=color.

Halo falloff: geometría más ancha con caída exponencial y desvanecimiento
hasta cero en el extremo. Halo bloom: fuente HDR con intensidad/color por
slice, blur gaussiano separable a media resolución y composición con la base.
El blur compartido usa el mayor radio y cantidad de pasadas entre los slices
que tienen bloom activo; la intensidad y el color se conservan individualmente.

## Extensión de estilos

Una clase deriva de BorderStyle, define name/defaults y opcionalmente su
fragmentShader y uniforms. Se registra desde su propio .cpp mediante
StyleRegistry::instance().add(...). ShaderManager carga el programa por ruta;
el renderer no requiere cambios para registrar otra clase/shader que use la
interfaz de atributos y uniforms compartida. Documentar los parámetros de
cada estilo nuevo aquí.
