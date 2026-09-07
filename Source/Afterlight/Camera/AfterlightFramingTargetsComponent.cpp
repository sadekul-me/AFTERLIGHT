#include "Camera/AfterlightFramingTargetsComponent.h"

UAfterlightFramingTargetsComponent::UAfterlightFramingTargetsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	HeadTarget = CreateDefaultSubobject<USceneComponent>(TEXT("HeadTarget"));
	HeadTarget->SetupAttachment(this);
	HeadTarget->SetRelativeLocation(FVector(0.f, 0.f, 78.f));

	ChestTarget = CreateDefaultSubobject<USceneComponent>(TEXT("ChestTarget"));
	ChestTarget->SetupAttachment(this);
	ChestTarget->SetRelativeLocation(FVector(0.f, 0.f, 28.f));

	DialogueLookTarget = CreateDefaultSubobject<USceneComponent>(TEXT("DialogueLookTarget"));
	DialogueLookTarget->SetupAttachment(this);
	DialogueLookTarget->SetRelativeLocation(FVector(18.f, 0.f, 72.f));

	CinematicFocusTarget = CreateDefaultSubobject<USceneComponent>(TEXT("CinematicFocusTarget"));
	CinematicFocusTarget->SetupAttachment(this);
	CinematicFocusTarget->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
}

FVector UAfterlightFramingTargetsComponent::GetHeadLocation() const
{
	return HeadTarget ? HeadTarget->GetComponentLocation() : GetComponentLocation();
}

FVector UAfterlightFramingTargetsComponent::GetChestLocation() const
{
	return ChestTarget ? ChestTarget->GetComponentLocation() : GetComponentLocation();
}

FVector UAfterlightFramingTargetsComponent::GetDialogueLookLocation() const
{
	return DialogueLookTarget ? DialogueLookTarget->GetComponentLocation() : GetComponentLocation();
}

FVector UAfterlightFramingTargetsComponent::GetCinematicFocusLocation() const
{
	return CinematicFocusTarget ? CinematicFocusTarget->GetComponentLocation() : GetComponentLocation();
}
