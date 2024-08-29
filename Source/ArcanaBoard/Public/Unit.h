// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArcanaClasses.h"
#include "GameFramework/Actor.h"
#include "Unit.generated.h"

UCLASS()
class ARCANABOARD_API AUnit : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AUnit();

	virtual void Tick(float DeltaTime) override;

	const FUnitStats& GetStats() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	class UParty* PartyMembership;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit|Stats")
	FUnitStats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit|Components")
	class USkeletalMeshComponent* SkelComp;



private:
	friend class UCombatComponent;


	UPROPERTY(VisibleAnywhere, Category = "Unit|Components")
	class USceneComponent* RootSceneComp;

	UPROPERTY(VisibleAnywhere, Category = "Unit|Components")
	class UCombatComponent* CombatComp;

	static class UGameplayTagsManager& TagsManager;

	static FGameplayTag WeaponTag;
	static FGameplayTag OneHanded;
	static FGameplayTag TwoHanded;

	UPROPERTY(VisibleAnywhere, Category = "Unit")
	int32 CurrentHealth;

public:
	void RegisterParty(class UParty* Party);

	UPROPERTY(VisibleAnywhere, Category = "Unit")
	TArray<FItem> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
	FText Name;

	void InitializePrimaryStats(const FUnitStats& InStats);

	void GenerateSecondaryStats();

	FStatLine& GetStat(EStat Type);

	const FStatLine& GetStatSafe(EStat Type) const;

	class UCombatComponent* GetCC() const;

	int32 GetCurrentHealth() const;

	int32 GetMovementRange() const;

	void OnPlacementChange() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasTakenTurn() const;

	void InitializeInventory(const TArray<FItem>& Items);

	UFUNCTION()
	void InitializeItemSlot(EEquipType Slot);

	bool EquipItem(FItem& ItemToEquip, EEquipType SlotToEquip);

	void AutoEquipBestWeapon();

	int32 GenerateValueForWeapon(const FItem& Weapon) const;
	
	void EquipWeapon(FItem& Weapon, EEquipType Slot);

	bool UnequipWeapon(EEquipType Slot);

	void ToggleWeaponSheath();

	void ToggleSlotSheath(EEquipType SlotType);

	FEquipmentSlot& GetSlot(EEquipType Slot);

	// What to do when you have 2h and want to equip 1h and vise versa
	
	bool IsWeapon(const FItem& Item);

	bool IsWeaponEquippedInHand(EEquipType Hand) ;

	bool IsEquipSlotExist(EEquipType SlotType) const;

	FName GetSocketName(const FItem& Item, EEquipType Slot, bool bIsSheathed = true) const;

	UPROPERTY(VisibleAnywhere, Category = "Unit")
	TMap<EEquipType, FEquipmentSlot> EquipmentSlots;


};
