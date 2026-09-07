#include "Narrative/AfterlightNarrativeSubsystem.h"
#include "Narrative/AfterlightBeatAsset.h"
#include "Narrative/AfterlightDialogueRunner.h"
#include "Core/AfterlightLog.h"

void UAfterlightNarrativeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	DialogueRunner = NewObject<UAfterlightDialogueRunner>(this);
}

bool UAfterlightNarrativeSubsystem::HasFlag(FGameplayTag Flag) const
{
	return Flag.IsValid() && State.StoryFlags.HasTag(Flag);
}

void UAfterlightNarrativeSubsystem::GrantFlag(FGameplayTag Flag)
{
	if (!Flag.IsValid() || State.StoryFlags.HasTag(Flag))
	{
		return;
	}
	State.StoryFlags.AddTag(Flag);
	OnFlagsChanged.Broadcast(State.StoryFlags);
	UE_LOG(LogAfterlight, Log, TEXT("Granted story flag %s"), *Flag.ToString());
}

void UAfterlightNarrativeSubsystem::GrantFlags(FGameplayTagContainer Flags)
{
	for (const FGameplayTag& Flag : Flags)
	{
		GrantFlag(Flag);
	}
}

bool UAfterlightNarrativeSubsystem::TryActivateBeat(UAfterlightBeatAsset* Beat)
{
	if (!Beat || !Beat->AreRequirementsMet(State.StoryFlags))
	{
		return false;
	}
	GrantFlags(Beat->GrantFlags);
	SetCurrentBeatId(Beat->BeatId);
	return true;
}

void UAfterlightNarrativeSubsystem::SetCurrentBeatId(FName BeatId)
{
	if (State.CurrentBeatId == BeatId)
	{
		return;
	}
	State.CurrentBeatId = BeatId;
	OnBeatChanged.Broadcast(BeatId);
}

void UAfterlightNarrativeSubsystem::RestoreState(const FAfterlightNarrativeState& InState)
{
	State = InState;
	OnFlagsChanged.Broadcast(State.StoryFlags);
	OnBeatChanged.Broadcast(State.CurrentBeatId);
}
