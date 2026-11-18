# NV Container Fuck Off

**A simple ON/OFF button for NVIDIA Display Container LS.**  
**by HXPRO · v1.1.1 · Windows 10/11 x64 · Unofficial · MIT**

## For everyone: one file, two buttons

**Download `NVContainerFuckOff.exe` from [Releases](../../releases), not the GitHub "Code → Download ZIP" source archive.**

1. Download `NVContainerFuckOff.exe` from the latest Release.
2. Open it with a double-click. Windows will ask for administrator approval (UAC); this is required to manage the service.
3. Click **TURN OFF** to stop and disable the service, or **TURN ON** to enable and start it.
4. Use **REFRESH STATUS** to read the current state.

No terminal, CMD, PowerShell, installer, or separate .NET runtime needed. Only the Windows display container service `NVDisplay.ContainerLocalSystem` is managed. The similarly named NVIDIA App `NvContainerLocalSystem` service is **never** modified.

**Important:** This program is **unsigned**. Some antivirus products or Microsoft SmartScreen may warn about unsigned software that requests administrator privileges. **Do not disable security protection or bypass a malware detection.** Review the public source and confirm a trusted distribution path; submit suspected false positives to your antivirus vendor.

## Why this exists

Some NVIDIA driver releases have been associated with excessive log writes to SSDs. This program makes it easy to switch the NVIDIA Display Container LS service off and back on. It does not repair the underlying driver or guarantee that all NVIDIA-related disk writes stop.

**Possible side effects when OFF:** NVIDIA Control Panel, tray icon, and some display-management settings may become unavailable. The main GPU driver, CUDA, DirectX, Vulkan, and OpenGL normally remain operative, but verify on your own computer. Do not use the toggle during driver installation or upgrades; these may reinstall and reactivate the service.

## Transparency and security

The full C# application source is included **for inspection and transparency**; users do not need to modify or compile it. The project does not solicit contributions. The app:

- Uses Windows Forms and Windows Service Control Manager APIs.
- Performs no network requests, telemetry, self-updating, registry edits, driver file deletion, or logging.
- Does not invoke `powershell.exe`, `cmd.exe`, or `sc.exe`.
- Requests administrator access in its application manifest; cannot silently bypass UAC.
- Changes the startup type and running state of **one explicitly named service only**.
- Shows a clear error if the service is missing.

The name *HXPRO* is an author credit, **not an Authenticode digital signature**. This source can be audited independently. No source distribution can guarantee that an untrusted third-party EXE is safe.

## Maintainer: build the EXE for GitHub Releases

The public GitHub Actions workflow builds a **self-contained single-file Windows x64 EXE**. This development step is not required for end users.

1. Create a GitHub repository and upload the **contents** of this source folder, including `.github/workflows/build-windows.yml`.
2. Open **Actions → Build NV Container Fuck Off for Windows → Run workflow** (or push to `main`).
3. Once the build succeeds, download the `NVContainerFuckOff-Windows-x64` artifact. Test `NVContainerFuckOff.exe` on Windows with security software enabled.
4. For the public release, create and push a version tag (for example `v1.1.1`); the workflow attaches the EXE and its SHA-256 checksum to the matching **GitHub Release**. If you use the GitHub Releases UI to create the tag, review the workflow run and attach the artifact yourself if needed.

Build locally on a Windows development machine if desired:

```powershell
dotnet publish NVContainerFuckOff.csproj -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true -o publish
```

The EXE is not bundled in this **source archive**. It must be compiled on a Windows build runner. Builds should be tested on a real Windows PC before public distribution.

## Author and license

Copyright (c) 2026 **HXPRO**. MIT License. Not affiliated with NVIDIA.
