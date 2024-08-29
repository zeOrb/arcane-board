// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "ArcanaClasses.generated.h"


/*======================		COMBAT			====================*/
/*=================================================================*/
// Start Battle
// Begin Round
// Roll for initiative
// Begin Turn
// - Wait for signal
// - Deny next step unless signal is received
// Resolve turn
// - Execute signal
// - Wait for 


UENUM(BlueprintType)
enum class EEffectTrigger : uint8
{
	ENone			= 0,
	EOnUse			UMETA(DisplayName = "On Use"),
	EOnHit			UMETA(DisplayName = "On Hit"),
	EOnCrit			UMETA(DisplayName = "On Critical"),
	EOnMiss			UMETA(DisplayName = "On Miss"),
	EOnThrow		UMETA(DisplayName = "On Throw"),
	EOnRoundStart	UMETA(DisplayName = "On Round Start"),
	EOnRoundFinish	UMETA(DisplayName = "On Round End"),
	EOnMapUse		UMETA(DisplayName = "On Map usage")
};

UENUM(BlueprintType)
enum class EEffectAction :uint8
{
	EDealDamage		UMETA(DisplayName = "Deal damage"),
	ERestoreHealth	UMETA(DisplayName = "Restore health"),
	EBlockDamage	UMETA(DisplayName = "Block damage"),
	ESetOnFire		UMETA(DisplayName = "Set on fire"),
	EGrantImmunity	UMETA(DisplayName = "Grant immunity"),
	EGrantEffect	UMETA(DisplayName = "Grant effect"),
	ECleanseEffect	UMETA(DisplayName = "Cleanse effect")
};


USTRUCT(BlueprintType)
struct ARCANABOARD_API FPersistentEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PersistentEffect)
	class AUnit* Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PersistentEffect)
	EEffectTrigger TriggerTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PersistentEffect)
	EEffectAction Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PersistentEffect)
	int32 Power {1};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PersistentEffect)
	int32 Duration{1};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PersistentEffect)
	FGameplayTag ElementTag;

	void Tick() { Duration = FMath::Clamp(Duration-1, 0, 999); }

	bool IsExpired() const { return Duration == 0; }

	FPersistentEffect() {};

	bool operator==(const FPersistentEffect& B) { return TriggerTime == B.TriggerTime && Action == B.Action && Power == B.Power && ElementTag == B.ElementTag; }

	FPersistentEffect(class AUnit* InSource, EEffectTrigger Trigger, EEffectAction InAction, int32 InPow, int32 InDur, FGameplayTag InElems) 
		: Source(InSource)
		, TriggerTime(Trigger)
		, Action(InAction)
		, Power(InPow)
		, Duration(InDur)
		, ElementTag(InElems) {};
	
};


USTRUCT(BlueprintType)
struct ARCANABOARD_API FGameEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameEffect)
	EEffectAction Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameEffect)
	int32 Power {1};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameEffect)
	FGameplayTag DamageTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = GameEffect)
	FPersistentEffect GrantedEffect;

	FGameEffect() {};

	FGameEffect(EEffectAction InAct, FGameplayTag InElems) : Action(InAct), DamageTypeTag(InElems) {};

	FGameEffect(EEffectAction InAct, int32 Pow, FGameplayTag InElems) : Action(InAct), Power(Pow), DamageTypeTag(InElems) {};
};

USTRUCT(BlueprintType)
struct ARCANABOARD_API  FEffectsContainer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameEffect)
	TArray<FGameEffect> Effects;
};



USTRUCT(BlueprintType)
struct ARCANABOARD_API FItem : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	FGameplayTag EquipType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	FGameplayTag ItemType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	UStaticMesh* Mesh;

 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
 	UParticleSystem* Particles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Effects)
	TMap<EEffectTrigger, FEffectsContainer> EffectsCollection; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	int32 ValueForHit {4};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	int32 ValueForCrit {6};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	uint8 bIsEquipped :1;

	int64 UID;

	bool operator==(const FItem& AnotherItem) const { 
		return Name.CompareTo(AnotherItem.Name) && UID == AnotherItem.UID;
	}

	bool IsValid() const { return !Name.IsEmpty() && UID != 0; }

	bool IsEquipped() const {return bIsEquipped;};

	bool IsEquippable () const {return EquipType.RequestDirectParent().GetTagName() == "Equipment";};

	int32 GetWeaponRange() const {return 1;};

};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	EInvalid = 0,
	EBattleStart =1 UMETA(DisplayName = "Battle has started!"),
	ERoundBegin		UMETA(DisplayName = "New Round"),
	EInitiativeRoll	UMETA(DisplayName = "Initiative phase"),
	ETurnBegin		UMETA(DisplayName = "Turn Begin"),
	ETurnResolve	UMETA(DisplayName = "Turn Resolve"),
	ERoundResolve	UMETA(DisplayName = "Round Resolve"),
	EBattleResolve	UMETA(DisplayName = "Battle Resolve")
};

