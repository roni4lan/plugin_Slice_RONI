# Formato XML de Resolume Arena — Fase 0

## Alcance y aceptación

Se leyó íntegramente el único XML disponible: `ejemplo_advanced.xml`, ubicado
en la raíz, no en `samples/`. Esta ubicación difiere del plan y se conserva.
El archivo es XML válido, sin namespace. Declara Arena 7.19.1, revisión 32983.
El análisis describe cómo extraer ambos contornos de los tres slices presentes.
La Fase 0 cumple su criterio documental para este ejemplo; las variantes sin
evidencia se enumeran al final. No se implementó cargador ni render.

## Estructura observada

```text
XmlState @name
  versionInfo @name @majorVersion @minorVersion @microVersion @revision
  ScreenSetup
    Params @name="ScreenSetupParams" (vacío)
    CurrentCompositionTextureSize @width @height
    screens
      Screen @name @uniqueId
        Params @name="Params" (Name, Enabled, Hidden)
        Params @name="Output" (Opacity, ajustes de color)
        guides/ScreenGuide (dos guías, type 0 y 1)
        layers
          Slice @uniqueId
            Params @name="Common" (Name, Enabled)
            Params @name="Input" (Input Source, Input Opacity,
                                  Input Bypass/Solo, SoftEdgeEnable)
            Params @name="Output" (Flip, ajustes de color, Is Key, Black BG)
            InputRect @orientation / v @x @y
            OutputRect @orientation / v @x @y
            Warper
              Params @name="Warper" (Point Mode, Flip)
              BezierWarper @controlWidth @controlHeight / vertices / v @x @y
              Homography / src / v @x @y
              Homography / dst / v @x @y
        OutputDevice/OutputDeviceVirtual
          @name @deviceId @idHash @width @height
          Params @name="Params" (Width, Height)
    SoftEdging/Params @name="Soft Edge" (Gamma, Luminance, Power...)
```

Las rutas se resuelven por hijos directos y por el atributo `name` del grupo.
No usar una búsqueda global de `Name`, `Flip`, `Width` o `Opacity`: se repiten
en contextos diferentes. Los valores actuales están en `value`, no en `default`
ni en `PhaseSourceStatic/@phase`. Hay `Param`, `ParamRange` y `ParamChoice`;
los rangos y comportamientos anidados no son puntos de geometría.

## Resoluciones variables

Composición: `/XmlState/ScreenSetup/CurrentCompositionTextureSize/@width` y
`@height`. En este export son 1920 y 1080. Se deben leer en cada carga/recarga,
validar como enteros positivos y dimensionar el espacio de entrada y el futuro
FBO a partir de ellos. Nunca fijar 1920 × 1080 en el programa.

Salida de cada screen: `OutputDevice/OutputDeviceVirtual/@width` y `@height`.
Aquí también son 1920 y 1080; sus `ParamRange` Width y Height coinciden.
No confundir esta resolución con la de composición ni deducir dimensiones
a partir de la extensión de los slices. Un dispositivo diferente o una
discrepancia entre atributos y parámetros necesita otro ejemplo; no se conoce
su regla de precedencia. Si falta la resolución de composición, pedirla
explícitamente y marcarla como externa, sin sustituirla silenciosamente.

Los puntos son números decimales en escala de píxeles compatible con esas
dimensiones, no valores normalizados 0..1. Conservar precisión decimal.
Si cambia la composición en Arena, usar un export actualizado; no hay evidencia
para decidir cómo reescalar un mapping antiguo automáticamente.

## Screens, identidad y estado

Enumerar `/XmlState/ScreenSetup/screens/Screen`; hay un screen:
`Screen 1`, ID `1791941903474`. Su nombre aparece tanto en `@name` como en
`Params[@name='Params']/Param[@name='Name']/@value`, con valores iguales.
Usar el parámetro como nombre visible y el atributo como respaldo; conservar
el ID como string (también los IDs de slices), sin convertirlo a float.

Estado del screen: `Params[@name='Params']/Param[@name='Enabled']/@value=1`,
`Hidden=0`. Opacity de salida vale 1. El significado de Hidden para el envío
final no se puede deducir con un único caso en cero: conservarlo separado de
Enabled, sin asumir que son equivalentes.

Enumerar los slices con `Screen/layers/Slice`, conservando el orden del XML.
Nombre y Enabled se leen de `Params[@name='Common']/Param[@name='Name' o
'Enabled']/@value`. Los tres tienen Enabled=1. El orden es Slice 3, Slice 2,
Slice 1; no ordenar por el número del nombre ni generar IDs desde el índice.

## Extracción de contornos de entrada y salida

Para cada slice del ejemplo:

1. Leer todos los hijos directos `InputRect/v` en orden documental y convertir
   `@x`, `@y` con formato decimal independiente de la configuración regional.
2. Esos cuatro puntos son el contorno de entrada en espacio de composición.
3. Leer `OutputRect/v` de la misma manera para el contorno de salida del screen.
4. Cerrar cada contorno uniendo el último punto con el primero. El XML no repite
   el primer punto al final.
5. Mantener ambos contornos separados: presentan pequeñas diferencias numéricas.
   No reconstruirlos con un bounding box ni redondear sus coordenadas.

En todos los casos el orden observado es superior izquierda, superior derecha,
inferior derecha, inferior izquierda, compatible con origen superior izquierdo
y eje Y hacia abajo. Es una interpretación consistente con estos rectángulos;
otros tipos y transformaciones deben verificarse con nuevos ejemplos.

