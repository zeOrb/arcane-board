// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"
#include "Unit.h"
#include "Party.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
ATile::ATile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bIsRevealed = false;
	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	SetRootComponent(SceneComp);
	MeshComp->SetupAttachment(GetRootComponent());
	MeshComp->SetCollisionProfileName("BlockTile");
	MeshComp->SetRenderCustomDepth(true);

	TempFogComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fog Component"));
	TempFogComp->SetupAttachment(SceneComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ATile::IsRevealed() const
{
	return bIsRevealed;
}

int32 ATile::GetFloor() const
{
	return Floor;
}

FVector2D ATile::GetCoords() const
{
	return Coords;
}

void ATile::AddStructure(EStructure Structure, int32 Direction)
{
	if (!StructureMap.Contains(Structure)) {
		StructureMap.Add(Structure, Direction);
	}
	else {
		StructureMap[Structure] |= Direction;
	}
}
                                   
bool ATile::HasStructure(EStructure Structure)
{
	if (Structure == EStructure::EInvalid) // Unnecessary
		return false;
	return StructureMap.Contains(Structure);
}

bool ATile::HasStructureInDir(EStructure Structure, int32 Direction)
{
	if(Structure == EStructure::EInvalid || !StructureMap.Contains(Structure))
		return false;

	return (StructureMap[Structure] & Direction) != 0;
}

void ATile::RemoveStructureInDir(EStructure Type, int32 Direction)
{
	if (StructureMap.Contains(Type)) {
		StructureMap[Type] &= ~Direction;
	}
	
}

int32 ATile::CountStructures(EStructure Type)
{
	int32 StructureCount {0};
	if(HasStructure(Type))
		for (int32 b = (1 << 1); b <= (1 << 4); b <<= 1) {
			StructureCount += (StructureMap[Type] & b) != 0;
		}
	
	return StructureCount;
}

void ATile::TestStructureMap()
{
	if(HasStructure(EStructure::EOuterWalls))
	{
		int32 Num {CountStructures(EStructure::EOuterWalls)};
		FString Str {FString::Printf(TEXT("Walls: %d"), Num)};
		GEngine->AddOnScreenDebugMessage(0, 1.3f, FColor::MakeRandomColor(), Str);
	}
	if (HasStructure(EStructure::EGates))
	{
		int32 Num{ CountStructures(EStructure::EGates) };
		FString Str{ FString::Printf(TEXT("Gates: %d"), Num) };
		GEngine->AddOnScreenDebugMessage(1, 1.3f, FColor::MakeRandomColor(), Str);
	}
	if (HasStructure(EStructure::EDistrictWalls))
	{
		int32 Num{ CountStructures(EStructure::EDistrictWalls) };
		FString Str{ FString::Printf(TEXT("District Walls: %d"), Num) };
		GEngine->AddOnScreenDebugMessage(1, 1.3f, FColor::MakeRandomColor(), Str);
	}
}

void ATile::RegisterParty(class UParty* InParty)
{
	Party = InParty;
	Party->OnCreate(this);
}

void ATile::UnregisterParty()
{
	Party = nullptr;
}

// Called every frame
void ATile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATile::Reveal()
{
	bIsRevealed = true;
	
	OnReveal();
}