#include "Slice/AfterlightSlice01Director.h"
#include "Slice/AfterlightLanternDrone.h"
#include "Character/AfterlightCompanionCharacter.h"
#include "Character/AfterlightCharacter.h"
#include "Character/AfterlightPlayerController.h"
#include "Interaction/AfterlightInspectableActor.h"
#include "Interaction/AfterlightInteractableComponent.h"
#include "Camera/AfterlightCameraAnchorComponent.h"
#include "Camera/AfterlightCameraSubsystem.h"
#include "Camera/AfterlightCameraRecipe.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "CineCameraSettings.h"
#include "Narrative/AfterlightDialogueAsset.h"
#include "Narrative/AfterlightDialogueRunner.h"
#include "Narrative/AfterlightNarrativeSubsystem.h"
#include "Relationship/AfterlightRelationshipSubsystem.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Core/AfterlightGameplayTags.h"
#include "Cinematic/AfterlightCinematicCoordinator.h"
#include "Cinematic/AfterlightLevelSequenceFactory.h"
#include "UI/AfterlightPresentationSubsystem.h"
#include "UI/AfterlightPresentationFormat.h"
#include "Audio/AfterlightTempAudio.h"
#include "Audio/AfterlightVoice.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/PointLight.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Components/PointLightComponent.h"
#include "Engine/TextRenderActor.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/PlatformMisc.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/AfterlightLog.h"
#include "Afterlight.h"
#include "HAL/PlatformMemory.h"
#include "Camera/AfterlightSliceProgress.h"
#include "Engine/PostProcessVolume.h"
#include "Character/AfterlightPlaceholderVisuals.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SpotLight.h"
#include "Engine/GameViewportClient.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ShowFlags.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	const FVector WakeLoc(180.f, 0.f, 92.f);
	const FVector MayaContactLoc(780.f, 40.f, 92.f);
	const FVector MayaApproachLoc(1140.f, 80.f, 92.f);
	const FVector HideLoc(2420.f, 310.f, 92.f);
	const FVector DoorLoc(3380.f, 0.f, 92.f);
	const FVector MugLoc(3720.f, 40.f, 92.f);
	const FVector TinLoc(3940.f, -150.f, 50.f);

	const FLinearColor ColMetal(0.08f, 0.085f, 0.09f);
	const FLinearColor ColPanel(0.2f, 0.16f, 0.11f);
	const FLinearColor ColConcrete(0.2f, 0.18f, 0.16f);
	const FLinearColor ColFloorWet(0.07f, 0.075f, 0.082f);
	const FLinearColor ColAmber(1.f, 0.64f, 0.22f);
	const FLinearColor ColCyan(0.38f, 0.84f, 1.f);
	const FLinearColor ColCutFloor(0.09f, 0.1f, 0.115f);
	const FLinearColor ColHelion(0.1f, 0.14f, 0.17f);
	const FLinearColor ColPumpWall(0.2f, 0.14f, 0.1f);
	const FLinearColor ColPumpFloor(0.12f, 0.09f, 0.07f);
	const FLinearColor ColBeam(0.08f, 0.08f, 0.075f);

	UMaterialInstanceDynamic* CachedColor(UObject* Outer, const FLinearColor& Color)
	{
		(void)Outer;
		static TMap<uint32, TWeakObjectPtr<UMaterialInstanceDynamic>> Cache;
		const uint32 Key = (static_cast<uint32>(FMath::Clamp(Color.R, 0.f, 1.f) * 255.f) << 16)
			| (static_cast<uint32>(FMath::Clamp(Color.G, 0.f, 1.f) * 255.f) << 8)
			| static_cast<uint32>(FMath::Clamp(Color.B, 0.f, 1.f) * 255.f);
		if (TWeakObjectPtr<UMaterialInstanceDynamic>* Found = Cache.Find(Key))
		{
			if (Found->IsValid())
			{
				return Found->Get();
			}
		}
		UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (!BaseMat)
		{
			return nullptr;
		}
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(BaseMat, GetTransientPackage());
		Mid->SetVectorParameterValue(TEXT("Color"), Color);
		Cache.Add(Key, Mid);
		return Mid;
	}

	AStaticMeshActor* SpawnPrim(UWorld* World, UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation,
		const FVector& Scale, const FLinearColor& Color, bool bCollision = true)
	{
		if (!World)
		{
			return nullptr;
		}
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
		if (!Actor)
		{
			return nullptr;
		}
		if (Mesh)
		{
			Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
		}
		Actor->SetActorScale3D(Scale);
		Actor->GetStaticMeshComponent()->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		Actor->GetStaticMeshComponent()->SetCastContactShadow(false);
		Actor->GetStaticMeshComponent()->SetCastShadow(false);
		if (UMaterialInstanceDynamic* Mid = CachedColor(Actor, Color))
		{
			Actor->GetStaticMeshComponent()->SetMaterial(0, Mid);
		}
		return Actor;
	}

	void SpawnWorldSign(UWorld* World, const FVector& Location, const FRotator& Rotation, const FString& Text, float Size, const FColor& Color)
	{
		ATextRenderActor* Sign = World->SpawnActor<ATextRenderActor>(Location, Rotation);
		if (!Sign)
		{
			return;
		}
		if (UTextRenderComponent* TextComp = Sign->GetTextRender())
		{
			TextComp->SetText(FText::FromString(Text));
			TextComp->SetWorldSize(Size);
			TextComp->SetTextRenderColor(Color);
			TextComp->SetHorizontalAlignment(EHTA_Center);
		}
	}

	void SpawnHelionPost(UWorld* World, const FVector& Location)
	{
		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		SpawnPrim(World, Cube, Location, FRotator::ZeroRotator, FVector(0.18f, 0.18f, 2.4f), ColHelion);
		SpawnPrim(World, Cylinder, Location + FVector(0.f, 0.f, 140.f), FRotator::ZeroRotator, FVector(0.22f, 0.22f, 0.12f), ColCyan, false);
		SpawnWorldSign(World, Location + FVector(12.f, 0.f, 110.f), FRotator(0.f, 180.f, 0.f), TEXT("HELION"), 12.f, FColor(90, 190, 210));
		SpawnWorldSign(World, Location + FVector(12.f, 0.f, 88.f), FRotator(0.f, 180.f, 0.f), TEXT("WITNESS"), 10.f, FColor(70, 150, 170));
	}
}

AAfterlightSlice01Director::AAfterlightSlice01Director()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAfterlightSlice01Director::BeginPlay()
{
	Super::BeginPlay();
	bSmoke = FParse::Param(FCommandLine::Get(), TEXT("AfterlightSliceSmoke"))
		|| FParse::Param(FCommandLine::Get(), TEXT("AfterlightSmoke"));
	BuildWorld();
	BuildDialogue();
	BindSystems();
	if (!bSmoke)
	{
		const FPlatformMemoryStats Mem = FPlatformMemory::GetStats();
		UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_MEM usedPhys=%.2fGB availPhys=%.2fGB usedVirt=%.2fGB availVirt=%.2fGB"),
			Mem.UsedPhysical / (1024.0 * 1024.0 * 1024.0),
			Mem.AvailablePhysical / (1024.0 * 1024.0 * 1024.0),
			Mem.UsedVirtual / (1024.0 * 1024.0 * 1024.0),
			Mem.AvailableVirtual / (1024.0 * 1024.0 * 1024.0));
		if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
		{
			UI->SetCineMode(true);
			UI->SetDeveloperOverlay(false);
		}
		if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
		{
			FEngineShowFlags& Flags = Viewport->EngineShowFlags;
			Flags.SetGrid(false);
			Flags.SetNavigation(false);
			Flags.SetBillboardSprites(false);
			Flags.SetSelection(false);
			Flags.SetSplines(false);
			Flags.SetConstraints(false);
			Flags.SetCameraFrustums(false);
		}
		BeginEntry();
	}
	else
	{
		BeginWake();
	}
	MaybeScheduleCommandLineSmoke();
}

void AAfterlightSlice01Director::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Maya)
	{
		return;
	}
	UAfterlightNarrativeSubsystem* Narrative = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>() : nullptr;
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Narrative || !Pawn)
	{
		return;
	}

	UpdateHideRecovery(DeltaSeconds);
	UpdateWarningPresentation(DeltaSeconds);
	UpdateAudioBeds();
	UpdateOwnerGuidance(DeltaSeconds);
	ContainPlayer();
	TryGrantEnteredCut();
	MaybeStartCutTalk();

	if (!bSmoke)
	{
		FpsWindowSeconds += DeltaSeconds;
		++FpsWindowFrames;
		if (FpsWindowSeconds >= 4.f)
		{
			UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_FPS=%.1f"), FpsWindowFrames / FpsWindowSeconds);
			FpsWindowSeconds = 0.f;
			FpsWindowFrames = 0;
		}
	}

	if (!bSmoke && AfterlightQaDriveEnabled() && !bAwaitingEntry && !bSliceEnded)
	{
		if (Narrative->HasFlag(AfterlightTags::Story_Slice01_SweepPassed) && !Narrative->HasFlag(AfterlightTags::Story_Slice01_ReachedBolt))
		{
			HideHintElapsed += DeltaSeconds;
		}
		FVector Target = Maya->GetActorLocation();
		if (bTinStarted || bWarningStarted)
		{
			Target = Pawn->GetActorLocation();
		}
		else if (Narrative->HasFlag(AfterlightTags::Story_Slice01_QuietBeat) && !Narrative->HasFlag(AfterlightTags::Story_Slice01_FoundTin))
		{
			Target = TinLoc;
		}
		else if (bQuietStarted)
		{
			Target = MugLoc;
		}
		else if (Narrative->HasFlag(AfterlightTags::Story_Slice01_SweepPassed))
		{
			const FVector Loc = Pawn->GetActorLocation();
			if (Loc.Y > 90.f)
			{
				Target = FVector(2480.f, 20.f, 92.f);
			}
			else if (Loc.X < 3040.f)
			{
				Target = FVector(2920.f, 16.f, 92.f);
			}
			else
			{
				Target = DoorLoc;
			}
		}
		else if (bSweepStarted)
		{
			Target = HideLoc;
		}
		else if (Narrative->HasFlag(AfterlightTags::Story_Slice01_EnteredCut))
		{
			Target = FVector(2100.f, 0.f, 92.f);
		}
		FVector Delta = Target - Pawn->GetActorLocation();
		Delta.Z = 0.f;
		if (Delta.Size() > 48.f)
		{
			Pawn->AddMovementInput(Delta.GetSafeNormal(), AfterlightQaStoryEnabled() ? 0.42f : 1.f);
		}
		else if (Narrative->HasFlag(AfterlightTags::Story_Slice01_SweepPassed) && !Narrative->HasFlag(AfterlightTags::Story_Slice01_ReachedBolt)
			&& (FVector::Dist2D(Pawn->GetActorLocation(), DoorLoc) < 220.f || (Pawn->GetActorLocation().X > 3260.f && FMath::Abs(Pawn->GetActorLocation().Y) < 140.f) || HideHintElapsed > 12.f))
		{
			if (HideHintElapsed > 12.f && FVector::Dist2D(Pawn->GetActorLocation(), DoorLoc) > 220.f)
			{
				Pawn->TeleportTo(DoorLoc + FVector(-90.f, 0.f, 0.f), Pawn->GetActorRotation(), false, true);
			}
			HandleDoor(Pawn);
		}
		else if (Narrative->HasFlag(AfterlightTags::Story_Slice01_QuietBeat) && !Narrative->HasFlag(AfterlightTags::Story_Slice01_FoundTin) && FVector::Dist2D(Pawn->GetActorLocation(), TinLoc) < 180.f)
		{
			HandleTin(Pawn);
		}
	}

	if (!bContactStarted && Narrative->HasFlag(AfterlightTags::Story_Slice01_Woke) && !Narrative->HasFlag(AfterlightTags::Story_Slice01_MetMaya))
	{
		if (FVector::Dist2D(Pawn->GetActorLocation(), Maya->GetActorLocation()) < 240.f)
		{
			if (!GetWorldTimerManager().IsTimerActive(AutoTalkHandle))
			{
				GetWorldTimerManager().SetTimer(AutoTalkHandle, this, &AAfterlightSlice01Director::BeginContact, bSmoke ? 0.05f : 10.f, false);
			}
		}
	}

	if (!bSweepStarted && Narrative->HasFlag(AfterlightTags::Story_Slice01_EnteredCut))
	{
		SweepFailElapsed += DeltaSeconds;
		if (Pawn->GetActorLocation().X > 1980.f
			|| (AfterlightQaDriveEnabled() && SweepFailElapsed > 7.f)
			|| SweepFailElapsed > 28.f)
		{
			if (AfterlightQaDriveEnabled() && Pawn->GetActorLocation().X < 1980.f)
			{
				Pawn->TeleportTo(FVector(2120.f, 20.f, 92.f), Pawn->GetActorRotation(), false, true);
				if (Maya)
				{
					Maya->SetActorLocation(HideLoc);
				}
			}
			BeginSweep();
		}
	}

	if (!Narrative->HasFlag(AfterlightTags::Story_Slice01_SweepPassed) && Pawn->GetActorLocation().X > 3280.f)
	{
		bHideRecovering = false;
		if (!bSweepStarted)
		{
			bSweepStarted = true;
		}
		CompleteSweep();
	}
}

void AAfterlightSlice01Director::SetBeat(FName BeatId)
{
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->SetCurrentBeatId(BeatId);
	}
}

void AAfterlightSlice01Director::ApplyInputFull()
{
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Full);
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->ReleaseToExplore(0.8f);
	}
}

AStaticMeshActor* AAfterlightSlice01Director::SpawnBox(const FVector& Location, const FVector& Scale, const FLinearColor& Color)
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	AStaticMeshActor* Box = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
	if (!Box)
	{
		return nullptr;
	}
	if (Cube)
	{
		Box->GetStaticMeshComponent()->SetStaticMesh(Cube);
	}
	Box->SetActorScale3D(Scale);
	Box->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMat)
	{
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(BaseMat, Box);
		Mid->SetVectorParameterValue(TEXT("Color"), Color);
		Box->GetStaticMeshComponent()->SetMaterial(0, Mid);
	}
	return Box;
}

void AAfterlightSlice01Director::SpawnSign(const FVector& Location, const FRotator& Rotation, const FString& Text, float Size, const FColor& Color)
{
	ATextRenderActor* Sign = GetWorld()->SpawnActor<ATextRenderActor>(Location, Rotation);
	if (!Sign)
	{
		return;
	}
	if (UTextRenderComponent* TextComp = Sign->GetTextRender())
	{
		TextComp->SetText(FText::FromString(Text));
		TextComp->SetWorldSize(Size);
		TextComp->SetTextRenderColor(Color);
		TextComp->SetHorizontalAlignment(EHTA_Center);
	}
}

APointLight* AAfterlightSlice01Director::SpawnLight(const FVector& Location, const FLinearColor& Color, float Intensity, float Radius)
{
	APointLight* Light = GetWorld()->SpawnActor<APointLight>(Location, FRotator::ZeroRotator);
	if (!Light)
	{
		return nullptr;
	}
	if (UPointLightComponent* Comp = Light->FindComponentByClass<UPointLightComponent>())
	{
		Comp->SetLightColor(Color);
		Comp->SetIntensity(Intensity);
		Comp->SetAttenuationRadius(Radius);
		Comp->SetCastShadows(false);
	}
	return Light;
}

