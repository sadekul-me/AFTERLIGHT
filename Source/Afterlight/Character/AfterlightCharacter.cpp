#include "Character/AfterlightCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Interaction/AfterlightInteractionComponent.h"
#include "Core/AfterlightPlayerContextSubsystem.h"

AAfterlightCharacter::AAfterlightCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 92.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 420.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking = 1400.f;
	GetCharacterMovement()->JumpZVelocity = 0.f;
	GetCharacterMovement()->AirControl = 0.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 280.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 40.f, 40.f);
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 8.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 70.f;

	Interaction = CreateDefaultSubobject<UAfterlightInteractionComponent>(TEXT("Interaction"));
}

void AAfterlightCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	if (UWorld* World = GetWorld())
	{
		if (UAfterlightPlayerContextSubsystem* Context = World->GetSubsystem<UAfterlightPlayerContextSubsystem>())
		{
			Context->RegisterProtagonist(this);
		}
	}
}

void AAfterlightCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (UWorld* World = GetWorld())
	{
		if (UAfterlightPlayerContextSubsystem* Context = World->GetSubsystem<UAfterlightPlayerContextSubsystem>())
		{
			Context->RegisterProtagonist(this);
		}
	}
}

void AAfterlightCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAfterlightCharacter::Move(const FInputActionValue& Value)
{
	if (!bMoveEnabled)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	const FRotator Yaw(0.f, GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Axis.X);
}

void AAfterlightCharacter::Look(const FInputActionValue& Value)
{
	if (!bLookEnabled)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AAfterlightCharacter::Interact()
{
	if (Interaction)
	{
		Interaction->TryInteract();
	}
}

void AAfterlightCharacter::SetMoveEnabled(bool bEnabled)
{
	bMoveEnabled = bEnabled;
}

void AAfterlightCharacter::SetLookEnabled(bool bEnabled)
{
	bLookEnabled = bEnabled;
}
