#include "Camera/AfterlightCameraSubsystem.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Character/AfterlightPlayerController.h"
#include "Character/AfterlightCharacter.h"
#include "Character/AfterlightCompanionCharacter.h"
#include "Camera/AfterlightFramingTargetsComponent.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "CineCameraSettings.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "HAL/IConsoleManager.h"
#include "Core/AfterlightLog.h"
#include "Camera/AfterlightSliceProgress.h"

static TAutoConsoleVariable<int32> CVarAfterlightAllowDOF(
	TEXT("Afterlight.Camera.AllowDOF"),
	1,
	TEXT("1 = allow cinematic DOF on Dialogue/Intimate/Reveal recipes. 0 = disable for Tier A profiling."),
	ECVF_Default);

void UAfterlightCameraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentRegister = EAfterlightCameraRegister::Explore;
	Authority = EAfterlightCameraAuthority::Gameplay;
	EnsureDefaultRecipes();
	DebugShotOrder = {
		AfterlightShotIds::DialogueOTSCompanion,
		AfterlightShotIds::DialogueOTSProtagonist,
		AfterlightShotIds::DialogueTwoShot,
		AfterlightShotIds::DialogueCloseUpCompanion,
		AfterlightShotIds::RevealInsert
	};
}

void UAfterlightCameraSubsystem::EnsureDefaultRecipes()
{
	if (Recipes.Num() > 0)
	{
		return;
	}
	const EAfterlightCameraRegister All[] = {
		EAfterlightCameraRegister::Explore,
		EAfterlightCameraRegister::Dialogue,
		EAfterlightCameraRegister::Intimate,
		EAfterlightCameraRegister::Reveal,
		EAfterlightCameraRegister::Threat,
		EAfterlightCameraRegister::Cinematic
	};
	for (EAfterlightCameraRegister Register : All)
	{
		Recipes.Add(Register, UAfterlightCameraRecipe::CreateDefault(this, Register));
	}
}

void UAfterlightCameraSubsystem::RegisterAnchor(EAfterlightCameraRegister Register, AActor* CameraActor)
{
	if (CameraActor)
	{
		Anchors.Add(Register, CameraActor);
	}
}

void UAfterlightCameraSubsystem::RegisterShot(FName ShotId, AActor* CameraActor)
{
	if (!ShotId.IsNone() && CameraActor)
	{
		Shots.Add(ShotId, CameraActor);
	}
}

void UAfterlightCameraSubsystem::SetRecipe(EAfterlightCameraRegister Register, UAfterlightCameraRecipe* Recipe)
{
	if (Recipe)
	{
		Recipes.Add(Register, Recipe);
	}
}

UAfterlightCameraRecipe* UAfterlightCameraSubsystem::GetRecipe(EAfterlightCameraRegister Register) const
{
	if (const TObjectPtr<UAfterlightCameraRecipe>* Found = Recipes.Find(Register))
	{
		return Found->Get();
	}
	return nullptr;
}

FName UAfterlightCameraSubsystem::GetActiveRecipeId() const
{
	if (const UAfterlightCameraRecipe* Recipe = GetRecipe(CurrentRegister))
	{
		return Recipe->RecipeId;
	}
	return AfterlightRegisterToName(CurrentRegister);
}

