// Copyright 2021 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelEditor : ModuleRules
{
    public VoxelEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Source/Editor/PropertyEditor/Private"));

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "AssetRegistry",
            });

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Voxel",
                "VoxelFoliage",
                "VoxelEditorDefault",
                "Landscape",
                "LandscapeEditor",
                "PlacementMode",
                "AdvancedPreviewScene",
                "DesktopPlatform",
                "UnrealEd",
                "InputCore",
                "ImageWrapper",
                "Slate",
                "SlateCore",
                "PropertyEditor",
                "EditorStyle",
                "Projects",
                "RHI",
                "MessageLog",
                "RawMesh",
                "DetailCustomizations",
                "WorkspaceMenuStructure",
                "BlueprintGraph",
                "KismetCompiler",
                "ApplicationCore",
                "EngineSettings",
                "ToolMenus",
#if UE_5_0_OR_LATER
                "EditorFramework",
#endif
#if UE_4_26_OR_LATER
                "DeveloperSettings",
#endif
            });

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
            });
    }
}
