// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIController.h"
#include "VehicleMovementComponent.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "TrafficPath.h"

// Sets default values
AVehiclePawn::AVehiclePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	VehicleMesh->SetupAttachment(RootComponent);

	Wheel_FL = CreateDefaultSubobject<USceneComponent>(TEXT("Wheel_FL"));
	Wheel_FL->SetupAttachment(VehicleMesh);
	Wheel_FR = CreateDefaultSubobject<USceneComponent>(TEXT("Wheel_FR"));
	Wheel_FR->SetupAttachment(VehicleMesh);
	Wheel_RL = CreateDefaultSubobject<USceneComponent>(TEXT("Wheel_RL"));
	Wheel_RL->SetupAttachment(VehicleMesh);
	Wheel_RR = CreateDefaultSubobject<USceneComponent>(TEXT("Wheel_RR"));
	Wheel_RR->SetupAttachment(VehicleMesh);

	VehicleAccelerationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VehicleAccelerationRoot"));
	VehicleAccelerationRoot->SetupAttachment(VehicleMesh);

	VehicleCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleCollision"));
	VehicleCollision->SetupAttachment(VehicleMesh);

	VehicleMovementComponent = CreateDefaultSubobject<UVehicleMovementComponent>(TEXT("VehicleMovementComponent"));
	VehicleMovementComponent->UpdatedComponent = VehicleMesh;
}

// Called when the game starts or when spawned
void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	if(!bIsAI)
	{
		PlayerController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

		if (PlayerController != nullptr)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}

			PlayerController->bShowMouseCursor = true;
		}
	}

	RayCastLocations.Add(Wheel_FL);
	RayCastLocations.Add(Wheel_FR);
	RayCastLocations.Add(Wheel_RL);
	RayCastLocations.Add(Wheel_RR);

	VehicleMesh->SetLinearDamping(LinearDamper);
	VehicleMesh->SetAngularDamping(AngularDamper);

}

// Called every frame
void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	float FloorNonContactCount = 0;
	//Raycast from the wheels to the ground to check if the vehicle is grounded and apply suspension force
	for (int i = 0; i < RayCastLocations.Num(); i++)
	{	
		FVector StartLocation = RayCastLocations[i]->GetComponentLocation();
		FVector EndLocation = StartLocation + (-this->GetActorUpVector() * WheelSize);

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams);	

		if (HitResult.bBlockingHit)
		{
			float Compression = HitResult.Distance / WheelSize;
			// impact normal
			GroundNormal = (HitResult.Normal);

			// lerp between 0 and wheel size
			float Suspension = FMath::Lerp(SuspensionForce, 0.0f, Compression);
			// add force
			FVector Force = GroundNormal * Suspension ;

			// add force to the wheel
			VehicleMesh->AddForceAtLocation(Force, StartLocation);
			
			bIsGrounded = true;
			FloorNonContactCount = 0;
		}
		else
		{
			FloorNonContactCount++;

			if (FloorNonContactCount == 4)
			{
				bIsGrounded = false;
			}
		}
	}	
	if(bIsAI)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		AAIController* AIController = Cast<AAIController>(GetController());
		FCollisionShape Shape = FCollisionShape::MakeBox(VehicleCollision->GetScaledBoxExtent());
		FHitResult HitResult;
		FCollisionQueryParams Params(TEXT("Trace"), true, this);
		bIsBlocked = false;
		
		// check if the player is close to the vehicle
		if(PlayerPawn && PlayerPawn->GetVelocity().Size() < 20 && FVector::Dist(PlayerPawn->GetActorLocation(), GetActorLocation()) < 400.0f)
			bIsCloseToPlayer = true;
		else
			bIsCloseToPlayer = false;

		// box trace to check if the vehicle is blocked by an obstacle
		GetWorld()->SweepSingleByChannel(HitResult,VehicleCollision->GetComponentLocation() , VehicleCollision->GetComponentLocation() + GetActorForwardVector() * DistanceThreshold, GetActorRotation().Quaternion(), ECC_Visibility, Shape, Params);
		// check if the vehicle is blocked by anything that is not the player and is not breakable
		if (HitResult.bBlockingHit && (HitResult.GetActor() != PlayerPawn || !bIsChasing) && !HitResult.GetActor()->ActorHasTag("Breakable") && !bIsCloseToPlayer)
		{
			ObstacleDistance = HitResult.Distance;
			bIsBlocked = true;
			// check if the vehicle is close enough needs to brake or can overtake
			bHasToBrake = (HitResult.GetActor()->GetVelocity().Size() < 500) ? true : false;

			if (HitResult.GetActor()->IsA(AVehiclePawn::StaticClass()))
			{
				bIsVehicle = true;
				// check angle between vehicle and obstacle velocity vectors
				FRotator Hitrotation = HitResult.GetActor()->GetVelocity().GetSafeNormal().Rotation();
				FRotator VehicleRotation = VehicleMesh->GetForwardVector().Rotation();
				FRotator FinalRotation = UKismetMathLibrary::NormalizedDeltaRotator(Hitrotation, VehicleRotation);

				float Angle = FinalRotation.Yaw;
				// Depending on the angle between the vehicle and the obstacle, the vehicle will have to brake brake
				if (Angle > 50 && Angle < 130 || Angle < -50 && Angle > -130)
				{
					bHasToBrake = true;
				}
				// check if the vehicle is moving at the same speed or slower than the obstacle to check if it is trully blocked
				else if(HitResult.GetActor()->GetVelocity().Size() >= this->GetVelocity().Size() && HitResult.Distance > 10.0f)
				{
					bIsBlocked = false;
				}	
			}
			else
				bIsVehicle = false;
			
		}
		// if vehicle is not chasing player then it is following the path
		if(!bIsChasing && CarPath)
		{
			FVector ForwardVector = CarPath->PathSpline->FindDirectionClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World);
			FVector NextPoint = CarPath->PathSpline->FindLocationClosestToWorldLocation(GetActorLocation() + ForwardVector * AIPathResolution, ESplineCoordinateSpace::World);

			AIController->MoveToLocation(NextPoint, 50.0f, false, true, true, false, 0, true);
		}
	}
	// not an AI so the player controlls the steering of the vehicle
	else
	{
		FVector WorldLocation;
		FVector WorldDirection;
		// get mouse position in world
		PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
		// get the rotation to look at the mouse position
		FRotator TargetRotation = WorldDirection.Rotation();
		FRotator Rotation = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, GetActorRotation());
		// turn the vehicle
		VehicleMovementComponent->TurnVehicle(Rotation.Yaw);
	}

}

// Called to bind functionality to input
void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVehiclePawn::MoveVehicle);
		
		//Drifting
		EnhancedInputComponent->BindAction(DriftAction, ETriggerEvent::Triggered, this, &AVehiclePawn::HandbrakeVehicle);

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AVehiclePawn::JumpVehicle);
	}

}

void AVehiclePawn::MoveVehicle(const FInputActionValue& Value)
{
	VehicleMovementComponent->ThrottleVehicle(Value.Get<float>());
}

void AVehiclePawn::HandbrakeVehicle(const FInputActionValue& Value)
{
	VehicleMovementComponent->HandbrakeVehicle(Value.Get<bool>());
}

void AVehiclePawn::JumpVehicle(const FInputActionValue& Value)
{
	// TODO: ADD COOL DOWN TO JUMP
	if(bIsGrounded){
		VehicleMesh->AddImpulse(this->GetActorUpVector() * JumpForce * VehicleMesh->GetMass());
	}
}

UPawnMovementComponent* AVehiclePawn::GetMovementComponent() const
{
	return VehicleMovementComponent;
}