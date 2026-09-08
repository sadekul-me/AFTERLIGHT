#include "Character/AfterlightPlaceholderVisuals.h"
#include "Audio/AfterlightTempAudio.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Animation/AnimationAsset.h"
#include "Engine/World.h"

namespace
{
	const FName TagMannequin(TEXT("AL.Mannequin"));
	const FName TagPrimitive(TEXT("AL.Primitive"));
	const FName TagRecover(TEXT("AL.Recover"));
	const FName TagWalkPhase(TEXT("AL.WalkPhase"));
	const FName TagFootstep(TEXT("AL.Footstep"));
	const FName TagPlayingWalk(TEXT("AL.PlayingWalk"));

	UAnimationAsset* IdleAnim();
	UAnimationAsset* WalkAnim();

	UStaticMeshComponent* MakePart(ACharacter* Character, FName Name, UStaticMesh* Mesh, UMaterialInstanceDynamic* Mid,
		const FVector& Location, const FRotator& Rotation, const FVector& Scale, FName Tag = NAME_None)
	{
		UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(Character, Name);
		Part->SetupAttachment(Character->GetCapsuleComponent());
		Part->SetStaticMesh(Mesh);
		Part->SetRelativeLocation(Location);
		Part->SetRelativeRotation(Rotation);
		Part->SetRelativeScale3D(Scale);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetCastShadow(true);
		Part->SetCastContactShadow(false);
		if (Mid)
		{
			Part->SetMaterial(0, Mid);
		}
		if (!Tag.IsNone())
		{
			Part->ComponentTags.Add(Tag);
		}
		Part->RegisterComponent();
		return Part;
	}

