// Copyright 2021 Phyronnaz
// Modifications Copyright 2024 vxru

#pragma once

#include "CoreMinimal.h"

class VOXEL_API IVoxelEditorDelegatesInterface
{
public:
#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE_TwoParams(FBindEditorDelegates, IVoxelEditorDelegatesInterface*, UObject*);
	static FBindEditorDelegates BindEditorDelegatesDelegate;

	void BindEditorDelegates(UObject* Self)
	{
		BindEditorDelegatesDelegate.Broadcast(this, Self);
	}
	
	virtual ~IVoxelEditorDelegatesInterface() = default;
	virtual void OnPreSaveWorld(UWorld* World, const FObjectPreSaveContext& SaveContext) {}
	virtual void OnPreBeginPIE(bool bIsSimulating) {}
	virtual void OnEndPIE(bool bIsSimulating) {}
	virtual void OnPrepareToCleanseEditorObject(UObject* Object) {}
	virtual void OnPreExit() {}
	virtual void OnApplyObjectToActor(UObject* Object, AActor* Actor) {}
#endif
};