USTRUCT(BlueprintType)
struct ARCANABOARD_API FBattleData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BattleData")
	int32 Round {0};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BattleData")
	int32 Turn {-1};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BattleData")
	ECombatState State;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BattleData")
	uint8 bIsPlayerTurn :1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BattleData")
	TArray<class AUnit*> Queue;

};

UENUM(BlueprintType, Category = "Combat|Action")
enum ECombatActionType 
{
	ENoneAction			= 0,
	EMoveAction,			
	EAttackAction,
	EMoveAndAttack,
	EUseItemAction,		
	EThrowItemAction,	
	EDodgeAction,		
	EBlockAction,		
	EEscapeAction		
};


USTRUCT(BlueprintType)
struct ARCANABOARD_API FCombatAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Action", meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
	TEnumAsByte<ECombatActionType> Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Action")
	FVector2D Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Action")
	TArray<FVector2D> Targets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Action")
	FItem Item;

	FCombatAction() : Type(ENoneAction), Source(-FVector2D::UnitVector), Targets(), Item() {};

};

UENUM(BlueprintType)
enum class EStat : uint8
{
	EInvalid = 0,
	EStrength UMETA(DisplayName = "Strength"),
	EAgility UMETA(DisplayName = "Agility"),
	EVitality UMETA(DisplayName = "Vitality"),
	EHealth UMETA(DisplayName = "Health"),
	EInitiative UMETA(DisplayName = "Initiative"),
	EMovement UMETA(DisplayName = "Movement")
};

UENUM(BlueprintType)
enum class EStatModType : uint8
{
	EInvalid = 0,
	ENullify = 1,
	EDecrease = 2,
	EIncrease = 3,
	EDouble = 4
};

USTRUCT(BlueprintType)
struct ARCANABOARD_API FStatMod
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatLine")
	EStatModType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatLine")
	int32 ModValue;

	FStatMod() :Type(EStatModType::EInvalid), ModValue(0) {}

};

USTRUCT(BlueprintType)
struct ARCANABOARD_API FStatLine
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatLine")
	EStat Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatLine")
	int32 Value;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "StatLine")
	TArray<FStatMod> Modificators;

	
	FORCEINLINE int32 GetValue() const {return Value;}
	
	
	FORCEINLINE void SetValue(int32 V)  {Value= V;};

	FStatLine(): Type(EStat::EInvalid), Value(0){};

	FStatLine(EStat InType, int32 InValue=0) : Type(InType), Value(InValue){};
};


USTRUCT(BlueprintType)
struct ARCANABOARD_API FUnitStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FStatLine Strength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FStatLine Agility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FStatLine Vitality;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	FStatLine Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	FStatLine Initiative;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	FStatLine Movement;

	FUnitStats() {
		Strength	= FStatLine(EStat::EStrength, 0);
		Agility		= FStatLine(EStat::EAgility, 0);
		Vitality	= FStatLine(EStat::EVitality, 0);
		Health		= FStatLine(EStat::EHealth, 0);
		Initiative	= FStatLine(EStat::EInitiative, 0);
		Movement	= FStatLine(EStat::EMovement, 0);
	}
};




/*======================	ITEM & EFFECT	=======================*/
/*=================================================================*/



UENUM(BlueprintType)
enum class EEquipType : uint8
{
	EInvalid = 0,
	ENonEquippable,
	EBody,
	EHead,
	EAccessory,
	ELeftHand,
	ERightHand,
	ETwoHands
};



USTRUCT(BlueprintType)
struct ARCANABOARD_API FWeightedObject
{
	GENERATED_BODY()

	FWeightedObject(){};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weighted Object")
	float Weight {1};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weighted Object")
	FDataTableRowHandle RowHandle {};
};


USTRUCT(BlueprintType)
struct ARCANABOARD_API FEquipmentSlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equipment)
	class UStaticMeshComponent* ItemMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	FVector OffsetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	FRotator OffsetRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	uint8 bIsSheathed :1;

	struct FItem* Item;


	FEquipmentSlot() : OffsetLocation(FVector::ZeroVector), OffsetRotation(FRotator::ZeroRotator)
					 , bIsSheathed(false), Item(nullptr), ItemMeshComponent(nullptr) {};

	FORCEINLINE bool HasItem() const {return Item != nullptr;};
};

USTRUCT(BlueprintType)
struct ARCANABOARD_API FTablePool
{
	GENERATED_BODY()