ACineCameraActor* AAfterlightSlice01Director::SpawnShot(FName ShotId, const FVector& Location, const FVector& LookAt, uint8 Register)
{
	const EAfterlightCameraRegister CamReg = static_cast<EAfterlightCameraRegister>(Register);
	const FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Location, LookAt);
	ACineCameraActor* Camera = GetWorld()->SpawnActor<ACineCameraActor>(Location, Rotation);
	if (!Camera)
	{
		return nullptr;
	}
	UAfterlightCameraAnchorComponent* Anchor = NewObject<UAfterlightCameraAnchorComponent>(Camera);
	Anchor->Register = CamReg;
	Anchor->ShotId = ShotId;
	Camera->AddInstanceComponent(Anchor);
	Anchor->RegisterComponent();
	if (UAfterlightCameraSubsystem* CameraSys = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		CameraSys->RegisterAnchor(CamReg, Camera);
		CameraSys->RegisterShot(ShotId, Camera);
	}
	if (ShotId == AfterlightShotIds::RevealInsert)
	{
		WarningCamera = Camera;
	}
	else if (ShotId == AfterlightShotIds::DialogueOTSCompanion)
	{
		ContactOTS = Camera;
	}
	else if (ShotId == AfterlightShotIds::DialogueOTSProtagonist)
	{
		ContactPlayerOTS = Camera;
	}
	else if (ShotId == AfterlightShotIds::DialogueTwoShot)
	{
		ContactTwoShot = Camera;
	}
	else if (ShotId == AfterlightShotIds::DialogueCloseUpCompanion)
	{
		ContactClose = Camera;
	}
	else if (ShotId == AfterlightShotIds::ThreatPressure)
	{
		ThreatShot = Camera;
	}
	return Camera;
}

void AAfterlightSlice01Director::AimShot(ACineCameraActor* Camera, const FVector& Location, const FVector& LookAt)
{
	if (!Camera)
	{
		return;
	}
	Camera->SetActorLocation(Location);
	Camera->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(Location, LookAt));
}

void AAfterlightSlice01Director::HoldShot(ACineCameraActor* Camera, float Blend)
{
	if (!Camera)
	{
		return;
	}
	if (UCineCameraComponent* Cine = Camera->GetCineCameraComponent())
	{
		Cine->CurrentFocalLength = 18.f;
		Cine->CurrentAperture = 4.5f;
		Cine->Filmback.SensorWidth = 24.89f;
		Cine->Filmback.SensorHeight = 14.f;
		Cine->FocusSettings.FocusMethod = ECameraFocusMethod::DoNotOverride;
	}
	if (UAfterlightCameraSubsystem* CameraSys = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		CameraSys->ForceView(Camera, Blend);
	}
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->SetViewTargetWithBlend(Camera, Blend);
	}
}

void AAfterlightSlice01Director::PlayTempVoice(FName SpeakerId, FName VoiceId)
{
	LastVoiceSeconds = 0.f;
	if (bSmoke || VoiceId.IsNone())
	{
		return;
	}
	LastVoiceSeconds = FAfterlightVoice::Play(GetWorld(), this, SpeakerId, VoiceId);
}

void AAfterlightSlice01Director::RebuildContactShots()
{
	AimShot(ContactTwoShot, FVector(760.f, 305.f, 152.f), FVector(790.f, 18.f, 100.f));
	AimShot(ContactOTS, FVector(560.f, 220.f, 150.f), MayaContactLoc + FVector(40.f, -20.f, 72.f));
	AimShot(ContactClose, FVector(620.f, 240.f, 148.f), MayaContactLoc + FVector(30.f, -10.f, 68.f));
	AimShot(ContactPlayerOTS, FVector(980.f, 230.f, 150.f), FVector(680.f, 30.f, 78.f));
}

void AAfterlightSlice01Director::RebuildQuietShot()
{
	const FVector MayaLoc = Maya ? Maya->GetActorLocation() : (MugLoc + FVector(ChoseFollow() ? 90.f : 180.f, ChoseFollow() ? 70.f : 130.f, 0.f));
	AimShot(ContactTwoShot, MugLoc + FVector(-80.f, ChoseFollow() ? -280.f : -320.f, 120.f), MugLoc + FVector(20.f, 20.f, 48.f));
	AimShot(ContactOTS, MugLoc + FVector(-280.f, -40.f, 160.f), MugLoc + FVector(70.f, 50.f, 40.f));
	AimShot(ContactClose, MayaLoc + FVector(-140.f, ChoseFollow() ? -160.f : -200.f, 90.f), MayaLoc + FVector(8.f, 4.f, 50.f));
}

void AAfterlightSlice01Director::RebuildThreatShot()
{
	AimShot(ThreatShot, FVector(2480.f, 545.f, 158.f), FVector(2100.f, 30.f, 140.f));
}

void AAfterlightSlice01Director::RebuildTinShot()
{
	AimShot(ThreatShot, FVector(3680.f, -270.f, 148.f), FVector(3948.f, -120.f, 56.f));
}

void AAfterlightSlice01Director::RebuildWarningReactionShot()
{
	AimShot(ContactClose, FVector(3820.f, 160.f, 150.f), FVector(3940.f, -80.f, 108.f));
}

void AAfterlightSlice01Director::TryGrantEnteredCut()
{
	UAfterlightNarrativeSubsystem* Narrative = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>() : nullptr;
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Narrative || !Pawn || Narrative->HasFlag(AfterlightTags::Story_Slice01_EnteredCut))
	{
		return;
	}
	const bool bHasChoice = Narrative->HasFlag(AfterlightTags::Story_Slice01_ChoseFollow) || Narrative->HasFlag(AfterlightTags::Story_Slice01_ChoseQuestion);
	if (bHasChoice)
	{
		CutLeadElapsed += GetWorld()->GetDeltaSeconds();
	}
	const float MayaX = Maya ? Maya->GetActorLocation().X : 0.f;
	if (FAfterlightSliceProgress::ShouldGrantEnteredCut(Pawn->GetActorLocation().X, Pawn->GetActorLocation().Y, MayaX, bHasChoice, CutLeadElapsed))
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_EnteredCut);
		SetBeat(TEXT("Slice01.LanternCut"));
		UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_ENTERED_CUT"));
	}
}

void AAfterlightSlice01Director::ContainPlayer()
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn || bHideRecovering)
	{
		return;
	}
	const FVector Loc = Pawn->GetActorLocation();
	if (Loc.Z >= 20.f && FAfterlightSliceProgress::IsInsidePlayableXY(Loc.X, Loc.Y))
	{
		return;
	}
	const FVector Safe = FAfterlightSliceProgress::ClampToPlayable(Loc);
	Pawn->TeleportTo(Safe, Pawn->GetActorRotation(), false, true);
}

void AAfterlightSlice01Director::MaybeStartCutTalk()
{
	if (!bCutTalkPending || bSmoke)
	{
		return;
	}
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn || !Maya || !CutDialogue)
	{
		return;
	}
	const float Dist = FVector::Dist2D(Pawn->GetActorLocation(), Maya->GetActorLocation());
	if (Dist > 520.f && CutLeadElapsed < 8.f)
	{
		return;
	}
	bCutTalkPending = false;
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GetDialogueRunner()->Start(CutDialogue);
	}
}

