# SliceBorders: app de bordes animados para slices de Resolume Arena

> **Instrucciones para Codex:** trabajá siguiendo este documento fase por fase. No avances a la siguiente fase hasta que la actual compile, corra y cumpla sus criterios de aceptación. Al terminar cada fase, actualizá la sección "Estado" al final de este archivo. Si algo de este plan contradice la realidad de los archivos de ejemplo, **mandan los archivos**, y documentás la diferencia en `docs/FORMAT.md`.

---

## 1. Objetivo

Una app independiente en **C++ / openFrameworks** que:

1. Carga el XML de *Advanced Output* exportado desde Resolume Arena (screens y slices).
2. Genera bordes alrededor de cada slice, con **estilos visuales totalmente parametrizables** y animados.
3. Renderiza todo en un FBO del tamaño de la composición, con **fondo transparente**.
4. Envía el resultado a Resolume por **Spout (Windows)**, **Syphon (Mac)** o **NDI (ambos)**, para usarlo como una fuente en una capa superior.
5. Se controla desde una GUI (ImGui) y por **OSC**.

### Por qué app aparte y no plugin FFGL
Un efecto FFGL solo ve la imagen de su clip/capa y no conoce la geometría de los slices. La app aparte lee el mapping, dibuja en espacio de composición y Resolume aplica después el warp y la máscara de cada slice, de modo que el borde queda pegado a la geometría final.

---

## 2. Stack

| Componente | Elección |
|---|---|
| Framework | openFrameworks (última versión estable, OpenGL 3.2+ core profile) |
| GUI | `ofxImGui` |
| XML | `ofXml` (pugixml incluido en oF) o `pugixml` directo |
| JSON (proyectos/presets) | `ofJson` (nlohmann, incluido en oF) |
| Salida Windows | `ofxSpout` |
| Salida Mac | `ofxSyphon` |
| Salida NDI | `ofxNDI` |
| OSC | `ofxOsc` (incluido en oF) |
| Audio (opcional) | `ofSoundStream` + FFT (`ofxFft` o implementación propia) |
| Shaders | GLSL 330 (`#version 330`) |

Compilación condicional con `#ifdef TARGET_WIN32` / `TARGET_OSX` para Spout y Syphon.

---

## 3. Arquitectura de render (decisión clave)

Para tener **muchos estilos con pocos shaders**, cada borde se construye como una **tira de triángulos (mesh)** alrededor del contorno del slice, con atributos por vértice:

- `u`: distancia acumulada a lo largo del perímetro (en píxeles), y también `uNorm` en 0..1.
- `v`: coordenada transversal al borde, de -1 (lado exterior) a +1 (lado interior). `v = 0` es la línea del contorno.

Con `(u, v)` el fragment shader puede producir cualquier estilo: línea sólida, punteada, gradiente a lo largo del perímetro, chase, glow por falloff en `v`, doble línea, etc.

Esto evita calcular distancias a polilíneas por píxel y se escala bien con muchos slices.

**Esquinas:** al construir la mesh hay que resolver las uniones (join) entre segmentos: `miter` (con límite), `round` o `bevel`, según el parámetro `joinStyle`. El parámetro `cornerRadius` redondea el contorno antes de generar la mesh (suavizado de la polilínea).

**Glow:** se resuelve de dos maneras, seleccionables:
- *Barato:* ancho extra de la tira con falloff exponencial en `v`.
- *Calidad:* pasada de blur gaussiano separable sobre un FBO intermedio, sumado aditivamente (`bloom`).

**Pipeline por frame:**
```
for cada slice visible (según su estilo/capas):
    actualizar parámetros animados (t, bpm, audio, OSC)
    regenerar mesh solo si cambió la geometría o parámetros geométricos
    dibujar mesh con el shader del estilo en FBO_borders
si glow=calidad: blur(FBO_borders) -> FBO_glow; componer
componer en FBO_final (premultiplied alpha)
enviar FBO_final por Spout/Syphon/NDI
mostrar vista previa en la GUI
```

---

## 4. Estructura del proyecto

