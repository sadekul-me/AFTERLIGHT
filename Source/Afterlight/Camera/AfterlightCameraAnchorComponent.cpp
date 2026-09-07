#include "Camera/AfterlightCameraAnchorComponent.h"
#include "Camera/AfterlightCameraSubsystem.h"

UAfterlightCameraAnchorComponent::UAfterlightCameraAnchorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAfterlightCameraAnchorComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		if (UAfterlightCameraSubsystem* Camera = World->GetSubsystem<UAfterlightCameraSubsystem>())
		{
			Camera->RegisterAnchor(Register, GetOwner());
		}
	}
}
