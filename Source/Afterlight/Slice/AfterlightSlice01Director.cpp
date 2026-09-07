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

namespace
{
	const FVector WakeLoc(180.f, 0.f, 92.f);
	const FVector MayaContactLoc(780.f, 40.f, 92.f);
	const FVector HideLoc(2420.f, 310.f, 92.f);
	const FVector DoorLoc(3380.f, 0.f, 92.f);
	const FVector MugLoc(3720.f, 40.f, 92.f);
	const FVector TinLoc(3940.f, -150.f, 50.f);
}

AAfterlightSlice01Director::AAfterlightSlice01Director()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAfterlightSlice01Director::BeginPlay()
{
	Super::BeginPlay();
	bSmoke = FParse::Param(FCommandLine::Get(), TEXT("AfterlightSliceSmoke"));
	BuildWorld();
	BuildDialogue();
	BindSystems();
	if (!bSmoke)
	{
		if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
		{
			UI->SetCineMode(true);
			UI->SetDeveloperOverlay(false);
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

	if (!Narrative->HasFlag(AfterlightTags::Story_Slice01_EnteredCut) && Pawn->GetActorLocation().X > 1680.f)
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_EnteredCut);
		SetBeat(TEXT("Slice01.LanternCut"));
	}

	if (!bSweepStarted && Narrative->HasFlag(AfterlightTags::Story_Slice01_EnteredCut) && Pawn->GetActorLocation().X > 2100.f)
	{
		BeginSweep();
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

void AAfterlightSlice01Director::SpawnShot(FName ShotId, const FVector& Location, const FVector& LookAt, uint8 Register)
{
	const EAfterlightCameraRegister CamReg = static_cast<EAfterlightCameraRegister>(Register);
	const FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Location, LookAt);
	ACineCameraActor* Camera = GetWorld()->SpawnActor<ACineCameraActor>(Location, Rotation);
	if (!Camera)
	{
		return;
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
}

void AAfterlightSlice01Director::BuildWorld()
{
	if (bBuilt)
	{
		return;
	}
	bBuilt = true;

	SpawnBox(FVector(800.f, 0.f, -50.f), FVector(16.f, 6.4f, 0.2f), FLinearColor::Gray);
	SpawnBox(FVector(800.f, -330.f, 140.f), FVector(16.f, 0.25f, 3.8f), FLinearColor::Gray);
	SpawnBox(FVector(800.f, 330.f, 140.f), FVector(16.f, 0.25f, 3.8f), FLinearColor::Gray);
	SpawnBox(FVector(-50.f, 0.f, 140.f), FVector(0.3f, 6.4f, 3.8f), FLinearColor::Gray);

	SpawnBox(FVector(2500.f, 0.f, -50.f), FVector(18.f, 8.2f, 0.2f), FLinearColor::Gray);
	SpawnBox(FVector(2500.f, -420.f, 150.f), FVector(18.f, 0.25f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(2100.f, 420.f, 150.f), FVector(10.f, 0.25f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(2800.f, 420.f, 150.f), FVector(8.f, 0.25f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(2420.f, 500.f, -50.f), FVector(3.4f, 2.2f, 0.2f), FLinearColor::Gray);
	SpawnBox(FVector(2420.f, 600.f, 150.f), FVector(3.4f, 0.25f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(2260.f, 520.f, 150.f), FVector(0.25f, 2.f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(2580.f, 520.f, 150.f), FVector(0.25f, 2.f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(2400.f, 220.f, 20.f), FVector(1.6f, 1.2f, 1.4f), FLinearColor::Gray);

	SpawnBox(FVector(3820.f, 0.f, -50.f), FVector(8.4f, 8.2f, 0.2f), FLinearColor::Gray);
	SpawnBox(FVector(3400.f, -420.f, 150.f), FVector(0.3f, 4.f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(3400.f, 420.f, 150.f), FVector(0.3f, 4.f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(3400.f, 220.f, 150.f), FVector(0.3f, 3.2f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(3400.f, -220.f, 150.f), FVector(0.3f, 3.2f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(4220.f, 0.f, 150.f), FVector(0.3f, 8.2f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(3820.f, -420.f, 150.f), FVector(8.4f, 0.25f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(3820.f, 420.f, 150.f), FVector(8.4f, 0.25f, 4.f), FLinearColor::Gray);
	SpawnBox(FVector(3720.f, 80.f, 20.f), FVector(1.1f, 0.6f, 0.7f), FLinearColor::Gray);

	SpawnLight(FVector(520.f, -40.f, 210.f), FLinearColor(0.45f, 0.58f, 0.78f), 3.2f, 900.f);
	SpawnLight(FVector(700.f, 0.f, 260.f), FLinearColor(0.5f, 0.66f, 0.82f), 4.4f, 1300.f);
	SpawnLight(FVector(2500.f, 0.f, 260.f), FLinearColor(1.f, 0.68f, 0.4f), 6.2f, 1500.f);
	SpawnLight(FVector(2400.f, 180.f, 170.f), FLinearColor(1.f, 0.78f, 0.48f), 5.5f, 520.f);
	SpawnLight(FVector(3720.f, 0.f, 210.f), FLinearColor(1.f, 0.82f, 0.58f), 6.4f, 850.f);
	WitnessLed = SpawnLight(FVector(3360.f, -80.f, 190.f), FLinearColor(0.9f, 0.95f, 1.f), 0.05f, 200.f);
	HatchLight = SpawnLight(FVector(3380.f, 0.f, 170.f), FLinearColor(0.95f, 0.88f, 0.62f), 1.2f, 420.f);
	TinLight = SpawnLight(FVector(3940.f, -150.f, 90.f), FLinearColor(1.f, 0.84f, 0.55f), 0.35f, 220.f);

	SpawnBox(FVector(520.f, -250.f, 70.f), FVector(0.35f, 0.22f, 1.1f), FLinearColor::Gray);
	SpawnSign(FVector(520.f, -250.f, 150.f), FRotator(0.f, 180.f, 0.f), TEXT("WITNESS"), 14.f, FColor(90, 100, 110));
	SpawnSign(FVector(1980.f, -390.f, 210.f), FRotator(0.f, 90.f, 0.f), TEXT("AFTERLIGHT 02:17"), 28.f, FColor(200, 210, 180));
	SpawnSign(FVector(2200.f, -390.f, 150.f), FRotator(0.f, 90.f, 0.f), TEXT("WITNESS COVERAGE VARIABLE"), 16.f, FColor(170, 170, 160));
	SpawnSign(FVector(3370.f, 90.f, 180.f), FRotator(0.f, -90.f, 0.f), TEXT("PMP-12"), 22.f, FColor(80, 140, 140));

	Maya = GetWorld()->SpawnActor<AAfterlightCompanionCharacter>(MayaContactLoc, FRotator(0.f, 180.f, 0.f));
	if (Maya)
	{
		TArray<FVector> Looks;
		Looks.Add(FVector(520.f, -250.f, 150.f));
		Looks.Add(FVector(1980.f, -390.f, 210.f));
		Looks.Add(HideLoc + FVector(0.f, 0.f, 40.f));
		Looks.Add(DoorLoc + FVector(0.f, 0.f, 80.f));
		Looks.Add(MugLoc + FVector(0.f, 0.f, 40.f));
		Maya->SetWorldLookTargets(Looks);
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
		Door->SetActorScale3D(FVector(0.4f, 1.6f, 2.4f));
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
	SpawnSign(TinLoc + FVector(0.f, 0.f, 30.f), FRotator(0.f, 180.f, 0.f), TEXT("FOR WHEN IT TAKES"), 12.f, FColor(90, 80, 70));

	WarningSlate = SpawnBox(FVector(4020.f, -150.f, 92.f), FVector(0.08f, 0.55f, 0.72f), FLinearColor::Gray);
	SpawnBox(FVector(4018.f, -150.f, 110.f), FVector(0.04f, 0.22f, 0.38f), FLinearColor::Gray);
	SlateLight = SpawnLight(FVector(4012.f, -150.f, 118.f), FLinearColor(0.75f, 0.82f, 0.7f), 0.4f, 180.f);

	Drone = GetWorld()->SpawnActor<AAfterlightLanternDrone>(FVector(1750.f, -120.f, 240.f), FRotator::ZeroRotator);
	if (Drone)
	{
		Drone->ResetSweep();
	}

	const FVector MayaHead = MayaContactLoc + FVector(0.f, 0.f, 76.f);
	SpawnShot(AfterlightShotIds::DialogueOTSCompanion, FVector(680.f, 110.f, 158.f), MayaHead, static_cast<uint8>(EAfterlightCameraRegister::Dialogue));
	SpawnShot(AfterlightShotIds::DialogueTwoShot, FVector(720.f, 260.f, 155.f), FVector(740.f, 40.f, 140.f), static_cast<uint8>(EAfterlightCameraRegister::Dialogue));
	SpawnShot(AfterlightShotIds::DialogueCloseUpCompanion, FVector(748.f, 62.f, 166.f), MayaHead, static_cast<uint8>(EAfterlightCameraRegister::Intimate));
	SpawnShot(AfterlightShotIds::ThreatPressure, FVector(2280.f, 80.f, 170.f), HideLoc + FVector(0.f, 0.f, 40.f), static_cast<uint8>(EAfterlightCameraRegister::Threat));
	SpawnShot(AfterlightShotIds::RevealInsert, FVector(3990.f, -40.f, 128.f), FVector(4020.f, -150.f, 108.f), static_cast<uint8>(EAfterlightCameraRegister::Reveal));

	WarningSequence = UAfterlightLevelSequenceFactory::CreateWarningSequence(this, WarningCamera, 22.f);
	if (WarningCamera)
	{
		WarningCamStart = WarningCamera->GetActorLocation();
	}
	if (!bSmoke)
	{
		RainBed = FAfterlightTempAudio::SpawnLoop(GetWorld(), this, TEXT("RainBed"), EAfterlightTempBed::Rain, 0.18f);
		HumBed = FAfterlightTempAudio::SpawnLoop(GetWorld(), this, TEXT("HumBed"), EAfterlightTempBed::Electric, 0.08f);
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
	FAfterlightDialogueNode Fingers = MakeNode(TEXT("Fingers"), TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Fingers", "How many fingers."), TEXT("Specific"), 2.6f, false);
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

float AAfterlightSlice01Director::DialogueDelay(const FText& Line, float OverrideSeconds) const
{
	if (bSmoke)
	{
		return 0.04f;
	}
	return FAfterlightPresentationFormat::SpokenHoldSeconds(Line, OverrideSeconds);
}

void AAfterlightSlice01Director::BeginEntry()
{
	bAwaitingEntry = true;
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Locked);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->ShowEntryCard();
	}
	UE_LOG(LogAfterlight, Display, TEXT("AFTERLIGHT_OWNER_ENTRY"));
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
		UI->ShowGuidance(NSLOCTEXT("Afterlight", "MoveHint", "WASD — Move\nMouse — Look"), 6.f);
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
			UI->ShowGuidance(NSLOCTEXT("Afterlight", "LostHint", "Walk toward her"), 5.f);
		}
	}

	if (Maya && Maya->IsWaitingForPlayer() && !Maya->HasReachedPathEnd() && bContactStarted && !bQuietStarted && !bWarningStarted)
	{
		FollowWaitElapsed += DeltaSeconds;
		if (!bFollowHintShown && FollowWaitElapsed > 5.5f)
		{
			bFollowHintShown = true;
			UI->ShowGuidance(NSLOCTEXT("Afterlight", "FollowHint", "Follow Maya"), 0.f);
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
	if (SlateLight)
	{
		if (UPointLightComponent* Comp = SlateLight->FindComponentByClass<UPointLightComponent>())
		{
			const float Flicker = 0.35f + FMath::Abs(FMath::Sin(WarningPresentElapsed * 11.f)) * 1.6f
				+ FMath::Abs(FMath::Sin(WarningPresentElapsed * 29.f)) * 0.45f;
			Comp->SetIntensity(Flicker);
		}
	}
	if (WarningCamera)
	{
		const FVector Target = FVector(4004.f, -150.f, 116.f);
		const float Push = FAfterlightPresentationFormat::HideRecoveryAlpha(WarningPresentElapsed, 22.f) * 0.42f;
		WarningCamera->SetActorLocation(FMath::Lerp(WarningCamStart, Target, Push));
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
		RainBed->SetVolumeMultiplier(bInterior ? 0.03f : 0.18f);
	}
	if (HumBed)
	{
		HumBed->SetVolumeMultiplier(bInterior ? 0.02f : 0.08f);
	}
	if (PumpBed)
	{
		float Pump = bInterior ? 0.11f : 0.f;
		if (bQuiet)
		{
			Pump = 0.045f;
		}
		PumpBed->SetVolumeMultiplier(Pump);
	}
	if (DroneBed)
	{
		const bool bDrone = Drone && Drone->IsSweeping();
		DroneBed->SetVolumeMultiplier(bDrone ? 0.22f : 0.f);
	}
}

void AAfterlightSlice01Director::BeginWake()
{
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->HideTitle();
	}
	SetBeat(TEXT("Slice01.Wake"));
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_Woke);
	}
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Pawn->TeleportTo(WakeLoc, FRotator::ZeroRotator, false, true);
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Constrained);
	}
	GetWorldTimerManager().SetTimer(WakeHandle, [this]()
	{
		ApplyInputFull();
		SetBeat(TEXT("Slice01.Contact"));
	}, bSmoke ? 0.05f : 8.f, false);
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
		Maya->NotifyDialogueStarted();
		Maya->ClearLeadPath();
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			Maya->GlanceAt(Pawn->GetActorLocation() + FVector(0.f, 0.f, 70.f));
		}
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::DialogueOTSCompanion, 0.85f);
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
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		TArray<FText> Empty;
		UI->ShowDialogue(SpeakerId, Line, Empty);
	}
	if (!Runner || !Runner->GetCurrentNode())
	{
		return;
	}
	const FAfterlightDialogueNode* Node = Runner->GetCurrentNode();
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
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Constrained);
	}
}

void AAfterlightSlice01Director::HandleChoice(FAfterlightDialogueChoice Choice)
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
		}, bSmoke ? 0.05f : 2.0f, false);
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
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GetDialogueRunner()->Start(CutDialogue);
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
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestRegister(EAfterlightCameraRegister::Threat, 0.45f);
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Full);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->ShowDialogue(TEXT("Maya"), ChoseFollow()
			? NSLOCTEXT("Afterlight", "S01Four", "Four. They turn at four.")
			: NSLOCTEXT("Afterlight", "S01Down", "Down."),
			TArray<FText>());
		UI->ClearGuidance();
	}
	if (Maya)
	{
		TArray<FVector> HidePath;
		HidePath.Add(HideLoc);
		Maya->SetLeadPath(HidePath, true);
		Maya->GlanceAt(HideLoc + FVector(0.f, 0.f, 40.f));
	}
	if (Drone)
	{
		Drone->BeginSweep(FVector(1750.f, -140.f, 240.f), FVector(3200.f, -140.f, 240.f), bSmoke ? 0.4f : 9.f);
	}
	if (DroneBed)
	{
		DroneBed->SetVolumeMultiplier(0.22f);
	}
	const float SweepHold = bSmoke ? 0.45f : (IsPlayerNearHide() ? 9.4f : 8.2f);
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
	if (IsPlayerInHide())
	{
		CompleteSweep();
		return;
	}
	if (bSmoke)
	{
		PullPlayerToHide();
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
	if (DroneBed)
	{
		DroneBed->SetVolumeMultiplier(0.f);
	}
	if (UAfterlightNarrativeSubsystem* Narrative = GetGameInstance()->GetSubsystem<UAfterlightNarrativeSubsystem>())
	{
		Narrative->GrantFlag(AfterlightTags::Story_Slice01_SweepPassed);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->ShowDialogue(TEXT("Maya"), NSLOCTEXT("Afterlight", "S01Okay", "Okay."), TArray<FText>());
	}
	ApplyInputFull();
	SetBeat(TEXT("Slice01.Hatch"));
	if (Maya)
	{
		TArray<FVector> Path;
		Path.Add(DoorLoc + FVector(-120.f, ChoseFollow() ? 40.f : -60.f, 0.f));
		Maya->SetLeadPath(Path, true);
		Maya->GlanceAt(DoorLoc + FVector(0.f, 0.f, 80.f));
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
	if (Maya)
	{
		Maya->SetActorLocation(MugLoc + FVector(ChoseFollow() ? 50.f : 140.f, ChoseFollow() ? 10.f : 80.f, 0.f));
		Maya->ClearLeadPath();
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
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::DialogueCloseUpCompanion, bSmoke ? 0.2f : 1.05f);
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Constrained);
	}
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Pawn->TeleportTo(MugLoc + FVector(-80.f, -10.f, 0.f), FRotator(0.f, 0.f, 0.f), false, true);
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
}

void AAfterlightSlice01Director::HandleTin(AActor* Interactor)
{
	BeginTinReaction();
}

void AAfterlightSlice01Director::BeginTinReaction()
{
	SetBeat(TEXT("Slice01.Warning"));
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::RevealInsert, 0.6f);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->ShowDialogue(TEXT("Maya"), ChoseFollow()
			? NSLOCTEXT("Afterlight", "S01DontTin", "Don't—")
			: NSLOCTEXT("Afterlight", "S01EliTin", "Eli—"),
			TArray<FText>());
	}
	GetWorldTimerManager().SetTimer(DialogueAdvanceHandle, this, &AAfterlightSlice01Director::BeginWarning, bSmoke ? 0.08f : 1.4f, false);
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
	if (Maya)
	{
		Maya->SetActorLocation(FVector(3920.f, -40.f, 92.f));
		Maya->ClearLeadPath();
		Maya->GlanceAt(FVector(4020.f, -150.f, 108.f));
		Maya->NotifyDialogueStarted();
	}
	if (WarningCamera)
	{
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
		UI->ShowDialogue(TEXT("Recording"), WarningLines[WarningLineIndex], TArray<FText>());
	}
	if (!bSmoke)
	{
		FAfterlightTempAudio::PlayOneShot(GetWorld(), this, EAfterlightTempBed::Warning, 0.12f);
	}
	const float Hold = bSmoke ? 0.06f : FMath::Max(3.6f, FAfterlightPresentationFormat::SpokenHoldSeconds(WarningLines[WarningLineIndex]));
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
	if (WitnessLed)
	{
		if (UPointLightComponent* Comp = WitnessLed->FindComponentByClass<UPointLightComponent>())
		{
			Comp->SetIntensity(18.f);
			Comp->SetLightColor(FLinearColor(0.85f, 0.95f, 1.f));
		}
	}
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestRegister(EAfterlightCameraRegister::Reveal, 0.5f);
	}
	if (AAfterlightPlayerController* PC = Cast<AAfterlightPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputState(EAfterlightInputState::Locked);
	}
	if (UAfterlightPresentationSubsystem* UI = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		UI->HideDialogue();
	}
	const float Silence = bSmoke ? 0.05f : 1.4f;
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
	bWarningStarted = false;
	bTitleStarted = false;
	bHideRecovering = false;
	bSliceEnded = false;
	bAwaitingEntry = false;
	bMoveHintShown = false;
	bLostHintShown = false;
	bFollowHintShown = false;
	bHideHintShown = false;
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
		RainBed->SetVolumeMultiplier(bSmoke ? 0.f : 0.18f);
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
