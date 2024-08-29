// Fill out your copyright notice in the Description page of Project Settings.


#include "City.h"
#include "Tile.h"
#include "District.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Pathfinder.h"
#include "GridSubsystem.h"

// Sets default values
ACity::ACity()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp")));

	CityWallsInstancedComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("City Walls Comp"));
	CityWallsInstancedComp->SetupAttachment(GetRootComponent());
	CityGatesInstancedComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("City Gates Comp"));
	CityGatesInstancedComp->SetupAttachment(GetRootComponent());
	CityTowerInstancedComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("City Tower Comp"));
	CityTowerInstancedComp->SetupAttachment(GetRootComponent());
	CityStairsInstancedComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("City Stairs Comp"));
	CityStairsInstancedComp->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void ACity::BeginPlay()
{
	Super::BeginPlay();

	if(!InitializeGridSubsystem())
		return;

	if (!InitializeRules())
		return;

	


// 	FPath Path(Districts[0]->StartCoords, Districts[1]->FinishCoords);
// 	BuildPathWithinFloor(Path, 0);
// 	GS->DrawDebugPath(Path);

}

bool ACity::CreateDistricts()
{
	if(DistrictsLayout.X <= 0 || DistrictsLayout.Y <= 0)
		return false;

	for (int32 DistrictColumn = 0; DistrictColumn < DistrictsLayout.X; DistrictColumn++) {
		for (int32 DistrictRow = 0; DistrictRow < DistrictsLayout.Y; DistrictRow++) {
			CreateDistrict(FVector2D(DistrictColumn, DistrictRow));
		}
	}
	return true;
}

bool ACity::PlaceDistricts()
{
	if(Districts.Num() == 0)
		return false;

	for (int32 i = 0; i < Districts.Num(); i++)
	{
		if (i != 0)
		{
			UDistrict* Current{ Districts[i] };
			UDistrict* Left{ Districts[i - 1] };
			UDistrict* Bottom{ IsFirstRow(*Current) ? nullptr : Districts[i - DistrictsLayout.Y] };
			FVector2D Origin{ FVector2D::ZeroVector };

			if (IsFirstRow(*Current)) {
				Origin.X = 0;
				Origin.Y = Left->Origin.Y + Left->RegionSize.Y;
			}
			else {
				Origin.X = Bottom->Origin.X + Bottom->RegionSize.X;
				Origin.Y = IsFirstColumn(*Current) ? Origin.Y = Bottom->Origin.Y : Origin.Y = Left->Origin.Y + Left->RegionSize.Y;
			}
			Current->InitPlacement(Origin);
		}
	}
	return true;
}

void ACity::OffsetOddDistricts()
{
	for (int32 i = 0; i < Districts.Num(); i++)
	{
		UDistrict& Current {*Districts[i]};
		if (!IsEvenRow(Current))
		{
			Current.Origin.Y += FMath::RandRange((int32)OddDistrictOffsetRow.X, (int32)OddDistrictOffsetRow.Y);
		}
		else if (IsFirstRow(Current))
		{
			Current.Origin.X += FMath::RandRange((int32)OddDistrictOffsetRow.X/2, (int32)OddDistrictOffsetRow.Y/2);
		}
	}
}

void ACity::SpawnDistrictsTiles()
{
	for (int32 i = 0; i < Districts.Num(); i++) {
		UDistrict* District = Districts[i];
		TArray<FVector2D> RemovedCoords;
		for (const FVector2D& Coords : District->Coords) {
			if(!GS->SpawnTileForDistrict(District->Origin + Coords, District))
				RemovedCoords.Add(Coords);
		}
		for (const FVector2D& Coords : RemovedCoords)
			District->Coords.Remove(Coords);
	}
}

void ACity::ConnectDistricts()
{
	for (int32 i = 0; i < Districts.Num(); i++)
	{
		UDistrict& Current { *Districts[i]};

		if ((IsLastColumn(Current) && IsEvenRow(Current) || (IsFirstColumn(Current) && !IsEvenRow(Current)))) {
			if(!IsLastRow(Current))
				Current.InitNext(Districts[i + DistrictsLayout.Y]);
		}
		else if (!IsLastDistrict(Current)) {
			Current.InitNext(Districts[i + (IsEvenRow(Current) ? +1: -1)]);
		}
	}
}


	


