// Fill out your copyright notice in the Description page of Project Settings.

#include "GridSubsystem.h"
#include "ArcanaClasses.h"
#include "ArcanaBoard/ArcanaBoardGameModeBase.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Pathfinder.h"
#include "District.h"
#include "City.h"
#include "Tile.h"

void UGridSubsystem::Setup(ACity* InCity)
{
	City = InCity;
	World = City->GetWorld();
	PathFinder = NewObject<UPathfinder>();
	PathFinder->Setup(&Tiles);
}

void UGridSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DebugBrushes.Add(EDebugBrush::ETile, FDebugDrawingBrush(FColor::Red, 8, 2, true));
	DebugBrushes.Add(EDebugBrush::ERegion, FDebugDrawingBrush(FColor::Yellow, 8, 8, true));

	DirectionsMap.Add(EDirection::ENorth, FVector2D(1, 0));
	DirectionsMap.Add(EDirection::EEast,  FVector2D(0, 1));
	DirectionsMap.Add(EDirection::ESouth, FVector2D(-1, 0));
	DirectionsMap.Add(EDirection::EWest,  FVector2D(0, -1));

	InverseDirectionsMap.Add(FVector2D(1, 0),	EDirection::ENorth);
	InverseDirectionsMap.Add(FVector2D(0, 1),	EDirection::EEast);
	InverseDirectionsMap.Add(FVector2D(-1, 0),	EDirection::ESouth);
	InverseDirectionsMap.Add(FVector2D(0, -1),	EDirection::EWest);

	WallRotationMap.Add(EDirection::ENorth, 0);
	WallRotationMap.Add(EDirection::EEast, 90);
	WallRotationMap.Add(EDirection::ESouth,180);
	WallRotationMap.Add(EDirection::EWest, 270);
}

void UGridSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGridSubsystem::GenerateCombatGrid(FVector2D Size)
{
	TArray<FVector2D> Coords;
	GenerateRectangle(FVector2D::ZeroVector, Size, Coords);
}

void UGridSubsystem::SetGameMode(class AArcanaBoardGameModeBase* InMode)
{
	GameMode = InMode;
}

class ACity* UGridSubsystem::GetCity() const
{
	return City;
}

const TMap<FVector2D, class ATile*>& UGridSubsystem::GetTileMap() const
{
	return Tiles;
}

class UPathfinder* UGridSubsystem::GetPathFinder()
{
	return PathFinder;
}

ATile* UGridSubsystem::GetTile(FVector2D Coords)
{
	ATile* const* Tile = Tiles.Find(Coords);
	return Tile ? *Tile : nullptr;
}

bool UGridSubsystem::IsTileExist(FVector2D Coords) const
{
	return Tiles.Contains(Coords);
}

bool UGridSubsystem::IsTileLower(const ATile* A, const ATile* B) const
{
	return A->Floor < B->Floor;
}

bool UGridSubsystem::IsTileHigher(const  ATile* A, const  ATile* B) const
{
	return A->Floor > B->Floor;
}

bool UGridSubsystem::IsTileFromPrevDistrict(const  ATile* A, const  ATile* B) const
{
	if(City->Districts[A->DistrictID]->HasEntry())
		return B->DistrictID == City->Districts[A->DistrictID]->PrevDistrict->ID;
	else
		return false;
}

bool UGridSubsystem::IsTileFromNextDistrict(const  ATile* A, const  ATile* B) const
{
	if(City->Districts[A->DistrictID]->HasExit())
		return B->DistrictID == City->Districts[A->DistrictID]->NextDistrict->ID;
	return false;
}

inline FVector UGridSubsystem::GetTileSize() const
{
	return City->TileConstructRules.GetTileSize();
}

float UGridSubsystem::GetTileLength() const
{
	return (City->TileConstructRules.TileExtents.X + City->TileConstructRules.TileSpacing) * City->TileConstructRules.TileScale.X;
}

bool UGridSubsystem::IsPathBlocked(FVector2D A, FVector2D B) 
{
	ATile* TileA = GetTile(A);
	ATile* TileB = GetTile(B);

	if (!TileB || !TileA)
		return true;
	else if(FindDirectionTo(A,B) == EInvalidDirection)
		return true;
	else if (TileA->HasStructureInDir(EStructure::EOuterWalls, FindDirectionTo(A, B))) {
		return true;
	}
	else if (TileA->HasStructureInDir(EStructure::EDistrictWalls, FindDirectionTo(A, B))) {
		return true;
		}
	return false;
}