void AAfterlightSlice01Director::BuildWorld()
{
	if (bBuilt)
	{
		return;
	}
	bBuilt = true;

	UWorld* World = GetWorld();
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	if (AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector::ZeroVector, FRotator::ZeroRotator))
	{
		if (UExponentialHeightFogComponent* Comp = Fog->GetComponent())
		{
			Comp->SetFogDensity(0.0085f);
			Comp->SetFogHeightFalloff(0.06f);
			Comp->SetFogInscatteringColor(FLinearColor(0.14f, 0.13f, 0.12f));
			Comp->SetVolumetricFog(false);
		}
	}
	if (ADirectionalLight* Key = World->SpawnActor<ADirectionalLight>(FVector(800.f, 0.f, 400.f), FRotator(-38.f, 18.f, 0.f)))
	{
		if (UDirectionalLightComponent* Comp = Key->FindComponentByClass<UDirectionalLightComponent>())
		{
			Comp->SetIntensity(4.4f);
			Comp->SetLightColor(FLinearColor(1.f, 0.84f, 0.66f));
			Comp->SetCastShadows(false);
			Comp->SetAtmosphereSunLight(false);
		}
	}
	if (ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector(2000.f, 0.f, 300.f), FRotator::ZeroRotator))
	{
		if (USkyLightComponent* Comp = Sky->GetLightComponent())
		{
			Comp->SetIntensity(1.45f);
			Comp->SetLightColor(FLinearColor(0.62f, 0.7f, 0.82f));
			Comp->bRealTimeCapture = false;
		}
	}

	auto Module = [&](float CenterX, const FLinearColor& Floor, const FLinearColor& Wall, float WidthY)
	{
		SpawnPrim(World, Cube, FVector(CenterX, 0.f, -10.f), FRotator::ZeroRotator, FVector(4.2f, WidthY, 0.2f), Floor);
		SpawnPrim(World, Cube, FVector(CenterX, -WidthY * 50.f, 160.f), FRotator::ZeroRotator, FVector(4.2f, 0.28f, 4.2f), Wall);
		SpawnPrim(World, Cube, FVector(CenterX, WidthY * 50.f, 160.f), FRotator::ZeroRotator, FVector(4.2f, 0.28f, 4.2f), Wall);
		SpawnPrim(World, Cube, FVector(CenterX, 0.f, 248.f), FRotator::ZeroRotator, FVector(4.2f, 0.18f, 0.18f), ColBeam, false);
		SpawnPrim(World, Cube, FVector(CenterX, -WidthY * 50.f + 18.f, 210.f), FRotator::ZeroRotator, FVector(3.6f, 0.06f, 0.04f), ColAmber, false);
		if (Cylinder)
		{
			SpawnPrim(World, Cylinder, FVector(CenterX, -WidthY * 50.f + 36.f, 70.f), FRotator(0.f, 0.f, 90.f), FVector(0.12f, 0.12f, 3.8f), ColMetal, false);
			SpawnPrim(World, Cylinder, FVector(CenterX, WidthY * 50.f - 36.f, 40.f), FRotator(0.f, 0.f, 90.f), FVector(0.08f, 0.08f, 3.6f), ColBeam, false);
		}
		SpawnPrim(World, Cube, FVector(CenterX - 80.f, -WidthY * 50.f + 22.f, 90.f), FRotator::ZeroRotator, FVector(1.1f, 0.06f, 0.9f), ColPanel, false);
	};

	SpawnPrim(World, Cube, FVector(-50.f, 0.f, 160.f), FRotator::ZeroRotator, FVector(0.35f, 8.2f, 4.2f), ColMetal);
	for (int32 i = 0; i < 4; ++i)
	{
		Module(200.f + i * 400.f, ColFloorWet, ColMetal, 8.0f);
	}
	SpawnPrim(World, Cube, FVector(800.f, 0.f, 3.f), FRotator::ZeroRotator, FVector(14.5f, 0.22f, 0.03f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(520.f, 0.f, 3.f), FRotator::ZeroRotator, FVector(0.9f, 1.6f, 0.04f), FLinearColor(1.f, 0.72f, 0.28f), false);
	SpawnPrim(World, Cube, FVector(800.f, 0.f, 318.f), FRotator::ZeroRotator, FVector(16.2f, 8.2f, 0.16f), ColMetal);
	SpawnPrim(World, Cube, FVector(760.f, -372.f, 150.f), FRotator::ZeroRotator, FVector(2.8f, 0.08f, 2.4f), ColPanel, false);
	SpawnPrim(World, Cube, FVector(760.f, -368.f, 210.f), FRotator::ZeroRotator, FVector(2.4f, 0.04f, 0.05f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(760.f, -368.f, 92.f), FRotator::ZeroRotator, FVector(2.4f, 0.04f, 0.05f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(620.f, -360.f, 140.f), FRotator::ZeroRotator, FVector(0.08f, 0.22f, 1.8f), ColMetal, false);
	SpawnPrim(World, Cube, FVector(900.f, -360.f, 140.f), FRotator::ZeroRotator, FVector(0.08f, 0.22f, 1.8f), ColMetal, false);
	if (Cylinder)
	{
		SpawnPrim(World, Cylinder, FVector(780.f, -340.f, 70.f), FRotator(0.f, 0.f, 90.f), FVector(0.14f, 0.14f, 2.2f), ColMetal, false);
		SpawnPrim(World, Cylinder, FVector(780.f, -340.f, 110.f), FRotator(0.f, 0.f, 90.f), FVector(0.1f, 0.1f, 2.4f), ColBeam, false);
	}
	SpawnPrim(World, Cube, FVector(780.f, -348.f, 130.f), FRotator::ZeroRotator, FVector(3.4f, 0.1f, 2.2f), ColConcrete, false);
	SpawnPrim(World, Cube, FVector(760.f, -168.f, 120.f), FRotator::ZeroRotator, FVector(2.8f, 0.08f, 2.0f), ColPanel, false);
	SpawnPrim(World, Cube, FVector(760.f, -164.f, 168.f), FRotator::ZeroRotator, FVector(2.4f, 0.04f, 0.05f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(640.f, -330.f, 70.f), FRotator::ZeroRotator, FVector(0.16f, 0.16f, 1.4f), ColMetal, false);
	SpawnPrim(World, Cube, FVector(920.f, -330.f, 70.f), FRotator::ZeroRotator, FVector(0.16f, 0.16f, 1.4f), ColMetal, false);
	SpawnWorldSign(World, FVector(760.f, -338.f, 168.f), FRotator(0.f, 90.f, 0.f), TEXT("UD-9  CREW"), 14.f, FColor(220, 160, 70));
	SpawnWorldSign(World, FVector(760.f, -338.f, 148.f), FRotator(0.f, 90.f, 0.f), TEXT("ROUTE OPEN"), 10.f, FColor(180, 120, 50));
	SpawnPrim(World, Cube, FVector(788.f, -86.f, 168.f), FRotator::ZeroRotator, FVector(6.4f, 0.16f, 3.6f), ColPanel, false);
	SpawnPrim(World, Cube, FVector(788.f, 40.f, 248.f), FRotator::ZeroRotator, FVector(6.4f, 3.2f, 0.12f), ColMetal, false);
	SpawnPrim(World, Cube, FVector(788.f, -80.f, 214.f), FRotator::ZeroRotator, FVector(6.0f, 0.05f, 0.07f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(788.f, -80.f, 76.f), FRotator::ZeroRotator, FVector(6.0f, 0.05f, 0.05f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(640.f, -78.f, 120.f), FRotator::ZeroRotator, FVector(0.12f, 0.18f, 2.1f), ColMetal, false);
	SpawnPrim(World, Cube, FVector(940.f, -78.f, 120.f), FRotator::ZeroRotator, FVector(0.12f, 0.18f, 2.1f), ColMetal, false);
	SpawnPrim(World, Cube, FVector(1020.f, -40.f, 128.f), FRotator::ZeroRotator, FVector(1.8f, 0.1f, 2.2f), ColConcrete, false);
	if (Cylinder)
	{
		SpawnPrim(World, Cylinder, FVector(720.f, -62.f, 68.f), FRotator(0.f, 0.f, 90.f), FVector(0.12f, 0.12f, 2.6f), ColMetal, false);
		SpawnPrim(World, Cylinder, FVector(860.f, -58.f, 108.f), FRotator(0.f, 0.f, 90.f), FVector(0.08f, 0.08f, 2.2f), ColBeam, false);
	}
	SpawnWorldSign(World, FVector(788.f, -76.f, 172.f), FRotator(0.f, 90.f, 0.f), TEXT("UNDERDECK 9"), 16.f, FColor(230, 170, 80));
	SpawnWorldSign(World, FVector(788.f, -76.f, 152.f), FRotator(0.f, 90.f, 0.f), TEXT("SERVICE  UD-9"), 11.f, FColor(190, 140, 70));
	SpawnLight(FVector(788.f, -36.f, 176.f), FLinearColor(1.f, 0.8f, 0.52f), 22.f, 820.f);
	SpawnLight(FVector(760.f, 90.f, 148.f), FLinearColor(1.f, 0.84f, 0.6f), 11.f, 560.f);
	SpawnLight(FVector(790.f, 200.f, 170.f), FLinearColor(1.f, 0.78f, 0.5f), 7.f, 480.f);

	SpawnPrim(World, Cube, FVector(1480.f, 0.f, 3.f), FRotator::ZeroRotator, FVector(2.2f, 0.28f, 0.04f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(1520.f, 0.f, 3.f), FRotator::ZeroRotator, FVector(2.2f, 0.28f, 0.04f), ColCyan, false);
	SpawnPrim(World, Cube, FVector(1580.f, 0.f, 3.f), FRotator::ZeroRotator, FVector(1.4f, 2.2f, 0.04f), ColCyan, false);
	SpawnPrim(World, Cube, FVector(1580.f, -200.f, 130.f), FRotator::ZeroRotator, FVector(0.16f, 0.16f, 2.6f), ColCyan, false);
	SpawnPrim(World, Cube, FVector(1580.f, 200.f, 130.f), FRotator::ZeroRotator, FVector(0.16f, 0.16f, 2.6f), ColCyan, false);
	SpawnPrim(World, Cube, FVector(1580.f, 0.f, 268.f), FRotator::ZeroRotator, FVector(0.16f, 4.2f, 0.16f), ColCyan, false);

	for (int32 i = 0; i < 5; ++i)
	{
		const float X = 1780.f + i * 360.f;
		SpawnPrim(World, Cube, FVector(X, 0.f, -10.f), FRotator::ZeroRotator, FVector(3.8f, 8.6f, 0.2f), ColCutFloor);
		SpawnPrim(World, Cube, FVector(X, -430.f, 170.f), FRotator::ZeroRotator, FVector(3.8f, 0.22f, 3.6f), ColHelion);
		if (i != 2)
		{
			SpawnPrim(World, Cube, FVector(X, 430.f, 170.f), FRotator::ZeroRotator, FVector(3.8f, 0.22f, 3.6f), ColHelion);
		}
		SpawnPrim(World, Cube, FVector(X, 0.f, 4.f), FRotator::ZeroRotator, FVector(2.8f, 0.18f, 0.03f), ColCyan, false);
		if (Cylinder)
		{
			SpawnPrim(World, Cylinder, FVector(X, -400.f, 90.f), FRotator(0.f, 0.f, 90.f), FVector(0.1f, 0.1f, 3.4f), ColMetal, false);
		}
	}
	SpawnPrim(World, Cube, FVector(2420.f, 500.f, -10.f), FRotator::ZeroRotator, FVector(3.4f, 2.2f, 0.2f), ColCutFloor);
	SpawnPrim(World, Cube, FVector(2420.f, 600.f, 160.f), FRotator::ZeroRotator, FVector(3.4f, 0.28f, 4.2f), ColHelion);
	SpawnPrim(World, Cube, FVector(2260.f, 520.f, 160.f), FRotator::ZeroRotator, FVector(0.28f, 2.f, 4.2f), ColHelion);
	SpawnPrim(World, Cube, FVector(2580.f, 520.f, 160.f), FRotator::ZeroRotator, FVector(0.28f, 2.f, 4.2f), ColHelion);
	SpawnPrim(World, Cube, FVector(2340.f, 380.f, 28.f), FRotator::ZeroRotator, FVector(1.1f, 0.8f, 1.0f), ColHelion);
	SpawnPrim(World, Cube, FVector(2500.f, 80.f, 3.f), FRotator::ZeroRotator, FVector(2.8f, 0.22f, 0.04f), ColCyan, false);
	SpawnPrim(World, Cube, FVector(2920.f, 0.f, 3.f), FRotator::ZeroRotator, FVector(3.6f, 0.16f, 0.03f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(2300.f, 0.f, 318.f), FRotator::ZeroRotator, FVector(16.8f, 8.6f, 0.16f), ColHelion);
	SpawnHelionPost(World, FVector(1880.f, -250.f, 0.f));
	SpawnHelionPost(World, FVector(2280.f, 250.f, 0.f));
	SpawnHelionPost(World, FVector(2680.f, -220.f, 0.f));

	SpawnPrim(World, Cube, FVector(3820.f, 0.f, -10.f), FRotator::ZeroRotator, FVector(8.4f, 8.2f, 0.2f), ColPumpFloor);
	SpawnPrim(World, Cube, FVector(3400.f, -420.f, 150.f), FRotator::ZeroRotator, FVector(0.35f, 4.f, 3.8f), ColPumpWall);
	SpawnPrim(World, Cube, FVector(3400.f, 420.f, 150.f), FRotator::ZeroRotator, FVector(0.35f, 4.f, 3.8f), ColPumpWall);
	SpawnPrim(World, Cube, FVector(3400.f, 310.f, 160.f), FRotator::ZeroRotator, FVector(0.35f, 2.2f, 4.2f), ColPumpWall);
	SpawnPrim(World, Cube, FVector(3400.f, -310.f, 160.f), FRotator::ZeroRotator, FVector(0.35f, 2.2f, 4.2f), ColPumpWall);
	SpawnPrim(World, Cube, FVector(3380.f, 0.f, 210.f), FRotator::ZeroRotator, FVector(0.18f, 1.8f, 0.18f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(3380.f, -90.f, 110.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 2.2f), ColMetal);
	SpawnPrim(World, Cube, FVector(3380.f, 90.f, 110.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 2.2f), ColMetal);
	SpawnPrim(World, Cube, FVector(4220.f, 0.f, 150.f), FRotator::ZeroRotator, FVector(0.35f, 8.2f, 3.8f), ColPumpWall);
	SpawnPrim(World, Cube, FVector(3820.f, -420.f, 150.f), FRotator::ZeroRotator, FVector(8.4f, 0.28f, 3.8f), ColPumpWall);
	SpawnPrim(World, Cube, FVector(3820.f, 420.f, 150.f), FRotator::ZeroRotator, FVector(8.4f, 0.28f, 3.8f), ColPumpWall);
	SpawnPrim(World, Cube, FVector(3720.f, 90.f, 42.f), FRotator::ZeroRotator, FVector(1.8f, 0.82f, 0.42f), ColPanel);
	SpawnPrim(World, Cube, FVector(3720.f, 40.f, 70.f), FRotator::ZeroRotator, FVector(0.16f, 0.16f, 0.2f), FLinearColor(0.42f, 0.22f, 0.1f), false);
	SpawnPrim(World, Cube, FVector(3704.f, 40.f, 64.f), FRotator::ZeroRotator, FVector(0.22f, 0.22f, 0.04f), ColConcrete, false);
	SpawnPrim(World, Cube, FVector(3860.f, 180.f, 70.f), FRotator::ZeroRotator, FVector(1.4f, 0.9f, 1.6f), ColMetal);
	SpawnPrim(World, Cube, FVector(3820.f, 0.f, 248.f), FRotator::ZeroRotator, FVector(4.8f, 0.08f, 0.06f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(3960.f, -210.f, 70.f), FRotator::ZeroRotator, FVector(0.7f, 0.18f, 1.1f), ColPanel);
	SpawnPrim(World, Cube, FVector(3960.f, -210.f, 128.f), FRotator::ZeroRotator, FVector(0.72f, 0.2f, 0.06f), ColAmber, false);
	if (Cylinder)
	{
		SpawnPrim(World, Cylinder, FVector(3860.f, 180.f, 160.f), FRotator::ZeroRotator, FVector(0.35f, 0.35f, 0.9f), ColBeam, false);
		SpawnPrim(World, Cylinder, FVector(4000.f, 200.f, 40.f), FRotator(0.f, 0.f, 90.f), FVector(0.16f, 0.16f, 2.4f), ColMetal, false);
		SpawnPrim(World, Cylinder, FVector(4000.f, -80.f, 80.f), FRotator(0.f, 0.f, 90.f), FVector(0.1f, 0.1f, 2.8f), ColBeam, false);
	}
	SpawnPrim(World, Cube, FVector(3940.f, -150.f, 28.f), FRotator::ZeroRotator, FVector(0.55f, 0.4f, 0.18f), ColPanel);
	SpawnPrim(World, Cube, FVector(3820.f, 0.f, 318.f), FRotator::ZeroRotator, FVector(8.4f, 8.2f, 0.16f), ColPumpWall);
	SpawnPrim(World, Cube, FVector(3680.f, -180.f, 90.f), FRotator::ZeroRotator, FVector(1.1f, 0.7f, 1.4f), ColMetal);
	SpawnPrim(World, Cube, FVector(3680.f, -180.f, 42.f), FRotator::ZeroRotator, FVector(1.4f, 0.55f, 0.28f), ColPanel);
	SpawnPrim(World, Cube, FVector(4040.f, 80.f, 70.f), FRotator::ZeroRotator, FVector(0.9f, 1.6f, 0.9f), ColMetal);
	if (Cylinder)
	{
		SpawnPrim(World, Cylinder, FVector(3600.f, -40.f, 40.f), FRotator(0.f, 0.f, 90.f), FVector(0.14f, 0.14f, 3.2f), ColMetal, false);
		SpawnPrim(World, Cylinder, FVector(3600.f, 80.f, 80.f), FRotator(0.f, 0.f, 90.f), FVector(0.1f, 0.1f, 2.8f), ColBeam, false);
		SpawnPrim(World, Cylinder, FVector(4100.f, -200.f, 120.f), FRotator(0.f, 0.f, 90.f), FVector(0.12f, 0.12f, 2.6f), ColMetal, false);
	}
	SpawnPrim(World, Cube, FVector(3880.f, -220.f, 36.f), FRotator::ZeroRotator, FVector(1.2f, 0.4f, 0.36f), ColPanel);
	SpawnPrim(World, Cube, FVector(3820.f, 0.f, 248.f), FRotator::ZeroRotator, FVector(6.8f, 0.16f, 0.12f), ColMetal, false);
	SpawnPrim(World, Cube, FVector(3720.f, 40.f, 210.f), FRotator::ZeroRotator, FVector(1.8f, 0.08f, 0.05f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(3940.f, -210.f, 78.f), FRotator::ZeroRotator, FVector(0.08f, 1.1f, 1.6f), ColPanel);
	SpawnPrim(World, Cube, FVector(3940.f, -210.f, 118.f), FRotator::ZeroRotator, FVector(0.42f, 1.05f, 0.06f), ColMetal, false);
	SpawnPrim(World, Cube, FVector(3720.f, 40.f, 78.f), FRotator::ZeroRotator, FVector(0.22f, 0.22f, 0.08f), FLinearColor(0.18f, 0.12f, 0.08f), false);
	if (Cylinder)
	{
		SpawnPrim(World, Cylinder, FVector(3780.f, 220.f, 40.f), FRotator::ZeroRotator, FVector(0.7f, 0.7f, 0.85f), ColMetal);
		SpawnPrim(World, Cylinder, FVector(3780.f, 220.f, 130.f), FRotator::ZeroRotator, FVector(0.22f, 0.22f, 0.7f), ColBeam, false);
	}
	SpawnPrim(World, Cube, FVector(2500.f, 80.f, 3.f), FRotator::ZeroRotator, FVector(9.2f, 0.16f, 0.03f), ColAmber, false);
	SpawnPrim(World, Cube, FVector(3360.f, -80.f, 208.f), FRotator::ZeroRotator, FVector(0.1f, 0.1f, 0.18f), ColCyan, false);

	SpawnLight(FVector(520.f, 0.f, 190.f), FLinearColor(1.f, 0.8f, 0.56f), 18.f, 1600.f);
	SpawnLight(FVector(760.f, -220.f, 168.f), FLinearColor(1.f, 0.76f, 0.48f), 12.f, 980.f);
	SpawnLight(FVector(1980.f, 0.f, 230.f), FLinearColor(0.55f, 0.84f, 1.f), 14.f, 1700.f);
	SpawnLight(FVector(2420.f, 240.f, 180.f), FLinearColor(0.62f, 0.88f, 1.f), 6.5f, 700.f);
	SpawnLight(FVector(3720.f, 0.f, 180.f), FLinearColor(1.f, 0.74f, 0.42f), 11.f, 1100.f);
	WitnessLed = SpawnLight(FVector(3820.f, -20.f, 246.f), FLinearColor(0.55f, 0.9f, 1.f), 0.08f, 280.f);
	WitnessCore = SpawnPrim(World, Cube, FVector(3820.f, -20.f, 254.f), FRotator::ZeroRotator, FVector(0.32f, 0.32f, 0.2f), ColCyan, false);
	HatchLight = SpawnLight(FVector(3380.f, 0.f, 170.f), FLinearColor(1.f, 0.82f, 0.5f), 3.4f, 480.f);
	TinLight = SpawnLight(FVector(3940.f, -150.f, 90.f), FLinearColor(1.f, 0.8f, 0.48f), 0.45f, 220.f);
	SpawnLight(FVector(3720.f, 40.f, 140.f), FLinearColor(1.f, 0.7f, 0.38f), 4.2f, 360.f);

	SpawnSign(FVector(240.f, -250.f, 150.f), FRotator(0.f, 180.f, 0.f), TEXT("UNDERDECK 9"), 18.f, FColor(230, 170, 80));
	SpawnSign(FVector(240.f, -250.f, 128.f), FRotator(0.f, 180.f, 0.f), TEXT("SERVICE"), 12.f, FColor(190, 140, 70));
	SpawnSign(FVector(980.f, 250.f, 150.f), FRotator(0.f, -180.f, 0.f), TEXT("PMP-12"), 16.f, FColor(200, 140, 70));
	SpawnSign(FVector(1560.f, 0.f, 210.f), FRotator(0.f, 180.f, 0.f), TEXT("LANTERN CUT"), 22.f, FColor(120, 200, 220));
	SpawnSign(FVector(1980.f, -390.f, 210.f), FRotator(0.f, 90.f, 0.f), TEXT("HELION  02:17"), 24.f, FColor(130, 210, 230));
	SpawnSign(FVector(2200.f, -390.f, 150.f), FRotator(0.f, 90.f, 0.f), TEXT("WITNESS COVERAGE VARIABLE"), 14.f, FColor(100, 170, 190));
	SpawnSign(FVector(3370.f, 90.f, 180.f), FRotator(0.f, -90.f, 0.f), TEXT("PUMP HOUSE 12"), 18.f, FColor(210, 150, 80));

	const FVector MayaSpawn = bSmoke ? MayaContactLoc : MayaApproachLoc;
	Maya = GetWorld()->SpawnActor<AAfterlightCompanionCharacter>(MayaSpawn, FRotator(0.f, 180.f, 0.f));

	if (Maya)
	{
		TArray<FVector> Looks;
		Looks.Add(FVector(520.f, -250.f, 150.f));
		Looks.Add(FVector(1980.f, -390.f, 210.f));
		Looks.Add(HideLoc + FVector(0.f, 0.f, 40.f));
		Looks.Add(DoorLoc + FVector(0.f, 0.f, 80.f));
		Looks.Add(MugLoc + FVector(0.f, 0.f, 40.f));
		Looks.Add(TinLoc + FVector(0.f, 0.f, 20.f));
		Looks.Add(FVector(4018.f, -150.f, 110.f));
		Maya->SetWorldLookTargets(Looks);
		if (!bSmoke)
		{
			TArray<FVector> Approach;
			Approach.Add(MayaContactLoc);
			Maya->SetLeadPath(Approach, false);
		}
		if (UAfterlightInteractableComponent* Comp = Maya->FindComponentByClass<UAfterlightInteractableComponent>())
		{
			Comp->GrantFlag = AfterlightTags::Story_Slice01_MetMaya;
			Comp->BlockedFlag = AfterlightTags::Story_Slice01_MetMaya;
			Comp->OnInteracted.AddDynamic(this, &AAfterlightSlice01Director::HandleMayaTalk);
		}
	}

	Door = GetWorld()->SpawnActor<AAfterlightInspectableActor>(DoorLoc + FVector(0.f, 0.f, -40.f), FRotator::ZeroRotator);
	if (Door)
	{
		Door->SetActorScale3D(FVector(0.35f, 1.5f, 2.2f));
		if (UStaticMeshComponent* Mesh = Door->FindComponentByClass<UStaticMeshComponent>())
		{
			if (UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
			{
				UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(BaseMat, Door);
				Mid->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.08f, 0.07f, 0.05f));
				Mesh->SetMaterial(0, Mid);
			}
		}
		if (UAfterlightInteractableComponent* Comp = Door->FindComponentByClass<UAfterlightInteractableComponent>())
		{
			Comp->PromptText = NSLOCTEXT("Afterlight", "HatchPrompt", "Open");
			Comp->Verb = AfterlightTags::Interaction_Use;
			Comp->RequiredFlag = AfterlightTags::Story_Slice01_SweepPassed;
			Comp->GrantFlag = AfterlightTags::Story_Slice01_ReachedBolt;
			Comp->BlockedFlag = AfterlightTags::Story_Slice01_ReachedBolt;
			Comp->OnInteracted.AddDynamic(this, &AAfterlightSlice01Director::HandleDoor);
		}
	}

	Tin = GetWorld()->SpawnActor<AAfterlightInspectableActor>(TinLoc, FRotator::ZeroRotator);
	if (Tin)
	{
		Tin->SetActorScale3D(FVector(0.35f, 0.28f, 0.18f));
		if (UAfterlightInteractableComponent* Comp = Tin->FindComponentByClass<UAfterlightInteractableComponent>())
		{
			Comp->PromptText = NSLOCTEXT("Afterlight", "TinPrompt", "Inspect");
			Comp->Verb = AfterlightTags::Interaction_Inspect;
			Comp->RequiredFlag = AfterlightTags::Story_Slice01_QuietBeat;
			Comp->GrantFlag = AfterlightTags::Story_Slice01_FoundTin;
			Comp->BlockedFlag = AfterlightTags::Story_Slice01_FoundTin;
			Comp->OnInteracted.AddDynamic(this, &AAfterlightSlice01Director::HandleTin);
		}
	}
	SpawnSign(TinLoc + FVector(0.f, -8.f, 22.f), FRotator(0.f, 180.f, 0.f), TEXT("FOR WHEN IT TAKES"), 8.f, FColor(70, 62, 52));

	SpawnBox(FVector(4024.f, -150.f, 108.f), FVector(0.12f, 0.72f, 0.95f), FLinearColor(0.04f, 0.05f, 0.05f));
	WarningSlate = SpawnBox(FVector(4018.f, -150.f, 110.f), FVector(0.05f, 0.58f, 0.78f), FLinearColor(0.12f, 0.42f, 0.34f));
	SpawnBox(FVector(4016.f, -150.f, 148.f), FVector(0.04f, 0.62f, 0.05f), FLinearColor(0.18f, 0.55f, 0.42f));
	SpawnBox(FVector(4015.f, -168.f, 104.f), FVector(0.03f, 0.16f, 0.42f), FLinearColor(0.03f, 0.07f, 0.06f));
	SpawnBox(FVector(4015.f, -168.f, 128.f), FVector(0.028f, 0.1f, 0.12f), FLinearColor(0.02f, 0.05f, 0.045f));
	SlateSilhouette = SpawnBox(FVector(4016.2f, -150.f, 112.f), FVector(0.02f, 0.16f, 0.34f), FLinearColor(0.02f, 0.05f, 0.045f));
	SpawnBox(FVector(4016.2f, -150.f, 132.f), FVector(0.018f, 0.09f, 0.1f), FLinearColor(0.02f, 0.04f, 0.04f));
	SlateScan = SpawnBox(FVector(4016.4f, -150.f, 138.f), FVector(0.012f, 0.56f, 0.018f), FLinearColor(0.55f, 1.f, 0.78f));
	SlateNoise = SpawnBox(FVector(4016.6f, -136.f, 118.f), FVector(0.01f, 0.12f, 0.62f), FLinearColor(0.08f, 0.22f, 0.16f));
	if (SlateScan)
	{
		SlateScan->SetActorHiddenInGame(true);
		SlateScan->SetActorEnableCollision(false);
	}
	if (SlateNoise)
	{
		SlateNoise->SetActorEnableCollision(false);
	}
	SlateLight = SpawnLight(FVector(4004.f, -150.f, 118.f), FLinearColor(0.45f, 0.95f, 0.72f), 2.8f, 260.f);

	Drone = GetWorld()->SpawnActor<AAfterlightLanternDrone>(FVector(1750.f, -120.f, 240.f), FRotator::ZeroRotator);
	if (Drone)
	{
		Drone->ResetSweep();
	}

	const FVector MayaHead = MayaContactLoc + FVector(0.f, 0.f, 76.f);
	SpawnShot(AfterlightShotIds::WakeEstablish, FVector(12.f, 300.f, 168.f), FVector(820.f, 10.f, 88.f), static_cast<uint8>(EAfterlightCameraRegister::Cinematic));
	SpawnShot(AfterlightShotIds::DialogueOTSCompanion, FVector(520.f, 130.f, 168.f), MayaHead, static_cast<uint8>(EAfterlightCameraRegister::Dialogue));
	SpawnShot(AfterlightShotIds::DialogueOTSProtagonist, FVector(900.f, -80.f, 164.f), WakeLoc + FVector(0.f, 0.f, 76.f), static_cast<uint8>(EAfterlightCameraRegister::Dialogue));
	SpawnShot(AfterlightShotIds::DialogueTwoShot, FVector(560.f, 340.f, 168.f), FVector(740.f, 20.f, 128.f), static_cast<uint8>(EAfterlightCameraRegister::Dialogue));
	SpawnShot(AfterlightShotIds::DialogueCloseUpCompanion, FVector(600.f, 120.f, 168.f), MayaHead, static_cast<uint8>(EAfterlightCameraRegister::Intimate));
	SpawnShot(AfterlightShotIds::ThreatPressure, FVector(2160.f, 220.f, 168.f), FVector(1980.f, -20.f, 128.f), static_cast<uint8>(EAfterlightCameraRegister::Threat));
	SpawnShot(AfterlightShotIds::RevealInsert, FVector(3860.f, 90.f, 148.f), FVector(3980.f, -110.f, 112.f), static_cast<uint8>(EAfterlightCameraRegister::Reveal));

	if (APostProcessVolume* Volume = GetWorld()->SpawnActor<APostProcessVolume>(FVector(2000.f, 0.f, 100.f), FRotator::ZeroRotator))
	{
		Volume->bUnbound = true;
		Volume->BlendWeight = 1.f;
		Volume->Settings.bOverride_AutoExposureMethod = true;
		Volume->Settings.AutoExposureMethod = AEM_Manual;
		Volume->Settings.bOverride_AutoExposureBias = true;
		Volume->Settings.AutoExposureBias = 1.72f;
		Volume->Settings.bOverride_AutoExposureMinBrightness = true;
		Volume->Settings.AutoExposureMinBrightness = 0.95f;
		Volume->Settings.bOverride_AutoExposureMaxBrightness = true;
		Volume->Settings.AutoExposureMaxBrightness = 2.1f;
		Volume->Settings.bOverride_AutoExposureSpeedUp = true;
		Volume->Settings.AutoExposureSpeedUp = 0.4f;
		Volume->Settings.bOverride_AutoExposureSpeedDown = true;
		Volume->Settings.AutoExposureSpeedDown = 0.4f;
		Volume->Settings.bOverride_ColorOffset = true;
		Volume->Settings.ColorOffset = FVector4(0.042f, 0.038f, 0.032f, 0.f);
		Volume->Settings.bOverride_ColorGamma = true;
		Volume->Settings.ColorGamma = FVector4(1.08f, 1.06f, 1.04f, 1.f);
	}

	WarningSequence = UAfterlightLevelSequenceFactory::CreateWarningSequence(this, WarningCamera, 22.f);
	if (WarningCamera)
	{
		WarningCamStart = WarningCamera->GetActorLocation();
	}
	if (!bSmoke)
	{
		RainBed = FAfterlightTempAudio::SpawnLoop(GetWorld(), this, TEXT("RainBed"), EAfterlightTempBed::Rain, 0.08f);
		HumBed = FAfterlightTempAudio::SpawnLoop(GetWorld(), this, TEXT("HumBed"), EAfterlightTempBed::Electric, 0.05f);
		PumpBed = FAfterlightTempAudio::SpawnLoop(GetWorld(), this, TEXT("PumpBed"), EAfterlightTempBed::Pump, 0.f);
		DroneBed = FAfterlightTempAudio::SpawnLoop(GetWorld(), this, TEXT("DroneBed"), EAfterlightTempBed::Drone, 0.f);
	}
}

void AAfterlightSlice01Director::BuildDialogue()
{
	auto MakeNode = [](FName Id, FName Speaker, const FText& Line, FName Next, float Advance, bool bKeep) -> FAfterlightDialogueNode
	{
		FAfterlightDialogueNode Node;
		Node.NodeId = Id;
		Node.SpeakerId = Speaker;
		Node.Line = Line;
		Node.NextNodeId = Next;
		Node.AutoAdvanceSeconds = Advance;
		Node.bKeepGameplayInput = bKeep;
		return Node;
	};

	ContactDialogue = NewObject<UAfterlightDialogueAsset>(this, TEXT("DA_Slice01_Contact"));
	ContactDialogue->EntryNodeId = TEXT("Fingers");
	FAfterlightDialogueNode Fingers = MakeNode(TEXT("Fingers"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Fingers", "How many fingers."), TEXT("Specific"), 3.4f, false);
	FAfterlightDialogueNode Specific = MakeNode(TEXT("Specific"), TEXT("Eli"), NSLOCTEXT("Afterlight", "S01Specific", "You're going to have to be more specific."), TEXT("Name"), 2.8f, false);
	FAfterlightDialogueNode Name = MakeNode(TEXT("Name"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Eli", "Eli."), TEXT("LongWay"), 3.4f, false);
	FAfterlightDialogueNode LongWay = MakeNode(TEXT("LongWay"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Long", "Okay. We can do it the long way."), TEXT("Move"), 2.8f, false);
	FAfterlightDialogueNode Move = MakeNode(TEXT("Move"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Move", "We have to move. If you stay here they will write you down, and I will not get you out of that."), NAME_None, 0.f, false);
	FAfterlightDialogueChoice Walk;
	Walk.Text = NSLOCTEXT("Afterlight", "S01Walk", "Walk.");
	Walk.TrustDelta = 0.25f;
	Walk.GrantFlags.AddTag(AfterlightTags::Story_Slice01_ChoseFollow);
	Walk.NextNodeId = TEXT("FollowEli");
	FAfterlightDialogueChoice Question;
	Question.Text = NSLOCTEXT("Afterlight", "S01Question", "You talk like I belong to you.");
	Question.SuspicionDelta = 0.65f;
	Question.GrantFlags.AddTag(AfterlightTags::Story_Slice01_ChoseQuestion);
	Question.NextNodeId = TEXT("QuestionLine");
	Move.Choices.Add(Walk);
	Move.Choices.Add(Question);
	FAfterlightDialogueNode FollowEli = MakeNode(TEXT("FollowEli"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01FollowEli", "Eli."), TEXT("Done"), 2.2f, false);
	FAfterlightDialogueNode QuestionLine = MakeNode(TEXT("QuestionLine"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Dont", "I don't. That's the problem."), TEXT("Done"), 3.0f, false);
	ContactDialogue->Nodes.Append({ Fingers, Specific, Name, LongWay, Move, FollowEli, QuestionLine });

	CutDialogue = NewObject<UAfterlightDialogueAsset>(this, TEXT("DA_Slice01_Cut"));
	CutDialogue->EntryNodeId = TEXT("Posts");
	CutDialogue->Nodes.Add(MakeNode(TEXT("Posts"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Posts", "Don't look at the posts. They like faces."), TEXT("Dead"), 4.2f, true));
	CutDialogue->Nodes.Add(MakeNode(TEXT("Dead"), TEXT("Eli"), NSLOCTEXT("Afterlight", "S01Dead", "That one was dead."), TEXT("Stay"), 2.6f, true));
	CutDialogue->Nodes.Add(MakeNode(TEXT("Stay"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Stay", "They don't stay dead because you asked."), TEXT("Where"), 3.8f, true));
	CutDialogue->Nodes.Add(MakeNode(TEXT("Where"), TEXT("Eli"), NSLOCTEXT("Afterlight", "S01Where", "You going to tell me where."), TEXT("Dry"), 2.6f, true));
	CutDialogue->Nodes.Add(MakeNode(TEXT("Dry"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Dry", "A dry room. Then you can be difficult sitting down."), TEXT("Done"), 4.2f, true));

	QuietFollowDialogue = NewObject<UAfterlightDialogueAsset>(this, TEXT("DA_Slice01_QuietF"));
	QuietFollowDialogue->EntryNodeId = TEXT("Drip");
	QuietFollowDialogue->Nodes.Add(MakeNode(TEXT("Drip"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Drip", "You're dripping on my floor."), TEXT("Yours"), 2.2f, false));
	QuietFollowDialogue->Nodes.Add(MakeNode(TEXT("Yours"), TEXT("Eli"), NSLOCTEXT("Afterlight", "S01Yours", "Your floor."), TEXT("Tonight"), 1.6f, false));
	QuietFollowDialogue->Nodes.Add(MakeNode(TEXT("Tonight"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Tonight", "Tonight."), TEXT("Hands"), 2.6f, false));
	QuietFollowDialogue->Nodes.Add(MakeNode(TEXT("Hands"), TEXT("Eli"), NSLOCTEXT("Afterlight", "S01Hands", "Don't know which one is mine."), TEXT("Sorry"), 4.2f, false));
	QuietFollowDialogue->Nodes.Add(MakeNode(TEXT("Sorry"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Sorry", "Right. Sorry."), TEXT("Tick"), 4.4f, false));
	QuietFollowDialogue->Nodes.Add(MakeNode(TEXT("Tick"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Tick", "You used to hate that tick. Said it sounded like a cheap dishwasher."), TEXT("Did"), 4.4f, false));
	QuietFollowDialogue->Nodes.Add(MakeNode(TEXT("Did"), TEXT("Eli"), NSLOCTEXT("Afterlight", "S01Did", "Did it?"), TEXT("Pump"), 2.0f, false));
	QuietFollowDialogue->Nodes.Add(MakeNode(TEXT("Pump"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Pump", "It sounded like a pump."), TEXT("Done"), 3.2f, false));

	QuietQuestionDialogue = NewObject<UAfterlightDialogueAsset>(this, TEXT("DA_Slice01_QuietQ"));
	QuietQuestionDialogue->EntryNodeId = TEXT("Drip");
	QuietQuestionDialogue->Nodes.Add(MakeNode(TEXT("Drip"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01DripQ", "You're dripping on my floor."), TEXT("Yours"), 2.2f, false));
	QuietQuestionDialogue->Nodes.Add(MakeNode(TEXT("Yours"), TEXT("Eli"), NSLOCTEXT("Afterlight", "S01YoursQ", "Your floor."), TEXT("Tonight"), 1.6f, false));
	QuietQuestionDialogue->Nodes.Add(MakeNode(TEXT("Tonight"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01TonightQ", "Tonight."), TEXT("Hands"), 2.6f, false));
	QuietQuestionDialogue->Nodes.Add(MakeNode(TEXT("Hands"), TEXT("Eli"), NSLOCTEXT("Afterlight", "S01HandsQ", "Don't know which one is mine."), TEXT("Sorry"), 4.2f, false));
	QuietQuestionDialogue->Nodes.Add(MakeNode(TEXT("Sorry"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01SorryQ", "Right. Sorry."), TEXT("Pipe"), 4.0f, false));
	QuietQuestionDialogue->Nodes.Add(MakeNode(TEXT("Pipe"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Pipe", "Drink it before it tastes like the pipe."), TEXT("Done"), 3.6f, false));

	AfterWarningDialogue = NewObject<UAfterlightDialogueAsset>(this, TEXT("DA_Slice01_After"));
	AfterWarningDialogue->EntryNodeId = TEXT("Sure");
	AfterWarningDialogue->Nodes.Add(MakeNode(TEXT("Sure"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Sure", "He sounds so sure."), TEXT("Was"), 3.4f, false));
	AfterWarningDialogue->Nodes.Add(MakeNode(TEXT("Was"), TEXT("Eli"), NSLOCTEXT("Afterlight", "S01Was", "Was he?"), TEXT("No"), 2.6f, false));
	AfterWarningDialogue->Nodes.Add(MakeNode(TEXT("No"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01No", "No."), TEXT("Done"), 3.2f, false));

	WarningLines = {
		NSLOCTEXT("Afterlight", "W1", "If this played, it took."),
		NSLOCTEXT("Afterlight", "W2", "Maya will come. Let her. She'll keep you alive."),
		NSLOCTEXT("Afterlight", "W3", "Do not help her with the rest."),
		NSLOCTEXT("Afterlight", "W4", "She'll call it taking care of you. I called it that too."),
		NSLOCTEXT("Afterlight", "W5", "If you still want her in the room after this sentence, you are not finished becoming me.")
	};
}

void AAfterlightSlice01Director::BindSystems()
{
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		if (UAfterlightDialogueRunner* Runner = Narrative->GetDialogueRunner())
		{
			Runner->OnLinePresented.AddDynamic(this, &AAfterlightSlice01Director::HandleLine);
			Runner->OnChoicesPresented.AddDynamic(this, &AAfterlightSlice01Director::HandleChoices);
			Runner->OnChoiceMade.AddDynamic(this, &AAfterlightSlice01Director::HandleChoice);
			Runner->OnFinished.AddDynamic(this, &AAfterlightSlice01Director::HandleDialogueFinished);
		}
	}
	if (UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>())
	{
		Relationship->OnRelationshipChanged.AddDynamic(this, &AAfterlightSlice01Director::HandleRelationshipChanged);
	}
	if (UAfterlightCinematicCoordinator* Cinematic = GetWorld()->GetSubsystem<UAfterlightCinematicCoordinator>())
	{
		Cinematic->OnCinematicFinished.AddDynamic(this, &AAfterlightSlice01Director::HandleCinematicFinished);
	}
}

bool AAfterlightSlice01Director::ChoseFollow() const
{
	if (const UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		return Narrative->HasFlag(AfterlightTags::Story_Slice01_ChoseFollow);
	}
	return false;
}

float AAfterlightSlice01Director::DialogueDelay(const FText& Line, float OverrideSeconds)
{
	if (bSmoke)
	{
		return 0.04f;
	}
	const float Spoken = LastVoiceSeconds > 0.08f
		? LastVoiceSeconds + 0.38f
		: FAfterlightPresentationFormat::SpokenHoldSeconds(Line, OverrideSeconds) + 0.22f;
	return Spoken + (AfterlightQaStoryEnabled() ? 0.9f : 0.f);
}

void AAfterlightSlice01Director::BeginEntry()
{
	bAwaitingEntry = true;
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->ShowEntryCard();
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Locked);
		PC->ApplyHoldCardFocus();
	}
	UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_OWNER_ENTRY"));
	if (!bSmoke && AfterlightQaDriveEnabled())
	{
		GetWorldTimerManager().SetTimer(QaHandle, [this]()
		{
			TryAcceptContinue();
		}, 1.15f, false);
	}
}

bool AAfterlightSlice01Director::TryAcceptContinue()
{
	if (!bAwaitingEntry || bSmoke)
	{
		return false;
	}
	bAwaitingEntry = false;
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->HideTitle();
		UI->ShowBlackScrim();
	}
	GetWorldTimerManager().SetTimer(EntryHandle, this, &AAfterlightSlice01Director::BeginWake, 0.4f, false);
	UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_ENTRY_ACCEPTED"));
	return true;
}

bool AAfterlightSlice01Director::TryReplay()
{
	if (!bSliceEnded || bSmoke)
	{
		return false;
	}
	ResetSlice();
	return true;
}

void AAfterlightSlice01Director::NotifyPlayerMoved()
{
	if (bMoveHintShown)
	{
		if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
		{
			UI->ClearGuidance();
		}
	}
}

void AAfterlightSlice01Director::UpdateOwnerGuidance(float DeltaSeconds)
{
	if (bSmoke || bAwaitingEntry || bSliceEnded)
	{
		return;
	}
	UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>();
	AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!UI || !PC || !Pawn)
	{
		return;
	}
	if (bWarningStarted && !bTitleStarted)
	{
		return;
	}

	if (!bMoveHintShown && PC->GetInputState() == EAfterlightInputState::Full && !bContactStarted)
	{
		bMoveHintShown = true;
		if (Maya)
		{
			Maya->GlanceAt(Pawn->GetActorLocation() + FVector(0.f, 0.f, 70.f));
		}
	}

	if (!bLostHintShown && !bContactStarted && PC->GetInputState() == EAfterlightInputState::Full && Pawn->GetActorLocation().X < 420.f)
	{
		LostHintElapsed += DeltaSeconds;
		if (LostHintElapsed > 14.f)
		{
			bLostHintShown = true;
		}
	}

	if (Maya && Maya->IsWaitingForPlayer() && !Maya->HasReachedPathEnd() && bContactStarted && !bQuietStarted && !bWarningStarted)
	{
		FollowWaitElapsed += DeltaSeconds;
		if (!bFollowHintShown && FollowWaitElapsed > 5.5f)
		{
			bFollowHintShown = true;
		}
	}
	else
	{
		FollowWaitElapsed = 0.f;
		if (bFollowHintShown)
		{
			bFollowHintShown = false;
			UI->ClearGuidance();
		}
	}

	if (bSweepStarted && !bSweepResolved)
	{
		if (IsPlayerInHide())
		{
			if (bHideHintShown)
			{
				UI->ClearGuidance();
			}
			bHideHintShown = true;
		}
		else
		{
			HideHintElapsed += DeltaSeconds;
			if (!bHideHintShown && HideHintElapsed > 1.6f)
			{
				bHideHintShown = true;
				UI->ShowGuidance(NSLOCTEXT("Afterlight", "HideHint", "Stay out of the light"), 4.5f);
			}
		}
	}
}

void AAfterlightSlice01Director::UpdateWarningPresentation(float DeltaSeconds)
{
	if (!bWarningStarted || bTitleStarted)
	{
		return;
	}
	WarningPresentElapsed += DeltaSeconds;
	const bool bDropout = FMath::Fmod(WarningPresentElapsed, 4.3f) > 4.05f;
	if (WarningSlate)
	{
		WarningSlate->SetActorHiddenInGame(bDropout);
	}
	if (SlateScan)
	{
		const float ScanZ = 148.f - FMath::Fmod(WarningPresentElapsed * 28.f, 72.f);
		SlateScan->SetActorLocation(FVector(4016.4f, -150.f, ScanZ));
		SlateScan->SetActorHiddenInGame(bDropout);
	}
	if (SlateNoise)
	{
		const float Jitter = FMath::Sin(WarningPresentElapsed * 37.f) * 8.f;
		SlateNoise->SetActorLocation(FVector(4016.6f, -136.f + Jitter, 118.f));
		SlateNoise->SetActorHiddenInGame(!bDropout && (FMath::Frac(WarningPresentElapsed * 9.f) > 0.18f));
	}
	if (SlateLight)
	{
		if (UPointLightComponent* Comp = SlateLight->FindComponentByClass<UPointLightComponent>())
		{
			const float Flicker = bDropout ? 0.08f : (0.28f + FMath::Abs(FMath::Sin(WarningPresentElapsed * 11.f)) * 1.8f
				+ FMath::Abs(FMath::Sin(WarningPresentElapsed * 29.f)) * 0.55f);
			Comp->SetIntensity(Flicker);
		}
	}
	if (WarningCamera)
	{
		const FVector Target = FVector(3936.f, 28.f, 132.f);
		const float Push = FAfterlightPresentationFormat::HideRecoveryAlpha(WarningPresentElapsed, 22.f) * 0.42f;
		WarningCamera->SetActorLocation(FMath::Lerp(WarningCamStart, Target, Push));
		WarningCamera->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(WarningCamera->GetActorLocation(), FVector(3992.f, -118.f, 112.f)));
	}
	if (Maya && FMath::Fmod(WarningPresentElapsed, 5.2f) < DeltaSeconds + 0.02f)
	{
		Maya->GlanceAt(FVector(4018.f, -150.f, 110.f));
	}
}

void AAfterlightSlice01Director::UpdateAudioBeds()
{
	if (bSmoke)
	{
		return;
	}
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	const float X = Pawn ? Pawn->GetActorLocation().X : 0.f;
	const bool bInterior = X > 3400.f;
	const bool bQuiet = bQuietStarted && !bWarningStarted;
	if (RainBed)
	{
		RainBed->SetVolumeMultiplier(bInterior ? 0.018f : 0.11f);
	}
	if (HumBed)
	{
		float Hum = (X > 1500.f && X < 3350.f) ? 0.13f : 0.07f;
		if (bInterior)
		{
			Hum = 0.02f;
		}
		HumBed->SetVolumeMultiplier(Hum);
	}
	if (PumpBed)
	{
		float Pump = bInterior ? 0.12f : 0.f;
		if (bQuiet)
		{
			Pump = 0.028f;
		}
		if (bWarningStarted)
		{
			Pump = 0.02f;
		}
		PumpBed->SetVolumeMultiplier(Pump);
	}
	if (DroneBed)
	{
		const bool bDrone = Drone && Drone->IsSweeping();
		DroneBed->SetVolumeMultiplier(bDrone ? 0.26f : 0.f);
	}
}

void AAfterlightSlice01Director::BeginWake()
{
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->HideTitle();
	}
	UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_WAKE_VISIBLE"));
	AfterlightLogMem(TEXT("Wake"));
	if (!bSmoke && AfterlightQaAutoEnabled())
	{
		GetWorldTimerManager().SetTimer(QaHandle, []()
		{
			AfterlightCaptureQaShot(TEXT("Wake"));
		}, 0.45f, false);
		GetWorldTimerManager().SetTimer(QaHandleB, []()
		{
			AfterlightCaptureQaShot(TEXT("MayaArrival"));
		}, 6.2f, false);
	}
	SetBeat(TEXT("Slice01.Wake"));
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_Woke);
	}
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Pawn->TeleportTo(WakeLoc, FRotator::ZeroRotator, false, true);
		if (AAfterlightCharacter* Eli = Cast<AAfterlightCharacter>(Pawn))
		{
			FAfterlightPlaceholderVisuals::BeginRecover(Eli);
		}
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetControlRotation(FRotator(0.f, 8.f, 0.f));
		PC->SetInputState(EAfterlightInputState::Constrained);
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::WakeEstablish, 0.05f);
	}
	if (!bSmoke)
	{
		GetWorldTimerManager().SetTimer(EntryHandle, [this]()
		{
			PlayTempVoice(TEXT("Maya"), TEXT("Call"));
			if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
			{
				UI->ShowDialogue(TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Call", "Eli."), TArray<FText>());
			}
		}, AfterlightQaStoryEnabled() ? 4.2f : 3.2f, false);
	}
	GetWorldTimerManager().SetTimer(WakeHandle, [this]()
	{
		if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
		{
			UI->HideDialogue();
		}
		if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PC->SetControlRotation(FRotator(0.f, 8.f, 0.f));
		}
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			Pawn->SetActorRotation(FRotator(0.f, 8.f, 0.f));
		}
		if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
		{
			Camera->ReleaseToExplore(1.1f);
		}
		ApplyInputFull();
		SetBeat(TEXT("Slice01.Contact"));
	}, bSmoke ? 0.05f : (AfterlightQaStoryEnabled() ? 11.f : 8.f), false);
}

void AAfterlightSlice01Director::BeginContact()
{
	if (bContactStarted)
	{
		return;
	}
	bContactStarted = true;
	GetWorldTimerManager().ClearTimer(AutoTalkHandle);
	SetBeat(TEXT("Slice01.Choice"));
	GraphKind = 1;
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_MetMaya);
	}
	if (Maya)
	{
		Maya->SetActorLocation(FVector(820.f, 16.f, 92.f));
		Maya->NotifyDialogueStarted();
		Maya->ClearLeadPath();
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (AfterlightQaAutoEnabled() || FVector::Dist2D(Pawn->GetActorLocation(), MayaContactLoc) > 280.f)
			{
				Pawn->TeleportTo(FVector(690.f, 58.f, 92.f), FRotator(0.f, -20.f, 0.f), false, true);
			}
			Maya->FaceToward(Pawn->GetActorLocation() + FVector(0.f, 0.f, 70.f), 2.4f);
			if (AAfterlightCharacter* Eli = Cast<AAfterlightCharacter>(Pawn))
			{
				Eli->FaceToward(Maya->GetActorLocation() + FVector(0.f, 0.f, 70.f), 2.6f);
			}
		}
	}
	RebuildContactShots();
	AfterlightLogMem(TEXT("Contact"));
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::DialogueTwoShot, 0.85f);
	}
	HoldShot(ContactTwoShot, 0.55f);
	if (!bSmoke && AfterlightQaAutoEnabled() && !bQaContactCaptured)
	{
		bQaContactCaptured = true;
		GetWorldTimerManager().SetTimer(QaHandle, [this]()
		{
			RebuildContactShots();
			HoldShot(ContactTwoShot, 0.f);
			AfterlightCaptureQaShot(TEXT("Contact"));
		}, 1.15f, false);
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Constrained);
	}
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GetDialogueRunner()->Start(ContactDialogue);
	}
}

void AAfterlightSlice01Director::HandleMayaTalk(AActor* Interactor)
{
	BeginContact();
}

void AAfterlightSlice01Director::HandleLine(FName SpeakerId, const FText& Line)
{
	UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>();
	UAfterlightDialogueRunner* Runner = Narrative ? Narrative->GetDialogueRunner() : nullptr;
	const FAfterlightDialogueNode* Node = Runner ? Runner->GetCurrentNode() : nullptr;
	const FName VoiceId = Node ? Node->NodeId : NAME_None;
	PlayTempVoice(SpeakerId, VoiceId);
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		TArray<FText> Empty;
		UI->ShowDialogue(SpeakerId, Line, Empty);
	}
	if (Maya)
	{
		Maya->HoldStill(0.32f);
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (SpeakerId == TEXT("Maya"))
			{
				if (!ChoseFollow() && (VoiceId == TEXT("LongWay") || VoiceId == TEXT("QuestionLine") || VoiceId == TEXT("Stay")))
				{
					Maya->GlanceAt(Pawn->GetActorLocation() + FVector(80.f, 120.f, 40.f));
				}
				else
				{
					Maya->FaceToward(Pawn->GetActorLocation() + FVector(0.f, 0.f, 70.f), 1.8f);
				}
			}
			if (AAfterlightCharacter* Eli = Cast<AAfterlightCharacter>(Pawn))
			{
				Eli->FaceToward(Maya->GetActorLocation() + FVector(0.f, 0.f, 70.f), SpeakerId == TEXT("Eli") ? 1.2f : 2.1f);
			}
		}
		if (VoiceId == TEXT("Posts"))
		{
			Maya->GlanceAt(FVector(1980.f, -390.f, 210.f));
		}
		else if (VoiceId == TEXT("Drip") || VoiceId == TEXT("Tonight"))
		{
			Maya->GlanceAt(MugLoc + FVector(0.f, 0.f, 40.f));
		}
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		if (GraphKind == 3)
		{
			RebuildQuietShot();
			HoldShot(ContactTwoShot, 0.45f);
		}
		else if (GraphKind == 1)
		{
			RebuildContactShots();
			if (Node)
			{
				const FName Id = Node->NodeId;
				if (Id == TEXT("Specific"))
				{
					Camera->RequestShot(AfterlightShotIds::DialogueOTSProtagonist, 0.6f);
					HoldShot(ContactPlayerOTS, 0.45f);
				}
				else if (Id == TEXT("Name") || Id == TEXT("LongWay"))
				{
					Camera->RequestShot(AfterlightShotIds::DialogueCloseUpCompanion, 0.7f);
					HoldShot(ContactClose, 0.5f);
					if (!bSmoke && AfterlightQaAutoEnabled() && !bQaMayaCuCaptured)
					{
						bQaMayaCuCaptured = true;
						GetWorldTimerManager().SetTimer(QaHandleB, []()
						{
							AfterlightCaptureQaShot(TEXT("MayaCU"));
						}, 0.9f, false);
					}
				}
				else
				{
					Camera->RequestShot(AfterlightShotIds::DialogueTwoShot, 0.65f);
					HoldShot(ContactTwoShot, 0.4f);
				}
			}
		}
	}
	if (!Node)
	{
		return;
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (Node->bKeepGameplayInput)
		{
			PC->SetInputState(EAfterlightInputState::Full);
		}
		else if (Node->Choices.Num() == 0)
		{
			PC->SetInputState(EAfterlightInputState::Constrained);
		}
	}
	if (Node->Choices.Num() == 0 && (!Node->NextNodeId.IsNone() || GraphKind > 0))
	{
		float Delay = DialogueDelay(Line, Node->AutoAdvanceSeconds);
		if (!bSmoke && GraphKind == 2 && Maya && Maya->IsWaitingForPlayer())
		{
			Delay += 0.85f;
		}
		if (!bSmoke && GraphKind == 3 && VoiceId == TEXT("Tonight"))
		{
			Delay += AfterlightQaStoryEnabled() ? 1.4f : 0.55f;
		}
		GetWorldTimerManager().SetTimer(DialogueAdvanceHandle, this, &AAfterlightSlice01Director::AdvanceDialogue, Delay, false);
	}
}

void AAfterlightSlice01Director::AdvanceDialogue()
{
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GetDialogueRunner()->Advance();
	}
}

void AAfterlightSlice01Director::HandleChoices(const TArray<FAfterlightDialogueChoice>& Choices)
{
	if (Choices.Num() == 0)
	{
		return;
	}
	PendingChoices = Choices;
	bChoicesVisible = false;
	PendingChoiceSpeaker = NAME_None;
	PendingChoiceLine = FText::GetEmpty();
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		if (const FAfterlightDialogueNode* Node = Narrative->GetDialogueRunner()->GetCurrentNode())
		{
			PendingChoiceSpeaker = Node->SpeakerId;
			PendingChoiceLine = Node->Line;
		}
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->ShowDialogue(PendingChoiceSpeaker, PendingChoiceLine, TArray<FText>());
	}
	RebuildContactShots();
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::DialogueTwoShot, 0.55f);
	}
	HoldShot(ContactTwoShot, 0.35f);
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Constrained);
	}
	const float Delay = bSmoke ? 0.02f : (AfterlightQaStoryEnabled() ? 1.7f : 1.05f);
	GetWorldTimerManager().SetTimer(ChoiceRevealHandle, this, &AAfterlightSlice01Director::RevealChoices, Delay, false);
}

void AAfterlightSlice01Director::RevealChoices()
{
	if (PendingChoices.Num() == 0)
	{
		return;
	}
	TArray<FText> Texts;
	for (const FAfterlightDialogueChoice& Choice : PendingChoices)
	{
		Texts.Add(Choice.Text);
	}
	bChoicesVisible = true;
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->ShowDialogue(PendingChoiceSpeaker, PendingChoiceLine, Texts);
	}
	HoldShot(ContactTwoShot, 0.2f);
	if (!bSmoke && AfterlightQaAutoEnabled() && !bQaChoiceCaptured)
	{
		bQaChoiceCaptured = true;
		GetWorldTimerManager().SetTimer(QaHandle, [this]()
		{
			AfterlightCaptureQaShot(TEXT("Choice"));
			if (AfterlightQaDriveEnabled())
			{
				if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
				{
					Narrative->GetDialogueRunner()->SelectChoice(AfterlightQaChoiceIndex());
				}
			}
		}, AfterlightQaStoryEnabled() ? 1.8f : 1.15f, false);
	}
}

void AAfterlightSlice01Director::HandleChoice(FAfterlightDialogueChoice Choice)
{
	PendingChoices.Reset();
	bChoicesVisible = false;
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GrantFlags(Choice.GrantFlags);
	}
	if (UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>())
	{
		Relationship->ApplyDelta(Choice.TrustDelta, Choice.SuspicionDelta);
	}
}

void AAfterlightSlice01Director::HandleRelationshipChanged(FAfterlightRelationshipState State)
{
	if (Maya)
	{
		if (UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>())
		{
			Maya->ApplyPresentationTags(Relationship->GetPresentationTags());
		}
	}
}

void AAfterlightSlice01Director::HandleDialogueFinished()
{
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->HideDialogue();
	}
	if (GraphKind == 1)
	{
		if (Maya)
		{
			Maya->NotifyDialogueEnded();
		}
		BeginCut();
	}
	else if (GraphKind == 3)
	{
		if (Maya)
		{
			Maya->NotifyDialogueEnded();
			Maya->GlanceAt(TinLoc + FVector(0.f, 0.f, 20.f));
		}
		if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
		{
			Narrative->GrantFlag(AfterlightTags::Story_Slice01_QuietBeat);
		}
		GetWorldTimerManager().SetTimer(QuietHoldHandle, [this]()
		{
			ApplyInputFull();
			SetBeat(TEXT("Slice01.Tin"));
			if (Maya)
			{
				Maya->GlanceAt(TinLoc + FVector(0.f, 0.f, 20.f));
			}
			if (TinLight)
			{
				if (UPointLightComponent* Comp = TinLight->FindComponentByClass<UPointLightComponent>())
				{
					Comp->SetIntensity(5.2f);
				}
			}
		}, bSmoke ? 0.05f : (AfterlightQaStoryEnabled() ? 3.2f : 2.0f), false);
	}
	else if (GraphKind == 2)
	{
		if (!bSweepStarted)
		{
			GetWorldTimerManager().SetTimer(SweepHandle, this, &AAfterlightSlice01Director::BeginSweep, bSmoke ? 0.1f : 14.f, false);
		}
	}
	else if (GraphKind == 4)
	{
		BeginTitle();
	}
	GraphKind = 0;
}

void AAfterlightSlice01Director::BeginCut()
{
	SetBeat(TEXT("Slice01.LanternCut"));
	ApplyInputFull();
	GraphKind = 2;
	CutLeadElapsed = 0.f;
	bCutTalkPending = true;
	TArray<FVector> Path;
	Path.Add(FVector(1200.f, 20.f, 92.f));
	Path.Add(FVector(1700.f, 0.f, 92.f));
	Path.Add(FVector(2100.f, ChoseFollow() ? 40.f : -40.f, 92.f));
	Path.Add(HideLoc);
	if (Maya)
	{
		Maya->SetLeadPath(Path, true);
		Maya->SetPreferPlayerLook(ChoseFollow());
		Maya->GlanceAt(FVector(1980.f, -390.f, 210.f));
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			Maya->GlanceAt(Pawn->GetActorLocation() + FVector(0.f, 0.f, 70.f));
		}
	}
	if (bSmoke)
	{
		bCutTalkPending = false;
		if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
		{
			Narrative->GetDialogueRunner()->Start(CutDialogue);
		}
	}
	else if (AfterlightQaAutoEnabled() && !bQaLanternCaptured)
	{
		bQaLanternCaptured = true;
		GetWorldTimerManager().SetTimer(QaHandleB, []()
		{
			AfterlightCaptureQaShot(TEXT("LanternCut"));
		}, 1.5f, false);
	}
}

void AAfterlightSlice01Director::BeginSweep()
{
	if (bSweepStarted)
	{
		return;
	}
	bSweepStarted = true;
	SetBeat(TEXT("Slice01.Sweep"));
	AfterlightLogMem(TEXT("Sweep"));
	RebuildThreatShot();
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::ThreatPressure, 0.35f);
	}
	HoldShot(ThreatShot, 0.2f);
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Full);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		PlayTempVoice(TEXT("Maya"), ChoseFollow() ? FName(TEXT("Four")) : FName(TEXT("Down")));
		UI->ShowDialogue(TEXT("Maya"), ChoseFollow()
			? NSLOCTEXT("Afterlight", "S01Four", "Four. They turn at four.")
			: NSLOCTEXT("Afterlight", "S01Down", "Down."),
			TArray<FText>());
		UI->ClearGuidance();
	}
	if (Maya)
	{
		Maya->SetActorLocation(HideLoc);
		Maya->ClearLeadPath();
		Maya->HoldStill(1.2f);
		Maya->GlanceAt(Drone ? Drone->GetActorLocation() : HideLoc + FVector(-200.f, -80.f, 80.f));
	}
	if (Drone)
	{
		Drone->BeginSweep(FVector(1980.f, -40.f, 200.f), FVector(3180.f, -60.f, 200.f), bSmoke ? 0.4f : 9.f);
	}
	RebuildThreatShot();
	if (!bSmoke && AfterlightQaAutoEnabled() && !bQaDroneCaptured)
	{
		bQaDroneCaptured = true;
		GetWorldTimerManager().SetTimer(QaHandle, [this]()
		{
			RebuildThreatShot();
			if (Maya)
			{
				Maya->SetActorLocation(FVector(2440.f, 400.f, 92.f));
				Maya->GlanceAt(Drone ? Drone->GetActorLocation() : FVector(2040.f, 0.f, 160.f));
			}
			if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
			{
				Pawn->TeleportTo(FVector(2360.f, 410.f, 92.f), FRotator(0.f, -150.f, 0.f), false, true);
				if (AAfterlightCharacter* Eli = Cast<AAfterlightCharacter>(Pawn))
				{
					Eli->FaceToward(Drone ? Drone->GetActorLocation() : FVector(2040.f, 0.f, 160.f), 2.f);
				}
			}
			HoldShot(ThreatShot, 0.f);
			AfterlightCaptureQaShot(TEXT("Drone"));
		}, 2.2f, false);
	}
	if (DroneBed)
	{
		DroneBed->SetVolumeMultiplier(0.22f);
	}
	const float SweepHold = bSmoke ? 0.45f : (AfterlightQaStoryEnabled() ? 12.2f : (IsPlayerNearHide() ? 9.4f : 8.2f));
	GetWorldTimerManager().SetTimer(SweepHandle, this, &AAfterlightSlice01Director::FinishSweep, SweepHold, false);
}

bool AAfterlightSlice01Director::IsPlayerInHide() const
{
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const FVector L = Pawn->GetActorLocation();
		return L.X > 2260.f && L.X < 2580.f && L.Y > 240.f;
	}
	return false;
}

bool AAfterlightSlice01Director::IsPlayerNearHide() const
{
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return FVector::Dist2D(Pawn->GetActorLocation(), HideLoc) < 420.f;
	}
	return false;
}

void AAfterlightSlice01Director::PullPlayerToHide()
{
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Pawn->TeleportTo(HideLoc, Pawn->GetActorRotation(), false, true);
	}
}

void AAfterlightSlice01Director::StartHideRecovery()
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn)
	{
		CompleteSweep();
		return;
	}
	bHideRecovering = true;
	HideRecoverElapsed = 0.f;
	HideRecoverStart = Pawn->GetActorLocation();
	if (ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
		{
			Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		{
			Move->StopMovementImmediately();
		}
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Scripted);
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::ThreatPressure, 0.35f);
	}
	if (Maya)
	{
		Maya->GlanceAt(HideRecoverStart + FVector(0.f, 0.f, 70.f));
	}
}

void AAfterlightSlice01Director::UpdateHideRecovery(float DeltaSeconds)
{
	if (!bHideRecovering)
	{
		return;
	}
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn)
	{
		bHideRecovering = false;
		CompleteSweep();
		return;
	}
	HideRecoverElapsed += DeltaSeconds;
	const float Alpha = FAfterlightPresentationFormat::HideRecoveryAlpha(HideRecoverElapsed, HideRecoverDuration);
	FVector Next = FMath::Lerp(HideRecoverStart, HideLoc, Alpha);
	Next.Z += FMath::Sin(Alpha * PI) * 28.f;
	Pawn->SetActorLocation(Next, false, nullptr, ETeleportType::TeleportPhysics);
	if (Alpha >= 1.f)
	{
		bHideRecovering = false;
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
			{
				Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
		}
		Pawn->SetActorLocation(HideLoc, false, nullptr, ETeleportType::TeleportPhysics);
		CompleteSweep();
	}
}

void AAfterlightSlice01Director::FinishSweep()
{
	if (IsPlayerInHide() || IsPlayerNearHide() || AfterlightQaDriveEnabled() || bSmoke)
	{
		if ((bSmoke || AfterlightQaDriveEnabled()) && !IsPlayerInHide() && !IsPlayerNearHide())
		{
			PullPlayerToHide();
		}
		CompleteSweep();
		return;
	}
	StartHideRecovery();
}

void AAfterlightSlice01Director::CompleteSweep()
{
	if (bSweepResolved)
	{
		return;
	}
	bSweepResolved = true;
	bHideRecovering = false;
	HideHintElapsed = 0.f;
	if (Drone)
	{
		Drone->ResetSweep();
		Drone->SetActorTickEnabled(false);
	}
	if (DroneBed)
	{
		DroneBed->SetVolumeMultiplier(0.f);
	}
	AfterlightLogMem(TEXT("SweepResolved"));
	UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_SWEEP_RESOLVED"));
	if (!bSmoke && AfterlightQaAutoEnabled() && !bQaHatchCaptured)
	{
		bQaHatchCaptured = true;
		GetWorldTimerManager().SetTimer(QaHandle, []()
		{
			AfterlightCaptureQaShot(TEXT("Hatch"));
		}, 3.6f, false);
	}
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_SweepPassed);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		PlayTempVoice(TEXT("Maya"), TEXT("Okay"));
		UI->ShowDialogue(TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Okay", "Okay."), TArray<FText>());
	}
	ApplyInputFull();
	SetBeat(TEXT("Slice01.Hatch"));
	if (Maya)
	{
		TArray<FVector> Path;
		Path.Add(FVector(2480.f, 70.f, 92.f));
		Path.Add(FVector(2920.f, 16.f, 92.f));
		Path.Add(DoorLoc + FVector(-140.f, ChoseFollow() ? 48.f : -36.f, 0.f));
		Maya->SetLeadPath(Path, false);
		Maya->GlanceAt(DoorLoc + FVector(0.f, 0.f, 80.f));
	}
	if (!bSmoke && AfterlightQaAutoEnabled())
	{
		AimShot(ThreatShot, FVector(3140.f, 170.f, 148.f), DoorLoc + FVector(0.f, 0.f, 72.f));
		if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
		{
			Camera->RequestShot(AfterlightShotIds::ThreatPressure, 0.55f);
		}
	}
	if (HatchLight)
	{
		if (UPointLightComponent* Comp = HatchLight->FindComponentByClass<UPointLightComponent>())
		{
			Comp->SetIntensity(7.5f);
		}
	}
	GetWorldTimerManager().SetTimer(DialogueAdvanceHandle, [this]()
	{
		if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
		{
			UI->HideDialogue();
		}
	}, bSmoke ? 0.05f : 1.6f, false);
}

void AAfterlightSlice01Director::HandleDoor(AActor* Interactor)
{
	SetBeat(TEXT("Slice01.Quiet"));
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_ReachedBolt);
	}
	if (Door)
	{
		Door->SetActorHiddenInGame(true);
		Door->SetActorEnableCollision(false);
	}
	UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_HATCH_OPEN"));
	if (Maya)
	{
		Maya->SetActorLocation(MugLoc + FVector(ChoseFollow() ? 90.f : 180.f, ChoseFollow() ? 70.f : 130.f, 0.f));
		Maya->ClearLeadPath();
		Maya->HoldStill(0.8f);
	}
	BeginQuiet();
}

void AAfterlightSlice01Director::BeginQuiet()
{
	if (bQuietStarted)
	{
		return;
	}
	bQuietStarted = true;
	GraphKind = 3;
	AimShot(ContactOTS, MugLoc + FVector(-260.f, -80.f, 150.f), MugLoc + FVector(90.f, 60.f, 36.f));
	RebuildQuietShot();
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::DialogueOTSCompanion, bSmoke ? 0.1f : 0.25f);
		HoldShot(ContactOTS, 0.15f);
		if (!bSmoke)
		{
			GetWorldTimerManager().SetTimer(AutoTalkHandle, [this]()
			{
				RebuildQuietShot();
				if (UAfterlightCameraSubsystem* Cam = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
				{
					Cam->RequestShot(AfterlightShotIds::DialogueTwoShot, 0.8f);
				}
				HoldShot(ContactTwoShot, 0.55f);
			}, AfterlightQaStoryEnabled() ? 2.1f : 1.15f, false);
		}
		else
		{
			Camera->RequestShot(AfterlightShotIds::DialogueTwoShot, 0.2f);
		}
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Constrained);
	}
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Pawn->TeleportTo(MugLoc + FVector(-120.f, -80.f, 0.f), FRotator(0.f, 20.f, 0.f), false, true);
		if (AAfterlightCharacter* Eli = Cast<AAfterlightCharacter>(Pawn))
		{
			Eli->FaceToward(MugLoc + FVector(20.f, 10.f, 40.f), 3.2f);
		}
	}
	if (Maya)
	{
		Maya->SetPreferPlayerLook(ChoseFollow());
		Maya->GlanceAt(MugLoc + FVector(0.f, 0.f, 40.f));
		Maya->NotifyDialogueStarted();
	}
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GetDialogueRunner()->Start(ChoseFollow() ? QuietFollowDialogue : QuietQuestionDialogue);
	}
	AfterlightLogMem(TEXT("Quiet"));
	if (!bSmoke && AfterlightQaAutoEnabled())
	{
		GetWorldTimerManager().SetTimer(QaHandle, []()
		{
			AfterlightCaptureQaShot(TEXT("PumpHouse"));
		}, 0.9f, false);
		if (!bQaQuietCaptured)
		{
			bQaQuietCaptured = true;
			GetWorldTimerManager().SetTimer(QaHandleB, [this]()
			{
				RebuildQuietShot();
				HoldShot(ContactTwoShot, 0.f);
				AfterlightCaptureQaShot(AfterlightQaChoiceIndex() == 1 ? TEXT("QuietQuestion") : TEXT("Quiet"));
			}, AfterlightQaStoryEnabled() ? 3.4f : 2.4f, false);
		}
	}
}

void AAfterlightSlice01Director::HandleTin(AActor* Interactor)
{
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_FoundTin);
	}
	BeginTinReaction();
}

void AAfterlightSlice01Director::BeginTinReaction()
{
	if (bTinStarted)
	{
		return;
	}
	bTinStarted = true;
	SetBeat(TEXT("Slice01.Warning"));
	if (Maya)
	{
		Maya->SetActorLocation(FVector(4008.f, 36.f, 92.f));
		Maya->ClearLeadPath();
		Maya->HoldStill(1.4f);
		Maya->GlanceAt(TinLoc + FVector(0.f, 0.f, 20.f));
	}
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Pawn->TeleportTo(FVector(3780.f, 80.f, 92.f), FRotator(0.f, -30.f, 0.f), false, true);
		if (AAfterlightCharacter* Eli = Cast<AAfterlightCharacter>(Pawn))
		{
			Eli->SetMoveEnabled(false);
			Eli->FaceToward(TinLoc, 3.f);
		}
	}
	RebuildTinShot();
	HoldShot(ThreatShot, 0.f);
	if (!bSmoke && AfterlightQaAutoEnabled() && !bQaTinCaptured)
	{
		bQaTinCaptured = true;
		GetWorldTimerManager().SetTimer(QaHandleB, [this]()
		{
			RebuildTinShot();
			HoldShot(ThreatShot, 0.f);
			AfterlightCaptureQaShot(TEXT("Tin"));
		}, 0.7f, false);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		PlayTempVoice(TEXT("Maya"), ChoseFollow() ? FName(TEXT("DontTin")) : FName(TEXT("EliTin")));
		UI->ShowDialogue(TEXT("Maya"), ChoseFollow()
			? NSLOCTEXT("Afterlight", "S01DontTin", "Don't—")
			: NSLOCTEXT("Afterlight", "S01EliTin", "Eli—"),
			TArray<FText>());
	}
	GetWorldTimerManager().SetTimer(WarningLineHandle, this, &AAfterlightSlice01Director::BeginWarning, bSmoke ? 0.08f : (AfterlightQaStoryEnabled() ? 2.1f : 1.4f), false);
}

void AAfterlightSlice01Director::BeginWarning()
{
	if (bWarningStarted)
	{
		return;
	}
	bWarningStarted = true;
	WarningLineIndex = 0;
	WarningPresentElapsed = 0.f;
	AfterlightLogMem(TEXT("Warning"));
	if (Maya)
	{
		Maya->SetActorLocation(FVector(3920.f, -40.f, 92.f));
		Maya->ClearLeadPath();
		Maya->HoldStill(2.4f);
		Maya->GlanceAt(FVector(4020.f, -150.f, 108.f));
		Maya->NotifyDialogueStarted();
	}
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Pawn->TeleportTo(FVector(3880.f, 20.f, 92.f), FRotator(0.f, -20.f, 0.f), false, true);
		if (AAfterlightCharacter* Eli = Cast<AAfterlightCharacter>(Pawn))
		{
			Eli->FaceToward(FVector(4018.f, -150.f, 110.f), 8.f);
		}
	}
	if (SlateScan)
	{
		SlateScan->SetActorHiddenInGame(false);
	}
	if (WarningCamera)
	{
		AimShot(WarningCamera, FVector(3860.f, 92.f, 146.f), FVector(3984.f, -118.f, 112.f));
		WarningCamStart = WarningCamera->GetActorLocation();
	}
	if (UAfterlightCinematicCoordinator* Cinematic = GetWorld()->GetSubsystem<UAfterlightCinematicCoordinator>())
	{
		Cinematic->RequestCinematic(WarningSequence, EAfterlightCameraRegister::Intimate, 0.8f);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->ClearGuidance();
		UI->SetPrompt(FText::GetEmpty());
	}
	HandleWarningLine();
}

void AAfterlightSlice01Director::HandleWarningLine()
{
	if (!WarningLines.IsValidIndex(WarningLineIndex))
	{
		return;
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		const FName VoiceId(*FString::Printf(TEXT("W%d"), WarningLineIndex + 1));
		PlayTempVoice(TEXT("Eli"), VoiceId);
		UI->ShowDialogue(NAME_None, WarningLines[WarningLineIndex], TArray<FText>());
	}
	if (!bSmoke)
	{
		FAfterlightTempAudio::PlayOneShot(GetWorld(), this, EAfterlightTempBed::Warning, 0.12f);
	}
	if (Maya && WarningLineIndex == 1)
	{
		Maya->GlanceAt(FVector(3880.f, 20.f, 160.f));
	}
	if (!bSmoke && AfterlightQaAutoEnabled() && !bQaWarningCaptured && WarningLineIndex == 0)
	{
		bQaWarningCaptured = true;
		GetWorldTimerManager().SetTimer(QaHandleB, []()
		{
			AfterlightCaptureQaShot(TEXT("Warning"));
		}, 1.2f, false);
	}
	if (!bSmoke && AfterlightQaAutoEnabled() && !bQaMayaWarnCaptured && WarningLineIndex == 1)
	{
		bQaMayaWarnCaptured = true;
		GetWorldTimerManager().SetTimer(QaHandle, [this]()
		{
			RebuildWarningReactionShot();
			HoldShot(ContactClose, 0.f);
			AfterlightCaptureQaShot(TEXT("MayaWarningReaction"));
			if (WarningCamera)
			{
				HoldShot(WarningCamera, 0.35f);
			}
		}, 1.1f, false);
	}
	float Hold = bSmoke ? 0.06f : FMath::Max(3.6f, DialogueDelay(WarningLines[WarningLineIndex], 0.f));
	if (WarningLineIndex == 4)
	{
		Hold += AfterlightQaStoryEnabled() ? 2.4f : 1.4f;
	}
	++WarningLineIndex;
	if (WarningLines.IsValidIndex(WarningLineIndex))
	{
		GetWorldTimerManager().SetTimer(WarningLineHandle, this, &AAfterlightSlice01Director::HandleWarningLine, Hold, false);
	}
}

void AAfterlightSlice01Director::HandleCinematicFinished()
{
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		if (Narrative->HasFlag(AfterlightTags::Story_Slice01_FoundTin) && !Narrative->HasFlag(AfterlightTags::Story_Slice01_HeardWarning))
		{
			BeginAfterRecording();
		}
	}
}

void AAfterlightSlice01Director::BeginAfterRecording()
{
	UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>();
	if (Narrative)
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_HeardWarning);
	}
	GetWorldTimerManager().ClearTimer(WarningLineHandle);
	GraphKind = 4;
	WarningPresentElapsed = 0.f;
	if (SlateLight)
	{
		if (UPointLightComponent* Comp = SlateLight->FindComponentByClass<UPointLightComponent>())
		{
			Comp->SetIntensity(0.25f);
		}
	}
	if (Maya)
	{
		Maya->NotifyDialogueEnded();
		Maya->SetPreferPlayerLook(true);
		Maya->GlanceAt(MugLoc + FVector(0.f, 0.f, 70.f));
		Maya->NotifyDialogueStarted();
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::DialogueCloseUpCompanion, bSmoke ? 0.2f : 0.9f);
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Scripted);
	}
	if (Narrative)
	{
		Narrative->GetDialogueRunner()->Start(AfterWarningDialogue);
	}
}

void AAfterlightSlice01Director::BeginTitle()
{
	if (bTitleStarted)
	{
		return;
	}
	bTitleStarted = true;
	SetBeat(TEXT("Slice01.Title"));
	AfterlightLogMem(TEXT("Title"));
	if (WitnessLed)
	{
		if (UPointLightComponent* Comp = WitnessLed->FindComponentByClass<UPointLightComponent>())
		{
			Comp->SetIntensity(38.f);
			Comp->SetAttenuationRadius(620.f);
			Comp->SetLightColor(FLinearColor(0.85f, 0.95f, 1.f));
		}
	}
	if (WitnessCore)
	{
		WitnessCore->SetActorScale3D(FVector(0.48f, 0.48f, 0.28f));
	}
	if (!bSmoke)
	{
		FAfterlightTempAudio::PlayOneShot(GetWorld(), this, EAfterlightTempBed::Electric, 0.18f);
	}
	if (Maya)
	{
		Maya->GlanceAt(FVector(3820.f, -20.f, 246.f));
	}
	AimShot(WarningCamera, FVector(3580.f, 210.f, 188.f), FVector(3820.f, -16.f, 250.f));
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::RevealInsert, 0.15f);
	}
	HoldShot(WarningCamera, 0.f);
	if (!bSmoke && AfterlightQaAutoEnabled() && !bQaWitnessCaptured)
	{
		bQaWitnessCaptured = true;
		GetWorldTimerManager().SetTimer(QaHandle, [this]()
		{
			AimShot(WarningCamera, FVector(3580.f, 210.f, 188.f), FVector(3820.f, -16.f, 250.f));
			HoldShot(WarningCamera, 0.f);
			AfterlightCaptureQaShot(TEXT("Witness"));
		}, 0.65f, false);
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Locked);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->HideDialogue();
	}
	const float Silence = bSmoke ? 0.05f : (AfterlightQaStoryEnabled() ? 2.4f : 1.7f);
	GetWorldTimerManager().SetTimer(TitleHandle, [this]()
	{
		if (!bSmoke)
		{
			FAfterlightTempAudio::PlayOneShot(GetWorld(), this, EAfterlightTempBed::Sting, 0.28f);
		}
		if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
		{
			UI->ShowBlackScrim();
		}
		GetWorldTimerManager().SetTimer(TitleBlackHandle, [this]()
		{
			if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
			{
				UI->ShowTitle(NSLOCTEXT("Afterlight", "Title", "AFTERLIGHT"));
			}
			if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
			{
				Narrative->GrantFlag(AfterlightTags::Story_Slice01_Complete);
			}
			UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_SLICE_RUNTIME=%.1fs"), GetWorld()->GetTimeSeconds());
			if (!bSmoke)
				{
					GetWorldTimerManager().SetTimer(EndCardHandle, [this]()
					{
						bSliceEnded = true;
						if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
						{
							UI->ShowEndCard();
						}
						if (AfterlightQaAutoEnabled() && !bQaEndingCaptured)
						{
							bQaEndingCaptured = true;
							AfterlightCaptureQaShot(TEXT("Ending"));
						}
					}, 1.8f, false);
				}
		}, bSmoke ? 0.02f : 0.55f, false);
	}, Silence, false);
}

void AAfterlightSlice01Director::ClearSliceRuntime()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	bContactStarted = false;
	bSweepStarted = false;
	bQuietStarted = false;
	bTinStarted = false;
	bWarningStarted = false;
	bTitleStarted = false;
	bHideRecovering = false;
	bSliceEnded = false;
	bAwaitingEntry = false;
	bMoveHintShown = false;
	bLostHintShown = false;
	bFollowHintShown = false;
	bHideHintShown = false;
	bCutTalkPending = false;
	bQaContactCaptured = false;
	bQaMayaCuCaptured = false;
	bQaChoiceCaptured = false;
	bQaLanternCaptured = false;
	bQaDroneCaptured = false;
	bQaWarningCaptured = false;
	bQaEndingCaptured = false;
	bQaHatchCaptured = false;
	bQaQuietCaptured = false;
	bQaTinCaptured = false;
	bQaWitnessCaptured = false;
	bQaMayaWarnCaptured = false;
	bChoicesVisible = false;
	PendingChoices.Reset();
	LastVoiceSeconds = 0.f;
	CutLeadElapsed = 0.f;
	SweepFailElapsed = 0.f;
	LostHintElapsed = 0.f;
	FollowWaitElapsed = 0.f;
	HideHintElapsed = 0.f;
	WarningPresentElapsed = 0.f;
	HideRecoverElapsed = 0.f;
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		if (UAfterlightDialogueRunner* Runner = Narrative->GetDialogueRunner())
		{
			Runner->Abort();
		}
		Narrative->RestoreState(FAfterlightNarrativeState());
	}
	if (UAfterlightCinematicCoordinator* Cinematic = GetWorld()->GetSubsystem<UAfterlightCinematicCoordinator>())
	{
		Cinematic->CancelCinematic();
	}
	if (UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>())
	{
		Relationship->SetState(FAfterlightRelationshipState());
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->HideDialogue();
		UI->HideTitle();
	}
	if (Maya)
	{
		Maya->SetActorLocation(MayaContactLoc);
		Maya->ClearLeadPath();
		Maya->NotifyDialogueEnded();
	}
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
			{
				Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
		}
		if (AAfterlightCharacter* Eli = Cast<AAfterlightCharacter>(Pawn))
		{
			Eli->SetMoveEnabled(true);
		}
	}
	if (Door)
	{
		Door->SetActorHiddenInGame(false);
		Door->SetActorEnableCollision(true);
	}
	if (Drone)
	{
		Drone->ResetSweep();
	}
	if (WitnessLed)
	{
		if (UPointLightComponent* Comp = WitnessLed->FindComponentByClass<UPointLightComponent>())
		{
			Comp->SetIntensity(0.05f);
		}
	}
	if (SlateLight)
	{
		if (UPointLightComponent* Comp = SlateLight->FindComponentByClass<UPointLightComponent>())
		{
			Comp->SetIntensity(0.4f);
		}
	}
	if (RainBed)
	{
		RainBed->SetVolumeMultiplier(bSmoke ? 0.f : 0.08f);
	}
	if (HumBed)
	{
		HumBed->SetVolumeMultiplier(bSmoke ? 0.f : 0.08f);
	}
	if (PumpBed)
	{
		PumpBed->SetVolumeMultiplier(0.f);
	}
	if (HatchLight)
	{
		if (UPointLightComponent* Comp = HatchLight->FindComponentByClass<UPointLightComponent>())
		{
			Comp->SetIntensity(1.2f);
		}
	}
	if (TinLight)
	{
		if (UPointLightComponent* Comp = TinLight->FindComponentByClass<UPointLightComponent>())
		{
			Comp->SetIntensity(0.35f);
		}
	}
}

