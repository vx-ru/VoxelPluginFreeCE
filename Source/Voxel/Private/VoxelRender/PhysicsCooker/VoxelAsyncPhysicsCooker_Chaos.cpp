// Copyright 2021 Phyronnaz
// Modifications Copyright 2024-2025 vxru

#include "VoxelRender/PhysicsCooker/VoxelAsyncPhysicsCooker_Chaos.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelWorldRootComponent.h"

#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"

#include "Chaos/ImplicitObject.h"
#include "Chaos/CollisionConvexMesh.h"
#include "Chaos/TriangleMeshImplicitObject.h"
#include "Chaos/Convex.h"
#include "Chaos/AABB.h"

FVoxelAsyncPhysicsCooker_Chaos::FVoxelAsyncPhysicsCooker_Chaos(UVoxelProceduralMeshComponent* Component)
	: IVoxelAsyncPhysicsCooker(Component)
{
}

namespace
{
	// Helper function to build a Chaos convex from vertex data
	// Based on Chaos::Cooking::BuildConvexMeshes implementation
	Chaos::FConvexPtr BuildChaosConvexFromVertices(const TArray<FVector>& HullVerts)
	{
		const int32 NumHullVerts = HullVerts.Num();
		if (NumHullVerts == 0)
		{
			return nullptr;
		}

		// Calculate bounds for the convex (used for debugging/validation)
		Chaos::FAABB3 Bounds = Chaos::FAABB3::EmptyAABB();
		for (int32 VertIndex = 0; VertIndex < NumHullVerts; ++VertIndex)
		{
			const FVector& HullVert = HullVerts[VertIndex];
			Bounds.GrowToInclude(HullVert);
		}

		// Create the corner vertices for the convex
		TArray<Chaos::FConvex::FVec3Type> ConvexVertices;
		ConvexVertices.SetNumZeroed(NumHullVerts);

		for (int32 VertIndex = 0; VertIndex < NumHullVerts; ++VertIndex)
		{
			const FVector& HullVert = HullVerts[VertIndex];
			ConvexVertices[VertIndex] = Chaos::FConvex::FVec3Type(HullVert.X, HullVert.Y, HullVert.Z);
		}

		// Margin is always zero on convex shapes - they are intended to be instanced
		// This creates the convex hull from the vertices
		return Chaos::FConvexPtr(new Chaos::FConvex(ConvexVertices, 0.0f));
	}
}

bool FVoxelAsyncPhysicsCooker_Chaos::Finalize(
	UBodySetup& BodySetup,
	TVoxelSharedPtr<FVoxelSimpleCollisionData>& OutSimpleCollisionData,
	FVoxelProceduralMeshComponentMemoryUsage& OutMemoryUsage)
{
	// Pass our generated simple collision data back to the caller
	OutSimpleCollisionData = SimpleCollisionData;

	// 1. Copy simple collision data to BodySetup's AggGeom
	// Boxes and convex hulls will be used for simple collision
	if (SimpleCollisionData && !SimpleCollisionData->IsEmpty())
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Process Simple Collision");

		// Copy box elements directly - these don't need pre-cooking
		// The physics interface will handle them at runtime
		BodySetup.AggGeom.BoxElems = SimpleCollisionData->BoxElems;

		// Copy convex elements and ensure they have Chaos geometry
		BodySetup.AggGeom.ConvexElems = SimpleCollisionData->ConvexElems;

		// For each ConvexElem, ensure it has Chaos geometry
		const FString DebugName = FString::Printf(TEXT("Voxel Convex (LOD %d)"), LOD);
		for (FKConvexElem& ConvexElem : BodySetup.AggGeom.ConvexElems)
		{
			// If the ConvexElem doesn't already have a Chaos mesh, create one from vertex data
			if (!ConvexElem.GetChaosConvexMesh() && ConvexElem.VertexData.Num() > 0)
			{
				// Build Chaos convex from the vertex data
				Chaos::FConvexPtr ChaosConvex = BuildChaosConvexFromVertices(ConvexElem.VertexData);

				if (ChaosConvex && ChaosConvex->IsValidGeometry())
				{
					// Assign the Chaos convex to the element
					// Use UpdateConvexDataOnlyIfMissing since we're building from existing VertexData
					ConvexElem.SetConvexMeshObject(
						MoveTemp(ChaosConvex),
						FKConvexElem::EConvexDataUpdateMethod::UpdateConvexDataOnlyIfMissing
					);

#if TRACK_CHAOS_GEOMETRY
					ConvexElem.GetChaosConvexMesh()->Track(
						Chaos::MakeSerializable(ConvexElem.GetChaosConvexMesh()),
						DebugName
					);
#endif
				}
				else
				{
					UE_LOG(LogVoxel, Warning, TEXT("Failed to create Chaos convex for voxel collision (LOD %d) - invalid geometry"), LOD);
				}
			}
		}
	}

	// 2. Handle trimesh geometry (complex collision)
