param(
	[Parameter(Position = 0)]
	[ValidateSet("play", "build", "test", "smoke", "verify", "all")]
	[string]$Command = "play"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = $PSScriptRoot
if (-not $ProjectRoot) {
	$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
}
$ProjectFile = Join-Path $ProjectRoot "AFTERLIGHT.uproject"
$Map = "/Game/Environments/Slice01/L_Slice01_Greybox"
$EngineRoot = "D:\Epic Games\UE_5.8"
$Editor = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor.exe"
$LogFile = Join-Path $ProjectRoot "Saved\Logs\AFTERLIGHT.log"

function Get-AfterlightEditorProcess {
	Get-CimInstance Win32_Process -ErrorAction SilentlyContinue |
		Where-Object {
			$_.Name -eq "UnrealEditor.exe" -and
			$_.CommandLine -and
			$_.CommandLine -like "*AFTERLIGHT.uproject*"
		}
}

function Show-EditorWindow([int]$ProcessId) {
	Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public static class AfterlightWin32 {
	[DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
	[DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
}
"@ -ErrorAction SilentlyContinue
	$proc = Get-Process -Id $ProcessId -ErrorAction SilentlyContinue
	if ($proc -and $proc.MainWindowHandle -ne [IntPtr]::Zero) {
		[AfterlightWin32]::ShowWindow($proc.MainWindowHandle, 9) | Out-Null
		[AfterlightWin32]::SetForegroundWindow($proc.MainWindowHandle) | Out-Null
	}
}

function Pin-AfterlightPlayWindow([int]$ProcessId) {
	Add-Type -TypeDefinition @"
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class AfterlightPin {
	public delegate bool EnumProc(IntPtr hWnd, IntPtr lParam);
	[DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
	[DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
	[DllImport("user32.dll")] public static extern bool MoveWindow(IntPtr h, int x, int y, int w, int ht, bool repaint);
	[DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int n);
	[DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
	[DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
	[DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
	[DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
	[DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
	[DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
	[DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
	public struct RECT { public int L, T, R, B; }
}
"@ -ErrorAction SilentlyContinue
	try { [AfterlightPin]::SetProcessDPIAware() | Out-Null } catch {}
	Add-Type -AssemblyName System.Windows.Forms -ErrorAction SilentlyContinue
	$work = [System.Windows.Forms.Screen]::PrimaryScreen.WorkingArea
	$targetX = $work.X + 80
	$targetY = $work.Y + 72
	$editor = Get-Process -Id $ProcessId -ErrorAction SilentlyContinue
	if ($editor -and $editor.MainWindowHandle -ne [IntPtr]::Zero) {
		[AfterlightPin]::ShowWindow($editor.MainWindowHandle, 9) | Out-Null
	}
	$script:pinPid = [uint32]$ProcessId
	$script:preview = [IntPtr]::Zero
	$cb = [AfterlightPin+EnumProc] {
		param($h, $l)
		[uint32]$procId = 0
		[AfterlightPin]::GetWindowThreadProcessId($h, [ref]$procId) | Out-Null
		if ($procId -ne $script:pinPid) { return $true }
		$sbC = New-Object System.Text.StringBuilder 64
		$sbT = New-Object System.Text.StringBuilder 256
		[AfterlightPin]::GetClassName($h, $sbC, 64) | Out-Null
		[AfterlightPin]::GetWindowText($h, $sbT, 256) | Out-Null
		if ($sbC.ToString() -ne "UnrealWindow") { return $true }
		$title = $sbT.ToString()
		if ($title -match "Unreal Editor" -and $title -notmatch "Preview") { return $true }
		$script:preview = $h
		return $true
	}
	[AfterlightPin]::EnumWindows($cb, [IntPtr]::Zero) | Out-Null
	if ($script:preview -eq [IntPtr]::Zero) { return }
	[AfterlightPin]::ShowWindow($script:preview, 9) | Out-Null
	[AfterlightPin]::ShowWindow($script:preview, 5) | Out-Null
	[AfterlightPin]::MoveWindow($script:preview, $targetX, $targetY, 854, 510, $true) | Out-Null
	[AfterlightPin]::SetForegroundWindow($script:preview) | Out-Null
	Write-Host ("Play window pinned to primary display at {0},{1} (854x480)." -f $targetX, $targetY)
}

function Show-LogTail {
	if (Test-Path $LogFile) {
		Write-Host ""
		Write-Host "Last log lines ($LogFile):"
		Get-Content $LogFile -Tail 30
	}
}

function Test-CurrentSessionLog([datetime]$LaunchTime) {
	if (-not (Test-Path $LogFile)) {
		return $false
	}
	$first = Get-Content $LogFile -TotalCount 1 -ErrorAction SilentlyContinue
	if ($first -notmatch 'Log file open, (\d{2}/\d{2}/\d{2} \d{2}:\d{2}:\d{2})') {
		return $false
	}
	$openTime = [datetime]::ParseExact($Matches[1], 'MM/dd/yy HH:mm:ss', [cultureinfo]::InvariantCulture)
	return $openTime -ge $LaunchTime.AddSeconds(-8)
}

function Test-Slice01Loaded {
	if (-not (Test-Path $LogFile)) {
		return $false
	}
	$hits = Select-String -Path $LogFile -Pattern "MAP LOAD FILE=.*L_Slice01_Greybox\.umap|registered with world 'L_Slice01_Greybox'" -ErrorAction SilentlyContinue
	return [bool]$hits
}

function Get-LaunchFailureReason {
	if (-not (Test-Path $LogFile)) {
		return $null
	}
	$hits = Select-String -Path $LogFile -Pattern "E_OUTOFMEMORY|Ran out of memory|paging file is too small|Out of video memory|Fatal error:" -ErrorAction SilentlyContinue
	if ($hits) {
		return ($hits | Select-Object -Last 1).Line.Trim()
	}
	return $null
}

function Fail-Launch([string]$Message) {
	Write-Host "FAIL: $Message"
	$reason = Get-LaunchFailureReason
	if ($reason) {
		Write-Host "Log: $reason"
	}
	Show-LogTail
	exit 1
}

if (-not (Test-Path $ProjectFile)) {
	Write-Error "AFTERLIGHT.uproject was not found at $ProjectFile"
	exit 1
}
if (-not (Test-Path $Editor)) {
	Write-Error "Unreal Editor 5.8.2 was not found at $Editor"
	exit 1
}

$EditorCmd = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$BuildBat = Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat"
$LabMap = "/Game/Environments/Slice01/L_Dev_CinematicLab"

function Invoke-AfterlightBuild {
	Write-Host "Building AFTERLIGHTEditor Win64 Development..."
	& $BuildBat AFTERLIGHTEditor Win64 Development "-Project=$ProjectFile" -NoPCH -MaxParallelActions=1
	if ($LASTEXITCODE -ne 0) {
		throw "AFTERLIGHT build failed (exit $LASTEXITCODE)."
	}
	Write-Host "BUILD OK"
}

function Invoke-AfterlightAutomation {
	$log = Join-Path $ProjectRoot "Saved\Logs\AfterlightAutomation.log"
	Write-Host "Running Afterlight automation..."
	& $EditorCmd $ProjectFile -unattended -nop4 -nosplash -NullRHI -nosound -log -abslog="$log" -ExecCmds="Automation RunTests Afterlight; Quit" -TestExit="Automation Test Queue Empty" -ReportOutputPath=(Join-Path $ProjectRoot "Saved\Automation") | Out-Null
	if ($LASTEXITCODE -ne 0) {
		throw "AFTERLIGHT automation failed (exit $LASTEXITCODE)."
	}
	Write-Host "TEST OK"
}

function Invoke-AfterlightSmoke {
	$sliceLog = Join-Path $ProjectRoot "Saved\Logs\AfterlightSliceSmoke.log"
	$labLog = Join-Path $ProjectRoot "Saved\Logs\AfterlightSmoke.log"
	Write-Host "Running Slice01 smoke..."
	& $EditorCmd $ProjectFile $Map -game -unattended -nop4 -nosplash -NullRHI -nosound -log -abslog="$sliceLog" -AfterlightSliceSmoke -ExecCmds="DisableAllScreenMessages"
	if ($LASTEXITCODE -ne 0) {
		throw "Slice01 smoke failed (exit $LASTEXITCODE)."
	}
	Write-Host "Running Lab smoke..."
	& $EditorCmd $ProjectFile $LabMap -game -unattended -nop4 -nosplash -NullRHI -nosound -log -abslog="$labLog" -AfterlightSmoke -ExecCmds="DisableAllScreenMessages"
	if ($LASTEXITCODE -ne 0) {
		throw "Lab smoke failed (exit $LASTEXITCODE)."
	}
	Write-Host "SMOKE OK"
}

switch ($Command) {
	"play" {
		Write-Host "AFTERLIGHT"
		Write-Host "Editor:  $Editor"
		Write-Host "Project: $ProjectFile"
		Write-Host "Map:     $Map"
		Write-Host ""

		$os = Get-CimInstance Win32_OperatingSystem
		$freeGB = [math]::Round($os.FreePhysicalMemory / 1MB, 2)
		$totalGB = [math]::Round($os.TotalVisibleMemorySize / 1MB, 2)
		$virtFreeGB = [math]::Round($os.FreeVirtualMemory / 1MB, 2)
		$virtTotalGB = [math]::Round($os.TotalVirtualMemorySize / 1MB, 2)
		$commitGB = [math]::Round(($os.TotalVirtualMemorySize - $os.FreeVirtualMemory) / 1MB, 2)
		$pf = Get-CimInstance Win32_PageFileUsage -ErrorAction SilentlyContinue
		Write-Host ("Physical RAM: {0} / {1} GB free" -f $freeGB, $totalGB)
		Write-Host ("Commit:       {0} / {1} GB used (free virtual {2} GB)" -f $commitGB, $virtTotalGB, $virtFreeGB)
		if ($pf) {
			Write-Host ("Pagefile:     {0}  allocated {1} MB  used {2} MB  system-managed" -f $pf.Name, $pf.AllocatedBaseSize, $pf.CurrentUsage)
		}
		if ($freeGB -lt 3 -or $virtFreeGB -lt 8) {
			Write-Host "WARN: Commit/RAM headroom is low for Unreal Editor on this laptop."
		}

		$existing = @(Get-AfterlightEditorProcess)
		if ($existing.Count -gt 0) {
			$pidExisting = $existing[0].ProcessId
			Write-Host "Unreal Editor is already running (PID $pidExisting)."
			Show-EditorWindow -ProcessId $pidExisting
			if (Test-Slice01Loaded) {
				Write-Host "OK: Unreal Editor is already running (PID $pidExisting). Slice01 is loaded."
				Write-Host "The game is not playing yet."
				Write-Host "Press Alt+P to start AFTERLIGHT in a New Editor Window (854x480)."
				exit 0
			}
			Write-Host "Unreal Editor is running, but Slice01 is not confirmed in the log yet."
			Write-Host "Wait for the viewport, then press Alt+P to start AFTERLIGHT in a New Editor Window."
			exit 0
		}

		Write-Host "Launching Unreal Editor 5.8.2..."
		$savedPlay = Join-Path $ProjectRoot "Saved\Config\WindowsEditor\EditorPerProjectUserSettings.ini"
		if (Test-Path $savedPlay) {
			$ini = Get-Content $savedPlay -Raw
			$ini = $ini -replace "LastExecutedPlayModeLocation=.*", "LastExecutedPlayModeLocation=PlayLocation_NewWindow"
			if ($ini -notmatch "LastExecutedPlayModeType=") {
				$ini = $ini -replace "\[/Script/UnrealEd\.LevelEditorPlaySettings\]", "[/Script/UnrealEd.LevelEditorPlaySettings]`r`nLastExecutedPlayModeType=PlayMode_InEditorFloating"
			} else {
				$ini = $ini -replace "LastExecutedPlayModeType=.*", "LastExecutedPlayModeType=PlayMode_InEditorFloating"
			}
			$ini = $ini -replace "NewWindowWidth=.*", "NewWindowWidth=854"
			$ini = $ini -replace "NewWindowHeight=.*", "NewWindowHeight=480"
			$ini = $ini -replace "ClientWindowWidth=.*", "ClientWindowWidth=854"
			$ini = $ini -replace "ClientWindowHeight=.*", "ClientWindowHeight=480"
			$ini = $ini -replace "LastSize=.*", "LastSize=(X=854,Y=480)"
			$ini = $ini -replace "CenterNewWindow=.*", "CenterNewWindow=True"
			$ini = $ini -replace "bShouldMinimizeEditorOnNonVRPIE=.*", "bShouldMinimizeEditorOnNonVRPIE=False"
			Set-Content -Path $savedPlay -Value $ini -NoNewline
			Write-Host "Play settings: New Editor Window 854x480."
		}
		$launchTime = Get-Date
		try {
			$proc = Start-Process -FilePath $Editor -ArgumentList @(
				"`"$ProjectFile`"",
				$Map,
				"-ForceLogFlush",
				"-NoLiveCoding",
				"-NoSourceControl",
				"-nosplash",
				"-dx11",
				"-dpcvars=r.Streaming.PoolSize=64,r.ScreenPercentage=50,sg.ViewDistanceQuality=0,sg.AntiAliasingQuality=0,sg.ShadowQuality=0,sg.GlobalIlluminationQuality=0,sg.ReflectionQuality=0,sg.PostProcessQuality=0,sg.TextureQuality=0,sg.EffectsQuality=0,sg.FoliageQuality=0,sg.ShadingQuality=0,r.Lumen.DiffuseIndirect.Allow=0,r.Shadow.Virtual.Enable=0,r.Nanite=0,r.GenerateMeshDistanceFields=0,r.DefaultFeature.MotionBlur=0,r.DefaultFeature.Bloom=0,r.BloomQuality=0,r.AmbientOcclusionLevels=0,r.LightFunctionQuality=0,Afterlight.Camera.AllowDOF=0,Afterlight.QaAuto=1,Afterlight.QaDrive=1",
				"-AfterlightAutoPlay",
				"-ExecCmds=DisableAllScreenMessages"
			) -WorkingDirectory $ProjectRoot -PassThru
		}
		catch {
			Fail-Launch "Start-Process threw: $($_.Exception.Message)"
		}

		if (-not $proc) {
			Fail-Launch "Start-Process did not return a process."
		}

		Start-Sleep -Seconds 4
		if ($proc.HasExited) {
			Fail-Launch "Unreal Editor exited immediately (code $($proc.ExitCode))."
		}

		Write-Host "Started PID $($proc.Id). Waiting for Slice01 to load..."
		$deadline = (Get-Date).AddSeconds(180)
		$mapLoaded = $false
		while ((Get-Date) -lt $deadline) {
			Start-Sleep -Seconds 5
			if ($proc.HasExited) {
				Fail-Launch "Unreal Editor exited before the map finished loading (code $($proc.ExitCode))."
			}
			if (Test-CurrentSessionLog -LaunchTime $launchTime) {
				$reason = Get-LaunchFailureReason
				if ($reason) {
					Fail-Launch "Unreal Editor hit a fatal startup error."
				}
				if (Test-Slice01Loaded) {
					$mapLoaded = $true
					break
				}
			}
		}

		$still = Get-AfterlightEditorProcess
		if (-not $still) {
			Fail-Launch "Unreal Editor is not running after launch."
		}

		Show-EditorWindow -ProcessId $still.ProcessId
		if (-not $mapLoaded) {
			Fail-Launch "Unreal Editor is running (PID $($still.ProcessId)), but Slice01 did not load within 180 seconds."
		}

		Start-Sleep -Seconds 12
		if ($proc.HasExited) {
			Fail-Launch "Unreal Editor loaded Slice01, then exited (code $($proc.ExitCode))."
		}
		$reason = Get-LaunchFailureReason
		if ($reason) {
			Fail-Launch "Unreal Editor loaded Slice01, then hit a fatal error."
		}
		$still = Get-AfterlightEditorProcess
		if (-not $still) {
			Fail-Launch "Unreal Editor loaded Slice01, then closed."
		}

		Show-EditorWindow -ProcessId $still.ProcessId
		Write-Host "OK: Unreal Editor is running (PID $($still.ProcessId)). Slice01 is loaded."
		Write-Host "Waiting for auto New Editor Window PIE (854x480)..."
		$pieDeadline = (Get-Date).AddSeconds(90)
		$pieStarted = $false
		while ((Get-Date) -lt $pieDeadline) {
			Start-Sleep -Seconds 3
			if ($proc.HasExited) {
				Fail-Launch "Unreal Editor loaded Slice01, then exited (code $($proc.ExitCode))."
			}
			$reason = Get-LaunchFailureReason
			if ($reason) {
				Fail-Launch "Unreal Editor loaded Slice01, then hit a fatal error."
			}
			if (Select-String -Path $LogFile -Pattern "AFTERLIGHT_AUTOPLAY request_pie|PIE:|PlayInEditor|AFTERLIGHT_OWNER_ENTRY" -ErrorAction SilentlyContinue) {
				$pieStarted = $true
				break
			}
		}
		if ($pieStarted) {
			Start-Sleep -Seconds 2
			1..8 | ForEach-Object {
				Pin-AfterlightPlayWindow -ProcessId $still.ProcessId
				Start-Sleep -Milliseconds 400
			}
			Write-Host "OK: Play-in-editor was requested. Preview is pinned to the primary monitor."
			Write-Host "Click / Space / Enter on the card. Do not minimize the editor."
		} else {
			Write-Host "WARN: Auto-PIE was not confirmed in the log. Focus AFTERLIGHT and press Alt+P."
			Write-Host "Do not minimize the editor. Click the AFTERLIGHT Preview window, then click / Space / Enter on the card."
		}
		exit 0
	}
	"build" {
		Invoke-AfterlightBuild
		exit 0
	}
	"test" {
		Invoke-AfterlightAutomation
		exit 0
	}
	"smoke" {
		Invoke-AfterlightSmoke
		exit 0
	}
	"verify" {
		Invoke-AfterlightBuild
		Invoke-AfterlightAutomation
		Invoke-AfterlightSmoke
		Write-Host "VERIFY OK"
		exit 0
	}
	"all" {
		Invoke-AfterlightBuild
		Invoke-AfterlightAutomation
		Invoke-AfterlightSmoke
		Write-Host "VERIFY OK. Opening owner play..."
		& $PSCommandPath play
		exit $LASTEXITCODE
	}
	default {
		Write-Host "FAIL: Unknown command '$Command'."
		Write-Host "Supported: play, build, test, smoke, verify, all"
		exit 1
	}
}
