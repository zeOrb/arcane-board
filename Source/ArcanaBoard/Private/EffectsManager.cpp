// Fill out your copyright notice in the Description page of Project Settings.


#include "EffectsManager.h"

AEffectsManager::AEffectsManager()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AEffectsManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEffectsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

class UParticleSystem* AEffectsManager::GetParticlesForElement(const FGameplayTag& Tag)
{
	if(ElementParticles.Contains(Tag))
		return ElementParticles[Tag];

	return nullptr;
}

