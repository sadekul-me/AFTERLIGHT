#include "Interaction/AfterlightInspectableActor.h"
#include "Interaction/AfterlightInteractableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Core/AfterlightGameplayTags.h"

AAfterlightInspectableActor::AAfterlightInspectableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	Interactable = CreateDefaultSubobject<UAfterlightInteractableComponent>(TEXT("Interactable"));
	Interactable->PromptText = NSLOCTEXT("Afterlight", "InspectPrompt", "Inspect");
	Interactable->Verb = AfterlightTags::Interaction_Inspect;
	Interactable->GrantFlag = AfterlightTags::Story_Test_InspectedObject;
}

bool AAfterlightInspectableActor::CanInteract(AActor* Interactor) const
{
	return Interactable && Interactable->CanInteract(Interactor);
}

FText AAfterlightInspectableActor::GetPromptText() const
{
	return Interactable ? Interactable->GetPromptText() : FText::GetEmpty();
}

FGameplayTag AAfterlightInspectableActor::GetInteractionVerb() const
{
	return AfterlightTags::Interaction_Inspect;
}

void AAfterlightInspectableActor::ExecuteInteraction(AActor* Interactor)
{
	if (Interactable)
	{
		Interactable->ExecuteInteraction(Interactor);
	}
}
