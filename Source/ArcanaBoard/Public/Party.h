// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Party.generated.h"

class AUnit;

UCLASS()
class ARCANABOARD_API UParty : public UObject
{
	GENERATED_BODY()
protected:	
	TArray<AUnit*> Members;



public:
	class ATile* CurTile;

	class ATile* NextTile;

	void OnCreate(class ATile* InTile);
	
	void OnTravel(class ATile* InTile);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	class ATile* GetCurrentTile() const;

	void SetDestination(class ATile* InTile);

	class ATile* GetDestination();

	bool IsSingle() const;

	int32 GetMembersCount() const;

	UFUNCTION(BlueprintCallable)
	TArray<AUnit*>& GetMembers() ;

	AUnit* GetMember(int32 ID) const;


	bool RegisterMember(AUnit* NewMember);

};