```
SliceBorders/
├─ PLAN_SliceBorders.md          (este archivo)
├─ docs/
│  ├─ FORMAT.md                  (esquema real del XML de Resolume)
│  └─ STYLES.md                  (catálogo de estilos y parámetros)
├─ samples/                      (XML de ejemplo exportados desde Resolume)
├─ src/
│  ├─ main.cpp
│  ├─ ofApp.h / ofApp.cpp
│  ├─ io/
│  │  ├─ ResolumeXmlLoader.h/.cpp
│  │  ├─ ProjectIO.h/.cpp        (guardar/cargar proyecto JSON)
│  │  └─ FileWatcher.h/.cpp
│  ├─ model/
│  │  ├─ Composition.h
│  │  ├─ Screen.h
│  │  ├─ Slice.h
│  │  └─ StyleParams.h           (struct con todos los parámetros)
│  ├─ geometry/
│  │  ├─ Polyline.h/.cpp         (resample, suavizado, offset, longitud)
│  │  ├─ BorderMesh.h/.cpp       (generación de la tira u,v)
│  │  └─ Bezier.h/.cpp           (si el slice trae warp)
│  ├─ render/
│  │  ├─ BorderRenderer.h/.cpp
│  │  ├─ ShaderManager.h/.cpp    (carga y hot-reload de shaders)
│  │  ├─ GlowPass.h/.cpp
│  │  └─ Fbo.h/.cpp
│  ├─ styles/
│  │  ├─ BorderStyle.h           (interfaz común)
│  │  ├─ SolidStyle, DashedStyle, ChaseStyle, GradientStyle,
│  │  │  NeonStyle, DoubleLineStyle, NoiseStyle, PulseStyle,
│  │  │  StrobeStyle, SegmentsStyle, ScanlineStyle...
│  │  └─ StyleRegistry.h/.cpp
│  ├─ modulation/
│  │  ├─ LFO.h, Envelope.h, BeatClock.h, AudioAnalyzer.h
│  │  └─ ModulationMatrix.h/.cpp
│  ├─ output/
│  │  ├─ IOutput.h               (interfaz)
│  │  ├─ SpoutOutput.h/.cpp
│  │  ├─ SyphonOutput.h/.cpp
│  │  └─ NdiOutput.h/.cpp
│  ├─ control/
│  │  └─ OscServer.h/.cpp
│  └─ ui/
│     └─ Panels.h/.cpp           (ImGui)
└─ bin/data/
   ├─ shaders/
   │  ├─ border.vert
   │  ├─ border_common.glsl      (funciones compartidas: noise, hsv, etc.)
   │  ├─ border_solid.frag
   │  ├─ border_dashed.frag
   │  ├─ border_chase.frag
   │  ├─ border_neon.frag
   │  ├─ border_gradient.frag
   │  ├─ blur.frag
   │  └─ composite.frag
   ├─ presets/                   (JSON de presets de estilo)
   └─ projects/
```

---

## 5. Modelo de datos

```cpp
struct Composition { int width, height; };

struct Slice {
    std::string id, name;
    bool enabled;
    std::vector<glm::vec2> inputPoints;    // contorno en espacio de composición
    std::vector<glm::vec2> outputPoints;   // contorno en espacio de salida (screen)
    std::vector<glm::vec2> maskPoints;     // si existe
    bool hasWarp;                          // bezier / perspectiva
    // + lo que revele el análisis del XML real
};

struct Screen { std::string name; int width, height; std::vector<Slice> slices; };

struct StyleParams;   // ver sección 6
struct SliceBinding { std::string sliceId; std::string styleName; StyleParams params; bool enabled; };

struct Project {
    std::string xmlPath;
    std::vector<SliceBinding> bindings;
    StyleParams defaultStyle;
    OutputSettings output;      // tipo (Spout/Syphon/NDI), nombre, resolución, FPS
    OscSettings osc;
};
```

---

## 6. Parámetros de estilo visual (núcleo del proyecto)

Todos los parámetros viven en `StyleParams`, se serializan a JSON, se editan en la GUI, se controlan por OSC y pueden ser modulados (sección 7). Se pueden aplicar **por slice**, **por grupo de slices** o **globalmente** (con herencia: slice → grupo → global).

