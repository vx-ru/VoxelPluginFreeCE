// Copyright 2021 Phyronnaz

#include "VoxelData/VoxelSave.h"
#include "VoxelUtilities/VoxelSerializationUtilities.inl"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelMessages.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelUncompressedSavesMemory);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelCompressedSavesMemory);

void FVoxelUncompressedWorldSaveImpl::UpdateAllocatedSize() const
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, AllocatedSize);
	AllocatedSize =
		Chunks64.GetAllocatedSize() +
		ValueBuffers64.GetAllocatedSize() +
		MaterialBuffers64.GetAllocatedSize() +
		PlaceableItems64.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, AllocatedSize);
}

bool FVoxelUncompressedWorldSaveImpl::Serialize(FArchive& Ar)
{
	if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
	{
		if (Ar.IsSaving())
		{
			Version = FVoxelSaveVersion::LatestVersion;
		}

		// Serialize version & depth
		{
			int32 Dummy = 42;
			Ar << Dummy;
			if (Dummy == 42) // Trick to know the version, as Depth is always smaller than 42
			{
				Ar << Version;
				Ar << Depth;
			}
			else
			{
				Version = FVoxelSaveVersion::BeforeCustomVersionWasAdded;
				Depth = Dummy;
			}
		}
		
		const auto SerializationVersion =
			Version >= FVoxelSaveVersion::ValueConfigFlagAndSaveGUIDs
			? FVoxelSerializationVersion::ValueConfigFlagAndSaveGUIDs
			: Version >= FVoxelSaveVersion::RemoveEnableVoxelSpawnedActorsEnableVoxelGrass
			? FVoxelSerializationVersion::RemoveEnableVoxelSpawnedActorsEnableVoxelGrass
			: FVoxelSerializationVersion::BeforeCustomVersionWasAdded;

		static_assert(FVoxelSerializationVersion::LatestVersion == FVoxelSerializationVersion::SHARED_StoreMaterialChannelsIndividuallyAndRemoveFoliage, "Need to add a new FVoxelSaveVersion");

		// Serialize GUID
		if (Version >= FVoxelSaveVersion::ValueConfigFlagAndSaveGUIDs)
		{
			Ar << Guid;
		}
		else
		{
			Guid = FGuid::NewGuid();
		}

		// Serialize UserFlags
		if (Version >= FVoxelSaveVersion::AddUserFlagsToSaves)
		{
			Ar << UserFlags;
		}
		else
		{
			UserFlags = 0;
		}
		
		// Serialize value config
		uint32 ValueConfigFlag = GVoxelValueConfigFlag;
		if (Version >= FVoxelSaveVersion::ValueConfigFlagAndSaveGUIDs)
		{
			Ar << ValueConfigFlag;
		}

		// Serialize material config
		uint32 MaterialConfigFlag = GVoxelMaterialConfigFlag;
		Ar << MaterialConfigFlag;

		if (Ar.IsLoading() && Version < FVoxelSaveVersion::Use64BitIndices)
		{
			FVoxelMessages::Error(FString::Printf(
				TEXT("VoxelSave: this save is version %d. Versions before %d indexed their buffers with ")
				TEXT("32 bit integers and are no longer supported, it cannot be loaded"),
				Version, int32(FVoxelSaveVersion::Use64BitIndices)));

			*this = FVoxelUncompressedWorldSaveImpl();
			return false;
		}

		// Serialize value buffers
		FVoxelSerializationUtilities::SerializeValues(Ar, ValueBuffers64, ValueConfigFlag, SerializationVersion);
		FVoxelSerializationUtilities::SerializeValues(Ar, SingleValues64, ValueConfigFlag, SerializationVersion);

		// Serialize material buffers
		FVoxelSerializationUtilities::SerializeMaterials(Ar, MaterialsIndices64, MaterialConfigFlag);
		MaterialBuffers64.BulkSerialize(Ar);
		SingleMaterials64.BulkSerialize(Ar);

		// Serialize chunks indices
		// Note: make sure to not use BulkSerialize as data isn't aligned
		Ar << Chunks64;

		// Serialize placeable items
		Ar << PlaceableItems64;
		
		if (Ar.IsLoading() && Ar.IsError())
		{
			FVoxelMessages::Error("VoxelSave: Serialization failed, data is corrupted");
			*this = FVoxelUncompressedWorldSaveImpl();
		}
		
		UpdateAllocatedSize();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelCompressedWorldSaveImpl::~FVoxelCompressedWorldSaveImpl()
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, AllocatedSize);
}

bool FVoxelCompressedWorldSaveImpl::Serialize(FArchive& Ar)
{
	if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
	{
		if (Ar.IsSaving())
		{
			Version = FVoxelSaveVersion::LatestVersion;
		}

		Ar << Depth;
		Ar << Version;
		if (Version < FVoxelSaveVersion::ValueConfigFlagAndSaveGUIDs)
		{
			uint32 ConfigFlags;
			Ar << ConfigFlags;
			Guid = FGuid::NewGuid();
		}
		else
		{
			Ar << Guid;
		}
		Ar << CompressedData;

		UpdateAllocatedSize();
	}

	return true;
}

void FVoxelCompressedWorldSaveImpl::UpdateAllocatedSize() const
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, AllocatedSize);
	AllocatedSize = CompressedData.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, AllocatedSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelWorldSaveObject::PostLoad()
{
	Super::PostLoad();
	CopyDepthFromSave();
}

void UVoxelWorldSaveObject::CopyDepthFromSave()
{
	Depth = Save.Const().GetDepth();
}