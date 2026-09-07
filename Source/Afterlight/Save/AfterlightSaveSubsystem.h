#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AfterlightSaveSubsystem.generated.h"

UCLASS()
class AFTERLIGHT_API UAfterlightSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Afterlight|Save")
	bool SaveTestSlot();

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Save")
	bool LoadTestSlot();

	static const FString TestSlotName;
};
