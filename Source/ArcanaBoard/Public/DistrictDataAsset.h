// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ArcanaClasses.h"
#include "DistrictDataAsset.generated.h"
USTRUCT()
struct FDistrictData {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText Name;

	UPROPERTY(EditAnywhere)
	FTablePool PartiesPool;

	UPROPERTY(EditAnywhere)
	FColor DistrictColor;

	FDistrictData() {};
};


/**
 * 
 */
UCLASS()
class ARCANABOARD_API UDistrictDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "District")
	FDistrictData Data;
};
