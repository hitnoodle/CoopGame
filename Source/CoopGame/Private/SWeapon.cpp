// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/SWeapon.h"
#include "../CoopGame.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

static int32 DebugWeaponDrawings;
FAutoConsoleVariableRef CVarDebugWeaponDrawings(TEXT("COOP.DebugWeaponDrawings"),
	DebugWeaponDrawings,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TraceTargetName = "BeamEnd";

	BaseDamage = 20.0f;
	VulnerableDamageModifier = 4.0f;
	FireRate = 600;

	SetReplicates(true);
}

void ASWeapon::BeginPlay()
{
	TimeBetweenFire = 60.0f / FireRate;
}

void ASWeapon::StartFire()
{
	float firstDelay = LastFireTime + TimeBetweenFire - GetWorld()->TimeSeconds;
	GetWorldTimerManager().SetTimer(TimerWeaponHandle, this, &ASWeapon::Fire, TimeBetweenFire, true, firstDelay);
}

void ASWeapon::EndFire()
{
	GetWorldTimerManager().ClearTimer(TimerWeaponHandle);
}

void ASWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* owner = GetOwner();
	if (owner)
	{
		FVector eyeLocation;
		FRotator eyeRotation;
		owner->GetActorEyesViewPoint(eyeLocation, eyeRotation);

		FVector shotDirection = eyeRotation.Vector();
		FVector traceEnd = eyeLocation + (shotDirection * 10000);

		FCollisionQueryParams queryParams;
		queryParams.AddIgnoredActor(owner);
		queryParams.AddIgnoredActor(this);
		queryParams.bTraceComplex = true;
		queryParams.bReturnPhysicalMaterial = true;

		FVector traceEndLocation = traceEnd;

		FHitResult hit;
		if (GetWorld()->LineTraceSingleByChannel(hit, eyeLocation, traceEnd, COLLISION_WEAPON, queryParams))
		{
			AActor* hitActor = hit.GetActor();
			EPhysicalSurface surface = UPhysicalMaterial::DetermineSurfaceType(hit.PhysMaterial.Get());
			
			// Damage

			float actualDamage = BaseDamage;
			if (surface == SURFACE_FLESHVULNERABLE)
			{
				actualDamage *= VulnerableDamageModifier;
			}

			UGameplayStatics::ApplyPointDamage(hitActor, actualDamage, shotDirection, hit, owner->GetInstigatorController(), this, DamageType);

			// Hit Effects

			UParticleSystem* hitEffect = nullptr;
			switch (surface)
			{
				case SURFACE_FLESHDEFAULT:
				case SURFACE_FLESHVULNERABLE:
					hitEffect = FleshImpactEffect;
					break;
				default:
					hitEffect = DefaultImpactEffect;
					break;
			}

			if (hitEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), hitEffect, hit.ImpactPoint, hit.ImpactNormal.Rotation());
			}

			traceEndLocation = hit.Location;
		
			// General Effects
			PlayWeaponEffects(traceEndLocation);

			LastFireTime = GetWorld()->TimeSeconds;

			if (DebugWeaponDrawings > 0)
			{
				DrawDebugLine(GetWorld(), eyeLocation, traceEnd, FColor::White, false, 1.0f, 0, 1.0f);
			}
		}
	}
}

void ASWeapon::PlayWeaponEffects(FVector TraceEnd)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TraceEffect)
	{
		FVector muzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* traceEffect = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TraceEffect, muzzleLocation);

		if (traceEffect)
		{
			traceEffect->SetVectorParameter(TraceTargetName, TraceEnd);
		}
	}

	if (FireShakeEffect)
	{
		APawn* pawnOwner = Cast<APawn>(GetOwner());
		if (pawnOwner)
		{
			APlayerController* pc = Cast<APlayerController>(pawnOwner->GetController());
			if (pc)
			{
				pc->ClientPlayCameraShake(FireShakeEffect);
			}
		}
	}
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}
