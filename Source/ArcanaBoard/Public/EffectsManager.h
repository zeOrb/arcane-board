// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "EffectsManager.generated.h"

UCLASS()
class ARCANABOARD_API AEffectsManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEffectsManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Effects)
	TMap<FGameplayTag, class UParticleSystem*> ElementParticles;

	class UParticleSystem* GetParticlesForElement(const FGameplayTag& Tag);

};
