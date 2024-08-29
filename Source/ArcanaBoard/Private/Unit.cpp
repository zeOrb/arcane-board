// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTags/Classes/GameplayTagsManager.h"
#include "CombatComponent.h"
#include "Components/SceneComponent.h"

UGameplayTagsManager& AUnit::TagsManager = UGameplayTagsManager::Get();
FGameplayTag AUnit::WeaponTag = AUnit::TagsManager.RequestGameplayTag("Equipment.Weapon");
FGameplayTag AUnit::OneHanded = AUnit::TagsManager.RequestGameplayTag("Equipment.Weapon.OneHanded");
FGameplayTag AUnit::TwoHanded = AUnit::TagsManager.RequestGameplayTag("Equipment.Weapon.TwoHanded");

// Sets default values
AUnit::AUnit()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));
	SetRootComponent(RootSceneComp);

	SkelComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Component"));
	SkelComp->SetupAttachment(GetRootComponent());

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));

	InitializeItemSlot(EEquipType::ELeftHand);
	InitializeItemSlot(EEquipType::ERightHand);
	InitializeItemSlot(EEquipType::ETwoHands);
	Stats = FUnitStats();
}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();

	CombatComp->Initialize(this);
}

void AUnit::RegisterParty(class UParty* Party)
{
	PartyMembership = Party;
}

void AUnit::InitializePrimaryStats(const FUnitStats& InStats)
{
	Stats = InStats;
}


void AUnit::GenerateSecondaryStats()
{
	GetStat(EStat::EHealth).SetValue(GetStat(EStat::EVitality).GetValue() + FMath::RandRange(1,6));
	GetStat(EStat::EMovement).SetValue(GetStat(EStat::EAgility).GetValue());
}


FStatLine& AUnit::GetStat(EStat Type)
{
	switch (Type)
	{
	case EStat::EInvalid:
	default:
		checkNoEntry();
	case EStat::EStrength:
		return Stats.Strength;
	case EStat::EAgility:
		return Stats.Agility;
	case EStat::EVitality:
		return Stats.Vitality;
	case EStat::EHealth:
		return Stats.Health; 
	case EStat::EInitiative:
		return Stats.Initiative; 
	case EStat::EMovement:
		return Stats.Movement; 
	}
	return Stats.Movement;
}

const FStatLine& AUnit::GetStatSafe(EStat Type) const
{
	switch (Type)
	{
	case EStat::EInvalid:
	default:
		checkNoEntry();
	case EStat::EStrength:
		return Stats.Strength;
	case EStat::EAgility:
		return Stats.Agility;
	case EStat::EVitality:
		return Stats.Vitality;
	case EStat::EHealth:
		return Stats.Health;
	case EStat::EInitiative:
		return Stats.Initiative;
	case EStat::EMovement:
		return Stats.Movement;
	}
	return Stats.Movement;
}

UCombatComponent* AUnit::GetCC() const
{
	return CombatComp;
}

int32 AUnit::GetCurrentHealth() const
{
	return CurrentHealth;
}

int32 AUnit::GetMovementRange() const
{
	return GetStatSafe(EStat::EMovement).GetValue();
}

bool AUnit::HasTakenTurn() const
{
	return CombatComp->bFinished;
}

void AUnit::InitializeInventory(const TArray<struct FItem>& Items)
{
	Inventory.Append(Items);
	AutoEquipBestWeapon();
	AutoEquipBestWeapon();
}

void AUnit::InitializeItemSlot(EEquipType Slot)
{
	FName TempSocket {};
	switch (Slot)
	{
	case EEquipType::EInvalid:
	case EEquipType::ENonEquippable:
	case EEquipType::EBody:
	case EEquipType::EHead:
	case EEquipType::EAccessory:
	default:
		TempSocket = "Incorrect";
		break;
	case EEquipType::ELeftHand:
		TempSocket = "Left Hand Comp";
		break;
	case EEquipType::ERightHand:
		TempSocket = "Right Hand Comp";
		break;
	case EEquipType::ETwoHands:
		TempSocket = "Two Hand Comp";
		break;
	}
	EquipmentSlots.Add(Slot);
	EquipmentSlots[Slot].ItemMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TempSocket);
	EquipmentSlots[Slot].ItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquipmentSlots[Slot].ItemMeshComponent->SetupAttachment(SkelComp);
}


void AUnit::AutoEquipBestWeapon()
{
	FItem* BestWeapon {};
	int32 LastVal {0};
	int32 BestVal {0};
	for (FItem& Item : Inventory) {
		if (IsWeapon(Item) && !Item.IsEquipped()) {
			LastVal = GenerateValueForWeapon(Item);
			if(LastVal >= BestVal) {
				BestWeapon = &Item;
				BestVal = LastVal;
			}
		}
	}
	if(!BestWeapon) return;

	// Figure out Slot you want to use to equip your weapon
	// Figure it out based on 2 params: Equip Type and availability of slots
	// 4 cases: lH is free, rH is used | lH is used, rH is free| both free | both used

	if (BestWeapon->EquipType.MatchesTag(OneHanded)) {
		if (IsWeaponEquippedInHand(EEquipType::ELeftHand)) {
			if (IsWeaponEquippedInHand(EEquipType::ERightHand)) {
				// Left and Right busy
			}
			else {
				EquipWeapon(*BestWeapon, EEquipType::ERightHand);
			}
		}
		else {
			EquipWeapon(*BestWeapon, EEquipType::ELeftHand);
		}
	}
	else if (BestWeapon->EquipType.MatchesTag(TwoHanded)) {
		if (IsWeaponEquippedInHand(EEquipType::ETwoHands)) {
			// Two hands are busy
		}
		else {
			EquipWeapon(*BestWeapon, EEquipType::ETwoHands);
		}
	}
}