inline FVector UGridSubsystem::GridSnap(const FVector Location) const
{
	return { FMath::GridSnap(Location.X, GetTileSize().X), FMath::GridSnap(Location.Y, GetTileSize().Y), Location.Z };
}

inline FVector2D UGridSubsystem::LocationToCoords(FVector Location) const
{
	Location = GridSnap(Location) / GetTileSize();
	return FVector2D((int32)Location.X, (int32)Location.Y);
}

inline FVector UGridSubsystem::CoordsToLocation(FVector2D Coords) const
{
	return FVector(Coords.X, Coords.Y, 0) * GetTileSize();
}

FTransform UGridSubsystem::MakeWallTransform(FVector2D Coords, EDirection Direction, int32 Floor)
{
	FTransform Transform;
	Transform.SetLocation(CoordsToLocation(Coords));
	Transform.AddToTranslation(FVector(0,0, Floor * City->TileConstructRules.FloorDistance));
	Transform.SetRotation(FRotator(0, WallRotationMap[Direction], 0).Quaternion());
	Transform.SetScale3D(FVector::OneVector);
	return Transform;
}

FTransform UGridSubsystem::MakeTileTransform(FVector2D Coords, const FTileConstructRules& TileRules)
{
	FTransform OutTransform {};
	OutTransform.SetTranslation(CoordsToLocation(Coords));
	OutTransform.SetScale3D(City->TileConstructRules.TileScale);
	OutTransform.SetRotation(FQuat::Identity);
		
	if (TileRules.bIncludeActorOffset) {
		OutTransform.AddToTranslation(City->GetActorLocation());
	}

	if (TileRules.bIncludeRandomHeightOffset) {
		float RandomHeightOffset = (FMath::RandHelper(TileRules.HeightOffsetMaxStep) * TileRules.HeightOffsetStepSize);
		RandomHeightOffset *= City->TileConstructRules.TileScale.Z;
		OutTransform.AddToTranslation(FVector(0, 0, RandomHeightOffset));
	}

	if (TileRules.bIncludeRandomSlope) {
		OutTransform.SetRotation(FRotator(
			FMath::FRandRange(-TileRules.Slope.X, TileRules.Slope.X), 
			FMath::FRandRange(-TileRules.Slope.Y, TileRules.Slope.Y) + 90 * FMath::RandHelper(5),
			FMath::FRandRange(-TileRules.Slope.Z, TileRules.Slope.Z)).Quaternion());
	}
	return OutTransform;
}

void UGridSubsystem::SpawnBorderWall(FVector2D Coords, EDirection Direction, int32 Floor)
{
	FTransform Transform {MakeWallTransform(Coords, Direction, Floor)};
	City->CityWallsInstancedComp->AddInstanceWorldSpace(Transform);
}

void UGridSubsystem::SpawnGates(FVector2D Coords, EDirection Direction, int32 Floor)
{
	FTransform Transform{ MakeWallTransform(Coords, Direction, Floor) };
	City->CityGatesInstancedComp->AddInstanceWorldSpace(Transform);
}

void UGridSubsystem::SpawnStairs(FVector2D Coords, EDirection Direction, int32 Floor)
{
	FTransform Transform{ MakeWallTransform(Coords, Direction, Floor) };
	City->CityStairsInstancedComp->AddInstanceWorldSpace(Transform);
}

void UGridSubsystem::SpawnTower(FVector2D Coords, EDirection Direction, int32 Floor)
{
	FTransform Transform{ MakeWallTransform(Coords, Direction, Floor) };
	City->CityTowerInstancedComp->AddInstanceWorldSpace(Transform);
}

ATile* UGridSubsystem::SpawnTileActor(FVector2D Coords)
{
	if (IsTileExist(Coords))
		return false;

	FTransform BaseTileTransform{ MakeTileTransform(Coords, City->TileConstructRules) };
	
	if (ATile* Tile = World->SpawnActor<ATile>(City->TileConstructRules.TileBaseClass, BaseTileTransform))
	{
		//Tile->OnTileCreationFinish(Tile->GetTileData(), Tile->GetGameplayData());
		Tiles.Add(Coords, Tile);
		Tile->Coords = Coords;

		return Tile;
	}
	return nullptr;
}

