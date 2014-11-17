// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "AI/Navigation/NavLinkDefinition.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIAsyncTaskBlueprintProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOAISimpleDelegate, EPathFollowingResult::Type, MovementResult);

UCLASS(MinimalAPI)
class UAIAsyncTaskBlueprintProxy : public UObject
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FOAISimpleDelegate	OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOAISimpleDelegate	OnFail;

public:
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type MovementResult);

	void OnNoPath();

	// Begin UObject interface
	virtual void BeginDestroy() override;
	// End UObject interface

	TWeakObjectPtr<class AAIController> AIController;
	FAIRequestID MoveRequestId;
};