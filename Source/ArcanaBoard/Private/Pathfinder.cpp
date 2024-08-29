// Fill out your copyright notice in the Description page of Project Settings.


#include "Pathfinder.h"
#include "Tile.h"
#include "GridSubsystem.h"

int32 UPathfinder::HeuristicCost(const FVector2D& TileA, const FVector2D& TileB) const
{
	return (FMath::Abs(TileA.X - TileB.X) + FMath::Abs(TileA.Y - TileB.Y));
}

class ATile* UPathfinder::GetTile(FVector2D Coords) const
{
	ATile* const* Tile = TileMap->Find(Coords);
	return Tile ? *Tile : nullptr;
}

bool UPathfinder::IsTileExist(FVector2D Coords) const
{
	return TileMap->Contains(Coords);
}


bool UPathfinder::IsTileExistWithinList(FVector2D Coords, const TArray<FVector2D>& List) const
{
	return List.Contains(Coords);
}

EDirection UPathfinder::FindDirectionTo(FVector2D A, FVector2D B) const
{
	FVector2D Coords {B-A};
	if(Coords == FVector2D(1, 0))
		return ENorth;
	else if(Coords == FVector2D(-1, 0))
		return ESouth;
	else if(Coords == FVector2D(0, 1))
		return EEast;
	else if(Coords == FVector2D(0, -1))
		return EWest;

	return EDirection::EInvalidDirection;
}

bool UPathfinder::GenerateAStarGraph(FVector2D Start, FVector2D Destination, const TArray<FVector2D>& WorkingMap, TMap<FVector2D, FLinkCost>& OutAStarGraph, const FPathRules& Rules)
{
	auto SortByCost = [](const FLinkCost& A, const FLinkCost& B) { return A.Cost < B.Cost; };
	TMap<FVector2D, FLinkCost> Graph;
	TArray<FLinkCost> Anvil;
	FLinkCost CurrentLink;
	FVector2D CurrentTile;

	for (auto& TileCoords : WorkingMap) {
		Graph.Add(TileCoords, FLinkCost(Infinity));
		UE_LOG(LogTemp, Warning, TEXT("%f : %f"), TileCoords.X, TileCoords.Y)
	}
	UE_LOG(LogTemp, Error, TEXT("Start: %f : %f"), Start.X, Start.Y)
	Graph[Start] = { 0, Start };
	Anvil.HeapPush(Graph[Start], SortByCost);

	while (Anvil.Num() != 0)
	{
		Anvil.HeapPop(CurrentLink, SortByCost);
		CurrentTile = CurrentLink.CameFrom;
		if (CurrentTile == Destination) {
			break;
		}
		TArray<FVector2D> AdjacentCoords {CurrentTile + FVector2D(1, 0), CurrentTile + FVector2D(-1, 0), CurrentTile + FVector2D(0,1), CurrentTile + FVector2D(0,-1)};
		for (const FVector2D& AdjacentCoord : AdjacentCoords)
		{
			bool bIsTileValid = Rules.bUseEmpty ? true : IsTileExistWithinList(AdjacentCoord, WorkingMap);
			if (!bIsTileValid) continue;
			int32 NextCost = Graph[CurrentTile].Cost + CalculateCost(CurrentTile, AdjacentCoord, Rules);
			if (NextCost < Graph[AdjacentCoord].Cost)
			{
				Graph[AdjacentCoord] = { NextCost, CurrentTile };
				Anvil.HeapPush({ NextCost + HeuristicCost(AdjacentCoord, Destination), AdjacentCoord }, SortByCost);
			}
		}
	}
	OutAStarGraph = Graph;
	return true;
}

void UPathfinder::GeneratePath(FVector2D Start, FVector2D Destination, const TMap<FVector2D, FLinkCost>& DijkstraGraph, TArray<FVector2D>& Path)
{
	FVector2D CurrentTile = Destination;

	if (DijkstraGraph[CurrentTile].CameFrom != FVector2D(-9999, 9999) && DijkstraGraph[CurrentTile].Cost < Infinity)
	{
		Path.Push(CurrentTile);
		while (CurrentTile != Start)
		{
			Path.Push(DijkstraGraph[CurrentTile].CameFrom);
			CurrentTile = DijkstraGraph[CurrentTile].CameFrom;
		}
	}
	Algo::Reverse(Path);
}

int32 UPathfinder::CalculateCost(FVector2D TileA, FVector2D TileB, const FPathRules& Rules) const
{
	if (Rules.bUseAnyTile)
		return (IsTileExist(TileA) && IsTileExist(TileB)) ? 1 : Infinity;
	else if (Rules.bUseEmpty)
		return 1;
	else if (Rules.bIncludeWalls)
		return IsPathBlocked(TileA, TileB) ? Infinity : 1;

	return Infinity;
}

void UPathfinder::Setup(TMap<FVector2D, ATile*>* SourceMap)
{
	TileMap = SourceMap;
}

bool UPathfinder::IsPathBlocked(FVector2D TileA, FVector2D TileB) const
{
	auto A = GetTile(TileA); 
	auto B = GetTile(TileB);

	if(!B)
		return true;
	else if(A->HasStructureInDir(EStructure::EOuterWalls, FindDirectionTo(TileA, TileB)))
		return true;
	else if(A->HasStructureInDir(EStructure::EDistrictWalls, FindDirectionTo(TileA, TileB)))
		return true;
	
	return false;
}

bool UPathfinder::BuildPathBetweenTiles(const FVector2D& TileA, const FVector2D& TileB, const TArray<FVector2D>& InMap, const FPathRules& Rules, TArray<FVector2D>& OutPath)
{
	if (!Rules.bUseEmpty && !(IsTileExist(TileA) && IsTileExist(TileB)))
		return false;

	TMap<FVector2D, FLinkCost> Graph;
	GenerateAStarGraph(TileA, TileB, InMap, Graph, Rules);
	GeneratePath(TileA, TileB, Graph, OutPath);
	return OutPath.Num() != 0;
}
