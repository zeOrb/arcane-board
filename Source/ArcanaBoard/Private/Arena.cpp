// Fill out your copyright notice in the Description page of Project Settings.



#include "Arena.h"
#include "CombatComponent.h"
#include "Party.h"
#include "Unit.h"

#include "ArcanaController.h"

#include "Engine/StaticMesh.h"
#include "Engine/Public/TimerManager.h"
#include "Components/StaticMeshComponent.h"


UCombatEngine* AArena::GetCombatEngine() const
{
	return CEngine;
}

// Sets default values
AArena::AArena()
{
	DirMap.Add(ECombatDir::ENorth,		{ 1,0 });
	DirMap.Add(ECombatDir::ENorthEast,	{ 1,1 });
	DirMap.Add(ECombatDir::EEast,		{ 0,1 });
	DirMap.Add(ECombatDir::ESouthEast,	{ -1,1 });
	DirMap.Add(ECombatDir::ESouth,		{ -1,0 });
	DirMap.Add(ECombatDir::ESouthWest,	{ -1,-1 });
	DirMap.Add(ECombatDir::EWest,		{ 0, -1 });
	DirMap.Add(ECombatDir::ENorthWest,	{ 1, -1 });
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlacementIDs.Add(0, TArray<int32>{0});
	PlacementIDs.Add(1, TArray<int32>{3});
	PlacementIDs.Add(2, TArray<int32>{2,4});
	PlacementIDs.Add(3, TArray<int32>{2,3,4});
	PlacementIDs.Add(4, TArray<int32>{0,2,4,6});
	PlacementIDs.Add(5, TArray<int32>{1,2,3,4,5});
	PlacementIDs.Add(6, TArray<int32>{0,1,2,3,4,5});
	PlacementIDs.Add(7, TArray<int32>{0,1,2,3,4,5,6});
	// if [Party.Num % 2 != 0]ODD: 
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	PlayerSelection = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Selection Component"));
	PlayerSelection->SetupAttachment(GetRootComponent());

	
}

// Called when the game starts or when spawned
void AArena::BeginPlay()
{
	Super::BeginPlay();
	CEngine = NewObject<UCombatEngine>(this, TEXT("Combag Engine"));
	CEngine->OnUpdateStateHandle.BindUObject(CEngine, &UCombatEngine::UpdateState);
	CEngine->OnReceiveActionHandle.BindUObject(CEngine, &UCombatEngine::OnReceiveAction);
	CEngine->OnFinishActionHandle.BindUObject(CEngine, &UCombatEngine::OnFinishAction);
}




// Called every frame
void AArena::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AArena::SendActionSignal(const FCombatAction& Action)
{
	if (CEngine->IsPlayerTurn())
	{
		ClearMovementRange();
		SetPlayerSelectionNull();
	}
	CEngine->OnReceiveActionHandle.Execute(Action);
}

void AArena::OnActionFinished(const FCombatAction& Action)
{
	CEngine->OnFinishActionHandle.Execute(Action);
}

void AArena::GetGridSize(int32& X, int32& Y) const
{
	X = GridSize.X-1;
	Y = GridSize.Y-1;
}

float AArena::GetCellSize() const
{
	check(CellMesh);
	return (CellMesh->GetBounds().BoxExtent /2 * CellScale).X;
}

FVector AArena::GetCellLocation(FVector2D Coords) const
{
	FVector Location {FVector::ZeroVector};
	Location.X = Coords.X * GetCellSize() * 2;
	Location.Y = Coords.Y * GetCellSize() * 2;
	Location.Z = 3;
	Location += GetGridOrigin();
	return FVector(FMath::GridSnap(Location.X, GetCellSize() * 2), FMath::GridSnap(Location.Y, GetCellSize() * 2), Location.Z+1);
}

FVector AArena::GetGridOrigin() const
{
	FVector Location {GetActorLocation()};
	Location.X -= (GridSize.X * GetCellSize());
	Location.Y -= (GridSize.Y * GetCellSize());
 	Location.X += GetCellSize();
 	Location.Y += GetCellSize();
	return Location;
}

 UParty* AArena::GetAttackers()
{
	return Attacker;
}

 UParty* AArena::GetDefenders()
{
	return Defender;
}

 AArcanaController* AArena::GetPlayer()
 {
	return Player;
 }

 bool AArena::IsPlayerTurn() const
 {
	return CEngine->IsPlayerTurn();
 }

 UCombatComponent* AArena::GetActiveCC() const
 {
	return GetCombatEngine()->GetActiveCombatComp();
 }

 AUnit* AArena::GetActiveUnit() const
 {
	return CEngine->GetActiveUnit();
 }

