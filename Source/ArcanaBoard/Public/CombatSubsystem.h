// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CombatSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ARCANABOARD_API UCombatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	private:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	void InitializeCombatEncounter(class UParty* A, class UParty* B);

	void BeginCombatEncounter();
	
};