void UAfterlightCameraSubsystem::SetAuthority(EAfterlightCameraAuthority NewAuthority)
{
	Authority = NewAuthority;
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

AActor* UAfterlightCameraSubsystem::ResolveFocusActor() const
{
	for (TActorIterator<AAfterlightCompanionCharacter> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void UAfterlightCameraSubsystem::ApplyViewTarget(AActor* Target, float BlendTime, EViewTargetBlendFunction BlendFunction)
{
	APlayerController* PC = ResolveController();
	if (!PC || !Target || !PC->PlayerCameraManager)
	{
		return;
	}
	PC->SetViewTargetWithBlend(Target, FMath::Max(0.f, BlendTime), BlendFunction);
	CurrentViewTarget = Target;
}

void UAfterlightCameraSubsystem::ForceView(AActor* Target, float BlendTime)
{
	if (!Target)
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(PushTimer);
	PushActor.Reset();
	ApplyViewTarget(Target, BlendTime, VTBlend_Linear);
	if (Authority != EAfterlightCameraAuthority::Sequencer)
	{
		Authority = EAfterlightCameraAuthority::Register;
	}
}

void UAfterlightCameraSubsystem::ApplyRecipeToActor(AActor* Target, const UAfterlightCameraRecipe* Recipe, AActor* FocusActor)
{
	if (!Target || !Recipe)
	{
		return;
	}

	if (AAfterlightCharacter* Protagonist = Cast<AAfterlightCharacter>(Target))
	{
		Protagonist->ApplyExploreRecipe(Recipe);
		return;
	}

	ACineCameraActor* CineActor = Cast<ACineCameraActor>(Target);
	UCineCameraComponent* Cine = CineActor ? CineActor->GetCineCameraComponent() : nullptr;
	if (!Cine)
	{
		return;
	}

	Cine->CurrentFocalLength = Recipe->FocalLength;
	Cine->CurrentAperture = Recipe->Aperture;

	const bool bAllowDOF = CVarAfterlightAllowDOF.GetValueOnGameThread() != 0 && Recipe->bEnableDOF;
	Cine->FocusSettings.FocusMethod = bAllowDOF ? ECameraFocusMethod::Manual : ECameraFocusMethod::DoNotOverride;
	Cine->FocusSettings.bSmoothFocusChanges = false;

	FVector FocusLocation = Cine->GetComponentLocation() + Cine->GetForwardVector() * Recipe->ManualFocusDistance;
	if (FocusActor)
	{
		if (UAfterlightFramingTargetsComponent* Framing = FocusActor->FindComponentByClass<UAfterlightFramingTargetsComponent>())
		{
			FocusLocation = Framing->GetHeadLocation();
		}
		else
		{
			FocusLocation = FocusActor->GetActorLocation() + FVector(0.f, 0.f, 72.f);
		}
	}

	if (Recipe->FocusMode == EAfterlightCameraFocusMode::Manual)
	{
		Cine->FocusSettings.ManualFocusDistance = Recipe->ManualFocusDistance;
	}
	else
	{
		Cine->FocusSettings.ManualFocusDistance = FVector::Distance(Cine->GetComponentLocation(), FocusLocation);
	}
}

bool UAfterlightCameraSubsystem::IsShotBlocked(AActor* CameraActor, const FVector& FocusLocation) const
{
	if (!CameraActor || !GetWorld())
	{
		return false;
	}
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AfterlightShotValidity), false, CameraActor);
	if (AActor* Focus = ResolveFocusActor())
	{
		Params.AddIgnoredActor(Focus);
	}
	if (AActor* Player = ResolveExploreViewTarget())
	{
		Params.AddIgnoredActor(Player);
	}
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		CameraActor->GetActorLocation(),
		FocusLocation,
		ECC_WorldStatic,
		Params);
	return bHit && Hit.GetActor() && Hit.GetActor() != CameraActor;
}

bool UAfterlightCameraSubsystem::IsShotUsable(AActor* CameraActor, const FVector& FocusLocation) const
{
	if (!CameraActor)
	{
		return false;
	}
	const FVector Loc = CameraActor->GetActorLocation();
	if (!FAfterlightSliceProgress::IsShotValidPlacement(Loc, FocusLocation))
	{
		return false;
	}
	FCollisionQueryParams OverlapParams(SCENE_QUERY_STAT(AfterlightShotClearance), false, CameraActor);
	if (AActor* Focus = ResolveFocusActor())
	{
		OverlapParams.AddIgnoredActor(Focus);
	}
	if (AActor* Player = ResolveExploreViewTarget())
	{
		OverlapParams.AddIgnoredActor(Player);
	}
	if (GetWorld()->OverlapBlockingTestByChannel(Loc, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(18.f), OverlapParams))
	{
		return false;
	}
	return !IsShotBlocked(CameraActor, FocusLocation);
}

