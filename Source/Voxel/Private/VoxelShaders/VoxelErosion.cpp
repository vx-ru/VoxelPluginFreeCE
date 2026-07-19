// Copyright 2021 Phyronnaz

#include "VoxelShaders/VoxelErosion.h"
#include "VoxelShaders/VoxelErosionShader.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelMessages.h"

#include "Engine/Texture2D.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"

void UVoxelErosion::Initialize()
{
	if (bIsInit)
	{
		FVoxelMessages::Error("Erosion is already initialized!");
		return;
	}
	
	RealSize = FMath::Max(32, FMath::CeilToInt(Size / 32.f) * 32);

	auto WeakThis = MakeWeakObjectPtr(this);
	ENQUEUE_RENDER_COMMAND(Step)(
		[WeakThis](FRHICommandList& RHICmdList)
	{
		if (auto ThisPtr = WeakThis.Pin())
		{
			ThisPtr->Init_RenderThread(RHICmdList);
		}
	});

	FlushRenderingCommands();

	if (RainMapInit->GetSizeX() == RealSize &&
		RainMapInit->GetSizeY() == RealSize)
	{
		CopyTextureToRHI(*RainMapInit, RainMap);
	}
	else
	{
		FVoxelMessages::Error(
			FString::Printf(
				TEXT("Voxel Erosion Init: RainMapInit has size (%d, %d), but should have size (%d, %d)"),
				RainMapInit->GetSizeX(),
				RainMapInit->GetSizeY(),
				RealSize,
				RealSize),
			this);
	}
	
	if (HeightmapInit->GetSizeX() == RealSize &&
		HeightmapInit->GetSizeY() == RealSize)
	{
		CopyTextureToRHI(*HeightmapInit, TerrainHeight);
	}
	else
	{
		FVoxelMessages::Error(
			FString::Printf(
				TEXT("Voxel Erosion Init: HeightmapInit has size (%d, %d), but should have size (%d, %d)"),
				HeightmapInit->GetSizeX(),
				HeightmapInit->GetSizeY(),
				RealSize,
				RealSize),
			this);
	}

	bIsInit = true;
}

bool UVoxelErosion::IsInitialized() const
{
	return bIsInit;
}

void UVoxelErosion::Step(int32 Count)
{
	if (!bIsInit)
	{
		FVoxelMessages::Error("Erosion is not initialized!");
		return;
	}
	
	FVoxelErosionParameters Parameters;
	Parameters.size = RealSize;
	Parameters.dt = DeltaTime;

	Parameters.l = Scale;
	Parameters.g = Gravity;

	Parameters.Kc = SedimentCapacity;
	Parameters.Ks = SedimentDissolving;
	Parameters.Kd = SedimentDeposition;

	Parameters.Kr = RainStrength;
	Parameters.Ke = Evaporation;

	auto WeakThis = MakeWeakObjectPtr(this);
	ENQUEUE_RENDER_COMMAND(Step)(
		[Parameters, Count, WeakThis](FRHICommandList& RHICmdList)
	{
		if (auto ThisPtr = WeakThis.Pin())
		{
			ThisPtr->Step_RenderThread(Parameters, Count);
		}
	});
}

FVoxelFloatTexture UVoxelErosion::GetTerrainHeightTexture()
{
	if (!bIsInit)
	{
		FVoxelMessages::Error("Erosion is not initialized!");
		return {};
	}
	
	auto Texture = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
	CopyRHIToTexture(TerrainHeight, Texture);
	return { TVoxelTexture<float>(Texture) };
}


FVoxelFloatTexture UVoxelErosion::GetWaterHeightTexture()
{
	if (!bIsInit)
	{
		FVoxelMessages::Error("Erosion is not initialized!");
		return {};
	}
	
	auto Texture = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
	CopyRHIToTexture(WaterHeight, Texture);
	return { TVoxelTexture<float>(Texture) };
}


FVoxelFloatTexture UVoxelErosion::GetSedimentTexture()
{
	if (!bIsInit)
	{
		FVoxelMessages::Error("Erosion is not initialized!");
		return {};
	}
	
	auto Texture = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
	CopyRHIToTexture(Sediment, Texture);
	return { TVoxelTexture<float>(Texture) };
}

template<typename T>
void UVoxelErosion::RunShader(const FVoxelErosionParameters& Parameters)
{
	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();
	
	TShaderMapRef<T> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FRHIComputeShader* ShaderRHI = ComputeShader.GetComputeShader();
	SetComputePipelineState(RHICmdList, ShaderRHI);

	// The parameters are only staged into the batch - they must be submitted with
	// SetBatchedShaderParameters, else nothing is bound and D3D12 asserts on dispatch
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	ComputeShader->SetSurfaces(
		BatchedParameters,
		RainMapUAV,
		TerrainHeightUAV, 
		TerrainHeight1UAV, 
		WaterHeightUAV, 
		WaterHeight1UAV, 
		WaterHeight2UAV, 
		SedimentUAV, 
		Sediment1UAV, 
		OutflowUAV, 
		VelocityUAV);
	ComputeShader->SetUniformBuffers(BatchedParameters, Parameters);
	RHICmdList.SetBatchedShaderParameters(ShaderRHI, BatchedParameters);

	RHICmdList.DispatchComputeShader(RealSize / VOXEL_EROSION_NUM_THREADS_CS, RealSize / VOXEL_EROSION_NUM_THREADS_CS, 1);

	ComputeShader->UnbindBuffers(BatchedParameters);
	RHICmdList.SetBatchedShaderParameters(ShaderRHI, BatchedParameters);
}

