// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleMovementComponent.h"
#include "VehiclePawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"

void UVehicleMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    this->UpdatedComponent = GetOwner()->GetRootComponent();
    this->UpdateComponentVelocity();

    Torque = TurningTorque;
    Traction = TractionForce;

    VehiclePawn = Cast<AVehiclePawn>(GetOwner());
    VehicleMeshComp = VehiclePawn->GetStaticMeshComponent();
}

void UVehicleMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UVehicleMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
    FVector ForwardVector = VehiclePawn->GetActorForwardVector();
    float Throttle = 1.0f;
    float StoppingDistanceModifier = 0.0f;

    // which way to turn to get to the target
    FVector TargetDirection = MoveVelocity;

    if (VehiclePawn)
    {
        if(VehiclePawn->bIsBlocked)
        {
            float Distance = VehiclePawn->ObstacleDistance;
            if(VehiclePawn->bIsVehicle)
            {
                //scale distance to 0-100
                float scale = ((VehiclePawn->DistanceThreshold - Distance) / VehiclePawn->DistanceThreshold) * 100;
                // scale the move velocity to the oposite distance
                FVector AvoidDirection = MoveVelocity * scale;
                // new vector to move so to avoid the vehicle obstacle
                TargetDirection = MoveVelocity - AvoidDirection;

                StoppingDistanceModifier = 150.0f;
            }
            if(((Distance < VehiclePawn->StoppingDistance + StoppingDistanceModifier) ||  Distance < VehiclePawn->Reversedistance) && VehiclePawn->bHasToBrake)
            {
                Throttle = -1.0f;
            }
        }

        // find look at rotation
        FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(ForwardVector, TargetDirection);
        FRotator FinalRotation = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, GetOwner()->GetActorRotation());
        
        float Angle = FinalRotation.Yaw;
        // handbrake if the angle is too big and the vehicle is moving fast
        if((Angle > 45.0f || Angle < -45.0f) && VehiclePawn->GetVelocity().Size() > 700.0f)
		{
			HandbrakeVehicle(true);
		}
        else if (VehiclePawn->GetVelocity().Size() < 400.0f)
        {
            HandbrakeVehicle(false);
        }
        // Don't move if the vehicle is close to the player and the player is not moving
        if(!VehiclePawn->bIsCloseToPlayer)
        {
            ThrottleVehicle(Throttle);
            TurnVehicle(Angle);
        }
        
    }
}

void UVehicleMovementComponent::RequestPathMove(const FVector& MoveInput)
{
    // implementation not needed yet
}

void UVehicleMovementComponent::ThrottleVehicle(float AxisValue)
{
    //TODO: Do not move if the car is tipped over
	FVector ForceForward = FVector::ZeroVector;
	AccForce = (VehiclePawn->IsGrounded()) ? GroundAccForce * AxisValue : AirAccForce * AxisValue;
    Timer = (AxisValue == 0) ? 0.0f : Timer;
    if(AxisValue == 0 && bIsAccelerating) bIsAccelerating = false;

	// project forward vector onto ground plane
	FVector ForwardToGround = FVector::VectorPlaneProject(VehiclePawn->GetActorForwardVector(), VehiclePawn->GetGroundNormal());
	ForceForward = ForwardToGround * AccForce * VehicleMeshComp->GetMass();
    
    // if the vehicle is drifting, apply a boost to the force
	if(bIsHandbraking && !bIsAccelerating)
	{
		Timer += GetWorld()->GetDeltaSeconds();
		float ForceMultiplier = FMath::Lerp(MinDriftBoost, MaxDriftBoost, Timer/DriftBoostTimer);
		ForceForward = ForceForward * ForceMultiplier;
		if(Timer > DriftBoostTimer)
		{
			bIsAccelerating = true;
			bIsDrifting = false;
		}
	}
	else
	{
		bIsAccelerating = true;
		bIsDrifting = false;
	}

	VehicleMeshComp->AddForceAtLocation(ForceForward, VehiclePawn->GetAccelerationRoot()->GetComponentLocation());
}

void UVehicleMovementComponent::HandbrakeVehicle(bool bIsHandbrake)
{
    bIsHandbraking = bIsHandbrake;
	FVector ForwardToGround = FVector::VectorPlaneProject(VehiclePawn->GetActorForwardVector(), VehiclePawn->GetGroundNormal());

	if (bIsHandbraking)
	{
		FVector Force;
		// calculate max velocity of the vehicle based on the mass and force applied
		float MaxVelocity = 1300.0f;
		if(!bIsDrifting){
			bIsDrifting = true;
			TimeToStop = FMath::Lerp(0.1f, 2.0f, (VehicleMeshComp->GetPhysicsLinearVelocity().Size() / MaxVelocity));
		}
		// calculate force to apply to the vehicle to stop it if no acceleration is applied
		if( AccForce == 0)
			Force = ((-VehicleMeshComp->GetPhysicsLinearVelocity()/TimeToStop) * VehicleMeshComp->GetMass()) - (VehicleMeshComp->GetPhysicsLinearVelocity() * VehicleMeshComp->GetMass());
		// calculate force to apply to the vehicle to stop it if acceleration is applied
        else
			Force = ((-VehicleMeshComp->GetPhysicsLinearVelocity()/TimeToStop) * VehicleMeshComp->GetMass()) - (ForwardToGround * AccForce * VehicleMeshComp->GetMass());
		
		VehicleMeshComp->AddForceAtLocation(Force, VehiclePawn->GetAccelerationRoot()->GetComponentLocation());

		Traction = DriftTractionForce;
		Torque = DriftTorque;
	}
		
	else
	{
		bIsDrifting = false;
		Traction = TractionForce;
		Torque = TurningTorque;
	}
}

void UVehicleMovementComponent::TurnVehicle(float Angle)
{
    float RightVelocity = FVector::DotProduct(VehicleMeshComp->GetRightVector(), VehicleMeshComp->GetComponentVelocity());
	float ForwardVelocity = VehicleMeshComp->GetPhysicsLinearVelocity().Size();

	// scale turning ratio according to angle
	float TurningRatio = FMath::Lerp(0.0f, 1.0f, (FMath::Abs(Angle) / 20.0f));
	// clamp turning ratio between 0 and 1
	TurningRatio = FMath::Clamp(TurningRatio, 0.0f, 1.0f) * FMath::Clamp(ForwardVelocity / TurningThreshold, 0.0f, 1.0f);

	// force to counteract the drift
	float Force =  -RightVelocity * VehicleMeshComp->GetMass() * Traction;
	float TurnDirection = (Angle > 0.0f) ? 1.0f : -1.0f;

	if(VehiclePawn->IsGrounded())
	{
		VehicleMeshComp->AddTorqueInDegrees(FVector(0, 0, 1) * (TurnDirection * TurningRatio * Torque) * VehicleMeshComp->GetMass());
		VehicleMeshComp->AddForce(VehiclePawn->GetActorRightVector() * Force);
	}
}