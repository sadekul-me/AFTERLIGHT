#include "Afterlight.h"
#include "Cinematic/AfterlightLabDirector.h"
#include "Slice/AfterlightSlice01Director.h"
#include "Camera/AfterlightCameraSubsystem.h"
#include "Cinematic/AfterlightCinematicCoordinator.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
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

void FAfterlightModule::StartupModule()
{
}

void FAfterlightModule::ShutdownModule()
{
}
