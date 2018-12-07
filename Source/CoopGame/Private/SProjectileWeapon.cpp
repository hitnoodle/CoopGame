// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/SProjectileWeapon.h"
#include "../Public/SProjectile.h"
#include "Components/SkeletalMeshComponent.h"

ASProjectileWeapon::ASProjectileWeapon()
{

}

void ASProjectileWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* owner = GetOwner();
	if (owner && ProjectileClass)
	{
		FVector muzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector eyeLocation;
		FRotator eyeRotation;
		owner->GetActorEyesViewPoint(eyeLocation, eyeRotation);

		FActorSpawnParameters actorSpawnParams;
		actorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, muzzleLocation, eyeRotation, actorSpawnParams);
	}
}