void UVoxelErosion::CopyTextureToRHI(const TVoxelTexture<float>& Texture, const FTextureRHIRef& RHITexture)
{
	ENQUEUE_RENDER_COMMAND(CopyTextureToRHI)([Texture, RHITexture, ThisPtr = this](FRHICommandList& RHICmdList)
	{
		ThisPtr->CopyTextureToRHI_RenderThread(Texture, RHITexture);
	});

	FlushRenderingCommands();
}

void UVoxelErosion::CopyRHIToTexture(const FTextureRHIRef& RHITexture, TVoxelSharedRef<TVoxelTexture<float>::FTextureData>& Texture)
{
	ENQUEUE_RENDER_COMMAND(CopyRHIToTexture)(
		[RHITexture, Texture, ThisPtr = this](FRHICommandList& RHICmdList)
	{
		ThisPtr->CopyRHIToTexture_RenderThread(RHITexture, *Texture);
	});

	FlushRenderingCommands();
}

void UVoxelErosion::CopyTextureToRHI_RenderThread(const TVoxelTexture<float>& Texture, const FTextureRHIRef& RHITexture)
{
	check(IsInRenderingThread());

	const int32 Size = RHITexture->GetSizeX();
	if (!ensureAlways(RHITexture->GetSizeY() == Size)) return;

	if (!ensureAlways(Texture.GetSizeX() == Size)) return;
	if (!ensureAlways(Texture.GetSizeY() == Size)) return;

	uint32 MappedStride = 0;
	float* const RHIData = static_cast<float*>(RHILockTexture2D(RHITexture, 0, RLM_WriteOnly, MappedStride, false));
	if (!ensureAlways(RHIData)) return;

	check(Texture.GetTextureData().Num() == Size * Size);
	FMemory::Memcpy(RHIData, Texture.GetTextureData().GetData(), Size * Size * sizeof(float));

	RHIUnlockTexture2D(RHITexture, 0, false);
}


void UVoxelErosion::CopyRHIToTexture_RenderThread(const FTextureRHIRef& RHITexture, TVoxelTexture<float>::FTextureData& Texture)
{
	check(IsInRenderingThread());

	const int32 Size = RHITexture->GetSizeX();
	if (!ensureAlways(RHITexture->GetSizeY() == Size)) return;

	uint32 MappedStride = 0;
	const float* RESTRICT const RHIData = static_cast<float*>(RHILockTexture2D(RHITexture, 0, RLM_ReadOnly, MappedStride, false));
	if (!ensureAlways(RHIData)) return;

	Texture.SetSize(Size, Size);
	
	for (int32 Index = 0; Index < Size * Size; Index++)
	{
		Texture.SetValue(Index, RHIData[Index]);
	}

	RHIUnlockTexture2D(RHITexture, 0, false);
}

void UVoxelErosion::Init_RenderThread(FRHICommandList& RHICmdList)
{
	check(IsInRenderingThread());

	const ETextureCreateFlags Flags = TexCreate_ShaderResource | TexCreate_UAV;

	auto CreateTextureWithUAV = [&](TRefCountPtr<FRHITexture>& Texture, TRefCountPtr<FRHIUnorderedAccessView>& UAV, int32 SizeX, const TCHAR* DebugName)
	{
		FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(DebugName)
			.SetExtent(int32(SizeX * RealSize), int32(RealSize))
			.SetFormat(PF_R32_FLOAT)
			.SetNumMips(1)
			.SetNumSamples(1)
			.SetFlags(Flags)
			.SetInitialState(ERHIAccess::Unknown);

		Texture = RHICreateTexture(Desc);
		UAV = RHICmdList.CreateUnorderedAccessView(Texture, FRHIViewDesc::CreateTextureUAV().SetDimensionFromTexture(Texture));
	};

	// Create all textures and UAVs
	CreateTextureWithUAV(RainMap,       RainMapUAV,       1, TEXT("RainMap"));
	CreateTextureWithUAV(TerrainHeight, TerrainHeightUAV, 1, TEXT("TerrainHeight"));
	CreateTextureWithUAV(TerrainHeight1, TerrainHeight1UAV, 1, TEXT("TerrainHeight1"));
	CreateTextureWithUAV(WaterHeight,   WaterHeightUAV,   1, TEXT("WaterHeight"));
	CreateTextureWithUAV(WaterHeight1,  WaterHeight1UAV,  1, TEXT("WaterHeight1"));
	CreateTextureWithUAV(WaterHeight2,  WaterHeight2UAV,  1, TEXT("WaterHeight2"));
	CreateTextureWithUAV(Sediment,      SedimentUAV,      1, TEXT("Sediment"));
	CreateTextureWithUAV(Sediment1,     Sediment1UAV,     1, TEXT("Sediment1"));
	CreateTextureWithUAV(Outflow,       OutflowUAV,       4, TEXT("Outflow"));
	CreateTextureWithUAV(Velocity,      VelocityUAV,      2, TEXT("Velocity"));
}

void UVoxelErosion::Step_RenderThread(const FVoxelErosionParameters& Parameters, int32 Count)
{
	check(IsInRenderingThread());
	for (int32 Index = 0; Index < Count; Index++)
	{
		RunShader<FVoxelErosionWaterIncrementCS>(Parameters);
		RunShader<FVoxelErosionFlowSimulationCS>(Parameters);
		RunShader<FVoxelErosionErosionDepositionCS>(Parameters);
		RunShader<FVoxelErosionSedimentTransportationCS>(Parameters);
		RunShader<FVoxelErosionEvaporationCS>(Parameters);
	}
}