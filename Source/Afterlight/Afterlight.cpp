#include "Afterlight.h"
#include "Cinematic/AfterlightLabDirector.h"
#include "Slice/AfterlightSlice01Director.h"
#include "Camera/AfterlightCameraSubsystem.h"
#include "Cinematic/AfterlightCinematicCoordinator.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMemory.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "Templates/Function.h"
#include "Core/AfterlightLog.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FAfterlightModule, Afterlight, "Afterlight");

static FAutoConsoleCommand AfterlightResetLabCommand(
	TEXT("Afterlight.ResetLab"),
	TEXT("Reset AFTERLIGHT technical-lab narrative and relationship state."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		if (!GEngine)
		{
			return;
		}
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (UWorld* World = Context.World())
			{
				if (!World->IsGameWorld())
				{
					continue;
				}
				for (TActorIterator<AAfterlightLabDirector> It(World); It; ++It)
				{
					It->ResetTechnicalFlow();
				}
			}
		}
	}));

static FAutoConsoleCommand AfterlightSmokeLabCommand(
	TEXT("Afterlight.SmokeLab"),
	TEXT("Run the AFTERLIGHT technical-lab systems smoke (talk, choice, inspect, save/load)."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		if (!GEngine)
		{
			return;
		}
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (UWorld* World = Context.World())
			{
				if (!World->IsGameWorld())
				{
					continue;
				}
				for (TActorIterator<AAfterlightLabDirector> It(World); It; ++It)
				{
					FString Report;
					const bool bOk = It->RunTechnicalSmoke(Report);
					UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_SMOKE_RESULT=%s\n%s"), bOk ? TEXT("PASS") : TEXT("FAIL"), *Report);
				}
			}
		}
	}));

static void AfterlightForEachWorld(TFunctionRef<void(UWorld*)> Fn)
{
	if (!GEngine)
	{
		return;
	}
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (UWorld* World = Context.World())
		{
			if (World->IsGameWorld())
			{
				Fn(World);
			}
		}
	}
}

static FAutoConsoleCommand AfterlightCameraExploreCommand(
	TEXT("Afterlight.Camera.Explore"),
	TEXT("Force Explore camera register."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		AfterlightForEachWorld([](UWorld* World)
		{
			if (UAfterlightCameraSubsystem* Camera = World->GetSubsystem<UAfterlightCameraSubsystem>())
			{
				Camera->ReleaseToExplore(0.6f);
			}
		});
	}));

static FAutoConsoleCommand AfterlightCameraDialogueCommand(
	TEXT("Afterlight.Camera.Dialogue"),
	TEXT("Force Dialogue OTS shot."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		AfterlightForEachWorld([](UWorld* World)
		{
			if (UAfterlightCameraSubsystem* Camera = World->GetSubsystem<UAfterlightCameraSubsystem>())
			{
				Camera->RequestShot(AfterlightShotIds::DialogueOTSCompanion, 0.5f);
			}
		});
	}));

static FAutoConsoleCommand AfterlightCameraPlayRevealCommand(
	TEXT("Afterlight.Camera.PlayReveal"),
	TEXT("Play the technical inspect Level Sequence."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		AfterlightForEachWorld([](UWorld* World)
		{
			for (TActorIterator<AAfterlightLabDirector> It(World); It; ++It)
			{
				It->PlayInspectReveal();
			}
		});
	}));

static FAutoConsoleCommand AfterlightCameraCycleCommand(
	TEXT("Afterlight.Camera.Cycle"),
	TEXT("Cycle authored lab camera shots."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		AfterlightForEachWorld([](UWorld* World)
		{
			if (UAfterlightCameraSubsystem* Camera = World->GetSubsystem<UAfterlightCameraSubsystem>())
			{
				Camera->CycleDebugShot();
			}
		});
	}));

static FAutoConsoleCommand AfterlightResetSliceCommand(
	TEXT("Afterlight.ResetSlice"),
	TEXT("Reset AFTERLIGHT Slice01 narrative and relationship state."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		AfterlightForEachWorld([](UWorld* World)
		{
			for (TActorIterator<AAfterlightSlice01Director> It(World); It; ++It)
			{
				It->ResetSlice();
			}
		});
	}));

