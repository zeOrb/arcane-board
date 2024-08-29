// Fill out your copyright notice in the Description page of Project Settings.


#include "Dice.h"
#include "Components/StaticMeshComponent.h"
// Sets default values

TArray<FRotator> ADice::Matrix;

ADice::ADice()
{
	PrimaryActorTick.bCanEverTick = false;
	DiceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Dice Mesh Component"));
	SetRootComponent(DiceMesh);
	Matrix.Add(FRotator(-90, 0, 0));
	Matrix.Add(FRotator(0, 0, -90));
	Matrix.Add(FRotator(0, 0, 0));
	Matrix.Add(FRotator(0, 180, 180));
	Matrix.Add(FRotator(0, 0, 90));
	Matrix.Add(FRotator(90, 0, 0));
}

void ADice::BeginPlay()
{
	Super::BeginPlay();
}

// void ADice::OnDiceStill(UPrimitiveComponent* SleepingComponent, FName BoneName)
// {
// 	const FVector Up{ FVector::UpVector };
// 
// 	if (FVector::DotProduct(Up, GetActorForwardVector()) >= 0.8f)
// 		RolledValue = 6;
// 	else if (FVector::DotProduct(Up, -GetActorForwardVector()) >= 0.8f)
// 		RolledValue = 1;
// 	else if (FVector::DotProduct(Up, GetActorRightVector()) >= 0.8f)
// 		RolledValue = 2;
// 	else if (FVector::DotProduct(Up, -GetActorRightVector()) >= 0.8f)
// 		RolledValue = 5;
// 	else if (FVector::DotProduct(Up, GetActorUpVector()) >= 0.8f)
// 		RolledValue = 3;
// 	else if (FVector::DotProduct(Up, -GetActorUpVector()) >= 0.8f)
// 		RolledValue = 4;
// }

void ADice::RollDice()
{
	FVector RollDirection{ FMath::RandHelper(6) * 90.0f, FMath::RandHelper(6) * 90.0f , FMath::RandHelper(6) * 90.0f };
}

 FRotator ADice::GetRotationFacing(FRotator Rot) const
{
	FVector CameraLocation	{ GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation()};
	FVector ActorLocation	{ GetActorLocation()};
	FRotator LookAtRotation { FRotationMatrix::MakeFromX(CameraLocation - ActorLocation).Rotator()};
	FVector Forward			{ -LookAtRotation.Quaternion().GetForwardVector()}; Forward.Normalize();
	FVector Right			{  LookAtRotation.Quaternion().GetRightVector()};	Right.Normalize();
	FVector Up				{ -LookAtRotation.Quaternion().GetUpVector()};		Up.Normalize();
	FQuat RotQuaternion			{ FMatrix(Up, Right, Forward, FVector::ZeroVector).ToQuat()};
	FVector RolledForward	{ Rot.Quaternion().GetForwardVector()};
	FVector RolledRight		{ Rot.Quaternion().GetRightVector()};	
	FVector RolledUp		{ Rot.Quaternion().GetUpVector()};			
	RolledForward = RotQuaternion.RotateVector(RolledForward); RolledForward.Normalize();
	RolledRight = RotQuaternion.RotateVector(RolledRight); RolledRight.Normalize();
	RolledUp = RotQuaternion.RotateVector(RolledUp); RolledUp.Normalize();
	FMatrix FinalMatrix(RolledForward, RolledRight, RolledUp, FVector::ZeroVector);
	return FinalMatrix.Rotator();
}


void ADice::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

