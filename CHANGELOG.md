# Changelog

## v1.2.0 — Native edition (HXPRO)

- Replaced .NET WinForms application (approximately 146 MB self-contained) with native Windows C/Win32 application.
- New MSVC GitHub Actions workflow builds a small single-file x64 EXE.
- Preserved ON/OFF controls, explicit service name, status checking, administrator UAC, warning and error display.
- No runtime dependencies, shell scripts, PowerShell or installers in the end-user release.
- The change preserves only the functionality of the earlier v1.1.1 release; users should test before deployment.

## v1.1.1

- Windows Forms toggle with public C# source (previous edition).
