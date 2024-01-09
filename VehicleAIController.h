// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "VehiclePawn.h"
#include "VehicleAIController.generated.h"

/**
 * 
 */
UCLASS()
class RL_POSTPERSON_API AVehicleAIController : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	AVehiclePawn* VehiclePawn;

public:
	virtual void Tick(float DeltaTime) override;
	
};