### 6.1 Geometría del borde

| Parámetro | Tipo | Rango / valores | Descripción |
|---|---|---|---|
| `space` | enum | `input`, `output` | Espacio donde se dibuja el contorno (ver riesgos) |
| `thickness` | float px | 0.5 – 200 | Grosor de la línea |
| `offset` | float px | -200 – 200 | Desplazamiento del contorno (negativo = hacia afuera, positivo = hacia adentro) |
| `alignment` | enum | `center`, `inside`, `outside` | Hacia qué lado crece el grosor |
| `cornerRadius` | float px | 0 – 200 | Redondeo de esquinas del contorno |
| `joinStyle` | enum | `miter`, `round`, `bevel` | Unión entre segmentos |
| `miterLimit` | float | 1 – 10 | Límite de punta en `miter` |
| `capStyle` | enum | `butt`, `round`, `square` | Extremos (cuando el borde es discontinuo) |
| `closePath` | bool | | Cierra el contorno |
| `subdivision` | int | 1 – 64 | Resolución de las curvas |
| `segmentsMode` | enum | `all`, `perEdge`, `selectedEdges` | Dibujar todo el contorno o solo ciertos lados |
| `edgeMask` | bitmask | | Qué lados se dibujan (útil para slices rectangulares) |

### 6.2 Color y relleno

| Parámetro | Tipo | Descripción |
|---|---|---|
| `colorA`, `colorB` | RGBA | Colores base |
| `colorMode` | enum | `solid`, `gradientLinear`, `gradientPerimeter`, `gradientRadial`, `rainbow`, `palette` |
| `gradientStops` | lista de (pos, RGBA) | Hasta 8 paradas de color |
| `gradientRotation` | float °/s | Velocidad de rotación del gradiente |
| `gradientOffset` | float 0..1 | Desfase del gradiente |
| `gradientRepeat` | float | Repeticiones a lo largo del perímetro |
| `hueShiftSpeed` | float | Rotación de tono en el tiempo |
| `saturation`, `brightness` | float 0..2 | Ajustes de color |
| `opacity` | float 0..1 | Opacidad global del borde |
| `blendMode` | enum | `alpha`, `add`, `screen`, `multiply` (dentro de la app) |
| `innerFillOpacity` | float 0..1 | Relleno tenue del slice (opcional) |
| `innerFillColor` | RGBA | Color de ese relleno |

### 6.3 Perfil transversal (cómo se ve la sección del borde)

| Parámetro | Tipo | Descripción |
|---|---|---|
| `profile` | enum | `hard`, `soft`, `gaussian`, `bevel`, `doubleLine`, `tripleLine` |
| `softness` | float 0..1 | Suavidad de los bordes de la línea (antialias extendido) |
| `coreWidth` | float 0..1 | Fracción del grosor que es el "núcleo" brillante |
| `coreColor` | RGBA | Color del núcleo (p. ej. blanco en neón) |
| `lineGap` | float px | Separación entre líneas en `doubleLine`/`tripleLine` |
| `lineCount` | int 1–6 | Número de líneas paralelas |

### 6.4 Glow / halo

| Parámetro | Tipo | Descripción |
|---|---|---|
| `glowEnabled` | bool | |
| `glowMode` | enum | `falloff` (barato), `bloom` (blur) |
| `glowRadius` | float px | 0 – 300 |
| `glowIntensity` | float | 0 – 5 |
| `glowFalloff` | float | Curva de caída (exponente) |
| `glowColor` | RGBA | Si es distinto del color del borde |
| `glowUseBorderColor` | bool | Heredar el color del borde |
| `glowPasses` | int 1–8 | Iteraciones de blur (calidad) |

### 6.5 Patrón a lo largo del perímetro

