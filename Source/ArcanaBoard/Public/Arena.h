// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArcanaClasses.h"
#include "Arena.generated.h"






DECLARE_DELEGATE_OneParam(FOnUpdateStateDelegate, ECombatState);
DECLARE_DELEGATE_OneParam(FOnReceiveActionDelegate, const FCombatAction&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBattleUiUpdate, const FBattleData&, Data, bool, bForceNextState);

UENUM(BlueprintType, Category = "Grid|Directions", meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum ETileFlags
{
	EInvalidFlag	= 0,
	EFree			= 1 << 0,
	EBlocked		= 1 << 1,
	EAttacker		= 1 << 2,
	EDefender		= 1 << 3,
	EUnconcious		= 1 << 4
};

enum class ECombatDir {
	EInvalid	= 0,
	ENorth		= 1 << 0,
	ENorthEast	= 1 << 1,
	EEast		= 1 << 2,
	ESouthEast	= 1 << 3,
	ESouth		= 1 << 4,
	ESouthWest	= 1 << 5,
	EWest		= 1 << 6,
	ENorthWest	= 1 << 7
};

USTRUCT(BlueprintType)
struct ARCANABOARD_API FCombatTile {
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "ETileFlags"), Category = "Combat Tile")
	int32 Flags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tile")
	FVector Location;

	FCombatTile() : Flags(0) {};

	FCombatTile(int32 InFlags, FVector InLoc) : Flags(InFlags), Location(InLoc) {};
};



UCLASS()
class ARCANABOARD_API UCombatEngine : public UObject
{
	GENERATED_BODY()
private:
	FBattleData BattleData;

	

	AArena* Arena;

	void StartBattle();

	void BeginRound();



	void RollInitative();

	void SortInitiative();

	int32 RollDice(int32 Bonus);

public:
	UCombatEngine();
	
	const FBattleData& GetBattleData() const;

	class AUnit* GetActiveUnit() const;

	bool IsOutOfTurns() const;

	class UCombatComponent* GetActiveCombatComp() const;

	bool IsPlayerTurn() const;

	void ActivatePlayerUnit();

	FOnUpdateStateDelegate OnUpdateStateHandle;

	FOnReceiveActionDelegate OnReceiveActionHandle;
	
	FOnReceiveActionDelegate OnFinishActionHandle;

	FCombatAction ActiveAction;

	FOnBattleUiUpdate OnUpdateUI;

	void Run(AArena* InArena);

	UFUNCTION(BlueprintCallable)
	void UpdateState(ECombatState State);

	void OnReceiveAction(const FCombatAction& Action);
	
	void OnFinishAction(const FCombatAction& Action);

};




UCLASS()
class ARCANABOARD_API AArena : public AActor
{
	GENERATED_BODY()
private:
	UPROPERTY(VisibleAnywhere, Category = BattleField)
	class UStaticMeshComponent* PlayerSelection;

	TArray<FVector2D> LastMovementRange;

	UPROPERTY()
	UCombatEngine* CEngine;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BattleField)
	TMap<FVector2D, FCombatTile> Map;

	TMap<ECombatDir, FVector2D> DirMap;
	
public:	
	
	UFUNCTION(BlueprintCallable)
	UCombatEngine* GetCombatEngine() const;

	void RollInitiative(class AUnit* Unit);

	AArena();
	   
	virtual void Tick(float DeltaTime) override;

	// BATTLE //
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	class UParty* Attacker;
	
	class UParty* Defender;

	class AArcanaController* Player;

	

	

	TMap<int32, TArray<int32>> PlacementIDs;

public:	
	void SendActionSignal(const FCombatAction& Action);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Action")
	void ExecuteMovement(const FCombatAction& Action);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Action")
	void ExecuteAttack(const FCombatAction& Action);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Action")
	void ExecuteMoveAttack(const FCombatAction& Action);

	UFUNCTION(BlueprintCallable, Category = "Combat|Action")
	void OnActionFinished(const FCombatAction& Action);



	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "BattleField|Camera")
	class ACineCameraActor* BattlefieldCam;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BattleField|Grid")
	FVector2D GridSize {1,1};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BattleField|Cell")
	float CellScale {0.45f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BattleField|Cell")
	class UStaticMesh* CellMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battlefield|Selection")
	float SelectionOffset;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BattleField|Grid")
	void GetGridSize(int32& X, int32& Y) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BattleField|Grid")
	float GetCellSize() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BattleField|Grid")
	FVector GetCellLocation(FVector2D Coords) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BattleField|Grid")
	FVector GetGridOrigin() const;

	class UParty* GetAttackers();

	class UParty* GetDefenders();

	class AArcanaController* GetPlayer();

	bool IsPlayerTurn() const;

	class UCombatComponent* GetActiveCC() const;

	class AUnit* GetActiveUnit() const;

	void GenerateMap();

	void TeleportPartiesToArena();

	FVector2D GetPlacementForUnit(class UParty* Party, int32 Num);

	FVector2D GetRandomCell() const;

	FVector2D GetRandomMove(class UCombatComponent* Comp) const;

	FVector2D GetCellForUnit(class UParty* Party, int32 Num);

	void SetUnitFacingAt(AUnit* Unit, FVector2D Tile);

	FVector2D GetClosestFacingCell(FVector2D A, FVector2D B) const;

	void InitializeArena(class UParty* InAttacker, class UParty* InDefender, class AArcanaController* Controller);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnPreReady();

	void OnReadyToBattle();

	UFUNCTION(BlueprintImplementableEvent)
	void OnShowMovementRange(const TArray<FVector2D>& Coords);

	UFUNCTION(BlueprintImplementableEvent)
	void OnClearMovementRange(const TArray<FVector2D>& Coords);

	UFUNCTION(BlueprintCallable)
	void OnNextStep();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowDiceRoll(class AUnit* Unit, int32 DiceResults, int32 DiceBonus, ECombatState Phase);

	void ClearMovementRange();

	void SetPlayerSelectionAtUnit(FVector2D Coords);

	void SetPlayerSelectionNull();

	UFUNCTION(BlueprintCallable, Category = "Combat|Unit")
	bool GetUnitAtCell(FVector2D Coords, class AUnit*& OutUnit);

	bool CanUnitMoveTo(AUnit* Unit, FVector2D Cell) const;

	bool IsCellHasFlags(FVector2D Cell, int32 Flags) const;

	bool AreCellsWithinRange(FVector2D A, FVector2D B, int32 Distance) const;

	int32 GetDistance(FVector2D A, FVector2D B) const;

	void GenerateMovementRange(class AUnit* Unit, TArray<FVector2D>& OutArray);

	void ShowMovementRangeForUnit(FVector2D Coords);
	
	// Grid temp fncs
	bool IsCellExist(FVector2D Coords) const;

	FVector2D GetNeighbourInDirection(FVector2D Origin, ECombatDir Dir) const;

	void FillWithNeighbours(FVector2D Origin, TArray<FVector2D>& OutCells);

	

	void FloodFill(FVector2D Coords, FVector2D Origin, int32 Distance, TArray<FVector2D>& Out);



};
