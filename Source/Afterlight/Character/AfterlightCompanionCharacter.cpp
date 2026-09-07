#include "Character/AfterlightCompanionCharacter.h"
#include "Interaction/AfterlightInteractableComponent.h"
#include "Camera/AfterlightFramingTargetsComponent.h"
#include "Core/AfterlightGameplayTags.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Character/AfterlightCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

AAfterlightCompanionCharacter::AAfterlightCompanionCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(40.f, 88.f);
	GetCharacterMovement()->MaxWalkSpeed = 180.f;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;

	Interactable = CreateDefaultSubobject<UAfterlightInteractableComponent>(TEXT("Interactable"));
	Interactable->PromptText = NSLOCTEXT("Afterlight", "TalkPrompt", "Talk");
	Interactable->Verb = AfterlightTags::Interaction_Talk;

	Framing = CreateDefaultSubobject<UAfterlightFramingTargetsComponent>(TEXT("Framing"));
	Framing->SetupAttachment(GetCapsuleComponent());
}

void AAfterlightCompanionCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentFollowDistance = DefaultFollowDistance;
	if (Interactable)
	{
		Interactable->Verb = AfterlightTags::Interaction_Talk;
		Interactable->GrantFlag = AfterlightTags::Story_Test_MetCompanion;
	}
}

APawn* AAfterlightCompanionCharacter::ResolveProtagonist() const
{
	if (const UAfterlightPlayerContextSubsystem* Context = GetWorld()->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		return Context->GetProtagonist();
	}
	return nullptr;
}

void AAfterlightCompanionCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdatePath(DeltaSeconds);
	UpdateFacing(DeltaSeconds);
}

void AAfterlightCompanionCharacter::SetLeadPath(const TArray<FVector>& Points, bool bInWaitForPlayer)
{
	PathPoints = Points;
	PathIndex = 0;
	bWaitForPlayer = bInWaitForPlayer;
	bWaitingForPlayer = false;
}

void AAfterlightCompanionCharacter::ClearLeadPath()
{
	PathPoints.Reset();
	PathIndex = 0;
	bWaitingForPlayer = false;
}

void AAfterlightCompanionCharacter::SetMoveEnabled(bool bEnabled)
{
	bMoveEnabled = bEnabled;
}

bool AAfterlightCompanionCharacter::HasReachedPathEnd() const
{
	return PathPoints.Num() == 0 || PathIndex >= PathPoints.Num();
}

void AAfterlightCompanionCharacter::UpdatePath(float DeltaSeconds)
{
	if (!bMoveEnabled || HasReachedPathEnd())
	{
		bWaitingForPlayer = false;
		return;
	}

	APawn* Protagonist = ResolveProtagonist();
	if (bWaitForPlayer && Protagonist)
	{
		const float Dist = FVector::Dist2D(GetActorLocation(), Protagonist->GetActorLocation());
		bWaitingForPlayer = Dist > CurrentFollowDistance + 220.f;
		if (bWaitingForPlayer)
		{
			return;
		}
	}
	else
	{
		bWaitingForPlayer = false;
	}

	const FVector Target = PathPoints[PathIndex];
	const FVector To = Target - GetActorLocation();
	if (To.SizeSquared2D() < 80.f * 80.f)
	{
		++PathIndex;
		return;
	}
	AddMovementInput(FVector(To.X, To.Y, 0.f).GetSafeNormal(), 1.f);
}

void AAfterlightCompanionCharacter::UpdateFacing(float DeltaSeconds)
{
	FVector LookAt = FVector::ZeroVector;
	bool bLook = false;
	if (!HasReachedPathEnd())
	{
		LookAt = PathPoints[PathIndex];
		bLook = true;
	}
	else if (APawn* Protagonist = ResolveProtagonist())
	{
		LookAt = Protagonist->GetActorLocation();
		const FVector ToPlayer = LookAt - GetActorLocation();
		bLook = bInDialogue || ToPlayer.Size2D() < CurrentFollowDistance + 80.f;
	}
	if (!bLook)
	{
		return;
	}
	const FVector To = LookAt - GetActorLocation();
	if (To.SizeSquared2D() < 1.f)
	{
		return;
	}
	const FRotator Target = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookAt);
	const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), FRotator(0.f, Target.Yaw, 0.f), DeltaSeconds, 4.f);
	SetActorRotation(NewRot);
}

bool AAfterlightCompanionCharacter::CanInteract(AActor* Interactor) const
{
	return Interactable && Interactable->CanInteract(Interactor) && !bInDialogue;
}

FText AAfterlightCompanionCharacter::GetPromptText() const
{
	return Interactable ? Interactable->GetPromptText() : FText::GetEmpty();
}

FGameplayTag AAfterlightCompanionCharacter::GetInteractionVerb() const
{
	return AfterlightTags::Interaction_Talk;
}

void AAfterlightCompanionCharacter::ExecuteInteraction(AActor* Interactor)
{
	if (Interactable)
	{
		Interactable->ExecuteInteraction(Interactor);
	}
}

void AAfterlightCompanionCharacter::NotifyDialogueStarted()
{
	bInDialogue = true;
}

void AAfterlightCompanionCharacter::NotifyDialogueEnded()
{
	bInDialogue = false;
}

void AAfterlightCompanionCharacter::ApplyPresentationTags(const FGameplayTagContainer& PresentationTags)
{
	if (PresentationTags.HasTag(AfterlightTags::Companion_Follow_Close))
	{
		CurrentFollowDistance = CloseFollowDistance;
	}
	else if (PresentationTags.HasTag(AfterlightTags::Companion_Follow_Far))
	{
		CurrentFollowDistance = FarFollowDistance;
	}
	else
	{
		CurrentFollowDistance = DefaultFollowDistance;
	}
}