| Parámetro | Tipo | Descripción |
|---|---|---|
| `pattern` | enum | `solid`, `dashed`, `dotted`, `segments`, `ticks`, `zigzag`, `wave`, `chevrons` |
| `dashLength` | float px | Largo del trazo |
| `gapLength` | float px | Largo del hueco |
| `dashSpeed` | float px/s | Velocidad de desplazamiento (con signo = dirección) |
| `dashPhase` | float | Fase inicial |
| `dashRoundCaps` | bool | Puntas redondeadas en cada trazo |
| `waveAmplitude` | float px | Amplitud en `wave`/`zigzag` |
| `waveFrequency` | float | Frecuencia espacial |
| `waveSpeed` | float | Velocidad de la onda |
| `tickSpacing`, `tickLength` | float px | Marcas perpendiculares |

### 6.6 Animación / efectos

| Parámetro | Tipo | Descripción |
|---|---|---|
| `animation` | enum | `none`, `pulse`, `blink`, `strobe`, `chase`, `multiChase`, `breathing`, `sweep`, `trail`, `ripple`, `noiseFlow`, `random` |
| `speed` | float | Multiplicador de velocidad global de la animación |
| `direction` | enum | `cw`, `ccw`, `pingPong`, `outwards`, `inwards` |
| `chaseCount` | int 1–16 | Cantidad de "cabezas" que recorren el perímetro |
| `chaseLength` | float 0..1 | Largo de cada cabeza (fracción del perímetro) |
| `chaseTail` | float 0..1 | Longitud de la estela |
| `chaseEasing` | enum | `linear`, `easeInOut`, `bounce`, etc. |
| `pulseRate` | float Hz | Frecuencia del pulso |
| `pulseDepth` | float 0..1 | Profundidad de la modulación |
| `pulseShape` | enum | `sine`, `triangle`, `square`, `saw`, `expDecay` |
| `strobeRate` | float Hz | |
| `strobeDuty` | float 0..1 | Porcentaje de tiempo encendido |
| `phaseSpread` | float 0..1 | Desfase entre slices (efecto "ola" entre slices) |
| `phaseSpreadOrder` | enum | `byIndex`, `byX`, `byY`, `byDistanceFromCenter`, `random` |
| `seed` | int | Semilla de aleatoriedad |

### 6.7 Ruido y distorsión

| Parámetro | Tipo | Descripción |
|---|---|---|
| `noiseEnabled` | bool | |
| `noiseType` | enum | `perlin`, `simplex`, `value`, `worley` |
| `noiseAmount` | float px | Desplazamiento transversal del borde |
| `noiseScale` | float | Escala espacial |
| `noiseSpeed` | float | Velocidad temporal |
| `noiseAffects` | bitmask | `position`, `thickness`, `opacity`, `color` |
| `glitchAmount` | float 0..1 | Cortes y desplazamientos aleatorios |
| `glitchRate` | float Hz | |
| `jitter` | float px | Temblor de alta frecuencia |
| `flicker` | float 0..1 | Parpadeo irregular estilo neón defectuoso |

### 6.8 Sincronización y modulación (ver sección 7)

| Parámetro | Tipo | Descripción |
|---|---|---|
| `syncMode` | enum | `free`, `bpm`, `audio`, `osc` |
| `bpm` | float | Si `syncMode = bpm` |
| `beatDivision` | enum | `1/4`, `1/2`, `1`, `2`, `4`, `8` beats por ciclo |
| `audioBand` | enum | `bass`, `lowMid`, `mid`, `high`, `all` |
| `audioAmount` | float | Cuánto afecta el audio |
| `audioSmoothing` | float 0..1 | Suavizado |

### 6.9 Ejemplo de JSON de estilo

```json
{
  "name": "Neón cian",
  "space": "input",
  "thickness": 6,
  "alignment": "center",
  "cornerRadius": 12,
  "joinStyle": "round",
  "colorMode": "gradientPerimeter",
  "gradientStops": [
    { "pos": 0.0, "color": [0.0, 0.9, 1.0, 1.0] },
    { "pos": 1.0, "color": [1.0, 0.0, 0.8, 1.0] }
  ],
  "gradientRotation": 40,
  "profile": "gaussian",
  "coreWidth": 0.3,
  "coreColor": [1, 1, 1, 1],
  "glowEnabled": true,
  "glowMode": "bloom",
  "glowRadius": 40,
  "glowIntensity": 1.6,
  "pattern": "solid",
  "animation": "pulse",
  "pulseRate": 1.0,
  "pulseDepth": 0.35,
  "syncMode": "bpm",
  "bpm": 128,
  "beatDivision": "1"
}
```

