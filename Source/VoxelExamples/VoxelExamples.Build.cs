// Copyright 2021 Phyronnaz
// Modifications Copyright 2024 vxru

using System.IO;
using UnrealBuildTool;

public class VoxelExamples : ModuleRules
{
    public VoxelExamples(ReadOnlyTargetRules Target) : base(Target)
{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Voxel",
                "Core",
                "CoreUObject",
                "Engine"
            }
        );
    }
}