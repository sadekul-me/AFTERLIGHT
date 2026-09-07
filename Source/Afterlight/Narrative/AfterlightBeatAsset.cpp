#include "Narrative/AfterlightBeatAsset.h"

bool UAfterlightBeatAsset::AreRequirementsMet(const FGameplayTagContainer& Flags) const
{
	return Flags.HasAll(RequiredFlags);
}
