// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ArcanaBoardGameModeBase.h"
#include "CinematicCamera/Public/CineCameraActor.h"
#include "ArcanaController.h"
#include "ArcanaBoard.h"
#include "Arena.h"

#include "GameplayTags/Classes/GameplayTagsManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"

#include "GridSubsystem.h"
#include "District.h"
#include "City.h"
#include "Tile.h"

#include "EffectsManager.h"
#include "CombatSubsystem.h"
#include "Party.h"
#include "Unit.h"

uint64 AArcanaBoardGameModeBase::ItemGlobalUID = 0;

AArcanaBoardGameModeBase::AArcanaBoardGameModeBase()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> PartyTemplateObject(TEXT("DataTable'/Game/DataTables/PartyTemplates.PartyTemplates'"));
	if (PartyTemplateObject.Succeeded()) {
		PartyDataTable = PartyTemplateObject.Object;
	}
}

void AArcanaBoardGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	GS = GetGameInstance()->GetSubsystem<UGridSubsystem>();
	CS = GetGameInstance()->GetSubsystem<UCombatSubsystem>();
	GS->SetGameMode(this);

	EffectsManager = GetWorld()->SpawnActor<AEffectsManager>(EffectsManagerClass);
}

AUnit* AArcanaBoardGameModeBase::SpawnUnit()
{
	return GetWorld()->SpawnActor<AUnit>(UnitClass, FTransform::FTransform());
}




bool AArcanaBoardGameModeBase::RequestPartyAction(UParty* Party, FVector2D Tile, EPartyAction Action)
{
	if(IsPartyActionValid(Party, Tile, Action))
	{
		switch (Action)
		{
		case EPartyAction::EInvalid:
			UE_LOG(LogTemp, Error, TEXT("Requested Action Type is not valid"))
			return false;
		case EPartyAction::EReveal:
			return TryRevealTile(Tile);
		case EPartyAction::ETravel:
			return TryPartyTravel(Party, Tile);
		case EPartyAction::EAttack:
			return TryPartyEncounter(Party, GS->GetTile(Tile)->GetParty());
		default:
			return false;
		}
	}
	return false;
}

bool AArcanaBoardGameModeBase::CreatePartyFromTemplate(FName TemplateName, FVector2D PossibleTile /* Add Pool*/)
{
	if(!GS->IsTileExist(PossibleTile)) {
		UE_LOG(LogArcana, Error, TEXT("Tile for party creation does not exist"))
		return false;
	}

	ATile* Tile {GS->GetTile(PossibleTile)};
	if (Tile->HasParty()) {
		UE_LOG(LogTemp, Error, TEXT("Tile already has assigned party"))
		return false;
	}

	const auto* PartyTemplate{ PartyDataTable->FindRow<FPartyTemplate>(TemplateName, PartyContext) };
	UParty* NewParty			{ NewObject<UParty>() };
		
	for (auto& UnitPoolObject : PartyTemplate->UnitsPool) {
		for (int32 u = 0; u < UnitPoolObject.GetAmount(); u++) {
			AUnit* NewMember = SpawnUnit();

			FUnitTemplate* UnitTemplate = UnitPoolObject.GetWeightedHandle().GetRow<FUnitTemplate>(TEXT("UnitsTemplates"));
			TArray<FItem> UnitItems;
			
			CreateItemsFromPool(UnitTemplate->ItemPool, UnitItems);

			NewMember->InitializePrimaryStats(UnitTemplate->DefaultStats);
			NewMember->GenerateSecondaryStats();
			NewMember->InitializeInventory(UnitItems);
			NewMember->Name = UnitTemplate->Name;
			NewParty->RegisterMember(NewMember);
		}
	}

	ActiveParties.Add(NewParty);
	Tile->RegisterParty(NewParty);
	NewParty->OnCreate(Tile);

	MovePartyToSpawn(ActiveParties.Last(), Tile);
	return true;
}


bool AArcanaBoardGameModeBase::CreateEncounterForDistrict(UDistrict* District, FVector2D SpawnTile)
{
	if(!District->RollForEncounter())
		return false;

	return CreatePartyFromTemplate(District->GetRandomEncounter(), SpawnTile);
}

void AArcanaBoardGameModeBase::CreateItemsFromPool(const TArray<FTablePool>& ItemPools, TArray<FItem>& OutItemList) const
{
	for (auto& ItemPoolObject : ItemPools) {
		const int32 ItemPoolSize { FMath::RandRange(ItemPoolObject.MinAmount, ItemPoolObject.MaxAmount) };
		for (int32 i = 0; i < ItemPoolSize; i++) {
			FItem* Item = ItemPoolObject.GetWeightedHandle().GetRow<FItem>(TEXT("ItemsTemplates"));
			OutItemList.Add(*Item);
			OutItemList.Last().UID = AArcanaBoardGameModeBase::ItemGlobalUID++;
		}
	}
}

bool AArcanaBoardGameModeBase::TryPartyEncounter(UParty* Attacker, UParty* Defender)
{
	if (Attacker == PlayerParty || Defender == PlayerParty) {
		PlayerController->SnapValue = City->Battlefield->GetCellSize()*2;
		City->Battlefield->InitializeArena(Attacker, Defender, PlayerController);

		PlayerController->BindArena(City->Battlefield);
		PlayerController->ActivateCombatMode();
		PlayerController->OnBeginTransitionToCombat(City->Battlefield->BattlefieldCam);
	}
	// Decide if it's gonna be a peaceful encounter or combat

	// Validate

	// Process different types of encounter

	// Start combat
	return true;
}

