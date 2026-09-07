#include "Interaction/AfterlightInteractableComponent.h"
#include "Narrative/AfterlightNarrativeSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"

UAfterlightInteractableComponent::UAfterlightInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAfterlightInteractableComponent::CanInteract(AActor* Interactor) const
{
	if (!bAvailable || !Interactor)
	{
		return false;
	}
	if (!RequiredFlag.IsValid() && !BlockedFlag.IsValid())
	{
		return true;
	}
	UGameInstance* GI = Interactor->GetGameInstance();
	const UAfterlightNarrativeSubsystem* Narrative = GI ? GI->GetSubsystem<UAfterlightNarrativeSubsystem>() : nullptr;
	if (!Narrative)
	{
		return false;
	}
	if (RequiredFlag.IsValid() && !Narrative->HasFlag(RequiredFlag))
	{
		return false;
	}
	if (BlockedFlag.IsValid() && Narrative->HasFlag(BlockedFlag))
	{
		return false;
	}
	return true;
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
