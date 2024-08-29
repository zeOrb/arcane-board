// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ArcanaClasses.h"
#include "GridSubsystem.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum ENeighbourType
{
	EInvalid		= 0,
	EEmptyTile		= 1 << 1,
	ELowerTile		= 1 << 2,
	EHigherTile		= 1 << 3,
	EPrevDistrict	= 1 << 4,
	ENextDistrict	= 1 << 5,
	EOtherDistrict	= 1 << 6,
	ESameDistrict	= 1 << 7,
	EStairsDistrict = 1 << 8
};

UCLASS()
class ARCANABOARD_API UGridSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	void Setup(class ACity* InCity);

protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	TMap<FVector2D, class ATile*> Tiles;


public:
	/*		  Tiles			*/		
	/* ==================== */
	
	void GenerateCombatGrid(FVector2D Size);

	void SetGameMode(class AArcanaBoardGameModeBase* InMode);

	class ACity* GetCity() const;

	const TMap<FVector2D, class ATile*>& GetTileMap() const;

	class UPathfinder* GetPathFinder();

	class ATile* GetTile(FVector2D Coords);

	bool IsTileExist(FVector2D Coords) const;

	inline bool IsTileLower(const class ATile* A, const class ATile* B) const;

	inline bool IsTileHigher(const class ATile* A, const class ATile* B) const;

	inline bool IsTileFromPrevDistrict(const class ATile* A, const class ATile* B) const;

	inline bool IsTileFromNextDistrict(const class ATile* A, const class ATile* B) const;

	inline FVector GetTileSize() const;

	inline float GetTileLength() const;

	bool IsPathBlocked(FVector2D A, FVector2D B) ;

	/*	  Coordinates		*/
	/* ==================== */
	inline FVector GridSnap(const FVector Location) const;

	inline FVector2D LocationToCoords(FVector Location) const;

	inline FVector CoordsToLocation(FVector2D Coords) const;

	
	FTransform MakeWallTransform(FVector2D Coords, EDirection Direction, int32 Floor);

	FTransform MakeTileTransform(FVector2D Coords, const struct FTileConstructRules& TileRules);

	/*	  Construction		*/
	/* ==================== */
	void SpawnBorderWall(FVector2D Coords, EDirection Direction, int32 Floor);

	void SpawnGates(FVector2D Coords, EDirection Direction, int32 Floor);
	
	void SpawnStairs(FVector2D Coords, EDirection Direction, int32 Floor);

	void SpawnTower(FVector2D Coords, EDirection Direction, int32 Floor);

	class ATile* SpawnTileActor(FVector2D Coords);

	bool SpawnTileForDistrict(FVector2D Coords, class UDistrict* District);

	
	/*	  LOWLEVEL ARCH		*/
	/* ==================== */
	void GenerateRectangle(FVector2D Origin, FVector2D Size, TArray<FVector2D>& OutCoords);


	/*ADJACENCY & NEIGHBOURS*/
	/* ==================== */
	FVector2D GetNeighbourInDirection(FVector2D Coords, EDirection Dir);

	TArray<FVector2D> GetNeighbours(FVector2D Coords);

	int32 MakeNeighbourBitfield(FVector2D A, EDirection Dir);

	EDirection FindDirectionTo(FVector2D A, FVector2D B);

	EDirection FindDirectionFrom(FVector2D A, FVector2D B);

	int32 GetDistanceBetweenTiles(FVector2D A, FVector2D B) const;

	/*	  UTILITY	*/
	bool IsEven(int32 Number) const;

	/*====================== BIT MASK UTILITY ===========================*/
		UFUNCTION(BlueprintCallable, Category = "Grid|Bitflags")
	bool AddFlags(UPARAM(ref) int32& Bitmask, int32 Flags);

	UFUNCTION(BlueprintCallable, Category = "Grid|Bitflags")
	bool RemoveFlags(UPARAM(ref) int32& Bitmask, int32 Flags);

	UFUNCTION(BlueprintCallable, Category = "Grid|Bitflags")
	bool EqualFlag(UPARAM(ref) int32& Bitmask, int32 Flags);

	UFUNCTION(BlueprintCallable, Category = "Grid|Bitflags")
	bool HasFlags(UPARAM(ref) int32& Bitmask, int32 Flags);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid|Bitflags")
	int32 Shift(int32 Bitmask, int32 BitmaskLength = 4, bool bClockwise = true);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid|Bitflags")
	int32 RotateBitmask(int32 Bitmask, int32 BitmaskLength = 4, bool bClockwise = true, int32 Count = 1);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid|Bitflags")
	int32 InvertBitmask(int32 Bitmask, int32 BitmaskLength = 4);


	// GLOBAL TEMP
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid|Bitflags")
	bool CanPlayerTravelToTile(class ATile* Tile) ;

private:
	class ACity* City;

	class UWorld* World;

	class AArcanaBoardGameModeBase* GameMode;

	UPROPERTY()
	class UPathfinder* PathFinder;

	TMap<EDirection, FVector2D> DirectionsMap;

	TMap<FVector2D, EDirection> InverseDirectionsMap;

	TMap<EDirection, float> WallRotationMap;

	// DEBUG
	enum class EDebugBrush :uint8
	{
		ENone = 0,
		ETile = 1,
		ERegion
	};

	struct FDebugDrawingBrush
	{
		FColor Color;
		float LifeTime;
		float Thickness;
		bool bIsPersistent;

		FDebugDrawingBrush() {};

		FDebugDrawingBrush(FColor InCol, float InLife, float InThick, bool InPersistent)
			: Color(InCol)
			, LifeTime(InLife)
			, Thickness(InThick)
			, bIsPersistent(InPersistent)
		{};
	};

	TMap<EDebugBrush, FDebugDrawingBrush> DebugBrushes;

public:
	void DrawDebugCoords(FVector2D Coords);

	void DrawDebugTile(FVector Location);

	void DrawDebugDistrict(FVector2D Origin, FVector2D RegionSize, FVector2D Center);

	void DrawDebugPath(const FPath& Path);
	
	void DrawDebugConnection(FVector Start, FVector End, int32 Floor);

	const FDebugDrawingBrush& GetTileBrush() const;

	const FDebugDrawingBrush& GetRegionBrush() const;

};
