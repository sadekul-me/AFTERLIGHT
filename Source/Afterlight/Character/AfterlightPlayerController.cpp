#include "Character/AfterlightPlayerController.h"
#include "Character/AfterlightCharacter.h"
#include "UI/AfterlightHUDWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Core/AfterlightPlayerContextSubsystem.h"
#include "Core/AfterlightGameplayTags.h"
#include "Narrative/AfterlightNarrativeSubsystem.h"
#include "Narrative/AfterlightDialogueRunner.h"
#include "UI/AfterlightPresentationSubsystem.h"
#include "Save/AfterlightSaveSubsystem.h"
#include "Camera/AfterlightCameraSubsystem.h"
#include "Cinematic/AfterlightCinematicCoordinator.h"
#include "Cinematic/AfterlightLabDirector.h"
#include "Slice/AfterlightSlice01Director.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "HAL/PlatformMisc.h"
#include "Engine/Engine.h"
#include "Camera/PlayerCameraManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Core/AfterlightLog.h"
#include "Afterlight.h"

AAfterlightPlayerController::AAfterlightPlayerController()
{
	bShowMouseCursor = false;
}

void AAfterlightPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (UAfterlightPlayerContextSubsystem* Context = GetWorld()->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		Context->RegisterController(this);
	}
	SetInputState(EAfterlightInputState::Full);
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = -48.f;
		PlayerCameraManager->ViewPitchMax = 18.f;
	}
}

void AAfterlightPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (UAfterlightPlayerContextSubsystem* Context = GetWorld()->GetSubsystem<UAfterlightPlayerContextSubsystem>())
	{
		Context->RegisterController(this);
		if (AAfterlightCharacter* AfterlightPawn = Cast<AAfterlightCharacter>(InPawn))
		{
			Context->RegisterProtagonist(AfterlightPawn);
		}
	}
}

void AAfterlightPlayerController::EnsureRuntimeInput()
{
	if (MappingContext)
	{
		return;
	}

	MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Afterlight"));
	MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;
	LookAction = NewObject<UInputAction>(this, TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;
	InteractAction = NewObject<UInputAction>(this, TEXT("IA_Interact"));
	ToggleCineAction = NewObject<UInputAction>(this, TEXT("IA_ToggleCine"));
	DebugAction = NewObject<UInputAction>(this, TEXT("IA_Debug"));
	Choice1Action = NewObject<UInputAction>(this, TEXT("IA_Choice1"));
	Choice2Action = NewObject<UInputAction>(this, TEXT("IA_Choice2"));
	ContinueAction = NewObject<UInputAction>(this, TEXT("IA_Continue"));
	ReplayAction = NewObject<UInputAction>(this, TEXT("IA_Replay"));
	ExitSliceAction = NewObject<UInputAction>(this, TEXT("IA_ExitSlice"));
	SaveTestAction = NewObject<UInputAction>(this, TEXT("IA_SaveTest"));
	LoadTestAction = NewObject<UInputAction>(this, TEXT("IA_LoadTest"));
	ForceExploreAction = NewObject<UInputAction>(this, TEXT("IA_ForceExplore"));
	ForceDialogueAction = NewObject<UInputAction>(this, TEXT("IA_ForceDialogue"));
	PlayRevealAction = NewObject<UInputAction>(this, TEXT("IA_PlayReveal"));

	auto AddSwizzleY = [this](FEnhancedActionKeyMapping& Mapping)
	{
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(this);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		Mapping.Modifiers.Add(Swizzle);
	};
	auto AddNegate = [this](FEnhancedActionKeyMapping& Mapping)
	{
		Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	};

	FEnhancedActionKeyMapping& MoveW = MappingContext->MapKey(MoveAction, EKeys::W);
	AddSwizzleY(MoveW);
	FEnhancedActionKeyMapping& MoveS = MappingContext->MapKey(MoveAction, EKeys::S);
	AddSwizzleY(MoveS);
	AddNegate(MoveS);
	FEnhancedActionKeyMapping& MoveA = MappingContext->MapKey(MoveAction, EKeys::A);
	AddNegate(MoveA);
	MappingContext->MapKey(MoveAction, EKeys::D);

	FEnhancedActionKeyMapping& LookMap = MappingContext->MapKey(LookAction, EKeys::Mouse2D);
	UInputModifierNegate* NegateY = NewObject<UInputModifierNegate>(this);
	NegateY->bX = false;
	NegateY->bY = true;
	NegateY->bZ = false;
	LookMap.Modifiers.Add(NegateY);

	MappingContext->MapKey(InteractAction, EKeys::E);
	MappingContext->MapKey(ToggleCineAction, EKeys::H);
	MappingContext->MapKey(DebugAction, EKeys::F8);
	MappingContext->MapKey(Choice1Action, EKeys::One);
	MappingContext->MapKey(Choice2Action, EKeys::Two);
	MappingContext->MapKey(ContinueAction, EKeys::SpaceBar);
	MappingContext->MapKey(ContinueAction, EKeys::Enter);
	MappingContext->MapKey(ContinueAction, EKeys::LeftMouseButton);
	MappingContext->MapKey(ReplayAction, EKeys::R);
	MappingContext->MapKey(ExitSliceAction, EKeys::Escape);
	MappingContext->MapKey(SaveTestAction, EKeys::F5);
	MappingContext->MapKey(LoadTestAction, EKeys::F6);
	MappingContext->MapKey(ForceExploreAction, EKeys::F7);
	MappingContext->MapKey(ForceDialogueAction, EKeys::F9);
	MappingContext->MapKey(PlayRevealAction, EKeys::F10);
}

void AAfterlightPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	EnsureRuntimeInput();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(MappingContext, 0);
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		return;
	}
	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAfterlightPlayerController::HandleMove);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAfterlightPlayerController::HandleLook);
	EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleInteract);
	EIC->BindAction(ToggleCineAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleToggleCine);
	EIC->BindAction(DebugAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleDebug);
	EIC->BindAction(Choice1Action, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleChoice1);
	EIC->BindAction(Choice2Action, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleChoice2);
	EIC->BindAction(ContinueAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleContinue);
	EIC->BindAction(ReplayAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleReplay);
	EIC->BindAction(ExitSliceAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleExitSlice);
	EIC->BindAction(SaveTestAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleSaveTest);
	EIC->BindAction(LoadTestAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleLoadTest);
	EIC->BindAction(ForceExploreAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleForceExplore);
	EIC->BindAction(ForceDialogueAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleForceDialogue);
	EIC->BindAction(PlayRevealAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandlePlayReveal);
}

