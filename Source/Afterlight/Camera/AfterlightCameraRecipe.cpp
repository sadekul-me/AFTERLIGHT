#include "Camera/AfterlightCameraRecipe.h"

FName AfterlightRegisterToName(EAfterlightCameraRegister Register)
{
	switch (Register)
	{
	case EAfterlightCameraRegister::Dialogue: return TEXT("Dialogue");
	case EAfterlightCameraRegister::Cinematic: return TEXT("Cinematic");
	case EAfterlightCameraRegister::Threat: return TEXT("Threat");
	case EAfterlightCameraRegister::Intimate: return TEXT("Intimate");
	case EAfterlightCameraRegister::Reveal: return TEXT("Reveal");
	default: return TEXT("Explore");
	}
}

EAfterlightCameraRegister AfterlightRegisterFromName(FName Name)
{
	if (Name == TEXT("Dialogue")) return EAfterlightCameraRegister::Dialogue;
	if (Name == TEXT("Cinematic")) return EAfterlightCameraRegister::Cinematic;
	if (Name == TEXT("Threat")) return EAfterlightCameraRegister::Threat;
	if (Name == TEXT("Intimate")) return EAfterlightCameraRegister::Intimate;
	if (Name == TEXT("Reveal")) return EAfterlightCameraRegister::Reveal;
	return EAfterlightCameraRegister::Explore;
}

bool AfterlightIsKnownRegisterName(FName Name)
{
	return Name == TEXT("Explore")
		|| Name == TEXT("Dialogue")
		|| Name == TEXT("Cinematic")
		|| Name == TEXT("Threat")
		|| Name == TEXT("Intimate")
		|| Name == TEXT("Reveal");
}

UAfterlightCameraRecipe* UAfterlightCameraRecipe::CreateDefault(UObject* Outer, EAfterlightCameraRegister Register)
{
	UAfterlightCameraRecipe* Recipe = NewObject<UAfterlightCameraRecipe>(Outer);
	Recipe->Register = Register;
	Recipe->RecipeId = AfterlightRegisterToName(Register);

	switch (Register)
	{
	case EAfterlightCameraRegister::Dialogue:
		Recipe->FocalLength = 32.f;
		Recipe->Aperture = 4.5f;
		Recipe->BlendTime = 0.85f;
		Recipe->FocusMode = EAfterlightCameraFocusMode::Target;
		Recipe->bEnableDOF = false;
		Recipe->ShoulderSide = 1.f;
		break;
	case EAfterlightCameraRegister::Intimate:
		Recipe->FocalLength = 35.f;
		Recipe->Aperture = 4.0f;
		Recipe->BlendTime = 0.7f;
		Recipe->FocusMode = EAfterlightCameraFocusMode::Target;
		Recipe->bEnableDOF = false;
		Recipe->ShoulderSide = 1.f;
		break;
	case EAfterlightCameraRegister::Reveal:
		Recipe->FocalLength = 35.f;
		Recipe->Aperture = 3.5f;
		Recipe->BlendTime = 0.7f;
		Recipe->FocusMode = EAfterlightCameraFocusMode::Manual;
		Recipe->ManualFocusDistance = 320.f;
		Recipe->bEnableDOF = true;
		Recipe->PushInDistance = 45.f;
		Recipe->PushInTime = 2.4f;
		break;
	case EAfterlightCameraRegister::Threat:
		Recipe->FocalLength = 24.f;
		Recipe->Aperture = 4.0f;
		Recipe->BlendTime = 0.45f;
		Recipe->GameplayFOV = 68.f;
		Recipe->MovementLag = 7.f;
		Recipe->bEnableDOF = false;
		break;
	case EAfterlightCameraRegister::Cinematic:
		Recipe->FocalLength = 24.f;
		Recipe->Aperture = 4.0f;
		Recipe->BlendTime = 0.5f;
		Recipe->FocusMode = EAfterlightCameraFocusMode::Manual;
		Recipe->ManualFocusDistance = 900.f;
		Recipe->bEnableDOF = false;
		break;
	default:
		Recipe->FocalLength = 40.f;
		Recipe->Aperture = 4.f;
		Recipe->GameplayFOV = 56.f;
		Recipe->ArmLength = 360.f;
		Recipe->CameraHeight = 72.f;
		Recipe->ShoulderSide = 1.f;
		Recipe->BlendTime = 0.8f;
		Recipe->MovementLag = 5.5f;
		Recipe->RotationLag = 9.f;
		Recipe->bEnableDOF = false;
		break;
	}
	return Recipe;
}