static FAutoConsoleCommand AfterlightSliceJumpCommand(
	TEXT("Afterlight.Slice01.Jump"),
	TEXT("Jump Slice01 debug checkpoint: Wake, Choice, Sweep, Quiet, Warning."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		const FName Checkpoint = Args.Num() > 0 ? FName(*Args[0]) : FName(TEXT("Wake"));
		AfterlightForEachWorld([&Checkpoint](UWorld* World)
		{
			for (TActorIterator<AAfterlightSlice01Director> It(World); It; ++It)
			{
				It->JumpCheckpoint(Checkpoint);
			}
		});
	}));

static FAutoConsoleCommand AfterlightSmokeSliceCommand(
	TEXT("Afterlight.SmokeSlice01"),
	TEXT("Run the AFTERLIGHT Slice01 greybox smoke (choice, sweep, tin, warning, title)."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		AfterlightForEachWorld([](UWorld* World)
		{
			for (TActorIterator<AAfterlightSlice01Director> It(World); It; ++It)
			{
				FString Report;
				const bool bOk = It->RunSliceSmoke(Report);
				UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_SLICE_SMOKE=%s\n%s"), bOk ? TEXT("PASS") : TEXT("FAIL"), *Report);
			}
		});
	}));

static TAutoConsoleVariable<int32> CVarAfterlightQaAuto(
	TEXT("Afterlight.QaAuto"),
	0,
	TEXT("If 1, capture Saved/QA screenshots at Slice01 visual beats."),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarAfterlightQaDrive(
	TEXT("Afterlight.QaDrive"),
	0,
	TEXT("If 1, walk the owner-play route and pick the first choice for unattended QA."),
	ECVF_Default);

bool AfterlightQaAutoEnabled()
{
	return CVarAfterlightQaAuto.GetValueOnAnyThread() != 0;
}

bool AfterlightQaDriveEnabled()
{
	return CVarAfterlightQaDrive.GetValueOnAnyThread() != 0;
}

void AfterlightCaptureQaShot(const TCHAR* Name)
{
	const FPlatformMemoryStats Mem = FPlatformMemory::GetStats();
	UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_MEM usedPhys=%.2fGB availPhys=%.2fGB usedVirt=%.2fGB availVirt=%.2fGB"),
		Mem.UsedPhysical / (1024.0 * 1024.0 * 1024.0),
		Mem.AvailablePhysical / (1024.0 * 1024.0 * 1024.0),
		Mem.UsedVirtual / (1024.0 * 1024.0 * 1024.0),
		Mem.AvailableVirtual / (1024.0 * 1024.0 * 1024.0));
	const FString ShotName = Name && *Name ? FString(Name) : FDateTime::Now().ToString(TEXT("HHmmss"));
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("QA");
	IFileManager::Get().MakeDirectory(*Dir, true);
	const FString Path = FPaths::ConvertRelativePathToFull(Dir / (ShotName + TEXT(".png")));
	FScreenshotRequest::RequestScreenshot(Path, false, false);
	UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_QA_SHOT=%s"), *Path);
}

static FAutoConsoleCommand AfterlightAcceptEntryCommand(
	TEXT("Afterlight.AcceptEntry"),
	TEXT("Dismiss the owner entry card if it is showing."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		AfterlightForEachWorld([](UWorld* World)
		{
			for (TActorIterator<AAfterlightSlice01Director> It(World); It; ++It)
			{
				It->TryAcceptContinue();
			}
		});
	}));

static FAutoConsoleCommand AfterlightQaShotCommand(
	TEXT("Afterlight.QaShot"),
	TEXT("Capture an internal QA screenshot under Saved/QA (not for Git)."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		AfterlightCaptureQaShot(Args.Num() > 0 ? *Args[0] : nullptr);
	}));

void FAfterlightModule::StartupModule()
{
}

void FAfterlightModule::ShutdownModule()
{
}
