// Copyright 2021 Phyronnaz
// Modifications Copyright 2024 vxru

#include "VoxelOpenAssetsOnStartup.h"
#include "VoxelMinimal.h"
#include "VoxelUtilities/VoxelConfigUtilities.h"
#include "VoxelUtilities/VoxelSystemUtilities.h"

#include "Editor.h"
#include "Engine/World.h"
#include "GameMapsSettings.h"
#include "ContentBrowserModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

void UVoxelOpenAssetsOnStartup::Init()
{
	FVoxelSystemUtilities::DelayedCall([]()
	{
		GetMutableDefault<UVoxelOpenAssetsOnStartup>()->ActualInit();
	});
}

void UVoxelOpenAssetsOnStartup::ActualInit()
{
	FVoxelConfigUtilities::LoadConfig(this, "OpenAssetsOnStartup");
	
	if (bEnableOpenAssetsOnStartup)
	{
		for (auto& It : AssetsToOpenOnStartup)
		{
			if (!It.Value)
			{
				continue;
			}

			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(It.Key.ToString());
		}
	}
	
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedAssets::CreateLambda([this](const TArray<FAssetData>& SelectedAssets)
	{
		const auto Extender = MakeShared<FExtender>();
		
		if (bEnableOpenAssetsOnStartup && SelectedAssets.Num() == 1)
		{
			const auto Asset = SelectedAssets[0];
			const FString Path = Asset.PackagePath.ToString() / Asset.AssetName.ToString();

			Extender->AddMenuExtension(
				"CommonAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateLambda([this, Path](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.AddMenuEntry(
					TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([this, Path]()
					{
						return AssetsToOpenOnStartup.FindRef(*Path) ? VOXEL_LOCTEXT("Stop opening on startup") : VOXEL_LOCTEXT("Open on startup");
					})),
					TAttribute<FText>(),
					FSlateIcon(NAME_None, NAME_None),
					FUIAction(FExecuteAction::CreateLambda([this, Path]()
					{
						bool& bValue = AssetsToOpenOnStartup.FindOrAdd(*Path);
						bValue = !bValue;
						FVoxelConfigUtilities::SaveConfig(this, "OpenAssetsOnStartup");
					})));
				}));
		}
		
		if (bShowSetAsStartupMap &&
			SelectedAssets.Num() == 1 && 
			SelectedAssets[0].GetClass() == UWorld::StaticClass() &&
			GetDefault<UGameMapsSettings>()->EditorStartupMap != SelectedAssets[0].ToSoftObjectPath())
		{
			const auto Asset = SelectedAssets[0];

			Extender->AddMenuExtension(
				"CommonAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateLambda([this, Asset](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.AddMenuEntry(
					VOXEL_LOCTEXT("Set as editor startup map"),
					TAttribute<FText>(),
					FSlateIcon(NAME_None, NAME_None),
					FUIAction(FExecuteAction::CreateLambda([Asset]()
					{
						auto* Settings = GetMutableDefault<UGameMapsSettings>();
						Settings->EditorStartupMap = Asset.ToSoftObjectPath();

						auto* Property = FindFProperty<FProperty>(UGameMapsSettings::StaticClass(), GET_MEMBER_NAME_CHECKED(UGameMapsSettings, EditorStartupMap));
						Settings->UpdateSinglePropertyInConfigFile(Property, Settings->GetDefaultConfigFilename());
					})));
				}));
		}

		return Extender;
	}));
}