#include "Relationship/AfterlightRelationshipSubsystem.h"

void UAfterlightRelationshipSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	State.Trust = 0.5f;
	State.Suspicion = 0.f;
}

void UAfterlightRelationshipSubsystem::ApplyDelta(float TrustDelta, float SuspicionDelta)
{
	SetState(FAfterlightRelationshipMath::ApplyDelta(State, TrustDelta, SuspicionDelta));
}

void UAfterlightRelationshipSubsystem::SetState(FAfterlightRelationshipState NewState)
{
	NewState.Trust = FAfterlightRelationshipMath::Clamp01(NewState.Trust);
	NewState.Suspicion = FAfterlightRelationshipMath::Clamp01(NewState.Suspicion);
	State = NewState;
	Broadcast();
}

FGameplayTagContainer UAfterlightRelationshipSubsystem::GetPresentationTags() const
{
	return FAfterlightRelationshipMath::EvaluateThresholdTags(State, Thresholds);
}

void UAfterlightRelationshipSubsystem::RestoreState(const FAfterlightRelationshipState& InState)
{
	SetState(InState);
}

void UAfterlightRelationshipSubsystem::Broadcast()
{
	OnRelationshipChanged.Broadcast(State);
}
