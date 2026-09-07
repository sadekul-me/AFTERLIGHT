#include "Narrative/AfterlightDialogueRunner.h"
#include "Narrative/AfterlightDialogueAsset.h"
#include "Core/AfterlightLog.h"

void UAfterlightDialogueRunner::Start(UAfterlightDialogueAsset* Asset)
{
	if (!Asset)
	{
		return;
	}
	ActiveAsset = Asset;
	RuntimeNodes.Reset();
	StartGraph(Asset->Nodes, Asset->EntryNodeId);
}

void UAfterlightDialogueRunner::StartGraph(const TArray<FAfterlightDialogueNode>& Nodes, FName EntryNodeId)
{
	RuntimeNodes = Nodes;
	bActive = true;
	if (const FAfterlightDialogueNode* Node = FindRuntimeNode(EntryNodeId))
	{
		PresentNode(*Node);
	}
	else
	{
		UE_LOG(LogAfterlight, Warning, TEXT("Dialogue entry node %s not found"), *EntryNodeId.ToString());
		Abort();
	}
}

const FAfterlightDialogueNode* UAfterlightDialogueRunner::FindRuntimeNode(FName NodeId) const
{
	return RuntimeNodes.FindByPredicate([&NodeId](const FAfterlightDialogueNode& Node)
	{
		return Node.NodeId == NodeId;
	});
}

void UAfterlightDialogueRunner::PresentNode(const FAfterlightDialogueNode& Node)
{
	CurrentNode = &Node;
	OnLinePresented.Broadcast(Node.SpeakerId, Node.Line);
	OnChoicesPresented.Broadcast(Node.Choices);
	if (Node.Choices.Num() == 0)
	{
		bActive = false;
		CurrentNode = nullptr;
		OnFinished.Broadcast();
	}
}

bool UAfterlightDialogueRunner::SelectChoice(int32 ChoiceIndex)
{
	if (!bActive || !CurrentNode || !CurrentNode->Choices.IsValidIndex(ChoiceIndex))
	{
		return false;
	}

	const FAfterlightDialogueChoice Choice = CurrentNode->Choices[ChoiceIndex];
	OnChoiceMade.Broadcast(Choice);
	if (Choice.NextNodeId.IsNone())
	{
		bActive = false;
		CurrentNode = nullptr;
		OnFinished.Broadcast();
		return true;
	}

	if (const FAfterlightDialogueNode* Next = FindRuntimeNode(Choice.NextNodeId))
	{
		PresentNode(*Next);
		return true;
	}

	bActive = false;
	CurrentNode = nullptr;
	OnFinished.Broadcast();
	return true;
}

void UAfterlightDialogueRunner::Abort()
{
	if (!bActive)
	{
		return;
	}
	bActive = false;
	CurrentNode = nullptr;
	OnFinished.Broadcast();
}