void AArena::GenerateMap()
{
	for (int32 x = 0; x < (int32)GridSize.X; x++) {
		for (int32 y = 0; y < (int32)GridSize.Y; y++) {
			Map.Add(FVector2D(x,y), FCombatTile(EFree, GetCellLocation(FVector2D(x,y))));
		}
	}
}

void AArena::TeleportPartiesToArena()
{
	for (int32 i = 0; i < Attacker->GetMembersCount(); i++) {
		FVector2D Coords = GetPlacementForUnit(Attacker, i);
		Map[Coords].Flags |= EAttacker;
		Map[Coords].Flags &= ~EFree;
		Attacker->GetMember(i)->SetActorLocation(GetCellLocation(Coords));
		Attacker->GetMember(i)->SetActorRotation(FRotator(0, 90, 0));
		Attacker->GetMember(i)->ToggleWeaponSheath();
		Attacker->GetMember(i)->GetCC()->SetCoords(Coords);
	}
	for (int32 i = 0; i < Defender->GetMembersCount(); i++) {
		FVector2D Coords = GetPlacementForUnit(Defender, i);
		Map[Coords].Flags |= EDefender;
		Map[Coords].Flags &= ~EFree;
		Defender->GetMember(i)->SetActorLocation(GetCellLocation(Coords));
		Defender->GetMember(i)->SetActorRotation(FRotator(0, -90, 0));
		Defender->GetMember(i)->ToggleWeaponSheath();
		Defender->GetMember(i)->GetCC()->SetCoords(Coords);
	}
}

FVector2D AArena::GetPlacementForUnit(class UParty* Party, int32 Num)
{
	FVector2D Coords{ FVector2D::ZeroVector };

	Coords.Y = Party == Attacker ? 0 : GridSize.Y - 1;
	Coords.X = PlacementIDs[Party->GetMembersCount()][Num];
	return Coords;
}

FVector2D AArena::GetRandomCell() const
{
	int32 X {FMath::RandRange(0, (int32)GridSize.X-1)};
	int32 Y {FMath::RandRange(0, (int32)GridSize.Y-1)};
	return FVector2D(X,Y);
	
	
}

FVector2D AArena::GetRandomMove(UCombatComponent* CombatComp) const
{
	FVector2D Origin = CombatComp->GetCoords();
	int32 MaxDistance = CombatComp->GetStatValue(EStat::EMovement);
	int32 X = FMath::RandRange(-MaxDistance, MaxDistance);
	int32 Y = FMath::RandRange(-MaxDistance, -MaxDistance);
	Origin += FVector2D(X,Y);
	Origin.X = FMath::Clamp(Origin.X, 0.0f, GridSize.X-1);
	Origin.Y = FMath::Clamp(Origin.Y, 0.0f, GridSize.Y-1);
	UE_LOG(LogTemp, Warning, TEXT("%f:%f"), Origin.X, Origin.Y);
	return Origin;
}

FVector2D AArena::GetCellForUnit(class UParty* Party, int32 Num)
{

	if (Party == Attacker) {
		return FVector2D(Num, 0);
		// [0:7, 0] vs [0:7, Size.Y-1]

	}
	else if (Party == Defender) {
		return FVector2D(Num, GridSize.Y - 1);
	}
	return FVector2D::ZeroVector;
}

void AArena::SetUnitFacingAt(AUnit* Unit, FVector2D Tile)
{
	FVector TileLoc = GetCellLocation(Tile);
	FRotator LookAtRotation{ FRotationMatrix::MakeFromX(TileLoc - Unit->GetActorLocation()).Rotator() };
	LookAtRotation.Roll = 0;
	LookAtRotation.Pitch = 0;
	Unit->SetActorRotation(LookAtRotation);
}

