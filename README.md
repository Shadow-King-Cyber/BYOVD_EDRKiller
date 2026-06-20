# BYOVD EDRKiller

## Disclaimer
⚠️ This project is provided exclusively for educational purposes and is intended to be used only in authorized environments. You may only run or deploy this project on systems you own or have explicit, documented permission to test. Any unauthorized use of this project against systems without consent is strictly prohibited and may be illegal.

By using this project, you agree to use it responsibly and ethically. The author assumes no liability for misuse or any consequences arising from the use of this project.

Tested on:
- Windows 11 (24H2)
- Windows Server 2022 (21H2)

---

## Project Structure

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

1. **Write** → Escribe el driver vulnerable en `C:\Windows\System32\Drivers\<FILE>`
2. **Load** → Registra el driver como servicio y lo inicia (SCM API)
3. **Kill loop** → Enumeración de procesos EDR mediante `NtQuerySystemInformation` + envío de IOCTL en loop hasta presionar `q`
4. **Cleanup** → Detiene el servicio, elimina el driver y borra el archivo .sys

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

Para clonar el repositorio y navegar el código:

```bash
git clone https://github.com/0xJs/BYOVD_EDRKiller.git
cd BYOVD_EDRKiller
code .
```

VS Code permite ver la estructura del proyecto, hacer búsquedas de texto en todos los archivos, y resaltar sintaxis de C.

### OpenCode — Revisión de código asistida por IA

[OpenCode](https://opencode.ai) es una herramienta CLI interactiva que permite revisar, entender y modificar el código del repositorio mediante comandos conversacionales. Se ejecuta directamente en la terminal:

```bash
opencode
```

OpenCode puede analizar la estructura del proyecto, identificar errores de compilación, y aplicar correcciones con explicaciones detalladas.

### Visual Studio 2022 — Compilación

Cada proyecto incluye su propio archivo `.vcxproj` para Visual Studio 2022. Para compilar desde la IDE:

1. Abrir `BYOVD_EDRKiller-main` como carpeta o agregar el `.vcxproj` deseado
2. Seleccionar configuración **Release** y plataforma **x64**
3. Build → Build Solution (Ctrl+Shift+B)

### Compilación desde cmd

Requisito: **Visual Studio 2022 Build Tools** o **Visual Studio 2022 Community** instalado.

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
