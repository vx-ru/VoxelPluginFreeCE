// Copyright 2021 Phyronnaz
// Modifications Copyright 2024 vxru

using System.IO;
using UnrealBuildTool;

public class VoxelEditorDefault : ModuleRules
{
    public VoxelEditorDefault(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
            });

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "Voxel",
                "UnrealEd"
            });

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
            });
    }
}
