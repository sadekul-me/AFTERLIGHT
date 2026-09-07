#include "Slice/AfterlightLanternDrone.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"

AAfterlightLanternDrone::AAfterlightLanternDrone()
{
	PrimaryActorTick.bCanEverTick = true;
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetWorldScale3D(FVector(0.45f, 0.45f, 0.28f));
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Spotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spotlight"));
	Spotlight->SetupAttachment(Body);
	Spotlight->SetRelativeRotation(FRotator(-70.f, 0.f, 0.f));
	Spotlight->SetIntensity(12000.f);
	Spotlight->SetInnerConeAngle(12.f);
	Spotlight->SetOuterConeAngle(28.f);
	Spotlight->SetAttenuationRadius(900.f);
	Spotlight->SetLightColor(FLinearColor(0.92f, 0.95f, 1.f));

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Body);
	Label->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetText(FText::FromString(TEXT("HELION")));
	Label->SetTextRenderColor(FColor(180, 200, 220));
	Label->SetWorldSize(22.f);
}

void AAfterlightLanternDrone::BeginPlay()
{
	Super::BeginPlay();
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		Body->SetStaticMesh(Sphere);
	}
}

void AAfterlightLanternDrone::BeginSweep(const FVector& Start, const FVector& End, float DurationSeconds)
{
	SweepStart = Start;
	SweepEnd = End;
	SweepDuration = FMath::Max(0.5f, DurationSeconds);
	SweepElapsed = 0.f;
	bSweeping = true;
	bFinished = false;
	SetActorLocation(Start);
	SetActorHiddenInGame(false);
}

void AAfterlightLanternDrone::ResetSweep()
{
	bSweeping = false;
	bFinished = false;
	SweepElapsed = 0.f;
	SetActorHiddenInGame(true);
}

void AAfterlightLanternDrone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bSweeping)
	{
		return;
	}
	SweepElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(SweepElapsed / SweepDuration, 0.f, 1.f);
	const float Ease = Alpha * Alpha * (3.f - 2.f * Alpha);
	SetActorLocation(FMath::Lerp(SweepStart, SweepEnd, Ease));
	if (Alpha >= 1.f)
	{
		bSweeping = false;
		bFinished = true;
	}
}
