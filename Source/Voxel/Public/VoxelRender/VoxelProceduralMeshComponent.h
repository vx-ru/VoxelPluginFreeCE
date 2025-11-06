// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelMinimal.h"
#include "VoxelPriorityHandler.h"
#include "VoxelRender/VoxelProcMeshSectionSettings.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ModelComponent.h"
#include "VoxelProceduralMeshComponent.generated.h"

struct FKConvexElem;
struct FVoxelProcMeshBuffers;
struct FMaterialRelevance;
struct FVoxelSimpleCollisionData;

class FVoxelPool;
class FVoxelTexturePool;
class FVoxelRuntimeData;
class FVoxelTexturePoolEntry;
class FDistanceFieldVolumeData;
class IVoxelAsyncPhysicsCooker;
class FVoxelToolRenderingManager;
class FVoxelSimpleCollisionHandle;
class UBodySetup;
class UMaterialInterface;
class UVoxelWorldRootComponent;
class UVoxelProceduralMeshComponent;
class AVoxelWorld;
class IVoxelRenderer;
class IVoxelProceduralMeshComponent_PhysicsCallbackHandler;

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Physics Triangle Meshes Memory"), STAT_VoxelPhysicsTriangleMeshesMemory, STATGROUP_VoxelMemory, VOXEL_API);

struct FVoxelProceduralMeshComponentMemoryUsage
{
	uint32 TriangleMeshes = 0;
};

enum class EVoxelProcMeshSectionUpdate : uint8
{
	UpdateNow,
	DelayUpdate
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFreezeVoxelCollisionChanged, bool);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Voxel), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelProceduralMeshComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
	
public:
	void Init(
		int32 InDebugLOD,
		uint32 InDebugChunkId,
		const FVoxelPriorityHandler& InPriorityHandler,
		const TVoxelWeakPtr<IVoxelProceduralMeshComponent_PhysicsCallbackHandler>& InPhysicsCallbackHandler,
		const IVoxelRenderer& Renderer);
	void ClearInit();
	
private:
	bool bInit = false;
	// Used for collisions
	TWeakObjectPtr<UVoxelWorldRootComponent> VoxelRootComponent;
	// Used to show LOD color in the mesh LOD visualization & for convex collision cooking
	int32 LOD = 0;
	// For debug
	uint32 DebugChunkId = 0;
	// Priority for physics cooking tasks
	FVoxelPriorityHandler PriorityHandler;
	// Will be triggered by the async cooker on an async thread, and then will trigger us on game thread
	TVoxelWeakPtr<IVoxelProceduralMeshComponent_PhysicsCallbackHandler> PhysicsCallbackHandler;
	// Weak ptr else the pool stays created until GC
	TVoxelWeakPtr<FVoxelPool> Pool;
	// Used to show tools overlays
	TVoxelWeakPtr<const FVoxelToolRenderingManager> ToolRenderingManager;
	// Used to render texture data in the greedy cubic mesher
	TVoxelWeakPtr<FVoxelTexturePool> TexturePool;
	// Track collision memory
	TVoxelWeakPtr<FVoxelRuntimeData> VoxelRuntimeData;
	// Collisions settings
	ECollisionTraceFlag CollisionTraceFlag = ECollisionTraceFlag::CTF_UseDefault;
	// If true, will use cubes given by the greedy mesher for simple collision
    bool bSimpleCubicCollision = false;
    // For convex collisions
	int32 NumConvexHullsPerAxis = 2;
	// Will clear the proc mesh buffers once navmesh + collisions have been built
	bool bClearProcMeshBuffersOnFinishUpdate = false;
	// Distance field bias
	float DistanceFieldSelfShadowBias = 0.f;
	// If true will create a static mesh component when baking lighting
	bool bContributesToStaticLighting = false;
	// If true, will try to use the static path when possible. Much cheaper on the render thread
	bool bUseStaticPath = false;
	
public:
	UVoxelProceduralMeshComponent();
	~UVoxelProceduralMeshComponent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Voxel")
	void InitChunk(uint8 ChunkLOD, FVoxelIntBox ChunkBounds);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel|Collisions")
	static bool AreVoxelCollisionsFrozen(const AVoxelWorld* VoxelWorld);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions")
	static void SetVoxelCollisionsFrozen(const AVoxelWorld* VoxelWorld, bool bFrozen);

	static void AddOnFreezeVoxelCollisionChanged(const AVoxelWorld* VoxelWorld, const FOnFreezeVoxelCollisionChanged::FDelegate& NewDelegate);
	
