#include "Relationship/AfterlightRelationshipTypes.h"
#include "Core/AfterlightGameplayTags.h"

float FAfterlightRelationshipMath::Clamp01(float Value)
{
	return FMath::Clamp(Value, 0.f, 1.f);
}

FAfterlightRelationshipState FAfterlightRelationshipMath::ApplyDelta(const FAfterlightRelationshipState& In, float TrustDelta, float SuspicionDelta)
{
	FAfterlightRelationshipState Out = In;
	Out.Trust = Clamp01(In.Trust + TrustDelta);
	Out.Suspicion = Clamp01(In.Suspicion + SuspicionDelta);
	return Out;
}

FGameplayTagContainer FAfterlightRelationshipMath::EvaluateThresholdTags(const FAfterlightRelationshipState& State, const FAfterlightRelationshipThresholds& Thresholds)
{
	FGameplayTagContainer Tags;
	if (State.Trust >= Thresholds.TrustHigh)
	{
		Tags.AddTag(AfterlightTags::Relationship_Trust_High);
	}
	if (State.Trust <= Thresholds.TrustLow)
	{
		Tags.AddTag(AfterlightTags::Relationship_Trust_Low);
	}
	if (State.Suspicion >= Thresholds.SuspicionHigh)
	{
		Tags.AddTag(AfterlightTags::Relationship_Suspicion_High);
	}
	if (State.Suspicion <= Thresholds.SuspicionLow)
	{
		Tags.AddTag(AfterlightTags::Relationship_Suspicion_Low);
	}
	if (State.Trust >= Thresholds.TrustHigh)
	{
		Tags.AddTag(AfterlightTags::Companion_Follow_Close);
	}
	else if (State.Suspicion >= Thresholds.SuspicionHigh || State.Trust <= Thresholds.TrustLow)
	{
		Tags.AddTag(AfterlightTags::Companion_Follow_Far);
	}
	return Tags;
}
