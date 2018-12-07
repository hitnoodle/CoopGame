// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	HealthDefault = 100.0f;
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* myOwner = GetOwner();
	if (myOwner)
	{
		myOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}

	Health = HealthDefault;
}

void UHealthComponent::HandleTakeAnyDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Damage <= 0.f)
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.f, HealthDefault);

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamagedActor);
}