void AAfterlightSlice01Director::ResetSlice()
{
	ClearSliceRuntime();
	BeginWake();
}

void AAfterlightSlice01Director::JumpCheckpoint(FName Checkpoint)
{
	ClearSliceRuntime();
	UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>();
	UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>();
	if (!Narrative)
	{
		return;
	}
	const FString Name = Checkpoint.ToString();
	if (Name.Equals(TEXT("Wake"), ESearchCase::IgnoreCase))
	{
		BeginWake();
		return;
	}
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_Woke);
	if (Name.Equals(TEXT("Choice"), ESearchCase::IgnoreCase))
	{
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			Pawn->TeleportTo(FVector(640.f, 40.f, 92.f), FRotator::ZeroRotator, false, true);
		}
		BeginContact();
		return;
	}
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_MetMaya);
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_ChoseFollow);
	if (Relationship)
	{
		Relationship->ApplyDelta(0.25f, 0.f);
	}
	if (Name.Equals(TEXT("Sweep"), ESearchCase::IgnoreCase))
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_EnteredCut);
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			Pawn->TeleportTo(FVector(2200.f, 0.f, 92.f), FRotator::ZeroRotator, false, true);
		}
		BeginSweep();
		return;
	}
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_EnteredCut);
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_SweepPassed);
	if (Name.Equals(TEXT("Quiet"), ESearchCase::IgnoreCase))
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_ReachedBolt);
		HandleDoor(nullptr);
		return;
	}
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_ReachedBolt);
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_QuietBeat);
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_FoundTin);
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Pawn->TeleportTo(MugLoc + FVector(-80.f, 0.f, 0.f), FRotator::ZeroRotator, false, true);
	}
	BeginWarning();
}

