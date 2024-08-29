




// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dice.generated.h"

UCLASS()
class ARCANABOARD_API ADice : public AActor
{
	GENERATED_BODY()
private:
	static TArray<FRotator> Matrix;

public:
	virtual void Tick(float DeltaTime) override;

	ADice();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Dice)
	class UStaticMeshComponent* DiceMesh;

// 	UFUNCTION()
// 	void OnDiceStill(UPrimitiveComponent* SleepingComponent, FName BoneName);

	UFUNCTION(BlueprintCallable, Category = Dice)
	void RollDice();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Dice)
	int32 RolledValue;

	UFUNCTION(BlueprintCallable, Category = Dice)
	FRotator GetRotationFacing(FRotator Rot) const;
};
