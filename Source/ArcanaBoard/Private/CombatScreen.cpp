// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatScreen.h"

UCombatScreen::UCombatScreen(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UCombatScreen::NativeConstruct()
{
	// Do some custom setup
	
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();
}

void UCombatScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{	
	// Make sure to call the base class's NativeTick function

	// Do your custom tick stuff here
	Super::NativeTick(MyGeometry, InDeltaTime);
}

