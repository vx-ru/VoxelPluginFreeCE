// Copyright 2021 Phyronnaz
// Modifications Copyright 2024 vxru

#define VOXEL_PLUGIN_PRO

using System.IO;
using UnrealBuildTool;

public class VoxelFoliage : ModuleRules
{
    public VoxelFoliage(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "RenderCore",
                "Voxel",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
            }
        );
    }
}
