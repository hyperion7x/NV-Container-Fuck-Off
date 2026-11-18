// NV Container Fuck Off - HXPRO - v1.1.1
// Uses Windows Service Control Manager APIs; no shell, PowerShell, hidden processes,
// execution-policy changes, downloads, logging, or background persistence.
using System.ComponentModel;
using System.Drawing;
using System.Runtime.InteropServices;
using System.ServiceProcess;
using System.Windows.Forms;

namespace NVContainerFuckOff;

internal static class Program
{
    [STAThread]
    private static void Main()
    {
        ApplicationConfiguration.Initialize();
        Application.Run(new ToggleForm());
    }
}

internal sealed class ToggleForm : Form
{
    private const string Target = "NVDisplay.ContainerLocalSystem";
    private readonly Label status = new();
    private readonly Label detail = new();
    private readonly Button offButton = new();
    private readonly Button onButton = new();
    private readonly Button refreshButton = new();

    public ToggleForm()
    {
        Text = "NV Container Fuck Off | HXPRO";
        StartPosition = FormStartPosition.CenterScreen;
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        ClientSize = new Size(570, 334);
        Font = new Font("Segoe UI", 10f);
        BackColor = Color.FromArgb(19, 26, 36);
        ForeColor = Color.White;

        AddLabel("NV CONTAINER FUCK OFF", 20, 15, 530, 38, 19f, true, Color.White);
        AddLabel("NVIDIA Display Container LS - service switch", 22, 57, 526, 27, 10f, false, Color.LightGray);

        var panel = new Panel { Bounds = new Rectangle(20, 99, 530, 95), BackColor = Color.FromArgb(32, 43, 58) };
        Controls.Add(panel);
        AddLabel("CURRENT STATUS", 13, 7, 500, 22, 9f, true, Color.LightGray, panel);
        status.Bounds = new Rectangle(13, 30, 500, 35);
        status.Font = new Font("Segoe UI", 15f, FontStyle.Bold);
        status.ForeColor = Color.Khaki;
        panel.Controls.Add(status);
        detail.Bounds = new Rectangle(13, 70, 505, 22);
        detail.Font = new Font("Segoe UI", 9f);
        detail.ForeColor = Color.LightGray;
        panel.Controls.Add(detail);

        ConfigureButton(offButton, "TURN OFF", 20, 211, 256, 52, Color.FromArgb(166, 54, 66));
        ConfigureButton(onButton, "TURN ON", 294, 211, 256, 52, Color.FromArgb(32, 126, 98));
        ConfigureButton(refreshButton, "REFRESH STATUS", 20, 277, 172, 35, Color.FromArgb(60, 77, 98));
        AddLabel("by HXPRO  |  v1.1.1  |  Unofficial", 207, 280, 340, 28, 9f, false, Color.LightGray);
        offButton.Click += (_, _) => SetState(false);
        onButton.Click += (_, _) => SetState(true);
        refreshButton.Click += (_, _) => RefreshState();
        Shown += (_, _) => RefreshState();
    }

    private void AddLabel(string text, int x, int y, int w, int h, float size,
        bool bold, Color color, Control? parent = null)
    {
        var label = new Label
        {
            Text = text,
            Bounds = new Rectangle(x, y, w, h),
            ForeColor = color,
            Font = new Font("Segoe UI", size, bold ? FontStyle.Bold : FontStyle.Regular)
        };
        (parent ?? this).Controls.Add(label);
    }

    private void ConfigureButton(Button button, string text, int x, int y, int w, int h, Color color)
    {
        button.Text = text;
        button.Bounds = new Rectangle(x, y, w, h);
        button.BackColor = color;
        button.ForeColor = Color.White;
        button.FlatStyle = FlatStyle.Flat;
        button.FlatAppearance.BorderSize = 0;
        button.Font = new Font("Segoe UI", 11f, FontStyle.Bold);
        button.UseVisualStyleBackColor = false;
        Controls.Add(button);
    }

