// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleAIController.h"

void AVehicleAIController::BeginPlay()
{
    Super::BeginPlay();

    AVehiclePawn* VehiclePawn = Cast<AVehiclePawn>(GetPawn());
}

void AVehicleAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if(VehiclePawn && VehiclePawn->bIsChasing)
    {
        AVehiclePawn* Player = Cast<AVehiclePawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
        MoveToActor(Player, -1.0f, false, true, false, 0, true);
        
        if(FVector::Dist(VehiclePawn->GetActorLocation(), Player->GetActorLocation()) < 280.0f && Player->GetVelocity().Size() < 800.0f)
		{
			StopMovement();
		}
    }
}