---

## 7. Sistema de modulación

Cualquier parámetro numérico puede recibir modulación sin tocar el código del estilo.

- **Fuentes:** `LFO` (sine, tri, square, saw, random/S&H), `Envelope` (AD/ADSR disparado por beat/OSC), `BeatClock` (BPM manual, tap tempo, OSC), `AudioBand` (FFT por bandas), `OSC` (valor recibido).
- **Matriz:** lista de rutas `fuente → parámetro` con `amount`, `offset` y `curve`.
- **Resolución:** el valor final del parámetro = `base + Σ(modulaciones)`, con clamp al rango definido.
- Cada parámetro declara en un registro su `min`, `max`, `default`, `unit` y si es modulable. La GUI y el OSC se generan automáticamente desde ese registro.

---

## 8. Interfaz común de estilos

```cpp
class BorderStyle {
public:
    virtual ~BorderStyle() = default;
    virtual std::string name() const = 0;
    virtual void setup(ShaderManager&) = 0;
    virtual void update(float t, float dt, const ModulationContext&) = 0;
    virtual void draw(const Slice&, const BorderMesh&, const StyleParams&, ofFbo& target) = 0;
    virtual void drawUi(StyleParams&) = 0;   // controles ImGui específicos
};
```

Los estilos se registran en `StyleRegistry`. Un estilo nuevo debe poder añadirse creando una clase + un shader, **sin modificar el resto de la app**.

---

## 9. Fases de implementación

### Fase 0: Análisis del formato (sin código de render)
**Tareas**
1. Leer todos los XML en `samples/`.
2. Identificar: resolución de composición, screens, slices, nombres, IDs, rectángulos de input/output, puntos de contorno, warp (bezier/perspectiva), máscaras, rotación, flip, visibilidad.
3. Escribir `docs/FORMAT.md` con el esquema real, ejemplos y cualquier ambigüedad.
4. Detectar diferencias entre XML de distintas versiones de Arena si hay más de un ejemplo.

**Criterio de aceptación:** `FORMAT.md` describe cómo obtener, para cada slice, su contorno en espacio de entrada y en espacio de salida. Si falta información en los XML, se lista qué ejemplos adicionales hacen falta.

> No asumas nombres de nodos. Todo lo que figure en este plan sobre el XML es una hipótesis hasta verificarla.

### Fase 1: Proyecto base y cargador
**Tareas**
1. Crear el proyecto oF con la estructura de la sección 4 y compilar "hola mundo" con ImGui.
2. Implementar `ResolumeXmlLoader` según `FORMAT.md`.
3. Cargar por *File → Open* y por drag & drop.
4. Vista previa 2D de la composición con los slices dibujados, seleccionables, con zoom y paneo.
5. Lista lateral de screens/slices con checkbox de habilitado.
6. `ProjectIO`: guardar y cargar proyecto JSON (ruta del XML + bindings + salida + OSC).

**Aceptación:** abrir un XML real y ver todos los slices en su posición correcta, con nombres correctos. Reabrir el proyecto restaura el estado.

### Fase 2: Geometría y renderer de bordes
**Tareas**
1. `Polyline`: remuestreo uniforme, cálculo de longitud acumulada, suavizado de esquinas (`cornerRadius`), offset (`offset`, `alignment`).
2. `BorderMesh`: generación de la tira con atributos `(u, v)`, uniones `miter/round/bevel`, soporte de contornos abiertos y cerrados.
3. Shader `border_solid` con antialias (usar `fwidth` o `smoothstep` sobre `v`).
4. FBO del tamaño de la composición, fondo transparente, premultiplied alpha.
5. Implementar parámetros 6.1, 6.2 (modos `solid` y `gradientPerimeter`) y 6.3 (`hard`, `soft`, `gaussian`).
6. Panel ImGui para editar `StyleParams` del slice seleccionado, del grupo y global, con herencia visible.

