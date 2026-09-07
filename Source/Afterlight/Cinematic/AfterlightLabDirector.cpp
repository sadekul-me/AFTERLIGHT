#include "Cinematic/AfterlightLabDirector.h"
#include "Character/AfterlightCompanionCharacter.h"
#include "Interaction/AfterlightInspectableActor.h"
#include "Interaction/AfterlightInteractableComponent.h"
#include "Camera/AfterlightCameraAnchorComponent.h"
#include "Camera/AfterlightCameraSubsystem.h"
#include "Camera/AfterlightCameraRecipe.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "Narrative/AfterlightDialogueAsset.h"
#include "Narrative/AfterlightDialogueRunner.h"
#include "Narrative/AfterlightNarrativeSubsystem.h"
#include "Relationship/AfterlightRelationshipSubsystem.h"
#include "Character/AfterlightPlayerController.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Core/AfterlightGameplayTags.h"
#include "Cinematic/AfterlightCinematicCoordinator.h"
#include "UI/AfterlightPresentationSubsystem.h"
#include "Character/AfterlightCharacter.h"
#include "Interaction/AfterlightInteractionComponent.h"
#include "Save/AfterlightSaveSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Cinematic/AfterlightLevelSequenceFactory.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "InputActionValue.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Core/AfterlightLog.h"

AAfterlightLabDirector::AAfterlightLabDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAfterlightLabDirector::BeginPlay()
{
	Super::BeginPlay();
	BuildGreybox();
	BindSystems();
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->SetCurrentBeatId(TEXT("Lab.Start"));
	}
	MaybeScheduleCommandLineSmoke();
}

void AAfterlightLabDirector::BuildGreybox()
{
	if (bBuilt)
	{
		return;
	}
	bBuilt = true;

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	auto SpawnBox = [&](const FVector& Location, const FVector& Scale)
	{
		AStaticMeshActor* Box = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
		if (!Box)
		{
			return;
		}
		if (Cube)
		{
			Box->GetStaticMeshComponent()->SetStaticMesh(Cube);
		}
		Box->SetActorScale3D(Scale);
		Box->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	};

	// Floor, corridor, room
	SpawnBox(FVector(0.f, 0.f, -50.f), FVector(18.f, 6.f, 0.2f));
	SpawnBox(FVector(0.f, -280.f, 120.f), FVector(18.f, 0.25f, 3.4f));
	SpawnBox(FVector(0.f, 280.f, 120.f), FVector(18.f, 0.25f, 3.4f));
	SpawnBox(FVector(900.f, 0.f, -50.f), FVector(8.f, 8.f, 0.2f));
	SpawnBox(FVector(900.f, -380.f, 140.f), FVector(8.f, 0.25f, 4.f));
	SpawnBox(FVector(900.f, 380.f, 140.f), FVector(8.f, 0.25f, 4.f));
	SpawnBox(FVector(1280.f, 0.f, 140.f), FVector(0.25f, 8.f, 4.f));

	Companion = GetWorld()->SpawnActor<AAfterlightCompanionCharacter>(FVector(720.f, 40.f, 92.f), FRotator(0.f, 180.f, 0.f));
	Inspectable = GetWorld()->SpawnActor<AAfterlightInspectableActor>(FVector(1040.f, -160.f, 50.f), FRotator::ZeroRotator);
	if (Inspectable)
	{
		Inspectable->SetActorScale3D(FVector(0.8f, 0.8f, 1.2f));
	}

	const FVector CompanionHead(720.f, 40.f, 168.f);
	const FVector PlayerApproach(540.f, 40.f, 160.f);
	SpawnShot(AfterlightShotIds::DialogueOTSCompanion, FVector(600.f, 95.f, 158.f), CompanionHead, EAfterlightCameraRegister::Dialogue);
	SpawnShot(AfterlightShotIds::DialogueOTSProtagonist, FVector(800.f, 110.f, 158.f), PlayerApproach, EAfterlightCameraRegister::Dialogue);
	SpawnShot(AfterlightShotIds::DialogueTwoShot, FVector(660.f, 280.f, 155.f), FVector(680.f, 40.f, 140.f), EAfterlightCameraRegister::Dialogue);
	SpawnShot(AfterlightShotIds::DialogueCloseUpCompanion, FVector(678.f, 58.f, 166.f), CompanionHead, EAfterlightCameraRegister::Intimate);
	RevealCamera = SpawnShot(AfterlightShotIds::RevealInsert, FVector(1120.f, 40.f, 145.f), FVector(1040.f, -160.f, 80.f), EAfterlightCameraRegister::Reveal);
	SpawnShot(AfterlightShotIds::ThreatPressure, FVector(200.f, -200.f, 160.f), FVector(0.f, 0.f, 120.f), EAfterlightCameraRegister::Threat);
	BuildInspectSequence();

	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Pawn->TeleportTo(FVector(-600.f, 0.f, 100.f), FRotator::ZeroRotator, false, true);
	}
	else
	{
		FTimerHandle PawnHandle;
		GetWorldTimerManager().SetTimer(PawnHandle, [this]()
		{
			if (APawn* DelayedPawn = UGameplayStatics::GetPlayerPawn(this, 0))
			{
				DelayedPawn->TeleportTo(FVector(-600.f, 0.f, 100.f), FRotator::ZeroRotator, false, true);
			}
		}, 0.15f, false);
	}
}

