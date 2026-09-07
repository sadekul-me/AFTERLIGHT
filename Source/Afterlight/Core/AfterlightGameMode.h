#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AfterlightGameMode.generated.h"

UCLASS()
class AFTERLIGHT_API AAfterlightGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAfterlightGameMode();
	virtual void BeginPlay() override;
};
