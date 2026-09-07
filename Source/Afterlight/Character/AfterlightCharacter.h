#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AfterlightCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAfterlightInteractionComponent;
class UAfterlightFramingTargetsComponent;
class UAfterlightCameraRecipe;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class AFTERLIGHT_API AAfterlightCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAfterlightCharacter();

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UAfterlightInteractionComponent* GetInteractionComponent() const { return Interaction; }
	UAfterlightFramingTargetsComponent* GetFramingTargets() const { return Framing; }
	void ApplyExploreRecipe(const UAfterlightCameraRecipe* Recipe);

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void SetMoveEnabled(bool bEnabled);
	void SetLookEnabled(bool bEnabled);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<UAfterlightInteractionComponent> Interaction;

	UPROPERTY(VisibleAnywhere, Category = "Afterlight")
	TObjectPtr<UAfterlightFramingTargetsComponent> Framing;

	UPROPERTY(EditAnywhere, Category = "Afterlight|Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditAnywhere, Category = "Afterlight|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Afterlight|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Afterlight|Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, Category = "Afterlight|Input")
	TObjectPtr<UInputAction> ToggleCineAction;

	UPROPERTY(EditAnywhere, Category = "Afterlight|Input")
	TObjectPtr<UInputAction> DebugAction;

	UPROPERTY(EditAnywhere, Category = "Afterlight|Movement")
	float WalkSpeed = 280.f;

	UPROPERTY(EditAnywhere, Category = "Afterlight|Movement")
	float DevFastSpeed = 520.f;

	bool bMoveEnabled = true;
	bool bLookEnabled = true;
};
