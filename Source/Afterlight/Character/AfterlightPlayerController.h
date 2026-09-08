#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AfterlightPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AAfterlightCharacter;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EAfterlightInputState : uint8
{
	Full,
	Constrained,
	Locked,
	Scripted
};

UCLASS()
class AFTERLIGHT_API AAfterlightPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAfterlightPlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Afterlight|Input")
	void SetInputState(EAfterlightInputState NewState);

	void ApplyHoldCardFocus();
	bool TryOwnerContinueInput();

	UFUNCTION(BlueprintPure, Category = "Afterlight|Input")
	EAfterlightInputState GetInputState() const { return InputState; }

	UFUNCTION(BlueprintPure, Category = "Afterlight|Input")
	FGameplayTag GetInputStateTag() const;

	void ChooseDialogue(int32 Index);

protected:
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleInteract();
	void HandleToggleCine();
	void HandleDebug();
	void HandleChoice1();
	void HandleChoice2();
	void HandleContinue();
	void HandleReplay();
	void HandleExitSlice();
	void HandleSaveTest();
	void HandleLoadTest();
	void HandleForceExplore();
	void HandleForceDialogue();
	void HandlePlayReveal();
	void EnsureRuntimeInput();
	void ApplyInputStateToPawn();
	AAfterlightCharacter* GetAfterlightPawn() const;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY()
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY()
	TObjectPtr<UInputAction> ToggleCineAction;

	UPROPERTY()
	TObjectPtr<UInputAction> DebugAction;

	UPROPERTY()
	TObjectPtr<UInputAction> Choice1Action;

	UPROPERTY()
	TObjectPtr<UInputAction> Choice2Action;

	UPROPERTY()
	TObjectPtr<UInputAction> ContinueAction;

	UPROPERTY()
	TObjectPtr<UInputAction> ReplayAction;

	UPROPERTY()
	TObjectPtr<UInputAction> ExitSliceAction;

	UPROPERTY()
	TObjectPtr<UInputAction> SaveTestAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LoadTestAction;

	UPROPERTY()
	TObjectPtr<UInputAction> ForceExploreAction;

	UPROPERTY()
	TObjectPtr<UInputAction> ForceDialogueAction;

	UPROPERTY()
	TObjectPtr<UInputAction> PlayRevealAction;

	EAfterlightInputState InputState = EAfterlightInputState::Full;
};
