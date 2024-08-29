// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArcanaClasses.h"
#include "City.generated.h"


class UInstancedStaticMeshComponent;
UCLASS()
class ARCANABOARD_API ACity : public AActor
{
	GENERATED_BODY()
	
public:	
	ACity();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	FTileVisualSet DefaultTileVisuals;
	
	void CreateDistrict(FVector2D LayoutPosition);

	bool CreateDistricts();

	bool PlaceDistricts();

	void OffsetOddDistricts();

	void SpawnDistrictsTiles();

	void SpawnDistrictArchitecture();

	void ConnectDistricts();

	void BackfillConnections();

	void BakeDistricts();

	void TEMP_DEBUG_ENTRIES();

	FTileVisualSet* GetRandomVisualSet() const;

public:
	inline int32 DistrictCount() const;

	inline bool IsFirstDistrict(const class UDistrict& District) const;
	
	inline bool IsLastDistrict(const class UDistrict& District) const;

	inline bool IsFirstColumn(const class UDistrict& District) const;

	inline bool IsFirstRow(const class UDistrict& District) const;

	inline bool IsLastColumn(const class UDistrict& District) const;

	inline bool IsLastRow(const class UDistrict& District) const;
	
	inline bool IsEvenRow(const class UDistrict& District) const;

	inline bool IsElevateDistrict(const class UDistrict& District) const;
	
	inline bool IsHigherDistrict(const class UDistrict& DistrictA, const class UDistrict& DistrictB) const;

	// District Accessors
	int32 PositionToID(FVector2D LayoutPos) const;

	UDistrict* DistrictFromTile(ATile* Tile);

	UDistrict* GetDistrictById(int32 ID) const;
	
	UDistrict* GetDistrictByLayout(FVector2D LayoutPos ) const;

	UDistrict* GetDistrictByRandom() const;

	UDistrict* GetDistrictByRowRandom(int32 Row) const;
	
	UDistrict* GetDistrictByColRandom(int32 Col) const;

	class UDistrict* GetLastDistrict() const;
	
	class UDistrict* GetFirstDistrict() const;

public:	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "City|Battlefield")
	class AArena* Battlefield;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile|Rules")
	FTileConstructRules TileConstructRules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile|Data")
	UDataTable* TileVisualsTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Districts")
	FVector2D DistrictsLayout;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City|Districts")
	TArray<class UDistrict*> Districts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Regions")
	FVector2D RegionSizeX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Regions")
	FVector2D RegionSizeY;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "City|Regions")
	FVector2D OddDistrictOffsetRow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City|Architecture")
	UInstancedStaticMeshComponent* CityWallsInstancedComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City|Architecture")
	UInstancedStaticMeshComponent* CityGatesInstancedComp;
	 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City|Architecture")
	UInstancedStaticMeshComponent* CityTowerInstancedComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City|Architecture")
	UInstancedStaticMeshComponent* CityStairsInstancedComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "City|Districts")
	TArray<class UDistrictDataAsset*> DistrictsData;

	FVector2D GenerateRegionSize() const;

	// Path building for pathfinding/ path visualization
	bool BuildPathWithinDistrict(FPath& OutPath, UDistrict* District);

	bool BuildPathWithinFloor(FPath& OutPath, int32 FloorNum);

	//bool BuildPathWithinCity(FPath& OutPath);

public:
	const FTileVisualSet& GetDefaultVisuals() const;
	
	
public: 
	void Build();
	

private:
	class UGridSubsystem* GS;

	bool InitializeRules();

	bool InitializeGridSubsystem();
};
