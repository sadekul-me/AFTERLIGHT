param(
	[Parameter(Position = 0)]
	[ValidateSet("play", "build", "test", "smoke", "verify", "all")]
	[string]$Command = "play"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = $PSScriptRoot
$ProjectFile = Join-Path $ProjectRoot "AFTERLIGHT.uproject"
$Map = "/Game/Environments/Slice01/L_Slice01_Greybox"
$EngineRoot = "D:\Epic Games\UE_5.8"
$Editor = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor.exe"

if (-not (Test-Path $ProjectFile)) {
	Write-Error "AFTERLIGHT.uproject was not found at $ProjectFile"
}

if (-not (Test-Path $Editor)) {
	Write-Error "Unreal Editor 5.8.2 was not found at $Editor"
}

switch ($Command) {
	"play" {
		Write-Host "AFTERLIGHT"
		Write-Host "Opening the playable slice in Unreal Editor 5.8.2."
		Write-Host ""
		Write-Host "When the Editor is ready:"
		Write-Host "  1. Click the viewport once."
		Write-Host "  2. Press Alt+P  (or the green Play button)."
		Write-Host "  3. Click / press any key on the AFTERLIGHT card."
		Write-Host ""
		Start-Process -FilePath $Editor -ArgumentList @(
			$ProjectFile,
			$Map
		) -WorkingDirectory $ProjectRoot
	}
	default {
		Write-Host "Reserved command: $Command"
		Write-Host "This launcher currently supports:  .\afterlight.ps1 play"
		Write-Host "Later verbs: build, test, smoke, verify, all"
		exit 1
	}
}
