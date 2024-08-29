// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArcanaClasses.h"
#include "ArcanaBoardGameModeBase.generated.h"
class UParty;
class AUnit;

/* Helper struct for placing units in grid manner using slots*/
USTRUCT(BlueprintType)
struct ARCANABOARD_API FPlacementGrid
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grids|Placement")
	FVector2D Layout { FVector2D(3,3) };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grids|Placement")
	float Length { 25 };

	FVector2D SlotIdToLayout(int32 Id) const { return FVector2D(Id / (int32)Layout.Y, Id % (int32)Layout.Y); };

	FVector Step(int32 Id) const {return FVector(SlotIdToLayout(Id), 0);};

	FVector BorderOffset() const {return FVector(Length, Length, 0);}
};

UCLASS()
class ARCANABOARD_API AArcanaBoardGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	AArcanaBoardGameModeBase();

	UFUNCTION(BlueprintCallable)
	void InitCityActor(class ACity* InCity);

	UFUNCTION(BlueprintCallable)
	void InitPlayerPawn(FVector2D Coords);

protected:
	virtual void BeginPlay() override;

	class AUnit* SpawnUnit();

	static uint64 ItemGlobalUID;

private:
	class AArcanaController* PlayerController;

	class UGridSubsystem* GS;

	class UCombatSubsystem* CS;
	
	class ACity* City;

protected:
	UPROPERTY()
	TArray<UParty*> ActiveParties;

	UPROPERTY()
	UParty* PlayerParty;

public:
	class UDataTable* PartyDataTable;

	const FString PartyContext{ TEXT("Party Templates") };

	UParty* GetPlayerParty() const;

	FVector2D GetPlayerPosition() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FPlacementGrid PartyGrid;

	// UNIT MANAGEMENT
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unit")
	TSubclassOf<AUnit> UnitClass;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unit")
	TSubclassOf<class AEffectsManager> EffectsManagerClass;

	class AEffectsManager* EffectsManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	UDataTable* ItemsLibrary;


	bool RequestPartyAction(UParty* Party, FVector2D Tile, EPartyAction Action);

	bool CreatePartyFromTemplate(FName PartyTemplate, FVector2D SpawnTile /* Add Pool*/);

	bool CreateEncounterForDistrict(class UDistrict* District, FVector2D SpawnTile);

	void CreateItemsFromPool(const TArray<FTablePool>& ItemPools, TArray<FItem>& OutItemList) const;

	//bool CreateItemForUnit(FName ItemName, AUnit* Unit);

	bool TryPartyEncounter(UParty* Attacker, UParty* Defendant);

	bool TryPartyTravel(UParty* Party, FVector2D Destination);

	bool TryRevealTile(FVector2D Coords);

	void MovePartyToTile(UParty* Party, class ATile* Tile);

	void MovePartyToSpawn(UParty* Party, class ATile* Tile);

	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void OnMoveParty(UParty* Party, class ATile* Start, class ATile* Finish);

	bool IsPartyActionValid(UParty* Party, FVector2D Coords, EPartyAction Action);

	FVector MakeLocationForUnit(int32 Id, FVector Center);
};
