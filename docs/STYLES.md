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

Perfil: hard, soft y gaussian, con softness, coreWidth y coreColor. lineGap
y lineCount quedan reservados para los perfiles de líneas múltiples de Fase 3.

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