	UMaterialInstanceDynamic* MakeColor(ACharacter* Character, UMaterialInterface* Base, const FLinearColor& Color)
	{
		if (!Base)
		{
			return nullptr;
		}
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, Character);
		Mid->SetVectorParameterValue(TEXT("Color"), Color);
		return Mid;
	}

	void AttachPrimitiveHumanoid(ACharacter* Character, bool bCompanion)
	{
		UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (!Cylinder || !Sphere)
		{
			return;
		}

		const FLinearColor Outfit = bCompanion
			? FLinearColor(0.62f, 0.28f, 0.12f)
			: FLinearColor(0.08f, 0.11f, 0.16f);
		const FLinearColor Skin = bCompanion
			? FLinearColor(0.72f, 0.48f, 0.32f)
			: FLinearColor(0.32f, 0.36f, 0.4f);
		const FLinearColor Accent = bCompanion
			? FLinearColor(0.85f, 0.62f, 0.28f)
			: FLinearColor(0.18f, 0.28f, 0.38f);

		UMaterialInstanceDynamic* OutfitMid = MakeColor(Character, BaseMat, Outfit);
		UMaterialInstanceDynamic* SkinMid = MakeColor(Character, BaseMat, Skin);
		UMaterialInstanceDynamic* AccentMid = MakeColor(Character, BaseMat, Accent);

		const float HipZ = -36.f;
		MakePart(Character, TEXT("Pelvis"), Cylinder, OutfitMid, FVector(0.f, 0.f, HipZ), FRotator::ZeroRotator, FVector(0.42f, 0.28f, 0.18f));
		MakePart(Character, TEXT("Torso"), Cylinder, OutfitMid, FVector(0.f, 0.f, -4.f), FRotator::ZeroRotator,
			bCompanion ? FVector(0.4f, 0.28f, 0.52f) : FVector(0.46f, 0.3f, 0.55f));
		MakePart(Character, TEXT("Shoulders"), Cylinder, OutfitMid, FVector(0.f, 0.f, 22.f), FRotator(0.f, 0.f, 90.f),
			bCompanion ? FVector(0.16f, 0.16f, 0.52f) : FVector(0.18f, 0.18f, 0.6f));

		MakePart(Character, TEXT("UpperArmL"), Cylinder, OutfitMid, FVector(0.f, 28.f, 10.f), FRotator(0.f, 0.f, 12.f), FVector(0.12f, 0.12f, 0.28f), TEXT("AL.ArmL"));
		MakePart(Character, TEXT("UpperArmR"), Cylinder, OutfitMid, FVector(0.f, -28.f, 10.f), FRotator(0.f, 0.f, -12.f), FVector(0.12f, 0.12f, 0.28f), TEXT("AL.ArmR"));
		MakePart(Character, TEXT("ForeArmL"), Cylinder, SkinMid, FVector(2.f, 30.f, -18.f), FRotator(8.f, 0.f, 8.f), FVector(0.1f, 0.1f, 0.24f), TEXT("AL.ForeL"));
		MakePart(Character, TEXT("ForeArmR"), Cylinder, SkinMid, FVector(2.f, -30.f, -18.f), FRotator(8.f, 0.f, -8.f), FVector(0.1f, 0.1f, 0.24f), TEXT("AL.ForeR"));

		MakePart(Character, TEXT("ThighL"), Cylinder, OutfitMid, FVector(0.f, 10.f, -62.f), FRotator::ZeroRotator, FVector(0.14f, 0.14f, 0.32f), TEXT("AL.LegL"));
		MakePart(Character, TEXT("ThighR"), Cylinder, OutfitMid, FVector(0.f, -10.f, -62.f), FRotator::ZeroRotator, FVector(0.14f, 0.14f, 0.32f), TEXT("AL.LegR"));
		MakePart(Character, TEXT("ShinL"), Cylinder, AccentMid, FVector(0.f, 10.f, -92.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 0.28f), TEXT("AL.ShinL"));
		MakePart(Character, TEXT("ShinR"), Cylinder, AccentMid, FVector(0.f, -10.f, -92.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 0.28f), TEXT("AL.ShinR"));

		MakePart(Character, TEXT("Head"), Sphere, SkinMid, FVector(0.f, 0.f, 52.f), FRotator::ZeroRotator,
			bCompanion ? FVector(0.34f, 0.32f, 0.36f) : FVector(0.32f, 0.3f, 0.34f));
		MakePart(Character, TEXT("Face"), Sphere, AccentMid, FVector(10.f, 0.f, 50.f), FRotator::ZeroRotator, FVector(0.16f, 0.18f, 0.12f), TEXT("AL.Face"));
		if (bCompanion)
		{
			MakePart(Character, TEXT("Hair"), Sphere, MakeColor(Character, BaseMat, FLinearColor(0.18f, 0.08f, 0.04f)),
				FVector(-4.f, 0.f, 66.f), FRotator::ZeroRotator, FVector(0.32f, 0.3f, 0.22f));
		}

		Character->GetCapsuleComponent()->ComponentTags.Add(TagPrimitive);
	}

	bool AttachMannequin(ACharacter* Character, bool bCompanion)
	{
		USkeletalMesh* MeshAsset = LoadObject<USkeletalMesh>(nullptr, bCompanion
			? TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple")
			: TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
		if (!MeshAsset)
		{
			return false;
		}

		USkeletalMeshComponent* Mesh = Character->GetMesh();
		if (!Mesh)
		{
			return false;
		}

		const float HalfHeight = Character->GetCapsuleComponent() ? Character->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() : 92.f;
		Mesh->SetSkeletalMeshAsset(MeshAsset);
		Mesh->SetRelativeLocation(FVector(0.f, 0.f, -HalfHeight));
		Mesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		Mesh->SetCastShadow(true);
		Mesh->SetCastContactShadow(false);
		Mesh->SetDisablePostProcessBlueprint(true);
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		Mesh->bEnableUpdateRateOptimizations = true;
		Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		Mesh->ComponentTags.Add(TagMannequin);
		if (!bCompanion)
		{
			Mesh->ComponentTags.Add(TagRecover);
		}

		const FLinearColor Tint = bCompanion
			? FLinearColor(1.08f, 0.62f, 0.28f, 1.f)
			: FLinearColor(0.38f, 0.48f, 0.58f, 1.f);
		for (int32 Slot = 0; Slot < Mesh->GetNumMaterials(); ++Slot)
		{
			if (UMaterialInstanceDynamic* Mid = Mesh->CreateAndSetMaterialInstanceDynamic(Slot))
			{
				Mid->SetVectorParameterValue(TEXT("Tint"), Tint);
				Mid->SetVectorParameterValue(TEXT("Paint"), Tint);
			}
		}

		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (Cube && BaseMat)
		{
			UMaterialInstanceDynamic* Gear = MakeColor(Character, BaseMat, bCompanion
				? FLinearColor(0.92f, 0.42f, 0.12f)
				: FLinearColor(0.05f, 0.07f, 0.1f));
			UStaticMeshComponent* Plate = NewObject<UStaticMeshComponent>(Character, TEXT("IdentityPlate"));
			Plate->SetupAttachment(Mesh, FName(TEXT("spine_01")));
			if (!Mesh->DoesSocketExist(FName(TEXT("spine_01"))))
			{
				Plate->SetupAttachment(Mesh);
				Plate->SetRelativeLocation(FVector(8.f, 0.f, 70.f));
			}
			else
			{
				Plate->SetRelativeLocation(bCompanion ? FVector(8.f, 0.f, 6.f) : FVector(6.f, 0.f, 4.f));
			}
			Plate->SetStaticMesh(Cube);
			Plate->SetRelativeScale3D(bCompanion ? FVector(0.18f, 0.28f, 0.08f) : FVector(0.22f, 0.32f, 0.28f));
			Plate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Plate->SetCastShadow(false);
			if (Gear)
			{
				Plate->SetMaterial(0, Gear);
			}
			Plate->RegisterComponent();
		}

		if (UAnimationAsset* Idle = IdleAnim())
		{
			Mesh->PlayAnimation(Idle, true);
		}
		return true;
	}

	void TickPrimitiveWalk(ACharacter* Character, float Speed, float DeltaSeconds)
	{
		static TMap<TWeakObjectPtr<ACharacter>, float> WalkPhase;
		float& Walk = WalkPhase.FindOrAdd(Character);
		Walk += DeltaSeconds * FMath::GetMappedRangeValueClamped(FVector2D(0.f, 220.f), FVector2D(0.f, 9.f), Speed);
		const float Swing = FMath::Sin(Walk) * FMath::GetMappedRangeValueClamped(FVector2D(0.f, 180.f), FVector2D(0.f, 26.f), Speed);
		const float Idle = FMath::Sin(Character->GetGameTimeSinceCreation() * 1.6f) * 2.2f;

		TArray<UStaticMeshComponent*> Parts;
		Character->GetComponents(Parts);
		auto Pose = [&Parts, Swing, Idle, Speed](FName Tag, float Sign, bool bLeg)
		{
			for (UStaticMeshComponent* Mesh : Parts)
			{
				if (!Mesh || !Mesh->ComponentHasTag(Tag))
				{
					continue;
				}
				const float Angle = (Speed > 18.f ? Swing : Idle) * Sign;
				Mesh->SetRelativeRotation(bLeg ? FRotator(Angle, 0.f, 0.f) : FRotator(-Angle * 0.7f, 0.f, Mesh->GetRelativeRotation().Roll));
			}
		};
		Pose(TEXT("AL.LegL"), 1.f, true);
		Pose(TEXT("AL.LegR"), -1.f, true);
		Pose(TEXT("AL.ShinL"), 1.f, true);
		Pose(TEXT("AL.ShinR"), -1.f, true);
		Pose(TEXT("AL.ArmL"), -1.f, false);
		Pose(TEXT("AL.ArmR"), 1.f, false);
	}

	UAnimationAsset* IdleAnim()
	{
		static TWeakObjectPtr<UAnimationAsset> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));
		}
		return Cached.Get();
	}

	UAnimationAsset* WalkAnim()
	{
		static TWeakObjectPtr<UAnimationAsset> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd"));
		}
		return Cached.Get();
	}

	void TickMannequin(ACharacter* Character, float Speed)
	{
		USkeletalMeshComponent* Mesh = Character->GetMesh();
		if (!Mesh)
		{
			return;
		}

		UAnimationAsset* Idle = IdleAnim();
		UAnimationAsset* Walk = WalkAnim();
		const bool bWalk = Speed > 28.f && Walk;
		const bool bPlayingWalk = Mesh->ComponentTags.Contains(TagPlayingWalk);
		if (bWalk != bPlayingWalk)
		{
			if (bWalk)
			{
				Mesh->PlayAnimation(Walk, true);
				Mesh->ComponentTags.AddUnique(TagPlayingWalk);
			}
			else if (Idle)
			{
				Mesh->PlayAnimation(Idle, true);
				Mesh->ComponentTags.Remove(TagPlayingWalk);
			}
		}
		Mesh->SetPlayRate(bWalk ? FMath::Clamp(Speed / 190.f, 0.8f, 1.15f) : 1.f);
	}

	void TickRecover(ACharacter* Character, float DeltaSeconds)
	{
		USkeletalMeshComponent* Mesh = Character->GetMesh();
		UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
		if (!Mesh || !Capsule || !Mesh->ComponentTags.Contains(TagRecover))
		{
			return;
		}

		static TMap<TWeakObjectPtr<ACharacter>, float> Recover;
		float& Alpha = Recover.FindOrAdd(Character, 1.f);
		Alpha = FMath::Max(0.f, Alpha - DeltaSeconds / 3.4f);
		const float HalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
		Mesh->SetRelativeLocation(FVector(18.f * Alpha, 0.f, -HalfHeight - 10.f * Alpha));
		Mesh->SetRelativeRotation(FRotator(22.f * Alpha, -90.f, 0.f));
		if (Alpha <= 0.f)
		{
			Mesh->ComponentTags.Remove(TagRecover);
			Recover.Remove(Character);
		}
	}

	void TickFootsteps(ACharacter* Character, float Speed, float DeltaSeconds)
	{
		if (Speed < 40.f)
		{
			return;
		}
		static TMap<TWeakObjectPtr<ACharacter>, float> Accel;
		float& Acc = Accel.FindOrAdd(Character);
		Acc += DeltaSeconds;
		const float Stride = FMath::GetMappedRangeValueClamped(FVector2D(80.f, 240.f), FVector2D(0.52f, 0.38f), Speed);
		if (Acc >= Stride)
		{
			Acc = 0.f;
			FAfterlightTempAudio::PlayOneShot(Character->GetWorld(), Character, EAfterlightTempBed::Footstep, 0.07f);
		}
	}
}

