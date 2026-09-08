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
#include "Character/AfterlightPlaceholderVisuals.h"

AAfterlightCompanionCharacter::AAfterlightCompanionCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(40.f, 88.f);
	GetCharacterMovement()->MaxWalkSpeed = 160.f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 380.f, 0.f);
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
	FAfterlightPlaceholderVisuals::Attach(this, true);
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
	FAfterlightPlaceholderVisuals::Tick(this, DeltaSeconds);
	UpdatePath(DeltaSeconds);
	UpdatePresence(DeltaSeconds);
	UpdateFacing(DeltaSeconds);
}

void AAfterlightCompanionCharacter::SetLeadPath(const TArray<FVector>& Points, bool bInWaitForPlayer)
{
	PathPoints = Points;
	PathIndex = 0;
	bWaitForPlayer = bInWaitForPlayer;
	bWaitingForPlayer = false;
	PathPause = 0.55f;
}

void AAfterlightCompanionCharacter::ClearLeadPath()
{
	PathPoints.Reset();
	PathIndex = 0;
	bWaitingForPlayer = false;
	PathPause = 0.f;
}

void AAfterlightCompanionCharacter::SetMoveEnabled(bool bEnabled)
{
	bMoveEnabled = bEnabled;
}

void AAfterlightCompanionCharacter::SetWorldLookTargets(const TArray<FVector>& Points)
{
	WorldLookTargets = Points;
}

void AAfterlightCompanionCharacter::SetPreferPlayerLook(bool bPreferPlayer)
{
	bPreferPlayerLook = bPreferPlayer;
}

void AAfterlightCompanionCharacter::GlanceAt(const FVector& WorldLocation)
{
	GlanceLocation = WorldLocation;
	GlanceHold = 2.2f;
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
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->bOrientRotationToMovement = false;
		}
		return;
	}

	if (PathPause > 0.f)
	{
		PathPause -= DeltaSeconds;
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->bOrientRotationToMovement = false;
		}
		return;
	}

	APawn* Protagonist = ResolveProtagonist();
	if (bWaitForPlayer && Protagonist)
	{
		const float Dist = FVector::Dist2D(GetActorLocation(), Protagonist->GetActorLocation());
		const float MaxLead = 320.f;
		bWaitingForPlayer = Dist > FMath::Min(CurrentFollowDistance + 60.f, MaxLead);
		if (bWaitingForPlayer)
		{
			if (UCharacterMovementComponent* Move = GetCharacterMovement())
			{
				Move->bOrientRotationToMovement = false;
			}
			if (GlanceHold <= 0.f)
			{
				GlanceAt(Protagonist->GetActorLocation() + FVector(0.f, 0.f, 70.f));
			}
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
		PathPause = 0.42f;
		if (WorldLookTargets.Num() > 0)
		{
			GlanceAt(WorldLookTargets[PresenceLookIndex % WorldLookTargets.Num()]);
		}
		return;
	}
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;
	}
	AddMovementInput(FVector(To.X, To.Y, 0.f).GetSafeNormal(), 1.f);
}

void AAfterlightCompanionCharacter::UpdatePresence(float DeltaSeconds)
{
	if (GlanceHold > 0.f)
	{
		GlanceHold -= DeltaSeconds;
		return;
	}
	PresenceTimer -= DeltaSeconds;
	if (PresenceTimer > 0.f)
	{
		return;
	}
	PresenceTimer = bPreferPlayerLook ? 3.4f : 4.6f;
	APawn* Protagonist = ResolveProtagonist();
	const bool bUsePlayer = Protagonist && (bWaitingForPlayer || bInDialogue || (bPreferPlayerLook && (PresenceLookIndex % 3) != 2));
	if (bUsePlayer)
	{
		GlanceAt(Protagonist->GetActorLocation() + FVector(0.f, 0.f, 70.f));
	}
	else if (WorldLookTargets.Num() > 0)
	{
		GlanceAt(WorldLookTargets[PresenceLookIndex % WorldLookTargets.Num()]);
	}
	++PresenceLookIndex;
}

void AAfterlightCompanionCharacter::UpdateFacing(float DeltaSeconds)
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		if (Move->bOrientRotationToMovement && GetVelocity().SizeSquared2D() > 400.f)
		{
			return;
		}
	}
	FVector LookAt = GlanceLocation;
	bool bLook = GlanceHold > 0.f;
	if (!HasReachedPathEnd() && !bWaitingForPlayer && GlanceHold <= 0.f)
	{
		LookAt = PathPoints[PathIndex];
		bLook = true;
	}
	else if (!bLook)
	{
		if (APawn* Protagonist = ResolveProtagonist())
		{
			LookAt = Protagonist->GetActorLocation();
			bLook = bInDialogue || FVector::Dist2D(LookAt, GetActorLocation()) < CurrentFollowDistance + 80.f;
		}
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
	const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), FRotator(0.f, Target.Yaw, 0.f), DeltaSeconds, 2.4f);
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
		bPreferPlayerLook = true;
	}
	else if (PresentationTags.HasTag(AfterlightTags::Companion_Follow_Far))
	{
		CurrentFollowDistance = FarFollowDistance;
		bPreferPlayerLook = false;
	}
	else
	{
		CurrentFollowDistance = DefaultFollowDistance;
	}
}
