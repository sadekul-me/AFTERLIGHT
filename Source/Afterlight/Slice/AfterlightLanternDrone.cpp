#include "Slice/AfterlightLanternDrone.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

namespace
{
	UMaterialInstanceDynamic* DroneColor(UObject* Outer, const FLinearColor& Color)
	{
		UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (!Base)
		{
			return nullptr;
		}
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, Outer);
		Mid->SetVectorParameterValue(TEXT("Color"), Color);
		return Mid;
	}

	UStaticMeshComponent* MakeDronePart(AActor* Owner, USceneComponent* Parent, FName Name, UStaticMesh* Mesh,
		const FVector& Location, const FRotator& Rotation, const FVector& Scale, UMaterialInstanceDynamic* Mid)
	{
		UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(Owner, Name);
		Part->SetupAttachment(Parent);
		Part->SetStaticMesh(Mesh);
		Part->SetRelativeLocation(Location);
		Part->SetRelativeRotation(Rotation);
		Part->SetRelativeScale3D(Scale);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetCastShadow(true);
		if (Mid)
		{
			Part->SetMaterial(0, Mid);
		}
		Part->RegisterComponent();
		return Part;
	}
}

AAfterlightLanternDrone::AAfterlightLanternDrone()
{
	PrimaryActorTick.bCanEverTick = true;
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetWorldScale3D(FVector(1.f, 1.f, 1.f));
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Spotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spotlight"));
	Spotlight->SetupAttachment(Body);
	Spotlight->SetRelativeLocation(FVector(28.f, 0.f, -10.f));
	Spotlight->SetRelativeRotation(FRotator(-58.f, 0.f, 0.f));
	Spotlight->SetIntensity(9000.f);
	Spotlight->SetInnerConeAngle(14.f);
	Spotlight->SetOuterConeAngle(34.f);
	Spotlight->SetAttenuationRadius(900.f);
	Spotlight->SetLightColor(FLinearColor(0.55f, 0.92f, 1.f));
	Spotlight->SetCastShadows(false);

	Beacon = CreateDefaultSubobject<UPointLightComponent>(TEXT("Beacon"));
	Beacon->SetupAttachment(Body);
	Beacon->SetRelativeLocation(FVector(0.f, 0.f, 22.f));
	Beacon->SetIntensity(22.f);
	Beacon->SetAttenuationRadius(520.f);
	Beacon->SetLightColor(FLinearColor(0.45f, 0.95f, 1.f));
	Beacon->SetCastShadows(false);

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Body);
	Label->SetRelativeLocation(FVector(0.f, 0.f, 36.f));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetText(FText::FromString(TEXT("HELION")));
	Label->SetTextRenderColor(FColor(180, 240, 255));
	Label->SetWorldSize(22.f);
}

void AAfterlightLanternDrone::BeginPlay()
{
	Super::BeginPlay();
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
	UMaterialInstanceDynamic* Hull = DroneColor(this, FLinearColor(0.22f, 0.28f, 0.34f));
	UMaterialInstanceDynamic* Accent = DroneColor(this, FLinearColor(0.15f, 0.85f, 1.f));
	UMaterialInstanceDynamic* Lens = DroneColor(this, FLinearColor(1.f, 1.f, 1.f));
	UMaterialInstanceDynamic* Beam = DroneColor(this, FLinearColor(0.35f, 0.82f, 1.f));
	if (Sphere)
	{
		Body->SetStaticMesh(Sphere);
		Body->SetRelativeScale3D(FVector(0.92f, 0.92f, 0.48f));
		if (Hull)
		{
			Body->SetMaterial(0, Hull);
		}
	}
	if (Cylinder)
	{
		MakeDronePart(this, Body, TEXT("Ring"), Cylinder, FVector(0.f, 0.f, 6.f), FRotator::ZeroRotator, FVector(1.05f, 1.05f, 0.08f), Hull);
		MakeDronePart(this, Body, TEXT("Mast"), Cylinder, FVector(0.f, 0.f, 28.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 0.22f), Accent);
	}
	if (Cone)
	{
		MakeDronePart(this, Body, TEXT("Lens"), Cone, FVector(36.f, 0.f, -10.f), FRotator(-80.f, 0.f, 0.f), FVector(0.28f, 0.28f, 0.34f), Lens);
		if (Spotlight)
		{
			ScanBeam = MakeDronePart(this, Spotlight, TEXT("ScanBeam"), Cone, FVector(70.f, 0.f, 0.f), FRotator(90.f, 0.f, 0.f), FVector(0.28f, 0.28f, 2.4f), Beam);
			if (ScanBeam)
			{
				ScanBeam->SetCastShadow(false);
			}
		}
	}
	if (Cube)
	{
		MakeDronePart(this, Body, TEXT("FinL"), Cube, FVector(-12.f, 38.f, 4.f), FRotator(0.f, 18.f, 12.f), FVector(0.38f, 0.06f, 0.18f), Hull);
		MakeDronePart(this, Body, TEXT("FinR"), Cube, FVector(-12.f, -38.f, 4.f), FRotator(0.f, -18.f, -12.f), FVector(0.38f, 0.06f, 0.18f), Hull);
		MakeDronePart(this, Body, TEXT("ArmF"), Cube, FVector(28.f, 0.f, 8.f), FRotator::ZeroRotator, FVector(0.28f, 0.07f, 0.07f), Accent);
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
	SetActorRotation(FRotator(0.f, SweepElapsed * 40.f, FMath::Sin(SweepElapsed * 1.4f) * 8.f));
	ScanYaw += DeltaSeconds * 40.f;
	if (Spotlight)
	{
		Spotlight->SetRelativeRotation(FRotator(-54.f, FMath::Sin(ScanYaw * 0.07f) * 10.f, 0.f));
	}
	if (Alpha >= 1.f)
	{
		bSweeping = false;
		bFinished = true;
	}
}