void ACity::BackfillConnections()
{
	for (int32 i = 0; i < Districts.Num(); i++)
	{
		UDistrict* Current {Districts[i]};
		if (!IsLastDistrict(*Current))
		{
			UDistrict* Next {Current->NextDistrict};
			Next->InitPrevConnection(Current);
		}
	}
}



void ACity::BakeDistricts()
{
	for (UDistrict* District : Districts)
	{
	for(FVector2D& Coord : District->Coords)
		Coord += District->Origin;
	}
}

void ACity::TEMP_DEBUG_ENTRIES()
{
	for (UDistrict* District : Districts)
	{
		if(District->StartCoords != FVector2D::ZeroVector)
			GS->DrawDebugTile(GS->GetTile(District->StartCoords)->GetActorLocation());
		if(District->FinishCoords!= FVector2D::ZeroVector)
			GS->DrawDebugTile(GS->GetTile(District->FinishCoords)->GetActorLocation());
	}
}

void ACity::SpawnDistrictArchitecture()
{
	for (int32 n = 0; n < Districts.Num(); n++) {
		UDistrict* District = Districts[n];
		for (FVector2D RawCoords : District->Coords)
		{
			FVector2D Coords = District->Origin + RawCoords;
			
			for (int32 b = 1 << 1; b <= 1 << 4; b <<= 1) 
			{
				EDirection Dir			{ (EDirection)b};
				ATile* Neighbour		{ GS->GetTile(GS->GetNeighbourInDirection(Coords, Dir))};
				ATile* Current			{ GS->GetTile(Coords)};
				int32 Bitfield			{ GS->MakeNeighbourBitfield(Coords, Dir)};

				if (GS->HasFlags(Bitfield, EEmptyTile | ELowerTile)) {

					for (int32 z = 0; z <= Current->GetFloor(); z++) {
						GS->SpawnBorderWall(Coords, Dir, z);
						if (Neighbour)
							Neighbour->AddStructure(EStructure::EOuterWalls, GS->InvertBitmask(b));
						Current->AddStructure(EStructure::EOuterWalls, b);
					}

					if(Current->CountStructures(EStructure::EOuterWalls) >= 2)
					{
						FVector2D DiagA = (Coords + FVector2D(-1, -1));
						FVector2D DiagB = (Coords + FVector2D(-1, 0));
						FVector2D DiagC = (Coords + FVector2D(1, 0));
						FVector2D DiagD = (Coords + FVector2D(1, -1));

						if (!GS->IsTileExist(DiagA) || !GS->IsTileExist(DiagB))
						{
							GS->SpawnTower(Coords, IsEvenRow(*Districts[Current->DistrictID]) ? Dir : Dir, Current->GetFloor());
						}
						else if (!GS->IsTileExist(DiagC) || !GS->IsTileExist(DiagD))
						{
							GS->SpawnTower(Coords, IsEvenRow(*Districts[Current->DistrictID]) ? (EDirection)GS->Shift((int32)Dir,4,false) : Dir, Current->GetFloor());
						}
					}
				}
				else if (GS->HasFlags(Bitfield, EPrevDistrict)) 
				{
					if (DistrictFromTile(Current)->NeedGate()) {
						DistrictFromTile(Current)->InitGate(Neighbour->GetCoords(), Coords);
						GS->SpawnGates(Coords, Dir, Current->GetFloor() + 1);
						Neighbour->AddStructure(EStructure::EGates, GS->InvertBitmask(b));
						Current->AddStructure(EStructure::EGates, b);
					}
					else {
						for (int32 z = 0; z <= Current->GetFloor(); z++) {
							GS->SpawnBorderWall(Coords, Dir, z + 1);
							Neighbour->AddStructure(EStructure::EDistrictWalls, GS->InvertBitmask(b));
							Current->AddStructure(EStructure::EDistrictWalls, b);
						}
					}
					FVector2D DiagA = (Coords + FVector2D(-1, -1));
					FVector2D DiagB = (Coords + FVector2D(-1, 0));
 					FVector2D DiagC = (Coords + FVector2D(1, 0));
 					FVector2D DiagD = (Coords + FVector2D(1, -1));

					if (!GS->IsTileExist(DiagA) || !GS->IsTileExist(DiagB))
					{
						GS->SpawnTower(Coords, IsEvenRow(*Districts[Current->DistrictID]) ? Dir : (EDirection)GS->Shift((int32)Dir), Current->GetFloor()+1);
					}
					else if (!GS->IsTileExist(DiagC) || !GS->IsTileExist(DiagD))
					{
						GS->SpawnTower(Coords, IsEvenRow(*Districts[Current->DistrictID]) ? (EDirection)GS->Shift((int32)Dir) : (EDirection)GS->Shift((int32)Dir), Current->GetFloor()+1);
					}
				}
				else if (IsElevateDistrict(*District) && GS->HasFlags(Bitfield, ENextDistrict | EHigherTile))
				{
					if(District->NeedStairs()) {
						GS->SpawnStairs(Coords, Dir, Current->GetFloor()+1);
						Current->AddStructure(EStructure::EStairs, b);
						//Current->RemoveStructureInDir(EStructure::EOuterWalls, b);
						//Neighbour->RemoveStructureInDir(EStructure::EOuterWalls, GS->InvertBitmask(b));
						District->bHasStairs = true;
					}
				}
			}
		}
	}

	for (int32 n = 0; n < Districts.Num(); n++) {
		UDistrict* District = Districts[n];
		for (FVector2D RawCoords : District->Coords)
		{
			FVector2D Coords = District->Origin + RawCoords;

			for (int32 b = 1 << 1; b <= 1 << 4; b <<= 1)
			{
				EDirection Dir{ (EDirection)b };
				ATile* Neighbour{ GS->GetTile(GS->GetNeighbourInDirection(Coords, Dir)) };
				ATile* Current{ GS->GetTile(Coords) };
				if(Neighbour && Current) {
					if (Current->HasStructure(EStructure::EStairs)) {
						Current->RemoveStructureInDir(EStructure::EOuterWalls, b);
						Neighbour->RemoveStructureInDir(EStructure::EOuterWalls, GS->InvertBitmask(b));
					}
				}
			}
		}
	}
}