	FTablePool() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pool")
	int32 MinAmount {1};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pool")
	int32 MaxAmount {1};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pool")
	TArray<FWeightedObject> WeightedHandles;

	int32 GetAmount() const { return  (MinAmount == MaxAmount == 1) ? 1 :FMath::RandRange(MinAmount, MaxAmount) ;};

	const FDataTableRowHandle& GetWeightedHandle() const 
	{
		float SumOfWeights { 0.0f };
		for (int32 i = 0; i < WeightedHandles.Num(); i++) {
			SumOfWeights += WeightedHandles[i].Weight;
		}
		float RolledWeight { FMath::RandRange(0.0f, SumOfWeights) };
		for (int32 i = 0; i < WeightedHandles.Num(); i++) {
			if(RolledWeight <= WeightedHandles[i].Weight)
				return WeightedHandles[i].RowHandle;
			else
				RolledWeight -= WeightedHandles[i].Weight;
		}
		return WeightedHandles[0].RowHandle;
	}
};


USTRUCT(BlueprintType)
struct ARCANABOARD_API FUnitTemplate : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Preset")
	FText Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Preset")
	FUnitStats DefaultStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Preset")
	TArray<FTablePool> ItemPool;

	FUnitTemplate() {};
};


/*======================		REQUEST		=======================*/
/*=================================================================*/

UENUM(BlueprintType)
enum class EMapRequest :uint8
{
	EInvalid = 0,
	ECreateUnit
};

UENUM(BlueprintType)
enum class EPartyAction :uint8
{
	EInvalid = 0,
	EReveal,
	ETravel,
	EAttack
};


/*======================		UNITS		=======================*/
/*=================================================================*/
USTRUCT(BlueprintType)
struct ARCANABOARD_API FPartyTemplate : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	TArray<FTablePool> UnitsPool;

	FPartyTemplate() {};
};

/*========================DATA TABLES==============================*/
/*=================================================================*/
USTRUCT(BlueprintType)
struct ARCANABOARD_API FTileVisualSet : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Visual Set")
	class UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Visual Set")
	class UMaterialInterface* Material;

	FTileVisualSet() {}
};


USTRUCT(BlueprintType)
struct ARCANABOARD_API FPathRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Rules")
	uint8 bUseEmpty : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Rules")
	uint8 bUseAnyTile : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Rules")
	uint8 bIncludeWalls : 1;

	FPathRules() : bUseEmpty(false), bUseAnyTile(false), bIncludeWalls(true) {}
};

USTRUCT(BlueprintType)
struct ARCANABOARD_API FPath
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Data")
	TArray<FVector2D> FinishedPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Data")
	FVector2D Start;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Data")
	FVector2D Finish;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Data")
	FPathRules Rules;

	FPath(): FinishedPath(), Start(FVector2D::ZeroVector), Finish(FVector2D::ZeroVector), Rules(){};

	FPath(FVector2D InS, FVector2D InF) : Start(InS), Finish(InF), Rules(){};
};


/*===============================GRID==============================*/
/*=================================================================*/
UENUM(BlueprintType, Category = "Grid|Directions", meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum EDirection
{
	EInvalidDirection	= 0,
	ENorth		= 1 << 1,
	EEast		= 1 << 2,
	ESouth		= 1 << 3,
	EWest		= 1 << 4
};

UENUM(BlueprintType)
enum class EStructure :uint8
{
	EInvalid,
	EOuterWalls,
	EDistrictWalls,
	ETowers,
	EBridges,
	EStairs,
	EGates
};

USTRUCT(BlueprintType)
struct ARCANABOARD_API FTileConstructRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Core")
	TSubclassOf<class ATile> TileBaseClass{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Core")
	FDataTableRowHandle DefaultTileVisualsRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Transform|Core")
	FVector TileScale {FVector::OneVector};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Transform|Core")
	float TileSpacing { 0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Transform|Core")
	float FloorDistance{0};
	   
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile|Transform|Core")
	FVector TileExtents{ FVector::ZeroVector };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Transform|Core")
	uint8 bIncludeActorOffset :1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Transform|Random")
	uint8 bIncludeRandomHeightOffset : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Transform|Random", meta = (EditCondition = "bIncludeRandomHeightOffset"))
	int32 HeightOffsetMaxStep;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Transform|Random", meta = (EditCondition = "bIncludeRandomHeightOffset"))
	float HeightOffsetStepSize;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Transform|Random")
	uint8 bIncludeRandomSlope :1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Transform|Random", meta = (EditCondition = "bIncludeRandomSlope"))
	FVector Slope {FVector::ZeroVector};

	FTileVisualSet DefaultTileVisuals{};

	FTileConstructRules(){};

	inline FVector GetTileSize() const { return (TileExtents + TileSpacing) * TileScale; }
};