AActor* UAfterlightCameraSubsystem::ResolveValidatedShot(FName ShotId, AActor* Focus) const
{
	auto Resolve = [this](FName Id) -> AActor*
	{
		if (const TWeakObjectPtr<AActor>* Found = Shots.Find(Id))
		{
			return Found->Get();
		}
		return nullptr;
	};

	FVector FocusLocation = Focus ? Focus->GetActorLocation() + FVector(0.f, 0.f, 72.f) : FVector::ZeroVector;
	if (Focus)
	{
		if (UAfterlightFramingTargetsComponent* Framing = Focus->FindComponentByClass<UAfterlightFramingTargetsComponent>())
		{
			FocusLocation = Framing->GetHeadLocation();
		}
	}

	const FName Order[] = {
		ShotId,
		AfterlightShotIds::DialogueTwoShot,
		AfterlightShotIds::DialogueOTSCompanion
	};
	for (FName Id : Order)
	{
		if (AActor* Candidate = Resolve(Id))
		{
			if (IsShotUsable(Candidate, FocusLocation))
			{
				return Candidate;
			}
		}
	}
	return ResolveFallbackTarget(EAfterlightCameraRegister::Dialogue);
}

AActor* UAfterlightCameraSubsystem::ResolveFallbackTarget(EAfterlightCameraRegister Register) const
{
	if (const TWeakObjectPtr<AActor>* TwoShot = Shots.Find(AfterlightShotIds::DialogueTwoShot))
	{
		if (AActor* Actor = TwoShot->Get())
		{
			return Actor;
		}
	}
	if (const TWeakObjectPtr<AActor>* Anchor = Anchors.Find(Register))
	{
		return Anchor->Get();
	}
	return ResolveExploreViewTarget();
}

void UAfterlightCameraSubsystem::BeginPush(AActor* Target, const UAfterlightCameraRecipe* Recipe)
{
	GetWorld()->GetTimerManager().ClearTimer(PushTimer);
	PushActor.Reset();
	if (!Target || !Recipe || Recipe->PushInDistance <= 1.f || Recipe->PushInTime <= 0.05f)
	{
		return;
	}
	PushActor = Target;
	PushStart = Target->GetActorLocation();
	PushEnd = PushStart + Target->GetActorForwardVector() * Recipe->PushInDistance;
	if (AActor* Focus = ResolveFocusActor())
	{
		FVector FocusLocation = Focus->GetActorLocation() + FVector(0.f, 0.f, 72.f);
		if (UAfterlightFramingTargetsComponent* Framing = Focus->FindComponentByClass<UAfterlightFramingTargetsComponent>())
		{
			FocusLocation = Framing->GetHeadLocation();
		}
		if (!FAfterlightSliceProgress::IsShotValidPlacement(PushEnd, FocusLocation))
		{
			PushActor.Reset();
			return;
		}
	}
	PushElapsed = 0.f;
	PushDuration = Recipe->PushInTime;
	GetWorld()->GetTimerManager().SetTimer(PushTimer, this, &UAfterlightCameraSubsystem::TickPush, 0.016f, true);
}

void UAfterlightCameraSubsystem::TickPush()
{
	AActor* Actor = PushActor.Get();
	if (!Actor || PushDuration <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(PushTimer);
		return;
	}
	PushElapsed += 0.016f;
	const float Alpha = FMath::Clamp(PushElapsed / PushDuration, 0.f, 1.f);
	const float Ease = Alpha * Alpha * (3.f - 2.f * Alpha);
	Actor->SetActorLocation(FMath::Lerp(PushStart, PushEnd, Ease));
	if (Alpha >= 1.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(PushTimer);
		PushActor.Reset();
	}
}

