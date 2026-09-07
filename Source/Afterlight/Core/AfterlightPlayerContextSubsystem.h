#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AfterlightPlayerContextSubsystem.generated.h"

class AAfterlightCharacter;
class AAfterlightPlayerController;

UCLASS()
class AFTERLIGHT_API UAfterlightPlayerContextSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterProtagonist(AAfterlightCharacter* Character);
	void RegisterController(AAfterlightPlayerController* Controller);

	UFUNCTION(BlueprintPure, Category = "Afterlight|Core")
	AAfterlightCharacter* GetProtagonist() const;

	UFUNCTION(BlueprintPure, Category = "Afterlight|Core")
	AAfterlightPlayerController* GetProtagonistController() const;

private:
	TWeakObjectPtr<AAfterlightCharacter> Protagonist;
	TWeakObjectPtr<AAfterlightPlayerController> ProtagonistController;
};
