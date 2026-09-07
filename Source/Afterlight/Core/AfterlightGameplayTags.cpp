#include "Core/AfterlightGameplayTags.h"

namespace AfterlightTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Register_Explore, "Camera.Register.Explore", "Gameplay exploration camera");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Register_Dialogue, "Camera.Register.Dialogue", "Conversation framing camera");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Register_Threat, "Camera.Register.Threat", "Pressure/threat camera");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Register_Cinematic, "Camera.Register.Cinematic", "Sequencer/cinematic camera");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Register_Intimate, "Camera.Register.Intimate", "Tight emotional framing");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Camera_Register_Reveal, "Camera.Register.Reveal", "Deliberate reveal composition");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Full, "Input.Full", "Full gameplay input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Constrained, "Input.Constrained", "Look/choices only");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Locked, "Input.Locked", "Gameplay input locked");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Scripted, "Input.Scripted", "Camera-driven scripted hold without Sequencer");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cinematic_Active, "Cinematic.Active", "Cinematic coordinator holds control");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Test_MetCompanion, "Story.Test.MetCompanion", "Technical-lab: talked to companion");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Test_InspectedObject, "Story.Test.InspectedObject", "Technical-lab: inspected world object");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_Woke, "Story.Slice01.Woke", "Slice01: wake completed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_MetMaya, "Story.Slice01.MetMaya", "Slice01: first Maya contact");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_ChoseFollow, "Story.Slice01.ChoseFollow", "Slice01: chose Walk");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_ChoseQuestion, "Story.Slice01.ChoseQuestion", "Slice01: chose to question Maya");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_EnteredCut, "Story.Slice01.EnteredCut", "Slice01: entered Lantern Cut");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_SweepPassed, "Story.Slice01.SweepPassed", "Slice01: lantern-drone sweep passed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_ReachedBolt, "Story.Slice01.ReachedBolt", "Slice01: entered Pump House 12");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_QuietBeat, "Story.Slice01.QuietBeat", "Slice01: quiet mug beat done");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_FoundTin, "Story.Slice01.FoundTin", "Slice01: tin inspected");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_HeardWarning, "Story.Slice01.HeardWarning", "Slice01: warning playback finished");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Story_Slice01_Complete, "Story.Slice01.Complete", "Slice01: title reached");

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