void ACity::CreateDistrict(FVector2D LayoutPosition)
{
	int32 CurrentID				{ Districts.Add(NewObject<UDistrict>())};
	FVector2D RegionSize		{ GenerateRegionSize()};
	TArray<FVector2D> Coords	{ };

	GS->GenerateRectangle(FVector2D::ZeroVector, RegionSize, Coords);
	GetLastDistrict()->InitBaseLayout(CurrentID, LayoutPosition, RegionSize, Coords);
	GetLastDistrict()->InitVisualSet(GetRandomVisualSet());
	GetLastDistrict()->DataAsset = DistrictsData[FMath::RandHelper(DistrictsData.Num())];
}

int32 ACity::DistrictCount() const
{
	return Districts.Num();
}

bool ACity::IsFirstDistrict(const class UDistrict& District) const
{
	return District.ID == 0;
}

bool ACity::IsLastDistrict(const class UDistrict& District) const
{
	return IsEvenRow(District) ? District.ID == Districts.Num()-1 : District.ID == Districts.Num() - DistrictsLayout.Y;
}

bool ACity::IsFirstColumn(const class UDistrict& District) const
{
	return District.LayoutPosition.Y == 0;
}

bool ACity::IsFirstRow(const class UDistrict& District) const
{
	return District.LayoutPosition.X == 0;
}

bool ACity::IsLastColumn(const class UDistrict& District) const
{
	return District.LayoutPosition.Y == DistrictsLayout.Y-1;
}

bool ACity::IsLastRow(const class UDistrict& District) const
{
	return  District.LayoutPosition.X == DistrictsLayout.X - 1;
}

bool ACity::IsEvenRow(const class UDistrict& District) const
{
	return GS->IsEven((int32)District.LayoutPosition.X);
}

bool ACity::IsElevateDistrict(const class UDistrict& District) const
{
	if(IsFirstDistrict(District) || IsLastDistrict(District))
		return false;
	
	return IsEvenRow(District) ? IsLastColumn(District) : IsFirstColumn(District);
}

bool ACity::IsHigherDistrict(const class UDistrict& DistrictA, const class UDistrict& DistrictB) const
{
	return DistrictA.LayoutPosition.X > DistrictB.LayoutPosition.X;
}

UDistrict* ACity::DistrictFromTile(ATile* Tile)
{
	return Districts[Tile->DistrictID];
}

UDistrict* ACity::GetDistrictById(int32 ID) const
{
	return ID < DistrictCount() ? Districts[ID] : nullptr;
}

