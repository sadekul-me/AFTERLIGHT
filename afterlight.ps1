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
				"-dpcvars=r.Streaming.PoolSize=96,r.ScreenPercentage=67,sg.ViewDistanceQuality=0,sg.AntiAliasingQuality=0,sg.ShadowQuality=0,sg.GlobalIlluminationQuality=0,sg.ReflectionQuality=0,sg.PostProcessQuality=0,sg.TextureQuality=0,sg.EffectsQuality=0,sg.FoliageQuality=0,sg.ShadingQuality=0,r.Lumen.DiffuseIndirect.Allow=0,r.Shadow.Virtual.Enable=0,r.Nanite=0,r.GenerateMeshDistanceFields=0,r.DefaultFeature.MotionBlur=0,Afterlight.QaAuto=1,Afterlight.QaDrive=1",
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
			Write-Host "OK: Play-in-editor was requested. Keep the editor visible; use the AFTERLIGHT Preview window."
			Write-Host "Click / Space / Enter on the card. Do not minimize the editor."
		} else {
			Write-Host "WARN: Auto-PIE was not confirmed in the log. Focus AFTERLIGHT and press Alt+P."
			Write-Host "Do not minimize the editor. Click the AFTERLIGHT Preview window, then click / Space / Enter on the card."
		}
		exit 0
	}
	default {
		Write-Host "FAIL: Reserved command '$Command'."
		Write-Host "This launcher currently supports:  .\afterlight.ps1 play"
		Write-Host "Later verbs: build, test, smoke, verify, all"
		exit 1
	}
}
