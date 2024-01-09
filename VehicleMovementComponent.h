// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "VehicleMovementComponent.generated.h"

class AVehiclePawn;
class UStaticMeshComponent;

/**
 * 
 */
UCLASS()
class RL_POSTPERSON_API UVehicleMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
	
	virtual void RequestPathMove(const FVector& MoveInput) override;

public:

protected:
	virtual void BeginPlay() override;

protected:

	AVehiclePawn* VehiclePawn;
	UStaticMeshComponent* VehicleMeshComp;

	/** Acceleration force to apply to the vehicle */
	float AccForce;
	/** Timer to keep track of how long the vehicle has been drift boosting*/
	float Timer = 0.0f;
	/** Current traction force to apply to the vehicle */
	float Traction;
	/** Current torque to apply to the vehicle */
	float Torque;
	/** Keep track of the current drift status of the vehicle */
	bool bIsDrifting = false;
	/** Keep track of the current handbrake status of the vehicle */
	bool bIsHandbraking = false;
	/** Keep track of the current acceleration status of the vehicle */
	bool bIsAccelerating = false;
	/** Keep track in how long the vehicle has will stop */
	float TimeToStop;

	/** Acceleration force to apply to the vehicle when on the ground */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float GroundAccForce = 2000;
	/** Acceleration force to apply to the vehicle when in the air */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float AirAccForce = 1000;
	/** Torque to apply to the vehicle when turning */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float TurningTorque = 3500000;
	/** Torque to apply to the vehicle when drifting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float DriftTorque = 5500000;
	/** Threshold to determine how fast the vehicle is turning to the new direction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float TurningThreshold = 500.0f;
	/** The traction force to apply to the vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float TractionForce = 20;
	/** The traction force to apply to the vehicle when drifting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float DriftTractionForce = 1.0f;
	/** The maximum drift boost to apply to the vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float MaxDriftBoost = 3.0f;
	/** The minimum drift boost to apply to the vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float MinDriftBoost = 1.5f;
	/** The time it takes to reach the maximum drift boost and stop boosting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float DriftBoostTimer = 1.5f;

public:
	/**
	 * Throttle the vehicle forward or backward based on the axis value
	 * @param AxisValue	-1.0f to 1.0f
	*/ 
	void ThrottleVehicle(float AxisValue);
	/**
	 * Activate handbrake
	 * @param bIsHandbrake true to activate handbrake, false to deactivate
	*/
	void HandbrakeVehicle(bool bIsHandbrake);
	/**
	 * Turn the vehicle left or right based on the angle
	 * @param Angle Angle to turn the vehicle from Forward Vector
	*/
	void TurnVehicle(float Angle);
	
};