UDistrict* ACity::GetDistrictByLayout(FVector2D LayoutPos) const
{
	return LayoutPos < DistrictsLayout ? Districts[PositionToID(LayoutPos)] : nullptr;
}

UDistrict* ACity::GetDistrictByRandom() const
{
	return Districts[FMath::RandHelper(DistrictCount())];
}

UDistrict* ACity::GetDistrictByRowRandom(int32 Row) const
{
	return Districts[PositionToID(FVector2D(Row, FMath::RandHelper(DistrictsLayout.Y)))];
}

UDistrict* ACity::GetDistrictByColRandom(int32 Col) const
{
	return Districts[PositionToID(FVector2D(FMath::RandHelper(DistrictsLayout.X), Col))];
}


int32 ACity::PositionToID(FVector2D LayoutPos) const
{
	return (int32)(LayoutPos.Y) + int32(LayoutPos.X * DistrictsLayout.Y);
}

UDistrict* ACity::GetLastDistrict() const
{
	return Districts.Last();
}

UDistrict* ACity::GetFirstDistrict() const
{
	return Districts[0];
}

FTileVisualSet* ACity::GetRandomVisualSet() const
{
	TArray<FTileVisualSet*> TempVisuals;
	int32 TempVisualsSize{ 0 };
	TileVisualsTable->GetAllRows(TEXT("TileVisuals"), TempVisuals);
	TempVisualsSize = TempVisuals.Num();
	return TempVisuals[FMath::RandHelper(TempVisualsSize)];
}

FVector2D ACity::GenerateRegionSize() const
{
	FVector2D RegionSize	{ FVector2D::ZeroVector};
	bool bIsEven			{ false};

	while(!bIsEven) {
		RegionSize = FVector2D(FMath::RandRange((int32)RegionSizeX.X, (int32)RegionSizeX.Y), FMath::RandRange((int32)RegionSizeY.X, (int32)RegionSizeY.Y));
		bIsEven = (int32)(RegionSize.X+RegionSize.Y) % 2 == 0;
	}
	
	return RegionSize;
}

bool ACity::BuildPathWithinDistrict(FPath& OutPath, UDistrict* District)
{
	UE_LOG(LogTemp, Warning, TEXT("AZAZ: %d"), District->Coords.Num());
	return GS->GetPathFinder()->BuildPathBetweenTiles(OutPath.Start, OutPath.Finish, District->Coords, OutPath.Rules, OutPath.FinishedPath);
}

bool ACity::BuildPathWithinFloor(FPath& OutPath, int32 FloorNum)
{
	TArray<FVector2D> Coords;
	for (int y = 0; y < DistrictsLayout.Y; y++)
	{
		int32 DistrictID {(int32)(FloorNum*DistrictsLayout.Y + y)};
		UE_LOG(LogTemp, Error, TEXT("LOL: %d"), DistrictID)
		Coords.Append(Districts[DistrictID]->Coords);
	}
	
	return GS->GetPathFinder()->BuildPathBetweenTiles(OutPath.Start, OutPath.Finish, Coords, OutPath.Rules, OutPath.FinishedPath);
}

const FTileVisualSet& ACity::GetDefaultVisuals() const
{
	return TileConstructRules.DefaultTileVisuals;
}

void ACity::Build()
{
	CreateDistricts();
	PlaceDistricts();
	OffsetOddDistricts();

	ConnectDistricts();
	BackfillConnections();

	SpawnDistrictsTiles();
	SpawnDistrictArchitecture();

	BakeDistricts();
}

bool ACity::InitializeRules()
{
	if (TileConstructRules.DefaultTileVisualsRow.GetRow<FTileVisualSet>(TEXT("TileVisuals")) == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Rules: Default Visuals are not set"))
			return false;
	}

	if (TileConstructRules.TileBaseClass == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Rules: Tile Class is not set"))
			return false;
	}
	
	TileConstructRules.DefaultTileVisuals = *TileConstructRules.DefaultTileVisualsRow.GetRow<FTileVisualSet>(TEXT("TileVisuals"));
	TileConstructRules.TileExtents = GetDefaultVisuals().Mesh->GetBounds().BoxExtent * 2;
	return true;
}

bool ACity::InitializeGridSubsystem()
{
	GS = GetGameInstance()->GetSubsystem<UGridSubsystem>();
	if(!GS)
		return false;

	GS->Setup(this);
	return true;
}

// Called every frame
void ACity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

