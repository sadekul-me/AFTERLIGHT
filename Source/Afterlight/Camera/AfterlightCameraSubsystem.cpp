#include "Camera/AfterlightCameraSubsystem.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Character/AfterlightPlayerController.h"
#include "Character/AfterlightCharacter.h"
#include "Core/AfterlightLog.h"
#include "Camera/PlayerCameraManager.h"

void UAfterlightCameraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentRegister = EAfterlightCameraRegister::Explore;
}

void UAfterlightCameraSubsystem::RegisterAnchor(EAfterlightCameraRegister Register, AActor* CameraActor)
{
	if (CameraActor)
	{
		Anchors.Add(Register, CameraActor);
	}
}

void UAfterlightCameraSubsystem::SetRecipe(EAfterlightCameraRegister Register, UAfterlightCameraRecipe* Recipe)
{
	if (Recipe)
	{
		Recipes.Add(Register, Recipe);
	}
}

APlayerController* UAfterlightCameraSubsystem::ResolveController() const
{
	if (const UAfterlightPlayerContextSubsystem* Context = GetWorld()->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		return Context->GetProtagonistController();
	}
	return nullptr;
}

AActor* UAfterlightCameraSubsystem::ResolveExploreViewTarget() const
{
	if (const UAfterlightPlayerContextSubsystem* Context = GetWorld()->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		return Context->GetProtagonist();
	}
	return nullptr;
}

void UAfterlightCameraSubsystem::ApplyViewTarget(AActor* Target, float BlendTime)
{
	APlayerController* PC = ResolveController();
	if (!PC || !Target || !PC->PlayerCameraManager)
	{
		return;
	}
	PC->SetViewTargetWithBlend(Target, FMath::Max(0.f, BlendTime), VTBlend_Cubic);
}

void UAfterlightCameraSubsystem::RequestRegister(EAfterlightCameraRegister Register, float BlendOverride)
{
	const UAfterlightCameraRecipe* Recipe = Recipes.FindRef(Register);
	const float Blend = BlendOverride >= 0.f ? BlendOverride : (Recipe ? Recipe->BlendTime : 0.6f);

	AActor* Target = nullptr;
	if (Register == EAfterlightCameraRegister::Explore)
	{
		Target = ResolveExploreViewTarget();
	}
	else if (const TWeakObjectPtr<AActor>* Found = Anchors.Find(Register))
	{
		Target = Found->Get();
	}

	if (!Target)
	{
		UE_LOG(LogAfterlight, Verbose, TEXT("Camera register %s has no target; staying on current."), *AfterlightRegisterToName(Register).ToString());
		CurrentRegister = Register;
		return;
	}

	ApplyViewTarget(Target, Blend);
	CurrentRegister = Register;
}

void UAfterlightCameraSubsystem::ReleaseToExplore(float BlendOverride)
{
	RequestRegister(EAfterlightCameraRegister::Explore, BlendOverride);
}
