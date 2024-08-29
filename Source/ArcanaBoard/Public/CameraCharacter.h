// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CameraCharacter.generated.h"

UCLASS()
class ARCANABOARD_API ACameraCharacter : public ACharacter
{
	GENERATED_BODY()
private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* CameraComp;

	class AArcanaController* ArcanaController;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	uint8 bIsPanButtonPressed :1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector MouseVector;

public:
	ACameraCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
// Input and controls
	virtual void MouseX(float Val);
	
	virtual void MouseY(float Val);

	virtual void OnEnablePanning();

	virtual void OnDisablePanning();

	FORCEINLINE bool IsPanning() const { return bIsPanButtonPressed;};

	void PanCamera();
};
