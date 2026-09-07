#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AfterlightRelationshipTypes.generated.h"

USTRUCT(BlueprintType)
struct AFTERLIGHT_API FAfterlightRelationshipState
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	float Trust = 0.5f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Afterlight")
	float Suspicion = 0.f;
};

USTRUCT(BlueprintType)
struct AFTERLIGHT_API FAfterlightRelationshipThresholds
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Afterlight")
	float TrustHigh = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Afterlight")
	float TrustLow = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Afterlight")
	float SuspicionHigh = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Afterlight")
	float SuspicionLow = 0.2f;
};

struct AFTERLIGHT_API FAfterlightRelationshipMath
{
	static float Clamp01(float Value);
	static FAfterlightRelationshipState ApplyDelta(const FAfterlightRelationshipState& In, float TrustDelta, float SuspicionDelta);
	static FGameplayTagContainer EvaluateThresholdTags(const FAfterlightRelationshipState& State, const FAfterlightRelationshipThresholds& Thresholds);
};