ACineCameraActor* AAfterlightLabDirector::SpawnShot(FName ShotId, const FVector& Location, const FVector& LookAt, EAfterlightCameraRegister Register)
{
	const FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Location, LookAt);
	ACineCameraActor* Camera = GetWorld()->SpawnActor<ACineCameraActor>(Location, Rotation);
	if (!Camera)
	{
		return nullptr;
	}
	if (UCineCameraComponent* Cine = Camera->GetCineCameraComponent())
	{
		Cine->CurrentFocalLength = Register == EAfterlightCameraRegister::Intimate ? 75.f : Register == EAfterlightCameraRegister::Dialogue ? 50.f : 35.f;
		Cine->CurrentAperture = 2.8f;
	}
	UAfterlightCameraAnchorComponent* Anchor = NewObject<UAfterlightCameraAnchorComponent>(Camera);
	Anchor->Register = Register;
	Anchor->ShotId = ShotId;
	Camera->AddInstanceComponent(Anchor);
	Anchor->RegisterComponent();
	if (UAfterlightCameraSubsystem* CameraSys = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		CameraSys->RegisterAnchor(Register, Camera);
		CameraSys->RegisterShot(ShotId, Camera);
	}
	return Camera;
}

void AAfterlightLabDirector::BuildInspectSequence()
{
	InspectSequence = UAfterlightLevelSequenceFactory::CreateInspectRevealSequence(this, RevealCamera, 3.f);
}

UAfterlightDialogueAsset* AAfterlightLabDirector::BuildPlaceholderDialogue()
{
	UAfterlightDialogueAsset* Asset = NewObject<UAfterlightDialogueAsset>(this, TEXT("DA_LabDialogue"));
	Asset->EntryNodeId = TEXT("Start");

	FAfterlightDialogueNode Start;
	Start.NodeId = TEXT("Start");
	Start.SpeakerId = TEXT("Companion");
	Start.Line = NSLOCTEXT("Afterlight", "LineHearMe", "Can you hear me?");

	FAfterlightDialogueChoice A;
	A.Text = NSLOCTEXT("Afterlight", "ChoiceFine", "I'm fine.");
	A.TrustDelta = 0.25f;
	A.GrantFlags.AddTag(AfterlightTags::Story_Test_MetCompanion);
	A.NextNodeId = TEXT("Fine");

	FAfterlightDialogueChoice B;
	B.Text = NSLOCTEXT("Afterlight", "ChoiceWho", "Who are you?");
	B.SuspicionDelta = 0.65f;
	B.GrantFlags.AddTag(AfterlightTags::Story_Test_MetCompanion);
	B.NextNodeId = TEXT("Who");

	Start.Choices.Add(A);
	Start.Choices.Add(B);

	FAfterlightDialogueNode Fine;
	Fine.NodeId = TEXT("Fine");
	Fine.SpeakerId = TEXT("Companion");
	Fine.Line = NSLOCTEXT("Afterlight", "LineStayClose", "Stay close. The corridor is quieter when you trust the dark.");

	FAfterlightDialogueNode Who;
	Who.NodeId = TEXT("Who");
	Who.SpeakerId = TEXT("Companion");
	Who.Line = NSLOCTEXT("Afterlight", "LineNamesLater", "Names later. Distance first.");

	Asset->Nodes.Add(Start);
	Asset->Nodes.Add(Fine);
	Asset->Nodes.Add(Who);
	return Asset;
}