bool UGridSubsystem::SpawnTileForDistrict(FVector2D Coords, UDistrict* District)
{
	if (ATile* Tile = SpawnTileActor(Coords))
	{
		Tile->MeshComp->SetStaticMesh(District->VisualSet->Mesh);
		Tile->MeshComp->SetMaterial(0, District->VisualSet->Material);
		Tile->AddActorWorldOffset(FVector(0, 0, District->GetFloor() * City->TileConstructRules.FloorDistance));
		Tile->DistrictID = District->GetID();
		Tile->Floor = District->GetFloor();
	}
	return true;
}

void UGridSubsystem::GenerateRectangle(FVector2D Origin, FVector2D Size, TArray<FVector2D>& OutCoords)
{
	for (int32 X = Origin.X; X < Origin.X + Size.X; X++)
	{
		for (int32 Y = Origin.Y; Y < Origin.Y + Size.Y; Y++)
		{
			OutCoords.Add(FVector2D(X,Y));
		}
	}
}

FVector2D UGridSubsystem::GetNeighbourInDirection(FVector2D Coords, EDirection Dir)
{
	return Coords + DirectionsMap[Dir];
}

TArray<FVector2D> UGridSubsystem::GetNeighbours(FVector2D Coords)
{
	TArray<FVector2D> Neigbhours;
	for (int32 i = 1 << 1; i <= 1 << 4; i <<= 1) {
		Neigbhours.Add(GetNeighbourInDirection(Coords, (EDirection)i));
	}
	return Neigbhours;
}

int32 UGridSubsystem::MakeNeighbourBitfield(FVector2D A, EDirection Dir)
{
	ATile* TileA	{GetTile(A)};
	ATile* TileB	{GetTile(GetNeighbourInDirection(A, Dir))};
	int32 Bitmask {0};

	if(!TileB)
		AddFlags(Bitmask, EEmptyTile);
	else
	{
		if(IsTileLower(TileB, TileA))
			AddFlags(Bitmask, ELowerTile);
		if(IsTileHigher(TileB, TileA))
			AddFlags(Bitmask, EHigherTile);
		if(IsTileFromPrevDistrict(TileA, TileB))
			AddFlags(Bitmask, EPrevDistrict);
		if(IsTileFromNextDistrict(TileA, TileB))
			AddFlags(Bitmask, ENextDistrict);
	}
	return Bitmask;
}

EDirection UGridSubsystem::FindDirectionTo(FVector2D A, FVector2D B)
{
	return InverseDirectionsMap.Contains(B - A) ? InverseDirectionsMap[(B - A)] : EDirection::EInvalidDirection;
}

EDirection UGridSubsystem::FindDirectionFrom(FVector2D A, FVector2D B)
{
	return InverseDirectionsMap.Contains(A-B) ? InverseDirectionsMap[(A - B)] : EDirection::EInvalidDirection;
}

int32 UGridSubsystem::GetDistanceBetweenTiles(FVector2D A, FVector2D B) const
{
	return FMath::CeilToInt(FVector2D::Distance(A,B));
}

bool UGridSubsystem::IsEven(int32 Number) const
{
	return (Number % 2 == 0);
}

// BITMASK
bool UGridSubsystem::AddFlags(int32& Bitmask, int32 Flags)
{
	if (Bitmask & Flags) return false;
	Bitmask |= Flags;
	return true;
}

bool UGridSubsystem::RemoveFlags(int32& Bitmask, int32 Flags)
{
	Bitmask &= ~Flags;
	return true;
}

bool UGridSubsystem::EqualFlag(int32& Bitmask, int32 Flags)
{
	return (Bitmask & Flags) == Flags;
}

bool UGridSubsystem::HasFlags(int32& Bitmask, int32 Flags)
{
	return (Bitmask & Flags) != 0;
}

