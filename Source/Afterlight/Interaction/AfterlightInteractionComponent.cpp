#include "Interaction/AfterlightInteractionComponent.h"
#include "Interaction/AfterlightInteractable.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"

UAfterlightInteractionComponent::UAfterlightInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
}

IAfterlightInteractable* UAfterlightInteractionComponent::ResolveInterface(AActor* Actor) const
{
	if (!Actor)
	{
		return nullptr;
	}
	if (Actor->Implements<UAfterlightInteractable>())
	{
		return Cast<IAfterlightInteractable>(Actor);
	}
	if (UActorComponent* Comp = Actor->FindComponentByInterface(UAfterlightInteractable::StaticClass()))
	{
		return Cast<IAfterlightInteractable>(Comp);
	}
	return nullptr;
}

void UAfterlightInteractionComponent::ConsiderActor(AActor* Actor, AActor* Owner, const FVector& From, const FVector& Forward, AActor*& Best, float& BestScore) const
{
	if (!Actor || Actor == Owner)
	{
		return;
	}
	IAfterlightInteractable* Interactable = ResolveInterface(Actor);
	if (!Interactable || !Interactable->CanInteract(Owner))
	{
		return;
	}

	const FVector ToTarget = Actor->GetActorLocation() - From;
	const float DistSq = ToTarget.SizeSquared();
	const float MaxDist = TraceDistance + Radius;
	if (DistSq > FMath::Square(MaxDist) || DistSq <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FVector Dir = ToTarget.GetSafeNormal();
	const float Facing = FVector::DotProduct(Forward, Dir);
	if (Facing < 0.2f)
	{
		return;
	}

	const float Score = DistSq * (2.f - Facing);
	if (Score < BestScore)
	{
		BestScore = Score;
		Best = Actor;
	}
}

void UAfterlightInteractionComponent::RefreshTarget()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		CurrentTarget = nullptr;
		CurrentPrompt = FText::GetEmpty();
		return;
	}

	const FVector Forward = Owner->GetActorForwardVector();
	const FVector Start = Owner->GetActorLocation() + FVector(0.f, 0.f, 40.f);
	const FVector Probe = Start + Forward * (TraceDistance * 0.5f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AfterlightInteract), false, Owner);
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_Pawn);
	Objects.AddObjectTypesToQuery(ECC_WorldStatic);
	Objects.AddObjectTypesToQuery(ECC_WorldDynamic);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps,
		Probe,
		FQuat::Identity,
		Objects,
		FCollisionShape::MakeSphere(FMath::Max(Radius, TraceDistance * 0.55f)),
		Params);

	AActor* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ConsiderActor(Overlap.GetActor(), Owner, Start, Forward, Best, BestScore);
	}

	CurrentTarget = Best;
	if (Best)
	{
		if (IAfterlightInteractable* Interactable = ResolveInterface(Best))
		{
			CurrentPrompt = Interactable->GetPromptText();
		}
	}
	else
	{
		CurrentPrompt = FText::GetEmpty();
	}
}

void UAfterlightInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RefreshTarget();
}

bool UAfterlightInteractionComponent::TryInteract()
{
	RefreshTarget();
	AActor* Target = CurrentTarget.Get();
	IAfterlightInteractable* Interactable = ResolveInterface(Target);
	if (!Interactable)
	{
		return false;
	}
	Interactable->ExecuteInteraction(GetOwner());
	return true;
}
