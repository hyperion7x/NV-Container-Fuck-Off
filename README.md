# NV Container Fuck Off

**Native Win32 ON/OFF utility for NVIDIA Display Container LS**  
**by HXPRO · v1.2.0 · Windows 10/11 x64 · MIT · Unofficial**

A tiny portable Windows program with two buttons, **TURN OFF** and **TURN ON**. Designed for people who don't want to use PowerShell.

## Download and use

Download the **compiled `NVContainerFuckOff.exe` from [Releases](../../releases)**. The GitHub **Code → Download ZIP** option downloads the *source code*, not the ready-to-run EXE.

1. Download the `NVContainerFuckOff.exe` release asset.
2. Double-click it and approve the standard Windows administrator (UAC) prompt.
3. Use **TURN OFF** to stop and disable the NVIDIA Display Container LS service, or **TURN ON** to enable and start it.
4. Use **REFRESH STATUS** to inspect the current state. The app does not stay running in the background after you close it.

No .NET runtime, installer, PowerShell, CMD launcher, network access or background component is required for end users.

### What it changes

The utility exclusively manages this Windows service:

`NVDisplay.ContainerLocalSystem` (NVIDIA Display Container LS)

It **never** modifies `NvContainerLocalSystem` (the separate NVIDIA App container), other services, NVIDIA driver files, registry keys, antivirus settings or system security settings.

When **OFF**, NVIDIA Control Panel, the tray icon and certain display-management features may be unavailable. Standard graphics and GPU compute normally remain functional, but results can vary by driver and configuration. NVIDIA driver updates can recreate or restart the service; use **REFRESH STATUS** to check it again.

This tool is a reversible workaround for the service, **not a driver bug fix**. It cannot guarantee stopping every NVIDIA-related disk write or preventing SSD wear. Do not toggle while a driver installation is in progress.

## Transparency and safety

- Public native C/Win32 source: `main.c`.
- Uses Windows Service Control Manager API with one hardcoded target service.
- UAC elevation is declared in the bundled `app.manifest` (no hidden elevation trick).
- No PowerShell, shell calls, telemetry, networking, startup tasks, self-updater or service installation.
- Clearly reports missing services and Windows errors.
- GitHub Actions compiles the application using MSVC on a hosted Windows runner.
- SHA-256 file lets users check that their downloaded EXE matches the published checksum.

**Unsigned binary:** The name HXPRO credits the author; it is **not** a code-signing certificate. Windows SmartScreen or security products may warn about an unsigned program that modifies a service. Never disable antivirus or bypass a malware detection just to run it. Inspect the source and, if necessary, submit the file to the antivirus vendor for analysis.

## Build for maintainers

The **public source is provided for transparency**. End users do not need to build or contribute to the code.

The native `.exe` is built by GitHub Actions:

1. Commit the contents of this repository, including `.github/workflows/build-windows.yml`.
2. Open **Actions → Build Native NV Container Fuck Off for Windows → Run workflow** (or push to `main`).
3. Open the completed run and download artifact **`NVContainerFuckOff-Native-Windows-x64`**.
4. Test the EXE on a Windows 10/11 x64 PC with antivirus enabled. Check the ON/OFF actions, service status, and other NVIDIA features you rely on.
5. Create GitHub Release `v1.2.0` and attach `NVContainerFuckOff.exe` and `NVContainerFuckOff.exe.sha256` from the artifact.

Optional local build: install Visual Studio C++ Build Tools with Windows SDK and run `build.cmd` in the project folder. Builds with Microsoft `cl.exe` and `rc.exe`; no dependencies are downloaded by the build script.

## License

MIT License. Copyright (c) 2026 HXPRO. Unofficial project, not affiliated with NVIDIA.
