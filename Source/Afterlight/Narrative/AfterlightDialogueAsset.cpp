#include "Narrative/AfterlightDialogueAsset.h"

const FAfterlightDialogueNode* UAfterlightDialogueAsset::FindNode(FName NodeId) const
{
	return Nodes.FindByPredicate([&NodeId](const FAfterlightDialogueNode& Node)
	{
		return Node.NodeId == NodeId;
	});
}
