// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"
#include "Unit.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UCombatComponent::Initialize(AUnit* InOwner)
{
	Owner = InOwner;
}

AUnit* UCombatComponent::GetOwner() const
{
	return Owner;
}

int32 UCombatComponent::GetStatValue(EStat Stat) const
{
	return GetOwner()->GetStatSafe(Stat).GetValue();
}

int32 UCombatComponent::GetCurrentHealth() const
{
	return GetOwner()->GetCurrentHealth();
}

bool UCombatComponent::IsDefeated() const
{
	return GetCurrentHealth() <= 0;
}

const FVector2D& UCombatComponent::GetCoords() const
{
	return Coords;
}

void UCombatComponent::SetCoords(const FVector2D& InCoords)
{
	Coords = InCoords;
}

void UCombatComponent::SetHealth(int32 Value)
{
	GetOwner()->CurrentHealth = FMath::Max(0, Value);
}

void UCombatComponent::SetStatValue(EStat Stat, int32 Value)
{
	GetOwner()->GetStat(Stat).SetValue(Value);
}

FCombatAction UCombatComponent::MakeAction(ECombatActionType Type, TArray<FVector2D> Targets, FItem Item/*=FItem()*/)
{
	FCombatAction Action;
	Action.Type = Type;
	Action.Source = Coords;
	Action.Targets = Targets;
	Action.Item = Item;
	return Action;
}
FVector2D UCombatComponent::MakeAttackCell(FVector2D A, FVector2D B) const
{
	return B+ FVector2D((A.X == B.X) ? 0 : (A.X > B.X) ? 1 : -1, (A.Y == B.Y) ? 0 : (A.Y > B.Y) ? 1 : -1);
}

FCombatAction UCombatComponent::MakeAction(ECombatActionType Type, FVector2D Target, FItem Item/*=FItem()*/)
{
	FCombatAction Action;

	Action.Type = Type;
	Action.Source = Coords;
	Action.Targets.Add(Target);
	if (Type == EMoveAndAttack) {
		FVector2D Temp = Action.Targets[0];
		Action.Targets.Add(Temp);
		Action.Targets[0] = MakeAttackCell(Coords, Target);
	}
	Action.Item = Item;
	return Action;
}

// Called every frame
// void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
// 
// 	// ...
// }

