#include "Core/AfterlightGameMode.h"
#include "Character/AfterlightCharacter.h"
#include "Character/AfterlightPlayerController.h"
#include "Cinematic/AfterlightLabDirector.h"
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
	bool bHasDirector = false;
	for (TActorIterator<AAfterlightLabDirector> It(GetWorld()); It; ++It)
	{
		bHasDirector = true;
		break;
	}
	if (!bHasDirector)
	{
		GetWorld()->SpawnActor<AAfterlightLabDirector>();
	}
}