**Aceptación:** cada slice muestra un borde correcto con grosor, offset, color y esquinas configurables, sin artefactos en uniones, y a 60 FPS con 50 slices.

### Fase 3: Patrones, animaciones y glow
**Tareas**
1. Patrones 6.5 (`dashed`, `dotted`, `segments`, `ticks`, `wave`, `zigzag`, `chevrons`).
2. Animaciones 6.6 (`pulse`, `blink`, `strobe`, `chase`, `multiChase`, `breathing`, `sweep`, `trail`, `ripple`).
3. Ruido y distorsión 6.7 (`noise`, `glitch`, `jitter`, `flicker`).
4. Glow 6.4: modo `falloff` y modo `bloom` (blur gaussiano separable, resolución reducida configurable para rendimiento).
5. `phaseSpread` entre slices.
6. Perfiles `doubleLine`, `tripleLine` y `coreColor` (neón).
7. `StyleRegistry` con presets iniciales: Sólido, Punteado animado, Chase, Neón, Gradiente rotatorio, Doble línea, Glitch, Latido.
8. Guardar y cargar presets como JSON en `bin/data/presets/`.

**Aceptación:** al menos 8 estilos seleccionables que se ven distintos entre sí, todos con parámetros editables en vivo, y 60 FPS con 50 slices con glow `falloff`.

### Fase 4: Salida a Resolume
**Tareas**
1. Interfaz `IOutput` con `setup`, `send(ofTexture&)`, `close`.
2. `SpoutOutput` (Windows), `SyphonOutput` (Mac), `NdiOutput`.
3. Selector en la GUI, nombre del emisor configurable, resolución y FPS configurables.
4. Verificar el canal alfa en Resolume (capa superior, blend Alpha o Add) y corregir premultiplicación si hay halos oscuros.
5. Indicador de estado de la salida y contador de FPS.

**Aceptación:** Resolume detecta la fuente, la transparencia es correcta y los bordes coinciden con los slices de la composición.

### Fase 5: Modulación, sincronización y OSC
**Tareas**
1. `LFO`, `Envelope`, `BeatClock` con tap tempo, `AudioAnalyzer` con FFT por bandas.
2. `ModulationMatrix` con panel ImGui.
3. Registro de parámetros con rango y tipo, usado por GUI y OSC.
4. `OscServer` con puerto configurable. Direcciones propuestas:
   - `/global/<param>` valor
   - `/slice/<id>/<param>` valor
   - `/group/<nombre>/<param>` valor
   - `/style/<id>` nombre de estilo
   - `/preset/load` nombre
   - `/clock/bpm` valor, `/clock/tap`, `/clock/beat`
   - `/output/enable` 0|1
5. Opcional: salida OSC para informar estado.

**Aceptación:** cambiar color, grosor, estilo y velocidad desde Resolume (o un cliente OSC) en vivo, y que un parámetro reaccione a audio y a BPM.

### Fase 6: Pulido
**Tareas**
1. Recarga en caliente del XML (`FileWatcher`) conservando los bindings por ID/nombre de slice.
2. Recarga en caliente de shaders.
3. Grupos de slices y selección múltiple.
4. Undo/redo básico en la GUI.
5. Modo "performance" (UI oculta, solo salida) y atajos de teclado.
6. Manejo de errores: XML inválido, slice sin contorno, salida no disponible.
7. Empaquetado (Windows: carpeta portable o instalador; Mac: `.app`) y `README.md`.

**Aceptación:** editar el mapping en Resolume, volver a exportar y ver cómo la app actualiza los bordes sin perder los estilos asignados.

---

## 10. Rendimiento

- Regenerar la mesh solo cuando cambia la geometría o parámetros geométricos; la animación se resuelve en shader con uniforms.
- Agrupar slices con el mismo estilo para minimizar cambios de shader.
- Glow `bloom` a media o cuarta resolución, con calidad configurable.
- Objetivo: 60 FPS con 50 slices y glow a 1920x1080; 30+ FPS a 4K.
- Medir con un panel de estadísticas (ms por pasada, vértices, FPS).