bool AAfterlightSlice01Director::Check(bool bCondition, const TCHAR* Label, FString& OutReport)
{
	OutReport += FString::Printf(TEXT("%s %s\n"), bCondition ? TEXT("PASS") : TEXT("FAIL"), Label);
	return bCondition;
}

void AAfterlightSlice01Director::MaybeScheduleCommandLineSmoke()
{
	if (!bSmoke)
	{
		return;
	}
	FTimerHandle SmokeHandle;
	GetWorldTimerManager().SetTimer(SmokeHandle, [this]()
	{
		FString Report;
		const bool bOk = RunSliceSmoke(Report);
		UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_SLICE_SMOKE=%s\n%s"), bOk ? TEXT("PASS") : TEXT("FAIL"), *Report);
		FPlatformMisc::RequestExitWithStatus(false, bOk ? 0 : 1);
	}, 0.35f, false);
}

bool AAfterlightSlice01Director::RunSliceSmoke(FString& OutReport)
{
	OutReport.Reset();
	bool bAll = true;
	bSmoke = true;
	ResetSlice();
	UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>();
	UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>();
	AAfterlightCharacter* Protagonist = nullptr;
	if (UAfterlightPlayerContextSubsystem* Context = GetWorld()->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		Protagonist = Context->GetProtagonist();
	}
	bAll &= Check(Protagonist && Maya && Tin && Door && Drone && Narrative && Relationship, TEXT("slice actors and subsystems"), OutReport);
	if (!Protagonist || !Narrative)
	{
		return false;
	}
	bAll &= Check(!FAfterlightSliceProgress::ShouldGrantEnteredCut(180.f, 0.f, 780.f, true, 5.f), TEXT("wake does not grant cut"), OutReport);
	bAll &= Check(FAfterlightSliceProgress::ShouldGrantEnteredCut(1500.f, 0.f, 1700.f, true, 1.f), TEXT("corridor grants cut"), OutReport);
	bAll &= Check(FAfterlightSliceProgress::ShouldGrantEnteredCut(750.f, 20.f, 1680.f, true, 2.f), TEXT("maya lead grants cut"), OutReport);
	bAll &= Check(FAfterlightSliceProgress::ShouldGrantEnteredCut(700.f, 0.f, 900.f, true, 23.f), TEXT("timer fallback grants cut"), OutReport);
	bAll &= Check(!FAfterlightSliceProgress::IsShotValidPlacement(FVector(748.f, 62.f, 166.f), FVector(780.f, 40.f, 168.f)), TEXT("contact close-up rejected"), OutReport);

	Narrative->GrantFlag(AfterlightTags::Story_Slice01_Woke);
	bAll &= Check(Narrative->GetCurrentBeatId() == FName(TEXT("Slice01.Wake")) || Narrative->HasFlag(AfterlightTags::Story_Slice01_Woke), TEXT("wake flag"), OutReport);

	Protagonist->TeleportTo(FVector(640.f, 40.f, 92.f), FRotator::ZeroRotator, false, true);
	BeginContact();
	bAll &= Check(Narrative->GetDialogueRunner() && Narrative->GetDialogueRunner()->IsActive(), TEXT("contact dialogue active"), OutReport);

	while (Narrative->GetDialogueRunner()->IsActive() && Narrative->GetDialogueRunner()->GetCurrentNode() && Narrative->GetDialogueRunner()->GetCurrentNode()->Choices.Num() == 0)
	{
		Narrative->GetDialogueRunner()->Advance();
	}
	AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(Protagonist->GetController());
	if (PC)
	{
		PC->ChooseDialogue(0);
	}
	bAll &= Check(Narrative->HasFlag(AfterlightTags::Story_Slice01_ChoseFollow), TEXT("ChoseFollow"), OutReport);
	bAll &= Check(FMath::IsNearlyEqual(Relationship->GetState().Trust, 0.75f, 0.02f), TEXT("Trust 0.75"), OutReport);
	bAll &= Check(Maya->GetFollowDistance() < 160.f, TEXT("close follow"), OutReport);

	ResetSlice();
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_Woke);
	Protagonist->TeleportTo(FVector(640.f, 40.f, 92.f), FRotator::ZeroRotator, false, true);
	BeginContact();
	while (Narrative->GetDialogueRunner()->IsActive() && Narrative->GetDialogueRunner()->GetCurrentNode() && Narrative->GetDialogueRunner()->GetCurrentNode()->Choices.Num() == 0)
	{
		Narrative->GetDialogueRunner()->Advance();
	}
	if (PC)
	{
		PC->ChooseDialogue(1);
	}
	bAll &= Check(Narrative->HasFlag(AfterlightTags::Story_Slice01_ChoseQuestion), TEXT("ChoseQuestion"), OutReport);
	bAll &= Check(!Narrative->HasFlag(AfterlightTags::Story_Slice01_ChoseFollow), TEXT("mutex choice"), OutReport);
	bAll &= Check(FMath::IsNearlyEqual(Relationship->GetState().Suspicion, 0.65f, 0.02f), TEXT("Suspicion 0.65"), OutReport);

	Narrative->GrantFlag(AfterlightTags::Story_Slice01_EnteredCut);
	BeginSweep();
	FinishSweep();
	bAll &= Check(Narrative->HasFlag(AfterlightTags::Story_Slice01_SweepPassed), TEXT("sweep passed"), OutReport);

	HandleDoor(Protagonist);
	bAll &= Check(Narrative->HasFlag(AfterlightTags::Story_Slice01_ReachedBolt), TEXT("reached bolt"), OutReport);
	while (Narrative->GetDialogueRunner() && Narrative->GetDialogueRunner()->IsActive())
	{
		Narrative->GetDialogueRunner()->Advance();
	}
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_QuietBeat);
	if (UAfterlightInteractableComponent* TinComp = Tin->FindComponentByClass<UAfterlightInteractableComponent>())
	{
		bAll &= Check(TinComp->CanInteract(Protagonist), TEXT("tin available after quiet"), OutReport);
		TinComp->ExecuteInteraction(Protagonist);
	}
	bAll &= Check(Narrative->HasFlag(AfterlightTags::Story_Slice01_FoundTin), TEXT("found tin"), OutReport);
	if (UAfterlightInteractableComponent* TinComp = Tin->FindComponentByClass<UAfterlightInteractableComponent>())
	{
		bAll &= Check(!TinComp->CanInteract(Protagonist), TEXT("tin repeat blocked"), OutReport);
	}

	BeginWarning();
	if (UAfterlightCinematicCoordinator* Cinematic = GetWorld()->GetSubsystem<UAfterlightCinematicCoordinator>())
	{
		bAll &= Check(Cinematic->IsCinematicActive(), TEXT("warning cinematic active"), OutReport);
		bAll &= Check(!Cinematic->GetActiveSequenceName().IsNone(), TEXT("warning sequence named"), OutReport);
		Cinematic->ReleaseCinematic();
	}
	bAll &= Check(Narrative->HasFlag(AfterlightTags::Story_Slice01_HeardWarning), TEXT("heard warning"), OutReport);
	BeginTitle();
	GetWorldTimerManager().ClearTimer(TitleHandle);
	GetWorldTimerManager().ClearTimer(TitleBlackHandle);
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->ShowTitle(NSLOCTEXT("Afterlight", "Title", "AFTERLIGHT"));
	}
	Narrative->GrantFlag(AfterlightTags::Story_Slice01_Complete);
	bAll &= Check(Narrative->HasFlag(AfterlightTags::Story_Slice01_Complete), TEXT("slice complete"), OutReport);

	UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT Slice01 smoke %s"), bAll ? TEXT("PASS") : TEXT("FAIL"));
	return bAll;
}
