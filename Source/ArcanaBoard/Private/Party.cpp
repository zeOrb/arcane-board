// Fill out your copyright notice in the Description page of Project Settings.


#include "Party.h"
#include "Unit.h"

void UParty::OnCreate(class ATile* InTile)
{
	CurTile = InTile;
}

void UParty::OnTravel(class ATile* InTile)
{
	CurTile = InTile;
}

class ATile* UParty::GetCurrentTile() const
{
	return CurTile;
}

void UParty::SetDestination(class ATile* InTile)
{
	NextTile = InTile;
}

class ATile* UParty::GetDestination()
{
	return NextTile;
}

bool UParty::IsSingle() const
{
	return Members.Num() ==1;
}

int32 UParty::GetMembersCount() const
{
	return Members.Num();
}

TArray<AUnit*>& UParty::GetMembers() 
{
	return Members;
}

AUnit* UParty::GetMember(int32 ID) const
{
	return ID < Members.Num() ? Members[ID] : nullptr;
}

bool UParty::RegisterMember(AUnit* NewMember)
{
	// Make checks if can be added

	if(Members.Contains(NewMember))
		return false;
	
	Members.Add(NewMember);
	NewMember->RegisterParty(this);
	return true;
}
