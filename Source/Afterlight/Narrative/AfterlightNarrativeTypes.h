#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AfterlightNarrativeTypes.generated.h"

USTRUCT(BlueprintType)
struct AFTERLIGHT_API FAfterlightDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FGameplayTagContainer GrantFlags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	float TrustDelta = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	float SuspicionDelta = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FName NextNodeId = NAME_None;
};

USTRUCT(BlueprintType)
struct AFTERLIGHT_API FAfterlightDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FName SpeakerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	TArray<FAfterlightDialogueChoice> Choices;
};

USTRUCT(BlueprintType)
struct AFTERLIGHT_API FAfterlightNarrativeState
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Afterlight")
	FGameplayTagContainer StoryFlags;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Afterlight")
	FName CurrentBeatId = NAME_None;
};