void AAfterlightLabDirector::BindSystems()
{
	PlaceholderDialogue = BuildPlaceholderDialogue();

	if (Companion && Companion->FindComponentByClass<UAfterlightInteractableComponent>())
	{
		Companion->FindComponentByClass<UAfterlightInteractableComponent>()->OnInteracted.AddDynamic(this, &AAfterlightLabDirector::HandleCompanionTalk);
	}
	if (Inspectable && Inspectable->FindComponentByClass<UAfterlightInteractableComponent>())
	{
		Inspectable->FindComponentByClass<UAfterlightInteractableComponent>()->OnInteracted.AddDynamic(this, &AAfterlightLabDirector::HandleInspect);
	}

	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		if (UAfterlightDialogueRunner* Runner = Narrative->GetDialogueRunner())
		{
			Runner->OnLinePresented.AddDynamic(this, &AAfterlightLabDirector::HandleLine);
			Runner->OnChoicesPresented.AddDynamic(this, &AAfterlightLabDirector::HandleChoices);
			Runner->OnChoiceMade.AddDynamic(this, &AAfterlightLabDirector::HandleChoice);
			Runner->OnFinished.AddDynamic(this, &AAfterlightLabDirector::HandleDialogueFinished);
		}
	}

	if (UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>())
	{
		Relationship->OnRelationshipChanged.AddDynamic(this, &AAfterlightLabDirector::HandleRelationshipChanged);
		HandleRelationshipChanged(Relationship->GetState());
	}
	if (UAfterlightCinematicCoordinator* Cinematic = GetWorld()->GetSubsystem<UAfterlightCinematicCoordinator>())
	{
		Cinematic->OnCinematicFinished.AddDynamic(this, &AAfterlightLabDirector::HandleCinematicFinished);
	}
}

void AAfterlightLabDirector::HandleCompanionTalk(AActor* Interactor)
{
	UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>();
	if (!Narrative || !PlaceholderDialogue)
	{
		return;
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::DialogueOTSCompanion, 0.85f);
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Constrained);
	}
	if (Companion)
	{
		Companion->NotifyDialogueStarted();
	}
	Narrative->SetCurrentBeatId(TEXT("Lab.Dialogue"));
	Narrative->GetDialogueRunner()->Start(PlaceholderDialogue);
}

void AAfterlightLabDirector::HandleLine(FName SpeakerId, const FText& Line)
{
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		TArray<FText> Empty;
		UI->ShowDialogue(SpeakerId, Line, Empty);
	}
}

void AAfterlightLabDirector::HandleChoices(const TArray<FAfterlightDialogueChoice>& Choices)
{
	TArray<FText> Texts;
	for (const FAfterlightDialogueChoice& Choice : Choices)
	{
		Texts.Add(Choice.Text);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		FName Speaker = NAME_None;
		FText Line;
		if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
		{
			if (const FAfterlightDialogueNode* Node = Narrative->GetDialogueRunner()->GetCurrentNode())
			{
				Speaker = Node->SpeakerId;
				Line = Node->Line;
			}
		}
		UI->ShowDialogue(Speaker, Line, Texts);
	}
}

