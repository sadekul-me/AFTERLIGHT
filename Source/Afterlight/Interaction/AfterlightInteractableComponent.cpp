#include "Interaction/AfterlightInteractableComponent.h"
#include "Narrative/AfterlightNarrativeSubsystem.h"
#include "Engine/GameInstance.h"

UAfterlightInteractableComponent::UAfterlightInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAfterlightInteractableComponent::CanInteract(AActor* Interactor) const
{
	return bAvailable && Interactor != nullptr;
}

FText UAfterlightInteractableComponent::GetPromptText() const
{
	return PromptText;
}

FGameplayTag UAfterlightInteractableComponent::GetInteractionVerb() const
{
	return Verb;
}

void UAfterlightInteractableComponent::ExecuteInteraction(AActor* Interactor)
{
	if (!CanInteract(Interactor))
	{
		return;
	}
	if (GrantFlag.IsValid())
	{
		if (UGameInstance* GI = Interactor->GetGameInstance())
		{
			if (UAfterlightNarrativeSubsystem* Narrative = GI->GetSubsystem<UAfterlightNarrativeSubsystem>())
			{
				Narrative->GrantFlag(GrantFlag);
			}
		}
	}
	OnInteracted.Broadcast(Interactor);
}
