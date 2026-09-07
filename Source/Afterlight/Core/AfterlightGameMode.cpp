#include "Core/AfterlightGameMode.h"
#include "Character/AfterlightCharacter.h"
#include "Character/AfterlightPlayerController.h"
#include "Cinematic/AfterlightLabDirector.h"
#include "Slice/AfterlightSlice01Director.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"

AAfterlightGameMode::AAfterlightGameMode()
{
	DefaultPawnClass = AAfterlightCharacter::StaticClass();
	PlayerControllerClass = AAfterlightPlayerController::StaticClass();
	HUDClass = AHUD::StaticClass();
}

void AAfterlightGameMode::BeginPlay()
{
	Super::BeginPlay();
	bool bHasLabDirector = false;
	bool bHasSliceDirector = false;
	for (TActorIterator<AAfterlightLabDirector> It(GetWorld()); It; ++It)
	{
		bHasLabDirector = true;
		break;
	}
	for (TActorIterator<AAfterlightSlice01Director> It(GetWorld()); It; ++It)
	{
		bHasSliceDirector = true;
		break;
	}
	if (!bHasLabDirector && !bHasSliceDirector)
	{
		GetWorld()->SpawnActor<AAfterlightLabDirector>();
	}
}
