#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "AfterlightCompanion.generated.h"

UINTERFACE(BlueprintType)
class AFTERLIGHT_API UAfterlightCompanion : public UInterface
{
	GENERATED_BODY()
};

class AFTERLIGHT_API IAfterlightCompanion
{
	GENERATED_BODY()

public:
	virtual void NotifyDialogueStarted() = 0;
	virtual void NotifyDialogueEnded() = 0;
	virtual void ApplyPresentationTags(const FGameplayTagContainer& PresentationTags) = 0;
};