| Nombre | ID | Entrada: x mínimo..máximo; y mínimo..máximo | Salida: x mínimo..máximo; y mínimo..máximo |
|---|---|---|---|
| Slice 3 | 1791941903616 | 0..441.000030517578125; 0..1080 | 0..441; 0..1080 |
| Slice 2 | 1791941903541 | 1479..1920; 0..1080 | 1479..1920; 0..1080 |
| Slice 1 | 1791941903475 | 453.9210205078125..1466.0791015625; 248.378173828125..831.621826171875 | 453.9210205078125..1466.0789794921875; 248.37811279296875..831.62188720703125 |

Ejemplo literal de entrada, Slice 2:

```xml
<InputRect orientation="0">
  <v x="1479" y="0"/>
  <v x="1920" y="0"/>
  <v x="1920" y="1080"/>
  <v x="1479" y="1080"/>
</InputRect>
```

Su OutputRect contiene los mismos cuatro puntos. Los otros dos slices tienen
las diferencias de precisión mostradas arriba.

## Warp: estructura presente frente a deformación efectiva

Los tres slices contienen `Warper`, incluso cuando su geometría es rectangular.
Por eso la existencia del nodo no demuestra que haya deformación efectiva.

- `Warper/Params[@name='Warper']/ParamChoice[@name='Point Mode']/@value`
  vale `PM_LINEAR` en los tres.
- `BezierWarper` declara controlWidth=4 y controlHeight=4 y contiene 16 puntos.
  El orden observado avanza en X por fila y luego en Y: índice `fila*4+columna`.
- Las esquinas de la grilla, índices 0, 3, 15 y 12, coinciden exactamente con
  OutputRect en ese orden. Los puntos de cada borde están alineados.
- `Homography/src/v` y `dst/v` tienen cuatro puntos cada uno. En todos los
  slices src=dst=OutputRect, por lo que la correspondencia es identidad.

Para estos slices no hace falta aplicar una transformación adicional al
contorno OutputRect. No sumar sus coordenadas otra vez a los vértices de la
grilla: ya coinciden con posiciones del screen, no con un rectángulo local
centrado en cero (Slice 2 empieza en x=1479).

No se ha establecido cómo Arena combina una grilla deformada con Homography,
ni cómo interpreta modos diferentes de PM_LINEAR. Una futura representación
deberá conservar modo, dimensiones, grilla y pares src/dst completos.
`hasWarp` debe distinguir estructura almacenada de deformación comprobada;
no decidirlo solamente por la presencia de BezierWarper.

## Máscaras, orientación, flips y fuente de entrada

InputRect y OutputRect tienen `orientation=0` en todos los slices.
No existe un parámetro explícito de rotación observado. No se conoce la
codificación de otras orientaciones ni si una rotación queda incorporada
en los puntos.

Hay dos parámetros Flip distintos:
`Params[@name='Output']/Param[@name='Flip']` y
`Warper/Params[@name='Warper']/Param[@name='Flip']`. Ambos tienen T=UINT8 y
value=0 en los tres slices. No inferir los bits horizontal/vertical ni aplicar
un flip adicional sin un ejemplo no cero.

No hay nodos de máscara en el archivo. `Is Key=0` y `Black BG=0` no proporcionan
un contorno de máscara. `SoftEdgeEnable=0` en todos los slices; el grupo global
SoftEdging no describe por sí solo una máscara geométrica.

`Input Source` tiene value="0:1" y storeChoices=0. El XML no incluye una tabla
que permita identificar con certeza esa fuente. Conservar el valor sin asumir
que todos los mappings toman siempre la composición completa.
Input Opacity e Input Bypass/Solo son BOOL con value=1; su semántica operativa
requiere una comparación de estados, no tratarlos como opacidad numérica.

## Diferencias respecto del plan y variantes pendientes

El modelo propuesto de Screen necesita contemplar ID, Enabled, Hidden y datos
del dispositivo de salida. Slice necesita conservar orientation de ambos
rectángulos, ambos flips, fuente de entrada y estructura completa de Warper.
No se modificó el modelo C++ porque corresponde a la Fase 1.

Solo hay una versión y un archivo: no es posible comparar versiones de Arena.
Faltan los siguientes ejemplos para ampliar el soporte con evidencia:

- Otra resolución de composición, idealmente con resolución de screen distinta.
- Input y output claramente diferentes, con perspectiva no identidad.
- Curva Bézier con borde deformado y modos de puntos distintos de PM_LINEAR.
- Slice poligonal y máscara real, si se van a utilizar.
- Rotación y cada flip activado por separado.
- Screen/slice deshabilitado y Hidden activado por separado.
- Varios screens y otros dispositivos de salida.
- Fuentes de entrada diferentes, con su selección identificada.
- Export de otra versión, si se necesita compatibilidad entre versiones.

Estas carencias no impiden extraer los contornos de los tres slices actuales;
impiden afirmar compatibilidad con geometrías o estados no representados.
El futuro cargador deberá informar una variante no soportada en lugar de
inventar un contorno válido.

## Verificación realizada

Se parseó el XML completo con el parser XML de PowerShell/.NET y se contrastaron
por slice: cuatro puntos de entrada, cuatro de salida, 16 de grilla, igualdad
de src/dst y coincidencia exacta de las cuatro esquinas de grilla con OutputRect.
Se extrajeron nombres, IDs y coordenadas para contrastar la tabla anterior.
La Fase 0 es documental: no hay código de aplicación que compilar o ejecutar.
