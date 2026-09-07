#include "Character/AfterlightCompanionCharacter.h"
#include "Interaction/AfterlightInteractableComponent.h"
#include "Core/AfterlightGameplayTags.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Character/AfterlightCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
	UpdateFacing(DeltaSeconds);
}

void AAfterlightCompanionCharacter::UpdateFacing(float DeltaSeconds)
{
	APawn* Protagonist = ResolveProtagonist();
	if (!Protagonist)
	{
		return;
	}
	const FVector ToPlayer = Protagonist->GetActorLocation() - GetActorLocation();
	if (ToPlayer.SizeSquared2D() < 1.f)
	{
		return;
	}
	const bool bShouldLook = bInDialogue || ToPlayer.Size2D() < CurrentFollowDistance + 80.f;
	if (!bShouldLook)
	{
		return;
	}
	const FRotator Target = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Protagonist->GetActorLocation());
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