private:
	struct FFreezeCollisionData
	{
		struct FWorldData
		{
			bool bFrozen = false;
			TSet<TWeakObjectPtr<UVoxelProceduralMeshComponent>> PendingCollisions;
			FOnFreezeVoxelCollisionChanged OnFreezeVoxelCollisionChanged;
		};
		TMap<TWeakObjectPtr<const AActor>, FWorldData> WorldData;
	};
	static FFreezeCollisionData FreezeCollisionData;

public:
	void SetDistanceFieldData(const TVoxelSharedPtr<const FDistanceFieldVolumeData>& InDistanceFieldData);
	void SetProcMeshSection(int32 Index, FVoxelProcMeshSectionSettings Settings, TUniquePtr<FVoxelProcMeshBuffers> Buffers, EVoxelProcMeshSectionUpdate Update);
	int32 AddProcMeshSection(FVoxelProcMeshSectionSettings Settings, TUniquePtr<FVoxelProcMeshBuffers> Buffers, EVoxelProcMeshSectionUpdate Update);
	void ReplaceProcMeshSection(FVoxelProcMeshSectionSettings Settings, TUniquePtr<FVoxelProcMeshBuffers> Buffers, EVoxelProcMeshSectionUpdate Update);
	void ClearSections(EVoxelProcMeshSectionUpdate Update);
	void FinishSectionsUpdates();

	void UpdateStaticMeshComponent();

	template<typename F>
	inline void IterateSectionsSettings(F Lambda)
	{
		for (auto& Section : ProcMeshSections)
		{
			Lambda(Section.Settings);
		}
	}
	template<typename F>
	inline void IterateSections(F Lambda) const
	{
		for (auto& Section : ProcMeshSections)
		{
			Lambda(Section.Settings, static_cast<const FVoxelProcMeshBuffers&>(*Section.Buffers));
		}
	}
	
public:
	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override final;
	virtual UBodySetup* GetBodySetup() override final;
	virtual UMaterialInterface* GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const override final;
	virtual int32 GetNumMaterials() const override final;
	virtual UMaterialInterface* GetMaterial(int32 ElementIndex) const override final;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const override;
	virtual bool DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const override final;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override final;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	//~ End UPrimitiveComponent Interface.

	FMaterialRelevance GetMaterialRelevance(ERHIFeatureLevel::Type InFeatureLevel) const;	
	
private:
	void UpdatePhysicalMaterials();
	void UpdateLocalBounds();
	void UpdateNavigation();
	void UpdateCollision();
	void FinishCollisionUpdate();
	
private:
	void PhysicsCookerCallback(uint64 CookerId);

	friend class IVoxelAsyncPhysicsCooker;
	friend class FVoxelAsyncPhysicsCooker_PhysX;
	friend class FVoxelAsyncPhysicsCooker_Chaos;
	friend class IVoxelProceduralMeshComponent_PhysicsCallbackHandler;

private:
	void UpdateCollisionStats();
	
	uint64 CollisionMemory = 0;
	int32 NumCollisionCubes = 0;
	
private:
	UPROPERTY(Transient)
	UBodySetup* BodySetup = nullptr;
	UPROPERTY(Transient)
	UBodySetup* BodySetupBeingCooked = nullptr;
	
	UPROPERTY(Transient)
	UStaticMeshComponent* StaticMeshComponent = nullptr;
	bool bNeedToRebuildStaticMesh = false;
	
	IVoxelAsyncPhysicsCooker* AsyncCooker = nullptr;
	TVoxelSharedPtr<FVoxelSimpleCollisionHandle> SimpleCollisionHandle;
	FVoxelProceduralMeshComponentMemoryUsage MemoryUsage;
	
	struct FVoxelProcMeshSection
	{
		FVoxelProcMeshSectionSettings Settings;
		TVoxelSharedPtr<FVoxelTexturePoolEntry> TexturePoolEntry;
		TVoxelSharedPtr<const FVoxelProcMeshBuffers> Buffers;
	};
	TArray<FVoxelProcMeshSection> ProcMeshSections;
	TVoxelSharedPtr<const FDistanceFieldVolumeData> DistanceFieldData;

	// Used to skip rebuilding collisions & navmesh
	// GUID to detect geometry change
	// Map to detect settings changes
	TArray<FGuid> ProcMeshSectionsSortedGuids;
	TMap<FGuid, FVoxelProcMeshSectionSettings> ProcMeshSectionsGuidToSettings;
	
	FBoxSphereBounds LocalBounds;

	double LastFinishSectionsUpdatesTime = 0;

	friend class FVoxelProceduralMeshSceneProxy;
};