// bool AArcanaBoardGameModeBase::CreateItemForUnit(FName ItemName, AUnit* Unit)
// {
// 	const auto* PartyTemplate{ ItemsLibrary->FindRow<FItem>(ItemName,) };
// }

bool AArcanaBoardGameModeBase::TryPartyTravel(UParty* Party, FVector2D Destination)
{
	ATile* NextTile{ GS->GetTile(Destination) };
	OnMoveParty(Party, Party->GetCurrentTile(), NextTile);

	Party->GetCurrentTile()->UnregisterParty();
	NextTile->RegisterParty(Party);
	Party->OnTravel(NextTile);
	
	

	return true;

}


bool AArcanaBoardGameModeBase::TryRevealTile(FVector2D Coords)
{
	auto* Tile = GS->GetTile(Coords);
	check(Tile);
	CreateEncounterForDistrict(City->DistrictFromTile(Tile), Coords);
	Tile->Reveal();
	return true;
}

void AArcanaBoardGameModeBase::MovePartyToTile(UParty* Party, ATile* Tile)
{

	return;

	if(Party->IsSingle()) {
		Party->GetMember(0)->SetActorLocation(Tile->GetActorLocation() + FVector(0,0,2));
		return;
	}

	for (int32 i = 0; i < Party->GetMembersCount() ; i++)
	{
		FVector NewPosition= MakeLocationForUnit(i, Tile->GetActorLocation());
		Party->GetMember(i)->SetActorLocation(NewPosition);
	}
}

void AArcanaBoardGameModeBase::MovePartyToSpawn(UParty* Party, class ATile* Tile)
{
	if (Party->IsSingle()) {
		Party->GetMember(0)->SetActorLocation(Tile->GetActorLocation() + FVector(0, 0, 2));
		return;
	}

	for (int32 i = 0; i < Party->GetMembersCount(); i++)
	{
		FVector NewPosition = MakeLocationForUnit(i, Tile->GetActorLocation());
		Party->GetMember(i)->SetActorLocation(NewPosition);
	}
}

bool AArcanaBoardGameModeBase::IsPartyActionValid(UParty* Party, FVector2D Coords, EPartyAction Action)
{
	if (!GS->IsTileExist(Coords)) {
		UE_LOG(LogArcana, Error, TEXT("Tile at [%d:%d] does not exist"), (int32)Coords.X, (int32)Coords.Y)
		return false;
	}

	if (Party == nullptr) {
		UE_LOG(LogArcana, Error, TEXT("Provided party does not exist"))
		return false;
	}
	ATile* ActionTile = GS->GetTile(Coords);
	ATile* CurrentTile = Party->GetCurrentTile();

	if (!ActionTile || !CurrentTile) {
		UE_LOG(LogArcana, Error, TEXT("Could not get tile pointers"))
		return false;
	}

	if (GS->GetDistanceBetweenTiles(Coords, CurrentTile->GetCoords()) > 1) {
		UE_LOG(LogArcana, Error, TEXT("Distance between tiles is too big: %d"), GS->GetDistanceBetweenTiles(Coords, CurrentTile->GetCoords()))
			return false;
	}

	if (CurrentTile == ActionTile) {
		UE_LOG(LogArcana, Error, TEXT("Same tile"))
			return false;
	}
	
	switch (Action)
	{
	case EPartyAction::EInvalid:
		return false;
	case EPartyAction::EReveal:
		if (ActionTile->IsRevealed()) {
			UE_LOG(LogArcana, Error, TEXT("Tile is already revealed"))
				return false;
		} break;
	case EPartyAction::ETravel:
		if (GS->IsPathBlocked(CurrentTile->GetCoords(), ActionTile->GetCoords())) {
			UE_LOG(LogTemp, Error, TEXT("Path is blocked"))
				return false;
		}
		if (ActionTile->HasParty()) {
			UE_LOG(LogTemp, Error, TEXT("Tile already has assigned party"))
				return false;
		} break;
	default:
		break;
	}
	return true;
}

FVector AArcanaBoardGameModeBase::MakeLocationForUnit(int32 Id, FVector Center)
{
	FVector Location { Center };
	Location -= GS->GetCity()->TileConstructRules.TileExtents/2;
	Location += PartyGrid.BorderOffset();
	Location += PartyGrid.Step(Id) * PartyGrid.Length;
	Location.Z  = Center.Z + 2;
	return Location;
}

void AArcanaBoardGameModeBase::InitCityActor(class ACity* InCity)
{
	City = InCity;
	City->Build();
}

void AArcanaBoardGameModeBase::InitPlayerPawn(FVector2D Coords)
{
	PlayerController = GetWorld()->GetFirstPlayerController<AArcanaController>();
	GS->GetTile(Coords)->Reveal();
	CreatePartyFromTemplate("Player", Coords);
	PlayerParty = ActiveParties.Last();
}

UParty* AArcanaBoardGameModeBase::GetPlayerParty() const
{
	return PlayerParty;
}

FVector2D AArcanaBoardGameModeBase::GetPlayerPosition() const
{
	return PlayerParty->GetCurrentTile()->GetCoords();
}
