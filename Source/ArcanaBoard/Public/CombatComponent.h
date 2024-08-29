// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArcanaClasses.h"
#include "CombatComponent.generated.h"

class AUnit;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARCANABOARD_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Data")
	AUnit* Owner {nullptr};

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Data")
	FVector2D Coords {-FVector2D::UnitVector};




public:	
	uint8 bFinished :1;

	void Initialize(AUnit* InOwner);

	AUnit* GetOwner() const;

	int32 GetStatValue(EStat Stat) const;

	int32 GetCurrentHealth() const;

	bool IsDefeated() const;

	const FVector2D& GetCoords() const;

	void SetCoords(const FVector2D& InCoords);

	void SetHealth(int32 Value);

	void SetStatValue(EStat Stat, int32 Value);

	FCombatAction MakeAction(ECombatActionType Type, TArray<FVector2D> Targets, FItem Item=FItem());

	FCombatAction MakeAction(ECombatActionType Type, FVector2D Target, FItem Item=FItem());

	FVector2D MakeAttackCell(FVector2D A, FVector2D B) const;
};
