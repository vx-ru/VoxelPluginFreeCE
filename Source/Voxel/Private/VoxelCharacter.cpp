// Copyright 2021 Phyronnaz

#include "VoxelCharacter.h"
#include "VoxelWorld.h"
#include "Components/PrimitiveComponent.h"

void AVoxelCharacter::SetBase(FMovementBaseInterfaceData* BaseData, const FName BoneName, const bool bNotifyActor)
{
	// Voxel world collision components are built at runtime and are not replicated,
	// so if the engine tries to base this character on one we substitute the voxel
	// world's root component, which is stable and safe to replicate against.
	FMovementBaseInterfaceData SubstituteData;
	if (BaseData && BaseData->IsValid())
	{
		if (const AVoxelWorld* const VoxelWorld = Cast<AVoxelWorld>(BaseData->GetMovementBaseObjectOwner()))
		{
			UPrimitiveComponent* const RootPrimitive = Cast<UPrimitiveComponent>(VoxelWorld->GetRootComponent());
			if (ensure(RootPrimitive))
			{
				SubstituteData = FMovementBaseInterfaceData(RootPrimitive);
				BaseData = &SubstituteData;
			}
		}
	}

	Super::SetBase(BaseData, BoneName, bNotifyActor);
}
