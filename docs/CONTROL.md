# Control en vivo

Activar OSC en el panel; puerto inicial 7000. Cliente local: 127.0.0.1.
Los IDs de slices aparecen debajo del nombre seleccionado.

- /global/thickness: float; /global/colorA: cuatro floats RGBA entre 0 y 1.
- /slice/<id>/speed: float; /group/<nombre>/opacity: float.
- /style/<id>: string Solid, Dashed, Chase, Neon, Gradient, DoubleLine, Glitch o Pulse.
- /preset/load: nombre del JSON dentro de data/presets, sin ruta.
- /clock/bpm: float 20–300; /clock/tap y /clock/beat: disparadores.
- /clock/phase: fase numérica para syncMode=osc.
- /output/enable: 0 o 1.
- /mod/<id>: float, disponible como fuente OSC de la matriz.
- /envelope/<id>: disparar AD; /envelope/<id>/gate: 0 o 1 para ADSR.

La matriz suma las rutas antes de limitar el valor al rango del parámetro.
El proyecto conserva valores base; la modulación no sobrescribe esos valores.
LFO, Beat, Envelope, Audio y OSC pueden controlar parámetros numéricos,
incluidos los componentes colorA.r/g/b/a. El audio usa la entrada elegida,
FFT de 1024 muestras y cinco bandas; elegir el dispositivo que reciba música.
T aplica tap tempo. BPM, divisiones y audio tienen controles en la interfaz.
