// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/SProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

static int32 DebugProjectileDrawings;
FAutoConsoleVariableRef CVarDebugProjectileDrawings(TEXT("COOP.DebugProjectileDrawings"),
	DebugProjectileDrawings,
	TEXT("Draw Debug Area for Projectiles"),
	ECVF_Cheat);

ASProjectile::ASProjectile()
{
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);

	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	LifeSpan = 1.0f;

	DamageRadius = 1.0f;
}

void ASProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSpan);
}

void ASProjectile::Destroyed()
{
	Super::Destroyed();

	Explode();
}

void ASProjectile::Explode()
{
	FVector explodeLocation = GetActorLocation();
	TArray<AActor*> ignoredActors;

	UGameplayStatics::ApplyRadialDamage(GetWorld(), 100.0f, explodeLocation, DamageRadius, DamageType, ignoredActors);

	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, explodeLocation);
	}

	if (DebugProjectileDrawings > 0)
	{
		DrawDebugSphere(GetWorld(), explodeLocation, DamageRadius, 12, FColor::White, false, 1.0f);
	}
}