void UAfterlightCameraSubsystem::RequestRegister(EAfterlightCameraRegister Register, float BlendOverride)
{
	EnsureDefaultRecipes();
	const UAfterlightCameraRecipe* Recipe = GetRecipe(Register);
	const float Blend = BlendOverride >= 0.f ? BlendOverride : (Recipe ? Recipe->BlendTime : 0.75f);
	const EViewTargetBlendFunction BlendFn = Recipe ? static_cast<EViewTargetBlendFunction>(Recipe->BlendFunction) : VTBlend_Cubic;

	AActor* Target = nullptr;
	FName ShotForRegister = NAME_None;
	if (Register == EAfterlightCameraRegister::Explore)
	{
		Target = ResolveExploreViewTarget();
		ActiveShotId = NAME_None;
	}
	else if (Register == EAfterlightCameraRegister::Dialogue)
	{
		ShotForRegister = AfterlightShotIds::DialogueOTSCompanion;
	}
	else if (Register == EAfterlightCameraRegister::Intimate)
	{
		ShotForRegister = AfterlightShotIds::DialogueCloseUpCompanion;
	}
	else if (Register == EAfterlightCameraRegister::Reveal)
	{
		ShotForRegister = AfterlightShotIds::RevealInsert;
	}
	else if (Register == EAfterlightCameraRegister::Threat)
	{
		ShotForRegister = AfterlightShotIds::ThreatPressure;
	}

	if (!ShotForRegister.IsNone())
	{
		if (const TWeakObjectPtr<AActor>* Found = Shots.Find(ShotForRegister))
		{
			Target = Found->Get();
			ActiveShotId = ShotForRegister;
		}
	}

	if (!Target)
	{
		if (const TWeakObjectPtr<AActor>* Found = Anchors.Find(Register))
		{
			Target = Found->Get();
		}
	}

	AActor* Focus = ResolveFocusActor();
	FVector FocusLocation = Target ? Target->GetActorLocation() : FVector::ZeroVector;
	if (Focus)
	{
		if (UAfterlightFramingTargetsComponent* Framing = Focus->FindComponentByClass<UAfterlightFramingTargetsComponent>())
		{
			FocusLocation = Framing->GetHeadLocation();
		}
	}
	if (Target && Register != EAfterlightCameraRegister::Explore && !IsShotUsable(Target, FocusLocation))
	{
		UE_LOG(LogAfterlight, Verbose, TEXT("Shot for %s invalid; using fallback."), *AfterlightRegisterToName(Register).ToString());
		Target = ResolveValidatedShot(ShotForRegister, Focus);
		if (!Target)
		{
			Target = ResolveFallbackTarget(Register);
		}
		if (Target == ResolveExploreViewTarget())
		{
			Register = EAfterlightCameraRegister::Explore;
			ActiveShotId = NAME_None;
		}
	}

	if (!Target)
	{
		UE_LOG(LogAfterlight, Verbose, TEXT("Camera register %s has no target; staying on current."), *AfterlightRegisterToName(Register).ToString());
		CurrentRegister = Register;
		if (Authority != EAfterlightCameraAuthority::Sequencer)
		{
			Authority = EAfterlightCameraAuthority::Register;
		}
		return;
	}

	ApplyRecipeToActor(Target, Recipe, Focus);
	ApplyViewTarget(Target, Blend, BlendFn);
	if (Register != EAfterlightCameraRegister::Explore)
	{
		BeginPush(Target, Recipe);
	}
	CurrentRegister = Register;
	if (Authority != EAfterlightCameraAuthority::Sequencer)
	{
		Authority = Register == EAfterlightCameraRegister::Explore ? EAfterlightCameraAuthority::Gameplay : EAfterlightCameraAuthority::Register;
	}
}

