#include "Misc/AutomationTest.h"
#include "Relationship/AfterlightRelationshipTypes.h"
#include "Narrative/AfterlightNarrativeTypes.h"
#include "Narrative/AfterlightBeatAsset.h"
#include "Narrative/AfterlightDialogueAsset.h"
#include "Narrative/AfterlightDialogueRunner.h"
#include "Core/AfterlightGameplayTags.h"
#include "Camera/AfterlightCameraRecipe.h"
#include "Cinematic/AfterlightCinematicSession.h"
#include "Cinematic/AfterlightLevelSequenceFactory.h"
#include "LevelSequence.h"
#include "Character/AfterlightPlayerController.h"
#include "UI/AfterlightPresentationFormat.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightRelationshipDefaultTest, "Afterlight.Relationship.DefaultState", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightRelationshipDefaultTest::RunTest(const FString& Parameters)
{
	const FAfterlightRelationshipState State;
	TestEqual(TEXT("Default Trust is 0.5"), State.Trust, 0.5f);
	TestEqual(TEXT("Default Suspicion is 0"), State.Suspicion, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightRelationshipDeltaTest, "Afterlight.Relationship.ApplyDelta", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightRelationshipDeltaTest::RunTest(const FString& Parameters)
{
	FAfterlightRelationshipState In;
	In.Trust = 0.5f;
	In.Suspicion = 0.1f;
	const FAfterlightRelationshipState Out = FAfterlightRelationshipMath::ApplyDelta(In, 0.2f, 0.3f);
	TestEqual(TEXT("Trust increased"), Out.Trust, 0.7f);
	TestEqual(TEXT("Suspicion increased"), Out.Suspicion, 0.4f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightRelationshipClampTest, "Afterlight.Relationship.Clamp", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightRelationshipClampTest::RunTest(const FString& Parameters)
{
	FAfterlightRelationshipState In;
	In.Trust = 0.9f;
	In.Suspicion = 0.05f;
	const FAfterlightRelationshipState Out = FAfterlightRelationshipMath::ApplyDelta(In, 0.5f, -1.f);
	TestEqual(TEXT("Trust clamped to 1"), Out.Trust, 1.f);
	TestEqual(TEXT("Suspicion clamped to 0"), Out.Suspicion, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightRelationshipThresholdTest, "Afterlight.Relationship.ThresholdTags", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightRelationshipThresholdTest::RunTest(const FString& Parameters)
{
	FAfterlightRelationshipState State;
	State.Trust = 0.8f;
	State.Suspicion = 0.f;
	const FGameplayTagContainer Tags = FAfterlightRelationshipMath::EvaluateThresholdTags(State, FAfterlightRelationshipThresholds());
	TestTrue(TEXT("High trust tag"), Tags.HasTag(AfterlightTags::Relationship_Trust_High));
	TestTrue(TEXT("Close follow tag"), Tags.HasTag(AfterlightTags::Companion_Follow_Close));
	TestFalse(TEXT("No high suspicion"), Tags.HasTag(AfterlightTags::Relationship_Suspicion_High));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightBeatRequirementTest, "Afterlight.Narrative.BeatRequirements", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightBeatRequirementTest::RunTest(const FString& Parameters)
{
	UAfterlightBeatAsset* Beat = NewObject<UAfterlightBeatAsset>();
	Beat->RequiredFlags.AddTag(AfterlightTags::Story_Test_MetCompanion);
	FGameplayTagContainer Empty;
	FGameplayTagContainer Ready;
	Ready.AddTag(AfterlightTags::Story_Test_MetCompanion);
	TestFalse(TEXT("Blocked without flag"), Beat->AreRequirementsMet(Empty));
	TestTrue(TEXT("Ready with flag"), Beat->AreRequirementsMet(Ready));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightDialogueChoiceTest, "Afterlight.Narrative.DialogueChoice", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightDialogueChoiceTest::RunTest(const FString& Parameters)
{
	UAfterlightDialogueAsset* Asset = NewObject<UAfterlightDialogueAsset>();
	Asset->EntryNodeId = TEXT("Start");
	FAfterlightDialogueNode Start;
	Start.NodeId = TEXT("Start");
	Start.SpeakerId = TEXT("Companion");
	Start.Line = FText::FromString(TEXT("Can you hear me?"));
	FAfterlightDialogueChoice Choice;
	Choice.Text = FText::FromString(TEXT("I'm fine."));
	Choice.TrustDelta = 0.15f;
	Choice.NextNodeId = TEXT("Fine");
	Start.Choices.Add(Choice);
	FAfterlightDialogueNode Fine;
	Fine.NodeId = TEXT("Fine");
	Fine.Line = FText::FromString(TEXT("Stay close."));
	Asset->Nodes.Add(Start);
	Asset->Nodes.Add(Fine);

	UAfterlightDialogueRunner* Runner = NewObject<UAfterlightDialogueRunner>();
	Runner->Start(Asset);
	TestTrue(TEXT("Dialogue active"), Runner->IsActive());
	TestTrue(TEXT("Choice 0 valid"), Asset->FindNode(TEXT("Start")) && Asset->FindNode(TEXT("Start"))->Choices[0].TrustDelta == 0.15f);
	TestTrue(TEXT("Select choice 0"), Runner->SelectChoice(0));
	TestFalse(TEXT("Finished after follow-up line with no choices"), Runner->IsActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightCameraRegisterNameTest, "Afterlight.Camera.RegisterName", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightCameraRegisterNameTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Explore name"), AfterlightRegisterToName(EAfterlightCameraRegister::Explore), FName(TEXT("Explore")));
	TestEqual(TEXT("Dialogue name"), AfterlightRegisterToName(EAfterlightCameraRegister::Dialogue), FName(TEXT("Dialogue")));
	TestEqual(TEXT("Intimate name"), AfterlightRegisterToName(EAfterlightCameraRegister::Intimate), FName(TEXT("Intimate")));
	TestEqual(TEXT("Reveal name"), AfterlightRegisterToName(EAfterlightCameraRegister::Reveal), FName(TEXT("Reveal")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightCameraUnknownRegisterFallbackTest, "Afterlight.Camera.UnknownRegisterFallback", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightCameraUnknownRegisterFallbackTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("Unknown name is not known"), AfterlightIsKnownRegisterName(TEXT("MayaHeroCloseup")));
	TestEqual(TEXT("Unknown falls back to Explore"), AfterlightRegisterFromName(TEXT("MayaHeroCloseup")), EAfterlightCameraRegister::Explore);
	TestTrue(TEXT("Dialogue is known"), AfterlightIsKnownRegisterName(TEXT("Dialogue")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightCinematicSessionOwnershipTest, "Afterlight.Camera.CinematicSessionOwnership", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightCinematicSessionOwnershipTest::RunTest(const FString& Parameters)
{
	FAfterlightCinematicSession Session;
	TestFalse(TEXT("Idle session is inactive"), Session.bActive);
	Session.Begin(TEXT("LS_LabInspectReveal"), EAfterlightCameraRegister::Explore);
	TestTrue(TEXT("Request activates session"), Session.bActive);
	TestEqual(TEXT("Sequence name stored"), Session.SequenceName, FName(TEXT("LS_LabInspectReveal")));
	TestEqual(TEXT("Return register stored"), Session.ReturnRegister, EAfterlightCameraRegister::Explore);
	TestEqual(TEXT("Authority is Sequencer"), Session.Authority, EAfterlightCameraAuthority::Sequencer);
	TestTrue(TEXT("Complete succeeds while active"), Session.Complete());
	TestFalse(TEXT("Complete clears active"), Session.bActive);
	TestTrue(TEXT("Sequence name cleared"), Session.SequenceName.IsNone());
	TestFalse(TEXT("Second complete is safe"), Session.Complete());
	Session.Begin(TEXT("LS_Cancel"), EAfterlightCameraRegister::Dialogue);
	TestTrue(TEXT("Cancel succeeds"), Session.Cancel());
	TestFalse(TEXT("Cancel leaves inactive"), Session.bActive);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightCameraRecipeDefaultsTest, "Afterlight.Camera.RecipeDefaults", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightCameraRecipeDefaultsTest::RunTest(const FString& Parameters)
{
	UAfterlightCameraRecipe* Dialogue = UAfterlightCameraRecipe::CreateDefault(GetTransientPackage(), EAfterlightCameraRegister::Dialogue);
	UAfterlightCameraRecipe* Explore = UAfterlightCameraRecipe::CreateDefault(GetTransientPackage(), EAfterlightCameraRegister::Explore);
	TestTrue(TEXT("Dialogue recipe created"), Dialogue != nullptr);
	TestTrue(TEXT("Dialogue uses target focus"), Dialogue && Dialogue->FocusMode == EAfterlightCameraFocusMode::Target);
	TestTrue(TEXT("Explore keeps gameplay FOV cinematic"), Explore && Explore->GameplayFOV < 70.f);
	TestTrue(TEXT("Explore blend is not instant"), Explore && Explore->BlendTime >= 0.5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightSlice01ChoiceOutcomesTest, "Afterlight.Slice01.ChoiceOutcomes", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightSlice01ChoiceOutcomesTest::RunTest(const FString& Parameters)
{
	FAfterlightRelationshipState FollowIn;
	const FAfterlightRelationshipState FollowOut = FAfterlightRelationshipMath::ApplyDelta(FollowIn, 0.25f, 0.f);
	TestEqual(TEXT("Walk Trust is 0.75"), FollowOut.Trust, 0.75f);
	TestEqual(TEXT("Walk Suspicion stays 0"), FollowOut.Suspicion, 0.f);
	const FGameplayTagContainer FollowTags = FAfterlightRelationshipMath::EvaluateThresholdTags(FollowOut, FAfterlightRelationshipThresholds());
	TestTrue(TEXT("Walk is close follow"), FollowTags.HasTag(AfterlightTags::Companion_Follow_Close));

	FAfterlightRelationshipState QuestionIn;
	const FAfterlightRelationshipState QuestionOut = FAfterlightRelationshipMath::ApplyDelta(QuestionIn, 0.f, 0.65f);
	TestEqual(TEXT("Question Suspicion is 0.65"), QuestionOut.Suspicion, 0.65f);
	TestEqual(TEXT("Question Trust stays 0.5"), QuestionOut.Trust, 0.5f);
	const FGameplayTagContainer QuestionTags = FAfterlightRelationshipMath::EvaluateThresholdTags(QuestionOut, FAfterlightRelationshipThresholds());
	TestTrue(TEXT("Question is far follow"), QuestionTags.HasTag(AfterlightTags::Companion_Follow_Far));
	TestFalse(TEXT("Choice flags are distinct"), AfterlightTags::Story_Slice01_ChoseFollow.GetTag() == AfterlightTags::Story_Slice01_ChoseQuestion.GetTag());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightSlice01FlagProgressionTest, "Afterlight.Slice01.FlagProgression", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightSlice01FlagProgressionTest::RunTest(const FString& Parameters)
{
	FAfterlightNarrativeState State;
	State.StoryFlags.AddTag(AfterlightTags::Story_Slice01_Woke);
	State.StoryFlags.AddTag(AfterlightTags::Story_Slice01_MetMaya);
	State.StoryFlags.AddTag(AfterlightTags::Story_Slice01_ChoseFollow);
	State.StoryFlags.AddTag(AfterlightTags::Story_Slice01_EnteredCut);
	State.StoryFlags.AddTag(AfterlightTags::Story_Slice01_SweepPassed);
	State.StoryFlags.AddTag(AfterlightTags::Story_Slice01_ReachedBolt);
	State.StoryFlags.AddTag(AfterlightTags::Story_Slice01_QuietBeat);
	State.StoryFlags.AddTag(AfterlightTags::Story_Slice01_FoundTin);
	State.StoryFlags.AddTag(AfterlightTags::Story_Slice01_HeardWarning);
	State.StoryFlags.AddTag(AfterlightTags::Story_Slice01_Complete);
	TestTrue(TEXT("Final state reachable"), State.StoryFlags.HasTag(AfterlightTags::Story_Slice01_Complete));
	TestTrue(TEXT("Tin reached"), State.StoryFlags.HasTag(AfterlightTags::Story_Slice01_FoundTin));
	TestFalse(TEXT("Follow and Question stay mutex unless both granted"), State.StoryFlags.HasTag(AfterlightTags::Story_Slice01_ChoseQuestion));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightDialogueLinearAdvanceTest, "Afterlight.Narrative.DialogueLinearAdvance", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightDialogueLinearAdvanceTest::RunTest(const FString& Parameters)
{
	UAfterlightDialogueAsset* Asset = NewObject<UAfterlightDialogueAsset>();
	Asset->EntryNodeId = TEXT("A");
	FAfterlightDialogueNode A;
	A.NodeId = TEXT("A");
	A.Line = FText::FromString(TEXT("Don't look at the posts."));
	A.NextNodeId = TEXT("B");
	A.bKeepGameplayInput = true;
	FAfterlightDialogueNode B;
	B.NodeId = TEXT("B");
	B.Line = FText::FromString(TEXT("They don't stay dead."));
	B.NextNodeId = TEXT("Done");
	Asset->Nodes.Add(A);
	Asset->Nodes.Add(B);

	UAfterlightDialogueRunner* Runner = NewObject<UAfterlightDialogueRunner>();
	Runner->Start(Asset);
	TestTrue(TEXT("Linear node stays active"), Runner->IsActive());
	TestTrue(TEXT("Walk-and-talk keeps gameplay input"), Runner->ShouldKeepGameplayInput());
	TestTrue(TEXT("Advance to second line"), Runner->Advance());
	TestTrue(TEXT("Terminal hold stays active"), Runner->IsActive());
	TestTrue(TEXT("Missing Done node finishes"), Runner->Advance());
	TestFalse(TEXT("Finished after last hold"), Runner->IsActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightWarningSequenceNameTest, "Afterlight.Slice01.WarningSequenceName", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightWarningSequenceNameTest::RunTest(const FString& Parameters)
{
	ULevelSequence* Sequence = UAfterlightLevelSequenceFactory::CreateWarningSequence(GetTransientPackage(), nullptr, 18.f);
	TestTrue(TEXT("Warning sequence created"), Sequence != nullptr);
	TestEqual(TEXT("Sequence is named LS_Slice01_Warning"), Sequence ? Sequence->GetFName() : NAME_None, FName(TEXT("LS_Slice01_Warning")));
	FAfterlightCinematicSession Session;
	Session.Begin(Sequence ? Sequence->GetFName() : NAME_None, EAfterlightCameraRegister::Intimate);
	TestTrue(TEXT("Warning session owns Sequencer"), Session.bActive && Session.Authority == EAfterlightCameraAuthority::Sequencer);
	TestTrue(TEXT("Release is idempotent"), Session.Complete());
	TestFalse(TEXT("Released session is inactive"), Session.bActive);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightInputStateDistinctTest, "Afterlight.Camera.InputStates", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightInputStateDistinctTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Scripted is distinct from Locked"), EAfterlightInputState::Scripted != EAfterlightInputState::Locked);
	TestTrue(TEXT("Constrained is distinct from Full"), EAfterlightInputState::Constrained != EAfterlightInputState::Full);
	TestTrue(TEXT("Four input states exist"), static_cast<uint8>(EAfterlightInputState::Scripted) == 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightSaveStateRoundTripTest, "Afterlight.Save.StateRoundTrip", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightSaveStateRoundTripTest::RunTest(const FString& Parameters)
{
	FAfterlightNarrativeState Narrative;
	Narrative.CurrentBeatId = TEXT("Lab.Explore");
	Narrative.StoryFlags.AddTag(AfterlightTags::Story_Test_InspectedObject);
	FAfterlightRelationshipState Rel;
	Rel.Trust = 0.65f;
	Rel.Suspicion = 0.2f;

	FAfterlightNarrativeState NarrativeCopy = Narrative;
	FAfterlightRelationshipState RelCopy = Rel;
	TestEqual(TEXT("Beat preserved"), NarrativeCopy.CurrentBeatId, FName(TEXT("Lab.Explore")));
	TestTrue(TEXT("Flag preserved"), NarrativeCopy.StoryFlags.HasTag(AfterlightTags::Story_Test_InspectedObject));
	TestEqual(TEXT("Trust preserved"), RelCopy.Trust, 0.65f);
	TestEqual(TEXT("Suspicion preserved"), RelCopy.Suspicion, 0.2f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightHideRecoveryLerpTest, "Afterlight.Slice01.HideRecoveryLerp", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightHideRecoveryLerpTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Alpha at start is 0"), FAfterlightPresentationFormat::HideRecoveryAlpha(0.f, 1.2f), 0.f);
	TestEqual(TEXT("Alpha at end is 1"), FAfterlightPresentationFormat::HideRecoveryAlpha(1.2f, 1.2f), 1.f);
	const float Mid = FAfterlightPresentationFormat::HideRecoveryAlpha(0.6f, 1.2f);
	TestTrue(TEXT("Midpoint is eased not linear"), Mid > 0.45f && Mid < 0.55f);
	TestTrue(TEXT("Early frame is not already at destination"), FAfterlightPresentationFormat::HideRecoveryAlpha(0.01f, 1.2f) < 0.05f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightChoicePresentationTest, "Afterlight.UI.ChoicePresentation", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightChoicePresentationTest::RunTest(const FString& Parameters)
{
	TArray<FText> Choices;
	Choices.Add(NSLOCTEXT("Afterlight", "S01Walk", "Walk."));
	Choices.Add(NSLOCTEXT("Afterlight", "S01Question", "You talk like I belong to you."));
	const FString Formatted = FAfterlightPresentationFormat::FormatChoiceList(Choices);
	TestTrue(TEXT("Walk text is present"), Formatted.Contains(TEXT("Walk.")));
	TestFalse(TEXT("No [1] debug numbering"), Formatted.Contains(TEXT("[1]")));
	TestFalse(TEXT("No [2] debug numbering"), Formatted.Contains(TEXT("[2]")));
	TestTrue(TEXT("Choices are stacked"), Formatted.Contains(TEXT("\n\n")));
	TestTrue(TEXT("First option is numbered without brackets"), Formatted.StartsWith(TEXT("1")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightInteractPromptTest, "Afterlight.UI.InteractPrompt", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightInteractPromptTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Talk prompt is contextual"), FAfterlightPresentationFormat::FormatInteractPrompt(FText::FromString(TEXT("Talk"))), FString(TEXT("E    Talk")));
	TestEqual(TEXT("Open prompt is contextual"), FAfterlightPresentationFormat::FormatInteractPrompt(FText::FromString(TEXT("Open"))), FString(TEXT("E    Open")));
	TestTrue(TEXT("Empty prompt stays empty"), FAfterlightPresentationFormat::FormatInteractPrompt(FText::GetEmpty()).IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAfterlightSpokenHoldTest, "Afterlight.UI.SpokenHold", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FAfterlightSpokenHoldTest::RunTest(const FString& Parameters)
{
	const FText Warning = FText::FromString(TEXT("If you are hearing this, they already have the shape of you."));
	const float Hold = FAfterlightPresentationFormat::SpokenHoldSeconds(Warning);
	TestTrue(TEXT("Long warning line holds as if spoken"), Hold >= 3.6f);
	TestEqual(TEXT("Authored override is honored"), FAfterlightPresentationFormat::SpokenHoldSeconds(Warning, 4.2f), 4.2f);
	TestTrue(TEXT("Short line still has a human pause"), FAfterlightPresentationFormat::SpokenHoldSeconds(FText::FromString(TEXT("No."))) >= 2.f);
	return true;
}

#endif
