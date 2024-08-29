// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArcanaClasses.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"

#include "ArcanaController.generated.h"

UCLASS()
class ARCANABOARD_API AArcanaController : public APlayerController
{
	GENERATED_BODY()
	AArcanaController();
private:
	void HandleMapClick();
	
	void HandleCombatClick();
	
	void HandleUnitClick();



private:
	class UGameplayTagsManager* TagsManager;

	class AArcanaBoardGameModeBase* GameMode;

	class AArena* Arena;
//!// COMBAT //!//
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UCombatScreen> CombatWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	class UCombatScreen* CombatWidget;

	bool IsUserMode(FName TagName) const;

	class UParty* GetParty();
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UserInput|Mode")
	FGameplayTag UserMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UserInput|Mouse")
	float ClickInterval;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UserInput|Mouse")
	float ClickTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UserInput|Mouse")
	uint8 bIsLeftButtonPressed :1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UserInput|Mouse")
	class UParticleSystem* PokeEffect;

public:
	float SnapValue{ 0 };

	FVector SnapRegionMin {FVector::ZeroVector};
	
	FVector SnapRegionMax {FVector::ZeroVector};
	
	virtual void Tick(float DeltaTime) override;

// Input
	virtual void OnLeftPressReceived();

	virtual void OnLeftReleaseReceived();

	FORCEINLINE bool IsLeftButtonHold() const {return bIsLeftButtonPressed;};

protected:
	virtual void OnLeftButtonClick();

	virtual void Poke();

protected:
	FVector2D GetHoveredCell() const;

	class AUnit* GetCurrentUnit() const;

	FVector2D GetCurrentCell() const;

	class UCombatComponent* GetCurrentCC() const;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnBeginTransitionToCombat(AActor* CameraActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnTransitionToCombatFinished();


	// COMBAT SELECTION AND STUFF
	void BindArena(class AArena* Arena);

	void ActivateMapMode();

	void ActivateCombatMode();

	void ActivateUnitMode();



	UFUNCTION(BlueprintCallable)
	FVector GetMouseProjectedToPlane(const FVector PlanePosition) const;

	UFUNCTION(BlueprintCallable)
	FVector SnapToGrid(const FVector Location) const;

	UFUNCTION(BlueprintCallable)
	FVector SnapWithinRegion(FVector Location) const;

	UFUNCTION(BlueprintCallable)
	FVector2D LocationToCombatCell(FVector Loc) const;

	void SetSnapRegion(FVector2D GridSize, FVector Origin);
};
