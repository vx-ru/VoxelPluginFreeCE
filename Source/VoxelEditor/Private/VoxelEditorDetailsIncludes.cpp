// Copyright 2021 Phyronnaz
// Modifications Copyright 2026 vxru

#include "VoxelEditorDetailsIncludes.h"

FSimpleDelegate FVoxelEditorUtilities::MakeRefreshDelegate(const IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	return FSimpleDelegate::CreateLambda([Utilities = MakeWeakPtr(CustomizationUtils.GetPropertyUtilities())]()
	{
		auto Pinned = Utilities.Pin();
		if (Pinned.IsValid())
		{
			Pinned->ForceRefresh();
		}
	});
}