---

## 11. Riesgos y decisiones abiertas

| Riesgo | Mitigación |
|---|---|
| El esquema del XML cambia según la versión de Arena | Fase 0 con ejemplos de tu versión; loader tolerante a campos faltantes |
| Slices con warp complejo no coinciden en espacio de entrada | Modo `space = output` y comparar visualmente; evaluar `output` por slice |
| Spout solo en Windows, Syphon solo en Mac | `IOutput` con compilación condicional; NDI como respaldo universal |
| NDI agrega latencia | Recomendar Spout/Syphon cuando estén disponibles |
| Halos oscuros por alfa mal premultiplicado | Trabajar en premultiplied alpha de punta a punta y probar en Resolume temprano |
| Muchos slices con glow `bloom` bajan FPS | Un solo blur sobre todo el FBO, no por slice |

---

## 12. Criterios de aceptación del MVP (Fases 0 a 4)

1. Cargar un XML real de Resolume y mostrar todos los slices correctamente.
2. Aplicar un borde configurable a cada slice con los parámetros de las secciones 6.1 a 6.3.
3. Al menos 8 estilos animados funcionando a 60 FPS, con glow.
4. Resolume recibe la salida con transparencia y los bordes coinciden con los slices.

---

## 13. Reglas de trabajo para Codex

- Un commit por tarea significativa, con mensaje claro.
- Compilar y ejecutar después de cada fase; no avanzar si no compila.
- Código comentado donde la lógica no sea obvia (especialmente geometría y shaders).
- Nada de rutas absolutas; usar `ofToDataPath`.
- Cada estilo nuevo debe incluir su entrada en `docs/STYLES.md` con la lista de parámetros que usa.
- Si falta información (XML de ejemplo, versión de Arena, sistema operativo), **detenerse y pedirla** en lugar de asumir.
- Mantener actualizada la sección "Estado".

---

## 14. Estado

| Fase | Estado | Notas |
|---|---|---|
| 0. Análisis del formato | ☑ completada para el ejemplo disponible | XML de Arena 7.19.1 analizado: 1 screen, 3 slices; extracción de contornos input/output documentada y verificada en `docs/FORMAT.md`. Variantes sin evidencia y ejemplos adicionales listados. Sin código de render; Fase 1 no iniciada. |
| 1. Base y cargador | ☑ completada | Compilacion Release y ejecucion verificadas; XML real con 3 slices en posiciones/nombres correctos, seleccion y zoom comprobados visualmente, guardado/reapertura JSON y resolucion variable comprobados en self-test. |
| 2. Geometría y renderer | ☑ completada | Release ejecutada; grosor/esquinas probados en vivo. Tests de geometria, transparencia, alfa premultiplicado, mezclas e herencia correctos. Benchmark final: 489 FPS del renderer con 50 slices a 1920x1080 (sin limite de FPS; UI separada a 60). |
| 3. Patrones, animaciones y glow | ☑ completada | 8 presets visualmente distintos, 7 animados por defecto; patrones/ruido y perfiles multiples implementados. Bloom y falloff verificados; 236 FPS del renderer con 50 slices y falloff a 1920x1080. |
| 4. Salida a Resolume | ☐ pendiente | |
| 5. Modulación y OSC | ☐ pendiente | |
| 6. Pulido | ☐ pendiente | |

---

## 15. Datos del entorno (completar antes de empezar)

- Sistema operativo: `Windows` (verificado en el entorno de trabajo)
- Versión de Resolume Arena: `7.19.1, revisión 32983 (confirmada por el XML)`
- Versión de openFrameworks: `0.12.1 MSYS2 MINGW64; GCC 16.1.0`
- Resolución de la composición: `1920 x 1080 en el ejemplo actual; variable, leer CurrentCompositionTextureSize en cada XML`
- Cantidad aproximada de slices: `3 en el ejemplo actual, dentro de 1 screen`
- Archivos XML de ejemplo: `ejemplo_advanced.xml en la raíz; samples/ aún no presente`
