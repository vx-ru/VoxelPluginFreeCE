// Copyright 2021 Phyronnaz

#include "AssetTypeActions_VoxelGraphGenerator.h"
#include "Misc/PackageName.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "VoxelGraphGenerator.h"

UClass* FAssetTypeActions_VoxelGraphGenerator::GetSupportedClass() const
{
	return UVoxelGraphGenerator::StaticClass();
}

void FAssetTypeActions_VoxelGraphGenerator::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	UE_LOG(LogTemp, Warning, TEXT("Voxel Graph Editor is disabled."));
}