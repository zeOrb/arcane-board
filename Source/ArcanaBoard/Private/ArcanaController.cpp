// Fill out your copyright notice in the Description page of Project Settings.


#include "ArcanaController.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "ArcanaBoard/ArcanaBoardGameModeBase.h"
#include "CombatComponent.h"
#include "CombatScreen.h"
#include "Arena.h"
#include "Tile.h"
#include "Unit.h"
#include "GameplayTags/Classes/GameplayTagsManager.h"

AArcanaController::AArcanaController()
{
	bShowMouseCursor = true;
	TagsManager = &UGameplayTagsManager::Get();
}

void AArcanaController::HandleMapClick()
{
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_GameTraceChannel1, false, Hit))
	{
		ATile* Tile = Cast<ATile>(Hit.GetActor());
		if (Tile->IsRevealed()) {
			if (Tile->HasParty())
				GameMode->RequestPartyAction(GameMode->GetPlayerParty(), Cast<ATile>(Hit.GetActor())->GetCoords(), EPartyAction::EAttack);
			else
				GameMode->RequestPartyAction(GameMode->GetPlayerParty(), Cast<ATile>(Hit.GetActor())->GetCoords(), EPartyAction::ETravel);
		}
		else {
			GameMode->RequestPartyAction(GameMode->GetPlayerParty(), Cast<ATile>(Hit.GetActor())->GetCoords(), EPartyAction::EReveal);
		}
	}
}



void AArcanaController::HandleCombatClick()
{

}

void AArcanaController::HandleUnitClick()
{
	if (Arena->CanUnitMoveTo(GetCurrentUnit(), GetHoveredCell())) {
		Arena->SendActionSignal(GetCurrentCC()->MakeAction(EMoveAction, GetHoveredCell()));
		
	}
	else if (Arena->IsCellHasFlags(GetHoveredCell(), EDefender) && Arena->AreCellsWithinRange(GetCurrentCell(), GetHoveredCell(), GetCurrentCC()->GetStatValue(EStat::EMovement))) {
		UE_LOG(LogTemp, Warning, TEXT("Enemy"));
		Arena->SetUnitFacingAt(Arena->GetActiveUnit(), GetHoveredCell());
		Arena->SendActionSignal(GetCurrentCC()->MakeAction(EMoveAndAttack, GetHoveredCell()));
	}
}


bool AArcanaController::IsUserMode(FName TagName) const
{
	return UserMode.MatchesTag(TagsManager->RequestGameplayTag(TagName));
}

UParty* AArcanaController::GetParty()
{
	return GameMode->GetPlayerParty();
}

void AArcanaController::BeginPlay()
{
	ActivateMapMode();
	GameMode = GetWorld()->GetAuthGameMode<AArcanaBoardGameModeBase>();

	CombatWidget = CreateWidget<UCombatScreen>(this, CombatWidgetClass); // Create Widget
	if (!CombatWidget)
		return;

	CombatWidget->AddToViewport(); // Add it to the viewport so the Construct() method in the UUserWidget:: is run.
	CombatWidget->SetVisibility(ESlateVisibility::Hidden); // Set it to hidden so its not open on spawn.

}

void AArcanaController::OnLeftPressReceived()
{
	bIsLeftButtonPressed = true;

}

void AArcanaController::OnLeftReleaseReceived()
{
	if(ClickTimer <= ClickInterval)
		OnLeftButtonClick();

	bIsLeftButtonPressed = false;
	ClickTimer = 0;
}

void AArcanaController::OnLeftButtonClick()
{
// Process context
	if (IsUserMode("UserMode.Map")) {
		HandleMapClick(); 
	}
	else if (IsUserMode("UserMode.Combat.Default"))
	{
		HandleCombatClick();
	}
	else if (IsUserMode("UserMode.Combat.SelectedUnit"))
	{
		HandleUnitClick();
	}
}

void AArcanaController::Poke()
{
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_GameTraceChannel1, false, Hit))
	{
		FTransform Transform {Hit.Actor->GetTransform()};
		Transform.SetLocation(Hit.Location);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PokeEffect, Transform);
	}
}


