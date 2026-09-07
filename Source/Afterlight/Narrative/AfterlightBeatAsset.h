#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AfterlightBeatAsset.generated.h"

class UAfterlightDialogueAsset;
class ULevelSequence;

UCLASS(BlueprintType)
class AFTERLIGHT_API UAfterlightBeatAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FName BeatId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FGameplayTagContainer RequiredFlags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FGameplayTagContainer GrantFlags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	TObjectPtr<UAfterlightDialogueAsset> Dialogue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FGameplayTag CameraRegister;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	TSoftObjectPtr<ULevelSequence> Sequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FName NextBeatId = NAME_None;

	bool AreRequirementsMet(const FGameplayTagContainer& Flags) const;
};
