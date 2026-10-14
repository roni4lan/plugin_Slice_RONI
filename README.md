# Pluguin Slice RONI RESO

App independiente C++/openFrameworks para bordes sobre slices de Resolume Arena.
El desarrollo sigue PLAN_SliceBorders.md, con aceptación por fase.

## Compilación en Windows

SDK: openFrameworks 0.12.1, distribución MSYS2 MINGW64, en vendor/of.
Interfaz: ofxImGui, rama develop, en vendor/of/addons/ofxImGui.
Las dependencias no se versionan. Fuentes oficiales:

- https://openframeworks.cc/download/
- https://openframeworks.cc/setup/msys2/
- https://github.com/jvcleave/ofxImGui/tree/develop

Con MSYS2 MINGW64 y sus dependencias instaladas, ejecutar:

```powershell
./scripts/build.ps1
```

El script crea y retira un alias temporal de unidad para evitar los espacios
de la carpeta de trabajo en los makefiles de openFrameworks. No mueve el proyecto.
Se puede elegir otra unidad libre con -Drive 'S:'.

## Uso de la base

Abrir el XML con Archivo → Abrir o arrastrarlo a la ventana. La resolución se
lee del XML en cada carga. La rueda hace zoom y el botón central desplaza la vista.
Seleccionar un slice con clic o desde la lista; usar los checkboxes para habilitar
screens y slices. Guardar/reabrir un proyecto JSON conserva los ajustes y una
ruta del XML relativa al proyecto. Mantener ambos archivos juntos al trasladarlos.

El soporte inicial está limitado a la geometría comprobada en docs/FORMAT.md.
Las variantes de warp no verificadas producen un error explícito.

La prueba integrada se ejecuta pasando el XML real y --self-test al ejecutable.
Comprueba carga y restauración de bindings, salida y OSC. Los resultados se
escriben en bin/data/self-test-report.json. La resolución esperada de esta prueba
corresponde al fixture; la aplicación no fija esa resolución.
