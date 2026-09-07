#include "Character/AfterlightPlayerController.h"
#include "Character/AfterlightCharacter.h"
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
#include "Engine/GameInstance.h"

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
	SaveTestAction = NewObject<UInputAction>(this, TEXT("IA_SaveTest"));
	LoadTestAction = NewObject<UInputAction>(this, TEXT("IA_LoadTest"));

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
	MappingContext->MapKey(SaveTestAction, EKeys::F5);
	MappingContext->MapKey(LoadTestAction, EKeys::F6);
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
	EIC->BindAction(SaveTestAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleSaveTest);
	EIC->BindAction(LoadTestAction, ETriggerEvent::Started, this, &AAfterlightPlayerController::HandleLoadTest);
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
	default: return AfterlightTags::Input_Full;
	}
}

void AAfterlightPlayerController::SetInputState(EAfterlightInputState NewState)
{
	InputState = NewState;
	ApplyInputStateToPawn();
	const bool bShowCursor = NewState == EAfterlightInputState::Constrained;
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
}

void AAfterlightPlayerController::ApplyInputStateToPawn()
{
	if (AAfterlightCharacter* AfterlightPawn = GetAfterlightPawn())
	{
		const bool bFull = InputState == EAfterlightInputState::Full;
		const bool bLocked = InputState == EAfterlightInputState::Locked;
		AfterlightPawn->SetMoveEnabled(bFull);
		AfterlightPawn->SetLookEnabled(!bLocked);
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
	}
}

void AAfterlightPlayerController::HandleLook(const FInputActionValue& Value)
{
	if (InputState == EAfterlightInputState::Locked)
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
