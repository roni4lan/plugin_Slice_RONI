# Pluguin Slice RONI RESO

Aplicación independiente para Windows que carga el mapping de Resolume Arena
7.19.1 y genera bordes con transparencia. La resolución se lee del XML; admite
cambios de composición. Desarrollo y validación por fases en PLAN_SliceBorders.md.

## Empezar

1. Extraer toda la carpeta portable y abrir PluguinSliceRONIRESO.exe.
2. Arrastrar tu XML de Advanced Output o usar Archivo → Abrir. Sin argumentos,
   la versión portable abre el ejemplo incluido.
3. Seleccionar un slice y elegir Solid, Dashed, Chase, Neon, Gradient,
   DoubleLine, Glitch o Pulse. Ajustar grosor, colores, patrón, animación y halo.
4. En Salida a Resolume elegir Spout y activar Enviar textura. En Arena buscar
   RONI RESO en Sources → Spout y colocarlo sobre el contenido, con mezcla Alpha
   o Add. Usar el mismo mapping y tamaño de composición en ambos programas.
5. Guardar un proyecto JSON para recuperar estilos, grupos, salida y controles.
   El JSON referencia al XML por ruta relativa: conservar ambos al trasladarlos.

NDI es una alternativa con mayor costo de CPU y compresión. Su alfa usa RGBA
directo; Spout permite elegir directo o premultiplicado. Ver docs/OUTPUT.md.

## Controles

Rueda: zoom. Botón central: desplazar. F: ajustar vista. Ctrl+clic: selección
múltiple, con botones para habilitar, aplicar estilo o asignar grupo a todos.
Crear grupos desde el panel del slice; sus parámetros se heredan y se pueden
sobrescribir por slice. Ctrl+O abre; Ctrl+S guarda; Ctrl+Z deshace; Ctrl+Y o
Ctrl+Shift+Z rehace. T marca tap tempo. F12 alterna performance con la interfaz
oculta y la salida activa. R recarga el mapping manualmente.

Al volver a exportar sobre el XML cargado, la app espera que la escritura se
estabilice y actualiza el contorno conservando los bindings por ID. Si Arena
cambia un ID, usa el nombre únicamente cuando es único en ambos mappings.
Si el archivo es inválido, mantiene la última versión válida y muestra el error.
Los shaders de data/shaders también se recargan: un error conserva los programas
anteriores. Guardar el proyecto conserva los cambios del operador.

La matriz admite LFO, envolventes AD/ADSR, Beat, bandas de audio y OSC.
Seleccionar el dispositivo de entrada de música y activar audio para utilizar
sus bandas. Configurar BPM y divisiones o marcar T para sincronizar pulsos.
Activar OSC en el puerto elegido; instrucciones y direcciones en docs/CONTROL.md.

## Compatibilidad comprobada

Windows x64, OpenGL 3.3, Arena 7.19.1. XML real: tres slices rectangulares;
Spout, NDI, transparencia, resolución variable y recarga validados. El formato
soportado y las variantes que requieren ejemplos se detallan en docs/FORMAT.md:
warps complejos y otras capas no verificadas se rechazan con un mensaje.
Syphon tiene integración condicional y falta compilar y probar la versión Mac.

## Desarrollo y pruebas

Fuentes C++17/openFrameworks 0.12.1, ofxImGui y ofxOsc. Dependencias en vendor/
no versionadas; instrucciones en docs/BUILD.md. Compilar con:

```powershell
./scripts/build.ps1 -AppName PluguinSliceRONIRESO
./scripts/package.ps1
```

Desde la carpeta portable, ejecutar la prueba integrada:

```powershell
./PluguinSliceRONIRESO.exe data/ejemplo_advanced.xml --self-test
```

El reporte se guarda en data/self-test-report.json. La prueba exige los tres
slices del fixture, mientras la aplicación usa las dimensiones del XML cargado.
Ver docs/VALIDATION.md y licenses/ para evidencia y avisos de dependencias.