int32 UGridSubsystem::Shift(int32 Bitmask, int32 BitmaskLength, bool bClockwise /*= true*/)
{
	int32 TempMask;

	if (bClockwise)
	{
		TempMask = Bitmask & (1 << BitmaskLength);
		Bitmask <<= 1;
		Bitmask |= TempMask >> (BitmaskLength - 1);
		Bitmask &= ~(1 << (BitmaskLength + 1));
	}
	else
	{
		TempMask = Bitmask & (1 << 1);
		Bitmask >>= 1;
		Bitmask |= TempMask << (BitmaskLength - 1);
		Bitmask &= ~(1);
	}
	return Bitmask;
}

int32 UGridSubsystem::RotateBitmask(int32 Bitmask, int32 BitmaskLength, bool bClockwise /*= true*/, int32 Count /*= 1 */)
{
	// O: Too many copies, maybe keep optimized internal shift function for C++ and allow BP to use only Rotate?
	for (int32 i = 0; i < Count; i++)
		Bitmask = Shift(Bitmask, BitmaskLength, bClockwise);
	return Bitmask;
}

int32 UGridSubsystem::InvertBitmask(int32 Bitmask, int32 BitmaskLength)
{
	return RotateBitmask(Bitmask, BitmaskLength, true, BitmaskLength / 2);
}

bool UGridSubsystem::CanPlayerTravelToTile(ATile* Tile) 
{
	if(GameMode->GetPlayerParty() == nullptr)
		return false;
	
	return  !IsPathBlocked(GameMode->GetPlayerPosition(), Tile->GetCoords());
	
}

void UGridSubsystem::DrawDebugCoords(FVector2D Coords)
{
	FVector Center = CoordsToLocation(Coords);
	DrawDebugBox(World, Center, GetTileSize()/2, GetTileBrush().Color, true, GetTileBrush().LifeTime, 0, GetTileBrush().Thickness);
}

void UGridSubsystem::DrawDebugTile(FVector Location)
{
	DrawDebugSolidBox(World, Location, GetTileSize() / 2, GetTileBrush().Color, true, GetTileBrush().LifeTime, 0);
}

void UGridSubsystem::DrawDebugDistrict(FVector2D Origin, FVector2D RegionSize, FVector2D Center)
{
	FVector CenterVec = CoordsToLocation(Center) - GetTileSize()/2;
	FVector RegionVec = GetTileSize() * (FVector(RegionSize.X, RegionSize.Y, 5)/2);
	DrawDebugBox(World, CenterVec, RegionVec, FColor::MakeRandomColor(), true, GetRegionBrush().LifeTime, 0, GetRegionBrush().Thickness);
}

void UGridSubsystem::DrawDebugPath(const FPath& Path)
{
	ATile* Tile {nullptr};
	ATile* NextTile {nullptr};
	for (int32 i = 0; i < Path.FinishedPath.Num(); i++)
	{
		if (i != Path.FinishedPath.Num() - 1)
		{
			Tile = GetTile(Path.FinishedPath[i]);
			NextTile = GetTile(Path.FinishedPath[i+1]);
			DrawDebugDirectionalArrow(World, Tile->GetActorLocation()+FVector(0,0,50),
				NextTile->GetActorLocation() + FVector(0, 0, 50), 40, FColor::Yellow, true, 16, 0, 8);
		}
	}
	DrawDebugSolidBox(World, GetTile(Path.Start)->GetActorLocation() + FVector(0, 0, 50), FVector(20), FColor::Red, true, 8, 0);
	DrawDebugSolidBox(World, GetTile(Path.Finish)->GetActorLocation() + FVector(0, 0, 50), FVector(20), FColor::Green, true, 8, 0);
}

void UGridSubsystem::DrawDebugConnection(FVector Start, FVector End, int32 Floor)
{
	DrawDebugDirectionalArrow(World, Start+FVector(0,0, 200 + (Floor*100)), End+FVector(0,0, 200 + (Floor*100)),50, FColor::MakeRandomColor(), true, 16, 0, 4);
	
}



const UGridSubsystem::FDebugDrawingBrush& UGridSubsystem::GetTileBrush() const
{
	return DebugBrushes[EDebugBrush::ETile];
}

const UGridSubsystem::FDebugDrawingBrush& UGridSubsystem::GetRegionBrush() const
{
	return DebugBrushes[EDebugBrush::ERegion];
}