#if TRACK_CHAOS_GEOMETRY
	for (auto& TriMesh : TriMeshGeometries)
	{
		TriMesh->Track(Chaos::MakeSerializable(TriMesh), "Voxel Mesh");
	}
#endif

	// Force trimesh collisions off
	// This is standard practice - prevents redundant collision checks
	for (auto& TriMesh : TriMeshGeometries)
	{
		TriMesh->SetDoCollide(false);
	}

	BodySetup.TriMeshGeometries = MoveTemp(TriMeshGeometries);
	TriMeshGeometries = {};
	BodySetup.bCreatedPhysicsMeshes = true;

	return true;
}

void FVoxelAsyncPhysicsCooker_Chaos::CookMesh()
{
	// Generate simple collision if requested
	if (CollisionTraceFlag == ECollisionTraceFlag::CTF_UseSimpleAsComplex ||
		CollisionTraceFlag == ECollisionTraceFlag::CTF_UseDefault)
	{
		CreateSimpleCollision();
	}

	// Generate complex (trimesh) collision if needed
	if (CollisionTraceFlag != ECollisionTraceFlag::CTF_UseSimpleAsComplex)
	{
		CreateTriMesh();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelAsyncPhysicsCooker_Chaos::CreateSimpleCollision()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	// Calculate bounds of the entire mesh
	FBox MeshBounds(ForceInit);
	int32 TotalVertices = 0;
	for (auto& Buffer : Buffers)
	{
		auto& PositionBuffer = Buffer->VertexBuffers.PositionVertexBuffer;
		const uint32 NumVerts = PositionBuffer.GetNumVertices();
		TotalVertices += NumVerts;

		for (uint32 Index = 0; Index < NumVerts; Index++)
		{
			MeshBounds += FVector(PositionBuffer.VertexPosition(Index));
		}
	}

	if (!MeshBounds.IsValid || TotalVertices == 0)
	{
		// No vertices, can't create collision
		UE_LOG(LogVoxel, Verbose, TEXT("Skipping collision for LOD %d - no vertices (TotalVertices=%d, BoundsValid=%d)"),
			LOD, TotalVertices, MeshBounds.IsValid);
		return;
	}

	// Create the simple collision data
	SimpleCollisionData = MakeVoxelShared<FVoxelSimpleCollisionData>();
	SimpleCollisionData->Bounds = MeshBounds;

	UE_LOG(LogVoxel, Verbose, TEXT("CreateSimpleCollision LOD %d: MeshBounds=%s Size=%s, LocalToRoot Scale=%s"),
		LOD, *MeshBounds.ToString(), *MeshBounds.GetSize().ToString(), *LocalToRoot.GetScale3D().ToString());

	// Check if mesh is too thin for convex hull generation (e.g., a flat plane)
	// Convex hulls don't work well for flat/thin meshes - use box collision instead
	const FVector MeshSize = MeshBounds.GetSize();
	const float MinThickness = 1.0f; // Minimum thickness in voxel units
	const bool bMeshTooThin = MeshSize.X < MinThickness || MeshSize.Y < MinThickness || MeshSize.Z < MinThickness;

	if (bSimpleCubicCollision || bMeshTooThin)
	{
		// Option 1: Single box collision (simplest and fastest)
		// This is the recommended option for voxel terrain as it provides good performance
		// Also used automatically for flat/thin meshes where convex hulls would create gaps
		FKBoxElem BoxElem;
		BoxElem.Center = MeshBounds.GetCenter();
		const FVector BoxSize = MeshBounds.GetSize();
		BoxElem.X = FMath::Max(BoxSize.X, MinThickness);
		BoxElem.Y = FMath::Max(BoxSize.Y, MinThickness);
		BoxElem.Z = FMath::Max(BoxSize.Z, MinThickness);

		// only in non-shipping builds
#if !UE_BUILD_SHIPPING
		if (bMeshTooThin)
		{
			UE_LOG(LogVoxel, Verbose, TEXT("Created simple box collision for LOD %d (mesh too thin for convex hulls): Center=%s, Size=%s, Vertices=%d"),
				LOD, *BoxElem.Center.ToString(), *BoxSize.ToString(), TotalVertices);
		}
		else
		{
			UE_LOG(LogVoxel, Verbose, TEXT("Created simple box collision for LOD %d: Center=%s, Size=%s, Vertices=%d"),
				LOD, *BoxElem.Center.ToString(), *BoxSize.ToString(), TotalVertices);
		}
#endif

		SimpleCollisionData->BoxElems.Add(BoxElem);
	}
	else
	{
		// Option 2: Convex hull collision using actual convex mesh generation
		VOXEL_ASYNC_SCOPE_COUNTER("Generate Convex Hulls");

		// Fast path: Use pre-computed CollisionCubes from Greedy Cubic Mesher if available
		// CollisionCubes are optimally merged boxes created during mesh generation
		bool bUsedCollisionCubes = false;
		for (auto& Buffer : Buffers)
		{
			if (Buffer->CollisionCubes.Num() > 0)
			{
				VOXEL_ASYNC_SCOPE_COUNTER("Generate Convex Hulls from CollisionCubes");

				// Build spatial hash of vertices for fast lookup
				TMap<FIntVector, TArray<FVector>> VertexHash;
				{
					auto& PositionBuffer = Buffer->VertexBuffers.PositionVertexBuffer;
					const uint32 NumVerts = PositionBuffer.GetNumVertices();

					// Hash vertices into 8-unit grid cells for fast spatial queries
					for (uint32 Index = 0; Index < NumVerts; Index++)
					{
						const FVector Vertex = FVector(PositionBuffer.VertexPosition(Index));
						const FIntVector Cell = FIntVector(Vertex / 8.0);
						VertexHash.FindOrAdd(Cell).Add(Vertex);
					}
				}

				// Use the pre-computed optimal collision boxes
				for (const FBox& Cube : Buffer->CollisionCubes)
				{
					TArray<FVector> HullVertices;

					// Query hash for cells that overlap this cube
					const FIntVector MinCell = FIntVector(Cube.Min / 8.0) - FIntVector(1);
					const FIntVector MaxCell = FIntVector(Cube.Max / 8.0) + FIntVector(1);

					for (int32 X = MinCell.X; X <= MaxCell.X; X++)
					{
						for (int32 Y = MinCell.Y; Y <= MaxCell.Y; Y++)
						{
							for (int32 Z = MinCell.Z; Z <= MaxCell.Z; Z++)
							{
								if (const TArray<FVector>* CellVerts = VertexHash.Find(FIntVector(X, Y, Z)))
								{
									for (const FVector& Vertex : *CellVerts)
									{
										if (Cube.IsInside(Vertex))
										{
											HullVertices.Add(Vertex);
										}
									}
								}
							}
						}
					}

					if (HullVertices.Num() >= 4)
					{
						// Calculate bounds of vertices we collected for debugging
						FBox VertexBounds(ForceInit);
						for (const FVector& V : HullVertices)
						{
							VertexBounds += V;
						}

						UE_LOG(LogVoxel, Verbose, TEXT("  CollisionCube: Min=%s Max=%s Size=%s | VertexBounds: Min=%s Max=%s Size=%s | NumVerts=%d"),
							*Cube.Min.ToString(), *Cube.Max.ToString(), *Cube.GetSize().ToString(),
							*VertexBounds.Min.ToString(), *VertexBounds.Max.ToString(), *VertexBounds.GetSize().ToString(),
							HullVertices.Num());

						FKConvexElem& ConvexElem = SimpleCollisionData->ConvexElems.Emplace_GetRef();
						ConvexElem.VertexData = MoveTemp(HullVertices);
						ConvexElem.UpdateElemBox();
					}
				}

				bUsedCollisionCubes = true;
				UE_LOG(LogVoxel, Verbose, TEXT("Generated %d convex hulls for LOD %d from %d CollisionCubes (optimized path)"),
					SimpleCollisionData->ConvexElems.Num(), LOD, Buffer->CollisionCubes.Num());
				break;
			}
		}

		// Standard path: Use spatial hash grid for general meshers
		if (!bUsedCollisionCubes)
		{
			VOXEL_ASYNC_SCOPE_COUNTER("Generate Convex Hulls via Spatial Hash");

			const int32 NumHullsPerAxis = FMath::Max(1, NumConvexHullsPerAxis);
			const FVector BoundsMin = MeshBounds.Min;
			const FVector CellSize = MeshBounds.GetSize() / NumHullsPerAxis;

			// Phase 1: Build spatial hash (O(N) - single pass over all vertices)
			TMap<FIntVector, TArray<FVector>> SpatialHash;
			{
				VOXEL_ASYNC_SCOPE_COUNTER("Build Spatial Hash");

				for (auto& Buffer : Buffers)
				{
					auto& PositionBuffer = Buffer->VertexBuffers.PositionVertexBuffer;
					const uint32 NumVerts = PositionBuffer.GetNumVertices();

					for (uint32 Index = 0; Index < NumVerts; Index++)
					{
						const FVector Vertex = FVector(PositionBuffer.VertexPosition(Index));

						// Compute cell coordinates via direct division (no bounds checking needed)
						FIntVector Cell(
							FMath::FloorToInt((Vertex.X - BoundsMin.X) / CellSize.X),
							FMath::FloorToInt((Vertex.Y - BoundsMin.Y) / CellSize.Y),
							FMath::FloorToInt((Vertex.Z - BoundsMin.Z) / CellSize.Z)
						);

						// Clamp to valid range
						Cell.X = FMath::Clamp(Cell.X, 0, NumHullsPerAxis - 1);
						Cell.Y = FMath::Clamp(Cell.Y, 0, NumHullsPerAxis - 1);
						Cell.Z = FMath::Clamp(Cell.Z, 0, NumHullsPerAxis - 1);

						SpatialHash.FindOrAdd(Cell).Add(Vertex);
					}
				}
			}

			// Phase 2: Create convex hulls from spatial buckets (O(NumCells))
			{
				VOXEL_ASYNC_SCOPE_COUNTER("Create Convex Elements");

				for (auto& Pair : SpatialHash)
				{
					TArray<FVector>& HullVertices = Pair.Value;

					if (HullVertices.Num() >= 4)
					{
						FKConvexElem& ConvexElem = SimpleCollisionData->ConvexElems.Emplace_GetRef();
						ConvexElem.VertexData = MoveTemp(HullVertices);
						ConvexElem.UpdateElemBox();
					}
				}
			}

			UE_LOG(LogVoxel, Verbose, TEXT("Generated %d convex hulls for LOD %d with %d total vertices (spatial hash optimization)"),
				SimpleCollisionData->ConvexElems.Num(), LOD, TotalVertices);
		}

		// If no convex hulls were created, fall back to a single box
		// This can happen for example for a flat surface
		if (SimpleCollisionData->ConvexElems.Num() == 0)
		{
			FKBoxElem BoxElem;
			BoxElem.Center = MeshBounds.GetCenter();
			const FVector BoxSize = MeshBounds.GetSize();
			BoxElem.X = BoxSize.X;
			BoxElem.Y = BoxSize.Y;
			BoxElem.Z = BoxSize.Z;
			SimpleCollisionData->BoxElems.Add(BoxElem);

			UE_LOG(LogVoxel, Verbose, TEXT("No convex hulls created for LOD %d, using fallback box"), LOD);
		}
	}
}

void FVoxelAsyncPhysicsCooker_Chaos::CreateTriMesh()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
				
	int32 NumIndices = 0;
	int32 NumVertices = 0;
	for (auto& Buffer : Buffers)
	{
		NumIndices += Buffer->GetNumIndices();
		NumVertices += Buffer->GetNumVertices();
	}

	const auto Process = [&](auto& Triangles)
	{
		Chaos::FTriangleMeshImplicitObject::ParticlesType Particles;

		{
			VOXEL_ASYNC_SCOPE_COUNTER("Copy data from buffers");

			{
				VOXEL_ASYNC_SCOPE_COUNTER("Allocate");
				ensure(NumIndices % 3 == 0);
				Triangles.SetNumUninitialized(NumIndices / 3);
				Particles.AddParticles(NumVertices);
			}

			int32 IndexIndex = 0;
			int32 VertexIndex = 0;
			for (int32 SectionIndex = 0; SectionIndex < Buffers.Num(); SectionIndex++)
			{
				auto& Buffer = *Buffers[SectionIndex];

				const int32 VertexOffset = VertexIndex;

				{
					VOXEL_ASYNC_SCOPE_COUNTER("Copy vertices");
					
					auto& PositionBuffer = Buffer.VertexBuffers.PositionVertexBuffer;
					for (uint32 Index = 0; Index < PositionBuffer.GetNumVertices(); Index++)
					{
						Particles.SetX(VertexIndex++, PositionBuffer.VertexPosition(Index));
					}
				}

				{
					VOXEL_ASYNC_SCOPE_COUNTER("Copy triangles");
					
					auto& IndexBuffer = Buffer.IndexBuffer;

					ensure(IndexBuffer.GetNumIndices() % 3 == 0);
					const int32 NumTriangles = IndexBuffer.GetNumIndices() / 3;

					const auto Lambda = [&](const auto* RESTRICT Data)
					{
						for (int32 Index = 0; Index < NumTriangles; Index++)
						{
							checkVoxelSlow(3 * Index + 2 < IndexBuffer.GetNumIndices());

							const Chaos::TVector<int32, 3> Triangle{
									int32(Data[3 * Index + 2]) + VertexOffset,
									int32(Data[3 * Index + 1]) + VertexOffset,
									int32(Data[3 * Index + 0]) + VertexOffset
							};

							FVoxelUtilities::Get(Triangles, IndexIndex++) = Triangle;

	#if VOXEL_DEBUG
							const auto A = Particles.X(Triangle.X);
							const auto B = Particles.X(Triangle.Y);
							const auto C = Particles.X(Triangle.Z);
							ensure(Chaos::FConvexBuilder::IsValidTriangle(A, B, C));
	#endif
						}
					};
					if (IndexBuffer.Is32Bit())
					{
						Lambda(IndexBuffer.GetData_32());
					}
					else
					{
						Lambda(IndexBuffer.GetData_16());
					}
				}
			}
			check(IndexIndex == Triangles.Num());
			check(VertexIndex == Particles.Size());
		}

		TArray<uint16> MaterialIndices;
		
		VOXEL_ASYNC_SCOPE_COUNTER("Build Tri Mesh");
		TriMeshGeometries.Emplace(new Chaos::FTriangleMeshImplicitObject(MoveTemp(Particles), MoveTemp(Triangles), MoveTemp(MaterialIndices)));
	};
	
	if (NumVertices < TNumericLimits<uint16>::Max())
	{
		TArray<Chaos::TVec3<uint16>> TrianglesSmallIdx;
		Process(TrianglesSmallIdx);
	}
	else
	{
		TArray<Chaos::TVec3<int32>> TrianglesLargeIdx;
		Process(TrianglesLargeIdx);
	}
}