    private void RefreshState()
    {
        try
        {
            using var svc = new ServiceController(Target);
            var state = svc.Status;
            var startType = svc.StartType;
            bool off = state == ServiceControllerStatus.Stopped && startType == ServiceStartMode.Disabled;
            bool on = state == ServiceControllerStatus.Running && startType == ServiceStartMode.Automatic;
            status.Text = off ? "OFF - Stopped / Disabled"
                : on ? "ON - Running / Automatic" : $"CUSTOM - {state} / {startType}";
            status.ForeColor = off ? Color.LightGreen : Color.Khaki;
            detail.Text = $"Service: {Target}";
            offButton.Enabled = !off;
            onButton.Enabled = !on;
        }
        catch (InvalidOperationException)
        {
            status.Text = "SERVICE NOT FOUND";
            status.ForeColor = Color.Salmon;
            detail.Text = "Finish installing the NVIDIA display driver first.";
            offButton.Enabled = false;
            onButton.Enabled = false;
        }
        catch (Exception ex)
        {
            status.Text = "UNABLE TO READ SERVICE";
            status.ForeColor = Color.Salmon;
            detail.Text = ex.Message;
            offButton.Enabled = false;
            onButton.Enabled = false;
        }
    }

    private void SetState(bool enable)
    {
        if (!enable && MessageBox.Show(this,
            "Turn OFF NVIDIA Display Container LS?\n\nNVIDIA Control Panel, the tray icon and some display settings may stop working.\nYou can restore the service with TURN ON.",
            "Confirm TURN OFF", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) != DialogResult.Yes)
            return;

        offButton.Enabled = onButton.Enabled = refreshButton.Enabled = false;
        UseWaitCursor = true;
        status.Text = "APPLYING CHANGE...";
        Application.DoEvents();
        try
        {
            using var svc = new ServiceController(Target);
            _ = svc.Status; // Explicitly fail if the service is absent.
            ServiceConfiguration.SetStartupType(Target, enable);
            svc.Refresh();
            if (enable)
            {
                if (svc.Status == ServiceControllerStatus.Stopped) svc.Start();
                if (svc.Status != ServiceControllerStatus.Running)
                    svc.WaitForStatus(ServiceControllerStatus.Running, TimeSpan.FromSeconds(20));
            }
            else
            {
                if (svc.Status == ServiceControllerStatus.Running || svc.Status == ServiceControllerStatus.Paused)
                    svc.Stop();
                if (svc.Status != ServiceControllerStatus.Stopped)
                    svc.WaitForStatus(ServiceControllerStatus.Stopped, TimeSpan.FromSeconds(20));
            }
        }
        catch (Exception ex)
        {
            MessageBox.Show(this, ex.Message, "Service operation failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
        finally
        {
            UseWaitCursor = false;
            refreshButton.Enabled = true;
            RefreshState();
        }
    }
}

internal static class ServiceConfiguration
{
    private const uint ScManagerConnect = 0x0001;
    private const uint ServiceChangeConfig = 0x0002;
    private const uint NoChange = 0xFFFFFFFF;
    private const uint AutoStart = 0x00000002;
    private const uint Disabled = 0x00000004;

    public static void SetStartupType(string serviceName, bool enable)
    {
        IntPtr manager = OpenSCManager(null, null, ScManagerConnect);
        if (manager == IntPtr.Zero) throw new Win32Exception(Marshal.GetLastWin32Error(), "Cannot access Service Control Manager");
        try
        {
            IntPtr service = OpenService(manager, serviceName, ServiceChangeConfig);
            if (service == IntPtr.Zero) throw new Win32Exception(Marshal.GetLastWin32Error(), "Cannot open target service");
            try
            {
                bool ok = ChangeServiceConfig(service, NoChange, enable ? AutoStart : Disabled,
                    NoChange, null, null, IntPtr.Zero, null, null, null, null);
                if (!ok) throw new Win32Exception(Marshal.GetLastWin32Error(), "Cannot change service startup type");
            }
            finally { CloseServiceHandle(service); }
        }
        finally { CloseServiceHandle(manager); }
    }

    [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    private static extern IntPtr OpenSCManager(string? machineName, string? databaseName, uint access);

    [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    private static extern IntPtr OpenService(IntPtr manager, string serviceName, uint access);

    [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool ChangeServiceConfig(IntPtr service, uint serviceType,
        uint startType, uint errorControl, string? binaryPath, string? loadOrderGroup,
        IntPtr tagId, string? dependencies, string? account, string? password, string? displayName);

    [DllImport("advapi32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool CloseServiceHandle(IntPtr handle);
}
