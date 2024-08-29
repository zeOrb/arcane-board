// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArcanaClasses.h"
#include "Tile.generated.h"

UCLASS()
class ARCANABOARD_API ATile : public AActor
{
	GENERATED_BODY()
private:
	uint8 bIsRevealed : 1;


public:	
	// Sets default values for this actor's properties
	ATile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tile)
	USceneComponent* SceneComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Tile)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Tile)
	UStaticMeshComponent* TempFogComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Tile)
	FVector2D Coords;

	int32 DistrictID;

	int32 Floor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile")
	TMap<EStructure, int32> StructureMap;

public:
	bool IsRevealed() const;

	int32 GetFloor() const;

	FVector2D GetCoords() const;

	void AddStructure(EStructure Type, int32 Directions);


	bool HasStructure(EStructure Type);

	bool HasStructureInDir(EStructure Structure, int32 Direction);

	void RemoveStructureInDir(EStructure Type, int32 Direction);
	
	int32 CountStructures(EStructure Type);

	UFUNCTION(BlueprintCallable)
	void TestStructureMap();

private:
// Automate
	const int32 MaxPartySize {9};



	class UParty* Party;


	
	
public: //Units
	
	FORCEINLINE bool HasParty() const {return Party != nullptr;};

	FORCEINLINE class UParty* GetParty() {return Party;}

	void RegisterParty(class UParty* InParty);

	void UnregisterParty();

	void OnPartyEnter();

	UFUNCTION(BlueprintImplementableEvent)
	void OnReveal();

	void Reveal();


};
