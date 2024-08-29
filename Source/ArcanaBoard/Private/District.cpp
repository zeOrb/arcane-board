// Fill out your copyright notice in the Description page of Project Settings.


#include "District.h"
#include "DistrictDataAsset.h"

FVector2D UDistrict::GetCenter() const
{
	return FVector2D((int32)RegionSize.X/2, (int32)RegionSize.Y/2) + Origin;
}

FVector2D UDistrict::GetEntryTile() const
{
	return StartCoords;
}

FVector2D UDistrict::GetExitTile() const
{
	return FinishCoords;
}

int32 UDistrict::GetFloor() const
{
	return (int32)LayoutPosition.X;
}

int32 UDistrict::GetID() const
{
	return ID;
}

bool UDistrict::RollForEncounter() const
{
	return FMath::RandRange(0, 100) <= DataAsset->Data.PartiesPool.MaxAmount;
}

FName UDistrict::GetRandomEncounter() const
{
	return DataAsset->Data.PartiesPool.GetWeightedHandle().RowName;
}

bool UDistrict::InitBaseLayout(int32 InID, FVector2D InLayoutPosition, FVector2D InRegionSize, const TArray<FVector2D>& InCoords)
{
	if(InitState != EInitState::ENonInitialized)
		return false;

	Origin = FVector2D::ZeroVector;
	LayoutPosition = InLayoutPosition;
	RegionSize = InRegionSize;
	ID = InID;
	Coords = InCoords;

	InitState = EInitState::EInitLayoutRegion;
	return true;
}

bool UDistrict::InitPlacement(FVector2D OriginPlacement)
{
	if(InitState != EInitState::EInitLayoutRegion)
		return false;
	
	Origin = OriginPlacement;
	InitState = EInitState::EInitPlacement;
	return true;
	
}

void UDistrict::InitVisualSet(FTileVisualSet* InVisualSet)
{
	VisualSet = InVisualSet;
}

void UDistrict::InitPrevConnection(UDistrict* District)
{
	PrevDistrict = District;
}

void UDistrict::InitNext(UDistrict* District)
{
	NextDistrict = District;
}


bool UDistrict::HasEntry() const
{
	return PrevDistrict != nullptr;
}

bool UDistrict::HasExit() const
{
	return NextDistrict != nullptr;
}


bool UDistrict::HasGate() const
{
	return bHasGate;
}

bool UDistrict::NeedGate() const
{
	return !bHasGate && FMath::RandRange(0.0f, 1.0f) >= 0.67f;
}

bool UDistrict::NeedStairs() const
{
	return !bHasStairs && FMath::RandRange(0.0f, 1.0f) >= 0.67f;
}

void UDistrict::InitGate(FVector2D FromTile, FVector2D ToTile)
{
	PrevDistrict->FinishCoords = FromTile;
	StartCoords = ToTile;
	bHasGate = true;
}

void UDistrict::GenerateBorderArray(TArray<FVector2D>& OutCoords)
{
	// Top, Bottom, Left, Right
	for (int32 X = 0; X < 1; X++)
	{
		for (int32 Y = 0; Y < 1; Y++)
		{

		}
	}
}
