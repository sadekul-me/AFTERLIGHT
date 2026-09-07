#include "Core/AfterlightGameplayTags.h"

namespace AfterlightTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Register_Explore, "Camera.Register.Explore", "Gameplay exploration camera");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Register_Dialogue, "Camera.Register.Dialogue", "Conversation framing camera");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Register_Threat, "Camera.Register.Threat", "Pressure/threat camera");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Register_Cinematic, "Camera.Register.Cinematic", "Sequencer/cinematic camera");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Full, "Input.Full", "Full gameplay input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Constrained, "Input.Constrained", "Look/choices only");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Locked, "Input.Locked", "Gameplay input locked");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cinematic_Active, "Cinematic.Active", "Cinematic coordinator holds control");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Test_MetCompanion, "Story.Test.MetCompanion", "Technical-lab: talked to companion");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Test_InspectedObject, "Story.Test.InspectedObject", "Technical-lab: inspected world object");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Relationship_Trust_High, "Relationship.Trust.High", "Trust at or above high threshold");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Relationship_Trust_Low, "Relationship.Trust.Low", "Trust at or below low threshold");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Relationship_Suspicion_High, "Relationship.Suspicion.High", "Suspicion at or above high threshold");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Relationship_Suspicion_Low, "Relationship.Suspicion.Low", "Suspicion at or below low threshold");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Companion_Follow_Close, "Companion.Follow.Close", "Companion uses close follow distance");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Companion_Follow_Far, "Companion.Follow.Far", "Companion uses far follow distance");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Talk, "Interaction.Talk", "Talk interaction verb");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Inspect, "Interaction.Inspect", "Inspect interaction verb");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Use, "Interaction.Use", "Use interaction verb");
}