void UAfterlightCameraSubsystem::RequestShot(FName ShotId, float BlendOverride)
{
	EAfterlightCameraRegister Register = EAfterlightCameraRegister::Dialogue;
	if (ShotId == AfterlightShotIds::DialogueCloseUpCompanion)
	{
		Register = EAfterlightCameraRegister::Intimate;
	}
	else if (ShotId == AfterlightShotIds::RevealInsert)
	{
		Register = EAfterlightCameraRegister::Reveal;
	}
	else if (ShotId == AfterlightShotIds::ThreatPressure)
	{
		Register = EAfterlightCameraRegister::Threat;
	}
	else if (ShotId == AfterlightShotIds::WakeEstablish)
	{
		Register = EAfterlightCameraRegister::Cinematic;
		if (const TWeakObjectPtr<AActor>* Found = Shots.Find(ShotId))
		{
			if (AActor* WakeCam = Found->Get())
			{
				ActiveShotId = ShotId;
				const UAfterlightCameraRecipe* Recipe = GetRecipe(Register);
				ApplyRecipeToActor(WakeCam, Recipe, ResolveFocusActor());
				const float Blend = BlendOverride >= 0.f ? BlendOverride : (Recipe ? Recipe->BlendTime : 0.75f);
				ApplyViewTarget(WakeCam, Blend, Recipe ? static_cast<EViewTargetBlendFunction>(Recipe->BlendFunction) : VTBlend_Cubic);
				CurrentRegister = Register;
				if (Authority != EAfterlightCameraAuthority::Sequencer)
				{
					Authority = EAfterlightCameraAuthority::Register;
				}
				return;
			}
		}
		RequestRegister(EAfterlightCameraRegister::Explore, BlendOverride);
		return;
	}

	AActor* Focus = ResolveFocusActor();
	AActor* Target = ResolveValidatedShot(ShotId, Focus);
	if (!Target)
	{
		RequestRegister(Register, BlendOverride);
		return;
	}

	FName ChosenId = ShotId;
	for (const TPair<FName, TWeakObjectPtr<AActor>>& Pair : Shots)
	{
		if (Pair.Value.Get() == Target)
		{
			ChosenId = Pair.Key;
			break;
		}
	}
	if (ChosenId == AfterlightShotIds::DialogueTwoShot)
	{
		Register = EAfterlightCameraRegister::Dialogue;
	}

	ActiveShotId = ChosenId;
	const UAfterlightCameraRecipe* Recipe = GetRecipe(Register);
	ApplyRecipeToActor(Target, Recipe, Focus);
	const float Blend = BlendOverride >= 0.f ? BlendOverride : (Recipe ? Recipe->BlendTime : 0.75f);
	ApplyViewTarget(Target, Blend, Recipe ? static_cast<EViewTargetBlendFunction>(Recipe->BlendFunction) : VTBlend_Cubic);
	if (Register != EAfterlightCameraRegister::Explore)
	{
		BeginPush(Target, Recipe);
	}
	CurrentRegister = Register;
	if (Authority != EAfterlightCameraAuthority::Sequencer)
	{
		Authority = EAfterlightCameraAuthority::Register;
	}
}

void UAfterlightCameraSubsystem::ReleaseToExplore(float BlendOverride)
{
	GetWorld()->GetTimerManager().ClearTimer(PushTimer);
	PushActor.Reset();
	if (Authority == EAfterlightCameraAuthority::Sequencer)
	{
		Authority = EAfterlightCameraAuthority::Gameplay;
	}
	RequestRegister(EAfterlightCameraRegister::Explore, BlendOverride);
}

void UAfterlightCameraSubsystem::CycleDebugShot()
{
	if (DebugShotOrder.Num() == 0)
	{
		return;
	}
	DebugShotIndex = (DebugShotIndex + 1) % DebugShotOrder.Num();
	RequestShot(DebugShotOrder[DebugShotIndex], 0.45f);
}