FVector2D AArena::GetClosestFacingCell(FVector2D A, FVector2D B) const
{
	return B+FVector2D((A.X == B.X) ? 0 : (A.X > B.X) ? 1 : -1, (A.Y == B.Y) ? 0 : (A.Y > B.Y) ? 1 : -1);
}

void AArena::InitializeArena(UParty* InAttacker, UParty* InDefender, AArcanaController* Controller)
{
	Attacker = InAttacker;
	Defender = InDefender;
	Player = Controller;

	GenerateMap();
}

void AArena::OnReadyToBattle()
{
	CEngine->Run(this);
}

void AArena::OnNextStep()
{
	CEngine->OnUpdateStateHandle.ExecuteIfBound(ECombatState((uint8)(CEngine->GetBattleData().State)+1));
}

void AArena::ClearMovementRange()
{
	if(LastMovementRange.Num() != 0 )
	{
		OnClearMovementRange(LastMovementRange);
		LastMovementRange.Empty();
	}
}

void AArena::SetPlayerSelectionAtUnit(FVector2D Coords)
{
	PlayerSelection->SetVisibility(true);
	PlayerSelection->SetWorldLocation(GetCellLocation(Coords) + FVector(0, 0, SelectionOffset));
}

void AArena::SetPlayerSelectionNull()
{
	PlayerSelection->SetVisibility(false);
	PlayerSelection->SetWorldLocation(FVector::ZeroVector);
}

bool AArena::GetUnitAtCell(FVector2D Coords, AUnit*& OutUnit)
{
	if ((Map[Coords].Flags & EAttacker) != 0) {
		for (AUnit* Member : Attacker->GetMembers()) {

			if (Member->GetCC()->GetCoords() == Coords) {
				OutUnit = &*Member;
				return true;
			}
		}
	}
	else if ((Map[Coords].Flags & EDefender) != 0) {
		for (AUnit* Member : Defender->GetMembers()) {
			if (Member->GetCC()->GetCoords() == Coords) {
				OutUnit = &*Member;
				return true;
			}
		}
	}
	else {
		OutUnit = nullptr;
		return false;
	}
	return true;
}

bool AArena::CanUnitMoveTo(AUnit* Unit, FVector2D Cell) const
{
	return Unit->GetCC()->GetCoords()!= Cell && IsCellHasFlags(Cell, EFree) && AreCellsWithinRange(Cell, Unit->GetCC()->GetCoords(), Unit->GetCC()->GetStatValue(EStat::EMovement));
}

bool AArena::IsCellHasFlags(FVector2D Cell, int32 Flags) const
{
	return (Map[Cell].Flags & Flags) != 0;
}

bool AArena::AreCellsWithinRange(FVector2D A, FVector2D B, int32 Distance) const
{
	return GetDistance(A,B) <= Distance;
}

void AArena::GenerateMovementRange(AUnit* Unit, TArray<FVector2D>& OutArray)
{
	FVector2D Origin = Unit->GetCC()->GetCoords();
	FloodFill(Origin, Origin, Unit->GetCC()->GetStatValue(EStat::EMovement), OutArray);
}

void AArena::ShowMovementRangeForUnit(FVector2D Coords)
{
	AUnit* Unit;
	TArray<FVector2D> Cells;
	if (GetUnitAtCell(Coords, Unit)) {
		GenerateMovementRange(Unit, Cells);
	}
	LastMovementRange.Append(Cells);
	OnShowMovementRange(Cells);
}

bool AArena::IsCellExist(FVector2D Coords) const
{
	return (Coords.X >= 0 && Coords.X < GridSize.X) && (Coords.Y >= 0 && Coords.Y < GridSize.Y);
}

FVector2D AArena::GetNeighbourInDirection(FVector2D Origin, ECombatDir Dir) const
{
	return Origin + DirMap[Dir];
}

void AArena::FillWithNeighbours(FVector2D Origin, TArray<FVector2D>& OutCells)
{
	for (int32 i = 1 << 0; i < (1 << 8); i <<= 1) {
		FVector2D Neighbour = GetNeighbourInDirection(Origin, (ECombatDir)i);
		if (IsCellExist(Neighbour)) {
			OutCells.Add(Neighbour);
		}
	}
}

int32 AArena::GetDistance(FVector2D A, FVector2D B) const
{
	return FMath::FloorToInt(FVector2D::Distance(A, B));
}

