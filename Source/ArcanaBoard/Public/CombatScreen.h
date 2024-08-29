// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ArcanaClasses.h"

#include "CombatScreen.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ARCANABOARD_API UCombatScreen : public UUserWidget
{
	GENERATED_BODY()
public:
	UCombatScreen(const FObjectInitializer& ObjectInitializer);

	// Optionally override the Blueprint "Event Construct" event
	virtual void NativeConstruct() override;

	// Optionally override the tick event
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateUI(const FBattleData& BattleData, bool bForceNextState);

};
