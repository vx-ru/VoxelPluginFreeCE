// Copyright 2021 Phyronnaz
// Modifications Copyright 2024-2025 vxru

#pragma once

#include "CoreMinimal.h"
#include "VoxelAsyncPhysicsCooker.h"

namespace Chaos
{
	class FTriangleMeshImplicitObject;
}

class IPhysXCooking;

class FVoxelAsyncPhysicsCooker_Chaos : public IVoxelAsyncPhysicsCooker
{
	GENERATED_VOXEL_ASYNC_WORK_BODY(FVoxelAsyncPhysicsCooker_Chaos)

public:
	explicit FVoxelAsyncPhysicsCooker_Chaos(UVoxelProceduralMeshComponent* Component);

protected:
	//~ Begin IVoxelAsyncPhysicsCooker Interface
	virtual bool Finalize(
		UBodySetup& BodySetup,
		TVoxelSharedPtr<FVoxelSimpleCollisionData>& OutSimpleCollisionData,
		FVoxelProceduralMeshComponentMemoryUsage& OutMemoryUsage) override;
	virtual void CookMesh() override;
	//~ End IVoxelAsyncPhysicsCooker Interface
	
private:
	void CreateTriMesh();
	void CreateSimpleCollision();

	TArray<Chaos::FTriangleMeshImplicitObjectPtr> TriMeshGeometries;
	TVoxelSharedPtr<FVoxelSimpleCollisionData> SimpleCollisionData;
};