void AAfterlightLabDirector::HandleChoice(FAfterlightDialogueChoice Choice)
{
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GrantFlags(Choice.GrantFlags);
	}
	if (UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>())
	{
		Relationship->ApplyDelta(Choice.TrustDelta, Choice.SuspicionDelta);
	}
}

void AAfterlightLabDirector::RestoreGameplayPresentation()
{
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->HideDialogue();
	}
	if (Companion)
	{
		Companion->NotifyDialogueEnded();
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->ReleaseToExplore(0.9f);
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Full);
	}
}

void AAfterlightLabDirector::HandleDialogueFinished()
{
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::DialogueCloseUpCompanion, 0.65f);
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Scripted);
	}
	GetWorldTimerManager().SetTimer(DialogueHoldHandle, [this]()
	{
		RestoreGameplayPresentation();
		if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
		{
			Narrative->SetCurrentBeatId(TEXT("Lab.Explore"));
		}
	}, 0.7f, false);
}

void AAfterlightLabDirector::HandleInspect(AActor* Interactor)
{
	PlayInspectReveal();
}

void AAfterlightLabDirector::PlayInspectReveal()
{
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->SetCurrentBeatId(TEXT("Lab.Inspect"));
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::RevealInsert, 0.6f);
	}
	if (UAfterlightCinematicCoordinator* Cinematic = GetWorld()->GetSubsystem<UAfterlightCinematicCoordinator>())
	{
		Cinematic->RequestCinematic(InspectSequence, EAfterlightCameraRegister::Explore, 0.85f);
	}
}

void AAfterlightLabDirector::HandleCinematicFinished()
{
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		if (Narrative->GetCurrentBeatId() == FName(TEXT("Lab.Inspect")))
		{
			Narrative->SetCurrentBeatId(TEXT("Lab.Complete"));
		}
	}
}

void AAfterlightLabDirector::HandleRelationshipChanged(FAfterlightRelationshipState State)
{
	if (!Companion)
	{
		return;
	}
	if (UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>())
	{
		Companion->ApplyPresentationTags(Relationship->GetPresentationTags());
	}
}

void AAfterlightLabDirector::ResetTechnicalFlow()
{
	GetWorldTimerManager().ClearTimer(InspectCinematicHandle);
	GetWorldTimerManager().ClearTimer(DialogueHoldHandle);
	if (UAfterlightCinematicCoordinator* Cinematic = GetWorld()->GetSubsystem<UAfterlightCinematicCoordinator>())
	{
		if (Cinematic->IsCinematicActive())
		{
			Cinematic->ReleaseCinematic();
		}
	}
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		if (UAfterlightDialogueRunner* Runner = Narrative->GetDialogueRunner())
		{
			Runner->Abort();
		}
		Narrative->RestoreState(FAfterlightNarrativeState());
	}
	if (UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>())
	{
		FAfterlightRelationshipState Defaults;
		Defaults.Trust = 0.5f;
		Defaults.Suspicion = 0.f;
		Relationship->SetState(Defaults);
	}
	RestoreGameplayPresentation();
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->SetCurrentBeatId(TEXT("Lab.Start"));
	}
}

bool AAfterlightLabDirector::Check(bool bCondition, const TCHAR* Label, FString& OutReport)
{
	OutReport += FString::Printf(TEXT("%s %s\n"), bCondition ? TEXT("PASS") : TEXT("FAIL"), Label);
	return bCondition;
}

void AAfterlightLabDirector::MaybeScheduleCommandLineSmoke()
{
	if (!FParse::Param(FCommandLine::Get(), TEXT("AfterlightSmoke")))
	{
		return;
	}

	FTimerHandle SmokeHandle;
	GetWorldTimerManager().SetTimer(SmokeHandle, [this]()
	{
		FString Report;
		const bool bOk = RunTechnicalSmoke(Report);
		UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_SMOKE_RESULT=%s\n%s"), bOk ? TEXT("PASS") : TEXT("FAIL"), *Report);
		FPlatformMisc::RequestExitWithStatus(false, bOk ? 0 : 1);
	}, 0.4f, false);
}

