// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "ArcanaController.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACameraCharacter::ACameraCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->MaxAcceleration = 100000.0;
	GetCharacterMovement()->BrakingDecelerationWalking = 300;
	GetCharacterMovement()->GravityScale = 0.0f;
	GetCharacterMovement()->bUseSeparateBrakingFriction = true;
	GetCharacterMovement()->BrakingFriction = 2;
	GetCharacterMovement()->MaxWalkSpeed = 900;
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0,0,1));
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Arm"));
	SpringArmComp->SetupAttachment(GetRootComponent());
	CameraComp= CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

	bIsPanButtonPressed = false;
	MouseVector = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void ACameraCharacter::BeginPlay()
{
	Super::BeginPlay();

	ArcanaController = Cast<AArcanaController>(GetController());

	InputComponent->BindAction("LeftMouseButton", IE_Pressed, ArcanaController, &AArcanaController::OnLeftPressReceived);
	InputComponent->BindAction("LeftMouseButton", IE_Released, ArcanaController, &AArcanaController::OnLeftReleaseReceived);
}

// Called every frame
void ACameraCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACameraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MouseX", this, &ACameraCharacter::MouseX);
	PlayerInputComponent->BindAxis("MouseY", this, &ACameraCharacter::MouseY);

	PlayerInputComponent->BindAction("PanningButton", IE_Pressed, this, &ACameraCharacter::OnEnablePanning);
	PlayerInputComponent->BindAction("PanningButton", IE_Released, this, &ACameraCharacter::OnDisablePanning);
	
	
}

void ACameraCharacter::MouseX(float Val)
{
	MouseVector.Y = Val * -1;
	if(IsPanning())
		PanCamera();
}
void ACameraCharacter::MouseY(float Val)
{
	MouseVector.X = Val * -1;
	if (IsPanning())
		PanCamera();
}

void ACameraCharacter::OnEnablePanning()
{
	bIsPanButtonPressed = true;
}

void ACameraCharacter::OnDisablePanning()
{
	bIsPanButtonPressed = false;
}


void ACameraCharacter::PanCamera()
{
	AddMovementInput(GetActorRotation().RotateVector(MouseVector));
}

