// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArcanaClasses.h"
#include "UObject/NoExportTypes.h"
#include "District.generated.h"

UENUM(BlueprintType)
enum class EInitState :uint8
{
	ENonInitialized = 0,
	EInitLayoutRegion,
	EInitPlacement,
	EInitConnection,
	EInitVisuals
};
/**
 * 
 */
UCLASS()
class ARCANABOARD_API UDistrict : public UObject
{
	GENERATED_BODY()
private:
	EInitState InitState {EInitState::ENonInitialized};

public:
	FText Name;

	TArray<FVector2D> Coords;

	int32 ID;

	class UDistrictDataAsset* DataAsset;

	FVector2D LayoutPosition;

	FVector2D Origin;
	
	FVector2D RegionSize;

	FVector2D StartCoords;
	
	FVector2D FinishCoords;

	uint8 bHasGate :1;
	
	uint8 bNeedStairs :1;
	
	uint8 bHasStairs :1;

	
	FTileVisualSet* VisualSet;

	UDistrict* PrevDistrict;

	UDistrict* NextDistrict;

	FVector2D GetCenter() const;
	
	FVector2D GetEntryTile() const;
	
	FVector2D GetExitTile() const;

	int32 GetFloor() const;

	int32 GetID() const;

	bool RollForEncounter() const;

	FName GetRandomEncounter() const;

	bool InitBaseLayout(int32 InID, FVector2D InLayoutPosition, FVector2D RegionSize, const TArray<FVector2D>& InCoords);

	bool InitPlacement(FVector2D OriginPlacement);

	void InitVisualSet(FTileVisualSet* InVisualSet);

	void InitPrevConnection(UDistrict* District);

	void InitNext(UDistrict* District);

	bool HasEntry() const;

	bool HasExit() const;

	bool HasGate() const;

	bool NeedGate() const;

	bool NeedStairs() const;

	void InitGate(FVector2D FromTile, FVector2D ToTile);

	void GenerateBorderArray(TArray<FVector2D>& OutCoords);
	
};
