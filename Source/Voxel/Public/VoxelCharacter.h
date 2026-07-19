// Copyright 2021 Phyronnaz

#pragma once

#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelCharacter.generated.h"

UCLASS(BlueprintType)
class VOXEL_API AVoxelCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Redirects runtime-built voxel world bases to the voxel world root component so that base replication behaves correctly.
	virtual void SetBase(FMovementBaseInterfaceData* BaseData, FName BoneName, bool bNotifyActor) override;
};