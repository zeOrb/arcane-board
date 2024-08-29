// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArcanaClasses.h"
#include "UObject/NoExportTypes.h"
#include "Pathfinder.generated.h"


struct FLinkCost
{
	int32 Cost = 9999999;
	FVector2D CameFrom = FVector2D(-9999, 9999);
	FLinkCost() = default;
	FLinkCost operator=(const FLinkCost& Other) { Cost = Other.Cost; CameFrom = Other.CameFrom; return *this; };
	FLinkCost(int32 InCost, FVector2D InTile) : Cost(InCost), CameFrom(InTile) {};
	FLinkCost(int32 InCost) :Cost(InCost) {};
};


UCLASS()
class ARCANABOARD_API UPathfinder : public UObject
{
	GENERATED_BODY()
private:
	const int32 Infinity{ 999999 };

	class UGridSubsystem* GS;

	TMap<FVector2D, class ATile*>* TileMap;

private:
	class ATile* GetTile(FVector2D Coords) const;

	bool IsTileExist(FVector2D Coords) const;

	bool IsTileExistWithinList(FVector2D Coords, const TArray<FVector2D>& List) const;

	EDirection FindDirectionTo(FVector2D A, FVector2D B) const;

	bool GenerateAStarGraph(FVector2D Start, FVector2D Destination, const TArray<FVector2D>& WorkingMap, TMap<FVector2D, FLinkCost>& DijkstraGraph, const FPathRules& Rules);

	void GeneratePath(FVector2D Start, FVector2D Destination, const TMap<FVector2D, FLinkCost>& DijkstraGraph, TArray<FVector2D>& OutPath);

	int32 CalculateCost(FVector2D TileA, FVector2D TileB, const FPathRules& Rules) const;

	int32 HeuristicCost(const FVector2D& TileA, const FVector2D& TileB) const;


public:
	void Setup(TMap<FVector2D, class ATile*>* SourceMap);

	bool IsPathBlocked(FVector2D TileA, FVector2D TileB) const;

	bool BuildPathBetweenTiles(const FVector2D& TileA, const FVector2D& TileB, const TArray<FVector2D>& InMap, const FPathRules& Rules, TArray<FVector2D>& OutPath);
};
