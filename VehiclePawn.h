// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Components/BoxComponent.h"
#include "VehiclePawn.generated.h"

class UInputComponent;
class UVehicleMovementComponent;
class ATrafficPath;

UCLASS()
class RL_POSTPERSON_API AVehiclePawn : public APawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* DriftAction;

public:
	// Sets default values for this pawn's properties
	AVehiclePawn();

	/////// AI Variables ///////
	/** Keep track if vehicle is blocked by an obstacle */
	bool bIsBlocked = false;
	/** Keep track of the distance to the obstacle */
	float ObstacleDistance;
	/** Keep track if vehicle has to brake */
	bool bHasToBrake = false;
	/** Keep track if obstacle is a vehicle */
	bool bIsVehicle = false;
	/** Keep track if this Vehicle is close to the player */
	bool bIsCloseToPlayer = false;

	/** Track if the vehicle is AI controlled */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	bool bIsAI = false;
	/** Track if the vehicle is chasing the player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	bool bIsChasing = false;
	/** Current path the vehicle is following */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	ATrafficPath* CarPath;
	/** How closely the vehicle follows the path */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	float AIPathResolution = 500.0f;
	/** How far the vehicle will be to start avoiding an obstacle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	float DistanceThreshold = 300.0f;
	/** How far the vehicle will be from the obstacle to start braking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	float StoppingDistance = 100.0f;
	/** How far the vehicle will Reverse before going forward again */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	float Reversedistance = 250.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	APlayerController *PlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh")
	UStaticMeshComponent* VehicleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh")
	UBoxComponent* VehicleCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RayCast")
	USceneComponent* Wheel_FL;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RayCast")
	USceneComponent* Wheel_FR;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RayCast")
	USceneComponent* Wheel_RL;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RayCast")
	USceneComponent* Wheel_RR;

	TArray<USceneComponent*> RayCastLocations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MovementComponent")
	UVehicleMovementComponent* VehicleMovementComponent;
	/** The how far from the ground the vehicle will be */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="RayCast")
	float WheelSize = 30;
	/** where the acceleration force will be applied */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	USceneComponent* VehicleAccelerationRoot;

	/////// Movement Variables ///////
	
	/** The normal of the ground the vehicle wheel is on */
	FVector GroundNormal;
	/** keep track if the vehicle is grounded */
	bool bIsGrounded = false;
	/** The force to apply to the vehicle when jumping */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float JumpForce = 300;
	/** Linear damping to apply to the vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float LinearDamper = 1.5f;
	/** Angular damping to apply to the vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float AngularDamper = 5.0f;
	/** The force to apply to the vehicle suspension */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float SuspensionForce = 1000000;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	/** Input to move the vehicle */
	void MoveVehicle(const FInputActionValue& Value);
	/** Input to activate the handbrake */
	void HandbrakeVehicle(const FInputActionValue& Value);
	/** Input to jump the vehicle */
	void JumpVehicle(const FInputActionValue& Value);
	/** Get the current movement component of this pawn */
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	/** Get the current Ground Normal of the vehicle */
	FVector GetGroundNormal() const { return GroundNormal; }
	/** Get if the vehicle is grounded or not */
	bool IsGrounded() const { return bIsGrounded; }
	/** Get the vehicle mesh component */
	UStaticMeshComponent * GetStaticMeshComponent() const { return VehicleMesh; }
	/** Get the vehicle acceleration root component */
	USceneComponent* GetAccelerationRoot() const { return VehicleAccelerationRoot; }


};