AAfterlightCharacter* AAfterlightPlayerController::GetAfterlightPawn() const
{
	return Cast<AAfterlightCharacter>(GetPawn());
}

FGameplayTag AAfterlightPlayerController::GetInputStateTag() const
{
	switch (InputState)
	{
	case EAfterlightInputState::Constrained: return AfterlightTags::Input_Constrained;
	case EAfterlightInputState::Locked: return AfterlightTags::Input_Locked;
	case EAfterlightInputState::Scripted: return AfterlightTags::Input_Scripted;
	default: return AfterlightTags::Input_Full;
	}
}

void AAfterlightPlayerController::SetInputState(EAfterlightInputState NewState)
{
	InputState = NewState;
	ApplyInputStateToPawn();
	bool bHoldCard = false;
	bool bCine = false;
	if (UWorld* World = GetWorld())
	{
		if (UAfterlightPresentationSubsystem* Presentation = World->GetSubsystem<UAfterlightPresentationSubsystem>())
		{
			bCine = Presentation->IsCineMode();
			bHoldCard = Presentation->IsHoldCard();
		}
	}
	const bool bEntryOrEnd = NewState == EAfterlightInputState::Locked && bHoldCard;
	if (bEntryOrEnd)
	{
		ApplyHoldCardFocus();
		if (AfterlightQaAutoEnabled())
		{
			bShowMouseCursor = false;
			bEnableClickEvents = false;
		}
		return;
	}
	const bool bShowCursor = NewState == EAfterlightInputState::Constrained && !bCine;
	bShowMouseCursor = bShowCursor;
	bEnableClickEvents = bShowCursor;
	if (bShowCursor)
	{
		SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}
}

void AAfterlightPlayerController::ApplyHoldCardFocus()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	if (UWorld* World = GetWorld())
	{
		if (UAfterlightPresentationSubsystem* Presentation = World->GetSubsystem<UAfterlightPresentationSubsystem>())
		{
			if (UAfterlightHUDWidget* Widget = Presentation->GetWidget())
			{
				Mode.SetWidgetToFocus(Widget->TakeWidget());
				Widget->SetKeyboardFocus();
			}
		}
	}
	SetInputMode(Mode);
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}
}

bool AAfterlightPlayerController::TryOwnerContinueInput()
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAfterlightSlice01Director> It(World); It; ++It)
		{
			if (It->TryAcceptContinue())
			{
				return true;
			}
			return false;
		}
	}
	return false;
}

void AAfterlightPlayerController::ApplyInputStateToPawn()
{
	if (AAfterlightCharacter* AfterlightPawn = GetAfterlightPawn())
	{
		const bool bFull = InputState == EAfterlightInputState::Full;
		AfterlightPawn->SetMoveEnabled(bFull);
		AfterlightPawn->SetLookEnabled(bFull);
	}
}

void AAfterlightPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (InputState != EAfterlightInputState::Full)
	{
		return;
	}
	if (AAfterlightCharacter* AfterlightPawn = GetAfterlightPawn())
	{
		AfterlightPawn->Move(Value);
		if (UWorld* MoveWorld = GetWorld())
		{
			static float LastMoveLog = -100.f;
			const float Now = MoveWorld->GetTimeSeconds();
			if (Now - LastMoveLog > 1.5f)
			{
				LastMoveLog = Now;
				UE_LOG(LogAfterlight, Verbose, TEXT("AFTERLIGHT_MOVE X=%.0f"), AfterlightPawn->GetActorLocation().X);
			}
		}
	}
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAfterlightSlice01Director> It(World); It; ++It)
		{
			It->NotifyPlayerMoved();
			break;
		}
	}
}

void AAfterlightPlayerController::HandleLook(const FInputActionValue& Value)
{
	if (InputState == EAfterlightInputState::Locked || InputState == EAfterlightInputState::Scripted)
	{
		return;
	}
	if (AAfterlightCharacter* AfterlightPawn = GetAfterlightPawn())
	{
		AfterlightPawn->Look(Value);
	}
}

void AAfterlightPlayerController::HandleInteract()
{
	if (InputState != EAfterlightInputState::Full)
	{
		return;
	}
	if (AAfterlightCharacter* AfterlightPawn = GetAfterlightPawn())
	{
		AfterlightPawn->Interact();
	}
}

void AAfterlightPlayerController::HandleToggleCine()
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAfterlightSlice01Director> It(World); It; ++It)
		{
			if (It->IsAwaitingEntry() || It->IsSliceComplete())
			{
				return;
			}
		}
	}
	if (UAfterlightPresentationSubsystem* Presentation = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		Presentation->ToggleCineMode();
	}
}

void AAfterlightPlayerController::HandleDebug()
{
	if (UAfterlightPresentationSubsystem* Presentation = GetWorld()->GetSubsystem<UAfterlightPresentationSubsystem>())
	{
		Presentation->ToggleDebugOverlay();
	}
}

void AAfterlightPlayerController::ChooseDialogue(int32 Index)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAfterlightNarrativeSubsystem* Narrative = GI->GetSubsystem<UAfterlightNarrativeSubsystem>())
		{
			if (UAfterlightDialogueRunner* Runner = Narrative->GetDialogueRunner())
			{
				Runner->SelectChoice(Index);
			}
		}
	}
}

void AAfterlightPlayerController::HandleChoice1()
{
	ChooseDialogue(0);
}

void AAfterlightPlayerController::HandleChoice2()
{
	ChooseDialogue(1);
}

void AAfterlightPlayerController::HandleContinue()
{
	TryOwnerContinueInput();
}

void AAfterlightPlayerController::HandleReplay()
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAfterlightSlice01Director> It(World); It; ++It)
		{
			It->TryReplay();
			return;
		}
	}
}

void AAfterlightPlayerController::HandleExitSlice()
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAfterlightSlice01Director> It(World); It; ++It)
		{
			if (!It->IsSliceComplete())
			{
				return;
			}
		}
		if (World->WorldType == EWorldType::Game)
		{
			FPlatformMisc::RequestExit(false);
		}
	}
}

void AAfterlightPlayerController::HandleSaveTest()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAfterlightSaveSubsystem* Save = GI->GetSubsystem<UAfterlightSaveSubsystem>())
		{
			Save->SaveTestSlot();
		}
	}
}

void AAfterlightPlayerController::HandleLoadTest()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAfterlightSaveSubsystem* Save = GI->GetSubsystem<UAfterlightSaveSubsystem>())
		{
			Save->LoadTestSlot();
		}
	}
}

void AAfterlightPlayerController::HandleForceExplore()
{
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->ReleaseToExplore(0.6f);
	}
	SetInputState(EAfterlightInputState::Full);
}

void AAfterlightPlayerController::HandleForceDialogue()
{
	if (UAfterlightCameraSubsystem* Camera = GetWorld()->GetSubsystem<UAfterlightCameraSubsystem>())
	{
		Camera->RequestShot(AfterlightShotIds::DialogueOTSCompanion, 0.5f);
	}
	SetInputState(EAfterlightInputState::Constrained);
}

void AAfterlightPlayerController::HandlePlayReveal()
{
	for (TActorIterator<AAfterlightLabDirector> It(GetWorld()); It; ++It)
	{
		It->PlayInspectReveal();
		return;
	}
}