void FAfterlightPlaceholderVisuals::Attach(ACharacter* Character, bool bCompanion)
{
	if (!Character)
	{
		return;
	}

	if (!AttachMannequin(Character, bCompanion))
	{
		AttachPrimitiveHumanoid(Character, bCompanion);
	}

	if (bCompanion)
	{
		UPointLightComponent* Rim = NewObject<UPointLightComponent>(Character, TEXT("PlaceholderRim"));
		Rim->SetupAttachment(Character->GetCapsuleComponent());
		Rim->SetRelativeLocation(FVector(14.f, 0.f, 44.f));
		Rim->SetIntensity(8.5f);
		Rim->SetAttenuationRadius(280.f);
		Rim->SetLightColor(FLinearColor(1.f, 0.62f, 0.28f));
		Rim->SetCastShadows(false);
		Rim->SetMobility(EComponentMobility::Movable);
		Rim->RegisterComponent();
	}
}

void FAfterlightPlaceholderVisuals::Tick(ACharacter* Character, float DeltaSeconds)
{
	if (!Character)
	{
		return;
	}
	const float Speed = Character->GetVelocity().Size2D();
	TickRecover(Character, DeltaSeconds);
	if (UsesMannequin(Character))
	{
		TickMannequin(Character, Speed);
	}
	else
	{
		TickPrimitiveWalk(Character, Speed, DeltaSeconds);
	}
	TickFootsteps(Character, Speed, DeltaSeconds);
}

void FAfterlightPlaceholderVisuals::BeginRecover(ACharacter* Character)
{
	if (!Character || !Character->GetMesh())
	{
		return;
	}
	Character->GetMesh()->ComponentTags.AddUnique(TagRecover);
}

bool FAfterlightPlaceholderVisuals::UsesMannequin(const ACharacter* Character)
{
	return Character && Character->GetMesh() && Character->GetMesh()->ComponentTags.Contains(TagMannequin);
}