void AArena::FloodFill(FVector2D Cell, FVector2D Origin, int32 Distance, TArray<FVector2D>& OutCells)
{
	if (IsCellExist(Cell)&& !OutCells.Contains(Cell) && GetDistance(Cell, Origin) <= Distance) {
		OutCells.Add(Cell);
		for (int32 i = 1 << 0; i < (1 << 8); i <<= 1) {
			FVector2D Neighbour = GetNeighbourInDirection(Cell, (ECombatDir)i);
			FloodFill(Neighbour, Origin, Distance, OutCells);
		}
	}
}



void AArena::OnPreReady_Implementation()
{
	TeleportPartiesToArena();
}

void UCombatEngine::StartBattle()
{
//	UpdateState(ECombatState::ERoundBegin);
}

void UCombatEngine::RollInitative()
{
	BattleData.Queue.Empty();

	for (AUnit* Member : Arena->GetAttackers()->GetMembers()) {
		int32 Roll = RollDice(Member->GetCC()->GetStatValue(EStat::EAgility));
		Member->GetCC()->SetStatValue(EStat::EInitiative, Roll);
		
		BattleData.Queue.Add(Member);
	}
	for (AUnit* Member : Arena->GetDefenders()->GetMembers()) {
		int32 Roll = RollDice(Member->GetCC()->GetStatValue(EStat::EAgility));
		Member->GetCC()->SetStatValue(EStat::EInitiative, Roll); 
		BattleData.Queue.Add(Member);
	}
	int32 i = 1;
	for (AUnit* Unit : BattleData.Queue) {
		Unit->GetCC()->bFinished = false;
		// Automate, track handles and delegates, integrate into struct and clear in engine
		FTimerHandle Handle;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(Arena, FName("ShowDiceRoll"), Unit, Unit->GetStat(EStat::EInitiative).GetValue() - Unit->GetStatSafe(EStat::EAgility).GetValue(), Unit->GetStatSafe(EStat::EAgility).GetValue(), ECombatState::EInitiativeRoll);
		Arena->GetWorldTimerManager().SetTimer(Handle, Delegate, .2f*i, false);
		i++;
	}
}

void UCombatEngine::SortInitiative()
{
	BattleData.Queue.HeapSort([](const AUnit& A, const AUnit& B) {
		return A.GetCC()->GetStatValue(EStat::EInitiative) > B.GetCC()->GetStatValue(EStat::EInitiative);
		});
}



int32 UCombatEngine::RollDice(int32 Bonus)
{
	return FMath::RandRange(1, 6) + Bonus;
}

// COMBAT ENGINE
UCombatEngine::UCombatEngine()
{

}

const FBattleData& UCombatEngine::GetBattleData() const
{
	return BattleData;
}

AUnit* UCombatEngine::GetActiveUnit() const
{
	return (BattleData.Turn < BattleData.Queue.Num() && BattleData.Turn >= 0) 
		? BattleData.Queue[BattleData.Turn] 
		: nullptr;
}

bool UCombatEngine::IsOutOfTurns() const
{
	return BattleData.Turn >= BattleData.Queue.Num();
}

class UCombatComponent* UCombatEngine::GetActiveCombatComp() const
{
	return GetActiveUnit()->GetCC();
}

bool UCombatEngine::IsPlayerTurn() const
{
	return Arena->GetPlayer()->GetParty()->GetMembers().Contains(GetActiveUnit());
}

void UCombatEngine::ActivatePlayerUnit()
{
	FVector2D Coords = GetActiveCombatComp()->GetCoords();
	Arena->GetPlayer()->ActivateUnitMode();
	Arena->SetPlayerSelectionAtUnit(Coords);
	Arena->ShowMovementRangeForUnit(Coords);
}

void UCombatEngine::Run(AArena* InArena)
{
	Arena = InArena;
	OnUpdateStateHandle.ExecuteIfBound(ECombatState::EBattleStart);
}


