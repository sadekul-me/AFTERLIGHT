#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "AfterlightInteractable.generated.h"

UINTERFACE(BlueprintType)
class AFTERLIGHT_API UAfterlightInteractable : public UInterface
{
	GENERATED_BODY()
};

class AFTERLIGHT_API IAfterlightInteractable
{
	GENERATED_BODY()

public:
	virtual bool CanInteract(AActor* Interactor) const = 0;
	virtual FText GetPromptText() const = 0;
	virtual FGameplayTag GetInteractionVerb() const = 0;
	virtual void ExecuteInteraction(AActor* Interactor) = 0;
};
