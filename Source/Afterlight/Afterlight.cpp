#include "Afterlight.h"
#include "Cinematic/AfterlightLabDirector.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
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
				for (TActorIterator<AAfterlightLabDirector> It(World); It; ++It)
				{
					FString Report;
					const bool bOk = It->RunTechnicalSmoke(Report);
					UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_SMOKE_RESULT=%s\n%s"), bOk ? TEXT("PASS") : TEXT("FAIL"), *Report);
				}
			}
		}
	}));

void FAfterlightModule::StartupModule()
{
}

void FAfterlightModule::ShutdownModule()
{
}