void UCombatEngine::UpdateState(ECombatState NewState)
{
	bool bUpdateUI {false};
	bool bNextStage {false};
	bool bUpdateState {false};
	bool bNewRound {false};
	switch (NewState)
	{
	case ECombatState::EInvalid:
		break;
	case ECombatState::EBattleStart:
		if(BattleData.State == ECombatState::EInvalid) {
			StartBattle();
			bUpdateUI = true;
			bNextStage = true;
		}
		break;
	case ECombatState::ERoundBegin:
		BattleData.Round++;
		bUpdateUI = true;
		bNextStage = true;
		break;
	case ECombatState::EInitiativeRoll:
			if (BattleData.Turn < 0 && BattleData.State == ECombatState::ERoundBegin) {
				RollInitative();
				BattleData.Turn = 0;
				bUpdateUI = true;
				bNextStage = false;
			}
			else if (BattleData.Turn + 1 < BattleData.Queue.Num()) {
				BattleData.Turn++;
				bUpdateUI = false;
				bNextStage = false;
			}
			else if (BattleData.Turn + 1 >= BattleData.Queue.Num()) {
				SortInitiative();
				BattleData.Turn = -1;
				bUpdateUI = false;
				bNextStage = true;
			}
		break;
	case ECombatState::ETurnBegin:
		bUpdateUI = true;
		if (BattleData.Turn < 0) {
			BattleData.Turn = 0;
			bNextStage = false;
		}
		else if (IsOutOfTurns()) {
			UE_LOG(LogTemp, Warning, TEXT("New round is set"))
			bUpdateUI = false;
			bNewRound = true;
		}
		if (IsPlayerTurn()) {
			ActivatePlayerUnit();
			// Wait for player click
		}
		else if(!IsOutOfTurns()){
			// Waiting for AI
			Arena->SendActionSignal(GetActiveCombatComp()->MakeAction(EMoveAction, Arena->GetRandomMove(GetActiveCombatComp())));
		}

		break;
	case ECombatState::ETurnResolve:
		break;
	case ECombatState::ERoundResolve:
		break;
	case ECombatState::EBattleResolve:
		break;
	default:
		break;
	}

	if(bUpdateUI) {
		BattleData.State = NewState;
		OnUpdateUI.Broadcast(BattleData, bNextStage);
	}
	else if (!bUpdateUI && bNextStage) {
		Arena->OnNextStep();
	}
	else if (bNewRound) {
		BattleData.Turn = -1;
		UE_LOG(LogTemp, Warning, TEXT("New round"))
		UpdateState(ECombatState::ERoundBegin);
	}
}

void UCombatEngine::OnReceiveAction(const FCombatAction& Action)
{
	ActiveAction = Action;

	switch (ActiveAction.Type)
	{
	case ENoneAction:
	case EAttackAction:
	case EUseItemAction:
	case EThrowItemAction:
	case EDodgeAction:
	case EBlockAction:
	case EEscapeAction:
	default:
		checkNoEntry();
	case EMoveAction:
		Arena->ExecuteMovement(ActiveAction);
		break;
	case EMoveAndAttack:
		Arena->ExecuteMoveAttack(ActiveAction);
		Arena->SetUnitFacingAt(GetActiveUnit(), ActiveAction.Targets[1]); //Move
		break;
	}
}

void UCombatEngine::OnFinishAction(const FCombatAction& Action)
{
	GetActiveCombatComp()->bFinished = true;
/*	if(Action.Type == EMoveAction) {*/
		GetActiveCombatComp()->SetCoords(Action.Targets[0]);
		Arena->Map[Action.Targets[0]].Flags |= IsPlayerTurn() ? EAttacker : EDefender;
		Arena->Map[Action.Targets[0]].Flags &= ~EFree;
		Arena->Map[Action.Source].Flags &= ~ (IsPlayerTurn() ? EAttacker : EDefender);
		Arena->Map[Action.Source].Flags |= EFree;
/*		}*/
// 	if(Action.Type == EMoveAction) { 
// 		BattleData.Turn++;
// 	}
// 	if (Action.Type == (EMoveAction | EAttackAction)) {
// 		UE_LOG(LogTemp, Error, TEXT("WAAA: %d"), ActiveAction.Targets.Num())
// 		if(ActiveAction.Targets.Num() != 1) {
// 			ActiveAction.Targets.RemoveAt(1);
// 			UE_LOG(LogTemp, Error, TEXT("WAAA: %d"), ActiveAction.Targets.Num())
// 			
// 			}
// 		else
			BattleData.Turn++;
/*	}*/
	UpdateState(ECombatState::ETurnBegin);
}
