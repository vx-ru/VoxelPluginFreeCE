// Copyright 2021 Phyronnaz

#include "AssetTypeActions_VoxelGraphMacro.h"
#include "Misc/PackageName.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "VoxelNodes/VoxelGraphMacro.h"

UClass* FAssetTypeActions_VoxelGraphMacro::GetSupportedClass() const
{
	return UVoxelGraphMacro::StaticClass();
}

void FAssetTypeActions_VoxelGraphMacro::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	UE_LOG(LogTemp, Warning, TEXT("Voxel Graph Editor is disabled."));
}