bool AAfterlightLabDirector::RunTechnicalSmoke(FString& OutReport)
{
	OutReport.Reset();
	bool bAll = true;

	AAfterlightCharacter* Protagonist = nullptr;
	if (UAfterlightPlayerContextSubsystem* Context = GetWorld()->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		Protagonist = Context->GetProtagonist();
	}
	bAll &= Check(Protagonist != nullptr, TEXT("protagonist resolved via player context"), OutReport);
	bAll &= Check(Companion != nullptr, TEXT("companion spawned"), OutReport);
	bAll &= Check(Inspectable != nullptr, TEXT("inspectable spawned"), OutReport);
	if (!Protagonist || !Companion || !Inspectable)
	{
		return false;
	}

	ResetTechnicalFlow();

	UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>();
	UAfterlightRelationshipSubsystem* Relationship = GetGameInstance()->GetSubsystem<UAfterlightRelationshipSubsystem>();
	UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>();
	UAfterlightCinematicCoordinator* Cinematic = GetWorld()->GetSubsystem<UAfterlightCinematicCoordinator>();
	UAfterlightPresentationSubsystem* Presentation = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>();
	AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(Protagonist->GetController());
	UAfterlightInteractionComponent* Interaction = Protagonist->GetInteractionComponent();

	bAll &= Check(Narrative && Relationship && Camera && Cinematic && Presentation && PC && Interaction, TEXT("core subsystems present"), OutReport);
	if (!Narrative || !Relationship || !Camera || !Cinematic || !Presentation || !PC || !Interaction)
	{
		return false;
	}

	bAll &= Check(FMath::IsNearlyEqual(Relationship->GetState().Trust, 0.5f, 0.01f), TEXT("default Trust 0.5"), OutReport);
	bAll &= Check(FMath::IsNearlyEqual(Relationship->GetState().Suspicion, 0.f, 0.01f), TEXT("default Suspicion 0"), OutReport);
	bAll &= Check(Narrative->GetCurrentBeatId() == FName(TEXT("Lab.Start")), TEXT("beat Lab.Start after reset"), OutReport);

	int32 DirectionalLights = 0;
	int32 SkyLights = 0;
	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		++DirectionalLights;
	}
	for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
	{
		++SkyLights;
	}
	bAll &= Check(DirectionalLights == 1, TEXT("exactly one directional light"), OutReport);
	bAll &= Check(SkyLights <= 1, TEXT("at most one sky light"), OutReport);

	const FRotator LookBefore = PC->GetControlRotation();
	Protagonist->Look(FInputActionValue(FVector2D(18.f, -5.f)));
	PC->UpdateRotation(0.016f);
	bAll &= Check(FMath::Abs(PC->GetControlRotation().Yaw - LookBefore.Yaw) > 1.f, TEXT("look yaw applied"), OutReport);
	Protagonist->Move(FInputActionValue(FVector2D(0.f, 1.f)));
	bAll &= Check(Protagonist->GetPendingMovementInputVector().Size() > 0.05f, TEXT("move input applied"), OutReport);
	Protagonist->ConsumeMovementInputVector();

	Protagonist->TeleportTo(FVector(540.f, 40.f, 92.f), FRotator::ZeroRotator, false, true);
	PC->SetControlRotation(FRotator(0.f, 0.f, 0.f));
	const bool bTalked = Interaction->TryInteract();
	bAll &= Check(bTalked, TEXT("talk via forward interaction probe"), OutReport);
	if (!bTalked)
	{
		UE_LOG(LogAfterlight, Warning, TEXT("Talk probe failed. Pawn=%s Fwd=%s Companion=%s Dist=%.1f"),
			*Protagonist->GetActorLocation().ToCompactString(),
			*Protagonist->GetActorForwardVector().ToCompactString(),
			*Companion->GetActorLocation().ToCompactString(),
			FVector::Dist(Protagonist->GetActorLocation(), Companion->GetActorLocation()));
	}
	bAll &= Check(Camera->GetCurrentRegister() == EAfterlightCameraRegister::Dialogue, TEXT("camera Dialogue after talk"), OutReport);
	bAll &= Check(PC->GetInputState() == EAfterlightInputState::Constrained, TEXT("input Constrained during dialogue"), OutReport);
	bAll &= Check(Narrative->HasFlag(AfterlightTags::Story_Test_MetCompanion), TEXT("flag Story.Test.MetCompanion"), OutReport);
	bAll &= Check(Narrative->GetDialogueRunner() && Narrative->GetDialogueRunner()->IsActive(), TEXT("dialogue runner active"), OutReport);

	PC->ChooseDialogue(0);
	bAll &= Check(FMath::IsNearlyEqual(Relationship->GetState().Trust, 0.75f, 0.01f), TEXT("Trust 0.75 after choice A"), OutReport);
	bAll &= Check(FMath::IsNearlyEqual(Companion->GetFollowDistance(), 140.f, 0.1f), TEXT("companion close follow after Trust high"), OutReport);
	bAll &= Check(Camera->GetCurrentRegister() == EAfterlightCameraRegister::Intimate, TEXT("camera Intimate after choice"), OutReport);
	bAll &= Check(PC->GetInputState() == EAfterlightInputState::Scripted, TEXT("input Scripted during post-dialogue hold"), OutReport);

	const bool bWasCine = Presentation->IsCineMode();
	Presentation->ToggleCineMode();
	bAll &= Check(Presentation->IsCineMode() != bWasCine, TEXT("cine/HUD toggle flipped"), OutReport);
	Presentation->SetCineMode(false);

	Protagonist->TeleportTo(FVector(900.f, -160.f, 92.f), FRotator::ZeroRotator, false, true);
	PC->SetControlRotation(FRotator(0.f, 0.f, 0.f));
	const bool bInspected = Interaction->TryInteract();
	bAll &= Check(bInspected, TEXT("inspect via forward interaction probe"), OutReport);
	bAll &= Check(Narrative->HasFlag(AfterlightTags::Story_Test_InspectedObject), TEXT("flag Story.Test.InspectedObject"), OutReport);
	bAll &= Check(Cinematic->IsCinematicActive(), TEXT("cinematic coordinator active after inspect"), OutReport);
	bAll &= Check(!Cinematic->GetActiveSequenceName().IsNone(), TEXT("Level Sequence name owned by coordinator"), OutReport);
	bAll &= Check(PC->GetInputState() == EAfterlightInputState::Locked, TEXT("input Locked during cinematic"), OutReport);
	bAll &= Check(Camera->GetCurrentRegister() == EAfterlightCameraRegister::Reveal || Camera->GetCurrentRegister() == EAfterlightCameraRegister::Cinematic, TEXT("camera Reveal/Cinematic after inspect"), OutReport);

	UAfterlightSaveSubsystem* Save = GetGameInstance()->GetSubsystem<UAfterlightSaveSubsystem>();
	bAll &= Check(Save && Save->SaveTestSlot(), TEXT("developer save slot"), OutReport);
	ResetTechnicalFlow();
	bAll &= Check(!Narrative->HasFlag(AfterlightTags::Story_Test_InspectedObject), TEXT("reset cleared inspect flag"), OutReport);
	bAll &= Check(Save && Save->LoadTestSlot(), TEXT("developer load slot"), OutReport);
	bAll &= Check(Narrative->HasFlag(AfterlightTags::Story_Test_InspectedObject), TEXT("load restored inspect flag"), OutReport);
	bAll &= Check(FMath::IsNearlyEqual(Relationship->GetState().Trust, 0.75f, 0.01f), TEXT("load restored Trust"), OutReport);

	Cinematic->ReleaseCinematic();
	RestoreGameplayPresentation();

	UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT technical smoke %s"), bAll ? TEXT("PASS") : TEXT("FAIL"));
	return bAll;
}