FVector2D AArcanaController::GetHoveredCell() const
{
	FVector Temp = GetMouseProjectedToPlane(FVector(0,0, -64.24897f));
	Temp = SnapToGrid(Temp);
	Temp = SnapWithinRegion(Temp);
	return LocationToCombatCell(Temp);
}

AUnit* AArcanaController::GetCurrentUnit() const
{
	return Arena->IsPlayerTurn() ? Arena->GetActiveUnit() : nullptr;
}

FVector2D AArcanaController::GetCurrentCell() const
{
	return Arena->IsPlayerTurn() ? Arena->GetActiveCC()->GetCoords() : -FVector2D::UnitVector;
}

UCombatComponent* AArcanaController::GetCurrentCC() const
{
	return Arena->IsPlayerTurn() ? Arena->GetActiveCC() : nullptr;
}

void AArcanaController::OnTransitionToCombatFinished_Implementation()
{
	CombatWidget->SetVisibility(ESlateVisibility::Visible);
	Arena->OnReadyToBattle();
}


void AArcanaController::BindArena(AArena* InArena)
{
	Arena = InArena;
	SnapValue = Arena->GetCellSize()*2;
	SetSnapRegion(Arena->GridSize, Arena->GetActorLocation());
	
	Arena->GetCombatEngine()->OnUpdateUI.AddDynamic(CombatWidget, &UCombatScreen::OnUpdateUI);
	//ActivateCombatMode();
}


void AArcanaController::ActivateMapMode()
{
	UserMode = TagsManager->RequestGameplayTag(FName("UserMode.Map"));
}

void AArcanaController::ActivateCombatMode()
{
	UserMode = TagsManager->RequestGameplayTag(FName("UserMode.Combat.Default"));
}

void AArcanaController::ActivateUnitMode()
{
	UserMode = TagsManager->RequestGameplayTag(FName("UserMode.Combat.SelectedUnit"));
}

FVector AArcanaController::GetMouseProjectedToPlane(const FVector PlanePosition) const
{
	const FVector CameraLocation{ PlayerCameraManager->GetCameraLocation() };
	const float MaxShootDistance{ 30000.0f };
	FPlane Plane{ PlanePosition,  FVector::UpVector };
	FVector MouseLocation, MouseDirection;	
	DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
	MouseLocation = FMath::LinePlaneIntersection(CameraLocation, CameraLocation + (MouseDirection * MaxShootDistance), Plane);

	return MouseLocation;
}

FVector AArcanaController::SnapToGrid(const FVector Location) const
{
	return FVector (FMath::GridSnap(Location.X, SnapValue), FMath::GridSnap(Location.Y, SnapValue), Location.Z);
	//return V.BoundToBox(SnapRegionMin, SnapRegionMax);
}

FVector AArcanaController::SnapWithinRegion(FVector Location) const
{
	return Location.BoundToBox(SnapRegionMin, SnapRegionMax);
}

FVector2D AArcanaController::LocationToCombatCell(FVector Loc) const
{
	FVector Location = Loc;
	Location = (Location - Arena->GetActorLocation())/SnapValue + FVector(Arena->GridSize/2,0);

	return FVector2D((int32)Location.X, (int32)Location.Y);
}

void AArcanaController::SetSnapRegion(FVector2D GridSize, FVector Origin) 
{
	const float H = 0.5f;
	SnapRegionMin = { (Origin) - (SnapValue*FVector(GridSize.X/2, GridSize.Y/2,0))};
	SnapRegionMin.Y += SnapValue * H;
	SnapRegionMin.X += SnapValue * H;
	SnapRegionMax = { SnapToGrid(Origin) + (SnapValue * FVector(GridSize.X / 2, GridSize.Y / 2,0)) };
	SnapRegionMax.X -= SnapValue * H;
	SnapRegionMax.X -= 1;
	SnapRegionMin.X -= 1;
	SnapRegionMax.X = (int32)SnapRegionMax.X;
	SnapRegionMax.Y = (int32)SnapRegionMax.Y;
	SnapRegionMin.X = (int32)SnapRegionMin.X;
	SnapRegionMin.Y = (int32)SnapRegionMin.Y;
}

void AArcanaController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(IsLeftButtonHold())
		ClickTimer+=DeltaTime;

}