int32 AUnit::GenerateValueForWeapon(const FItem& Weapon) const
{
	int32 Value {0};
	Value += (6-Weapon.ValueForHit);
	Value += (Weapon.ValueForCrit <= 6) ? (8 - Weapon.ValueForCrit) : 0;
	for (auto& EffectPair : Weapon.EffectsCollection) {
		if (EffectPair.Key == EEffectTrigger::EOnHit) {
			for (auto& Effect : EffectPair.Value.Effects) {
				if (Effect.Action == EEffectAction::EDealDamage) {
					Value += Effect.Power;
					Value += (Effect.DamageTypeTag.GetTagName() == "DamageType.Piercing") ? 1 : 0;
					Value += (Effect.DamageTypeTag.GetTagName() == "DamageType.Fire") ? 3 : 0;
				}
			}
		}
	}
	return Value;
}

void AUnit::EquipWeapon(FItem& Weapon, EEquipType SlotType)
{
	// In Combat Equip
	auto& Slot = GetSlot(SlotType);
	UnequipWeapon(SlotType);
	Slot.Item = &Weapon;
	Slot.bIsSheathed = true;
	Slot.Item->bIsEquipped = true;
	Slot.ItemMeshComponent->SetStaticMesh(Weapon.Mesh);
	Slot.ItemMeshComponent->AttachToComponent(SkelComp, FAttachmentTransformRules::SnapToTargetIncludingScale, GetSocketName(Weapon, SlotType, true));
}

bool AUnit::UnequipWeapon(EEquipType SlotType)
{
	auto& Slot = GetSlot(SlotType);

	if (Slot.HasItem()) {
		Slot.bIsSheathed = false;
		Slot.Item->bIsEquipped = false;
		Slot.ItemMeshComponent->SetStaticMesh(nullptr);
		return true;
	}
	return false;
}

void AUnit::ToggleWeaponSheath()
{
	if(GetSlot(EEquipType::ELeftHand).HasItem())
		ToggleSlotSheath(EEquipType::ELeftHand);
	if(GetSlot(EEquipType::ERightHand).HasItem())
		ToggleSlotSheath(EEquipType::ERightHand);
	if(GetSlot(EEquipType::ETwoHands).HasItem())
		ToggleSlotSheath(EEquipType::ETwoHands);
}

void AUnit::ToggleSlotSheath(EEquipType SlotType) 
{
	auto& Slot = GetSlot(SlotType);
	if (Slot.HasItem()) {
		Slot.bIsSheathed = !Slot.bIsSheathed;
		Slot.ItemMeshComponent->AttachToComponent(SkelComp, FAttachmentTransformRules::SnapToTargetIncludingScale, GetSocketName(*Slot.Item, SlotType, Slot.bIsSheathed));
	}
}

FEquipmentSlot& AUnit::GetSlot(EEquipType Slot)
{
	return EquipmentSlots[Slot];
}

bool AUnit::IsWeapon(const FItem& Item)
{
	return Item.EquipType.MatchesTag(WeaponTag);
}

bool AUnit::IsWeaponEquippedInHand(EEquipType Hand) 
{
	switch (Hand)
	{
	case EEquipType::ELeftHand:
	case EEquipType::ERightHand:
	case EEquipType::ETwoHands:
		return (GetSlot(Hand).HasItem() || GetSlot(EEquipType::ETwoHands).HasItem());
	case EEquipType::EInvalid:
	case EEquipType::ENonEquippable:
	case EEquipType::EBody:
	case EEquipType::EHead:
	case EEquipType::EAccessory:
	default:
		checkNoEntry()
	}
	return false;
}

bool AUnit::IsEquipSlotExist(EEquipType Slot) const
{
	return !EquipmentSlots.Contains(Slot);
}

FName AUnit::GetSocketName(const FItem& Item, EEquipType Slot, bool bIsSheathed) const
{
	// Should be a map of fnames for every sheathed/unsheathed position

	FString TagNameString = Item.EquipType.GetTagName().ToString();
	TagNameString.RemoveFromStart("Equipment.Weapon."); //OneHanded.Medium/Small/Large

	switch (Slot)
	{
	case EEquipType::EInvalid:
	case EEquipType::ENonEquippable:
	case EEquipType::EBody:
	case EEquipType::EHead:
	case EEquipType::EAccessory:
	default:
		checkNoEntry();
	case EEquipType::ELeftHand:
		if(bIsSheathed)
			TagNameString.Append(".L"); 
		else
			TagNameString = "LeftHandSocket";
		break;
	case EEquipType::ERightHand:
		if(bIsSheathed)
			TagNameString.Append(".R"); 
		else
			TagNameString = "RightHandSocket";
		break;
	case EEquipType::ETwoHands:
		if(!bIsSheathed)
			TagNameString.Append("Socket");
		break;
	}
	return FName(*TagNameString);
}

// Called every frame
void AUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

const FUnitStats& AUnit::GetStats() const
{
	return Stats;
}

