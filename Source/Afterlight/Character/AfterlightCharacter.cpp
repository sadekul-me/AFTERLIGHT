#include "Character/AfterlightCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Interaction/AfterlightInteractionComponent.h"
#include "Camera/AfterlightFramingTargetsComponent.h"
#include "Camera/AfterlightCameraRecipe.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Character/AfterlightPlaceholderVisuals.h"
#include "Camera/AfterlightSliceProgress.h"
#include "Engine/World.h"

AAfterlightCharacter::AAfterlightCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
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
	CameraBoom->TargetArmLength = 340.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 48.f, 64.f);
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeSize = 28.f;
	CameraBoom->ProbeChannel = ECC_Camera;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 5.5f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 9.f;
	CameraBoom->CameraLagMaxDistance = 80.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 58.f;

	Interaction = CreateDefaultSubobject<UAfterlightInteractionComponent>(TEXT("Interaction"));

	Framing = CreateDefaultSubobject<UAfterlightFramingTargetsComponent>(TEXT("Framing"));
	Framing->SetupAttachment(GetCapsuleComponent());
}

void AAfterlightCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	FAfterlightPlaceholderVisuals::Attach(this, false);
	if (UWorld* World = GetWorld())
	{
		if (UAfterlightPlayerContextSubsystem* Context = World->GetSubsystem<UAfterlightPlayerContextSubsystem>())
		{
			Context->RegisterProtagonist(this);
		}
	}
}

void AAfterlightCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FAfterlightPlaceholderVisuals::Tick(this, DeltaSeconds);
	if (!FollowCamera || !CameraBoom)
	{
		return;
	}
	const FVector CamLoc = FollowCamera->GetComponentLocation();
	if (FAfterlightSliceProgress::IsCameraBelowFloor(CamLoc.Z, 28.f))
	{
		CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength * 0.8f, 120.f, 340.f);
		FVector Offset = CameraBoom->SocketOffset;
		Offset.Z = FMath::Max(Offset.Z, 78.f);
		CameraBoom->SocketOffset = Offset;
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
	if (Controller)
	{
		FRotator Rot = GetControlRotation();
		float Pitch = Rot.Pitch;
		if (Pitch > 180.f)
		{
			Pitch -= 360.f;
		}
		Rot.Pitch = FMath::Clamp(Pitch, -48.f, 18.f);
		Controller->SetControlRotation(Rot);
	}
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

void AAfterlightCharacter::ApplyExploreRecipe(const UAfterlightCameraRecipe* Recipe)
{
	if (!Recipe || !CameraBoom || !FollowCamera)
	{
		return;
	}
	CameraBoom->TargetArmLength = Recipe->ArmLength;
	CameraBoom->SocketOffset = FVector(0.f, 52.f * Recipe->ShoulderSide, Recipe->CameraHeight);
	CameraBoom->CameraLagSpeed = Recipe->MovementLag;
	CameraBoom->CameraRotationLagSpeed = Recipe->RotationLag;
	CameraBoom->bEnableCameraLag = Recipe->MovementLag > 0.1f;
	CameraBoom->bEnableCameraRotationLag = Recipe->RotationLag > 0.1f;
	FollowCamera->FieldOfView = Recipe->GameplayFOV;
}
