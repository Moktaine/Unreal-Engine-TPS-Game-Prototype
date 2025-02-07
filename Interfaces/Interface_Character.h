// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Enums_PlayerCharacter.h"
#include "Interface_Character.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInterface_Character : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTKEATS_API IInterface_Character
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void GotShot(FVector ImpactPoint, FVector ForceDirection, FName BoneName){}

	virtual void DealDamage(float DamageAmount){}

	virtual void SetCanShoot(bool bCanShoot){}

	virtual ECombatState_Enemy GetCombatState_Enemy() { return ECombatState_Enemy::Default; }

	virtual void SetCombatState_Enemy(ECombatState_Enemy EnemyState) {}

	virtual void Enemy_PlayMontage(UAnimMontage* Montage_Enemy) {}

	virtual void Enemy_MotionWarp(FName WarpTarget, FVector Location, FRotator Rotation){}

	virtual void SetCanAimDuringTakedown(ETakedownState Player_TakedownState){}

	virtual void SetCanAimDuringVault(EVaultState Player_VaultState) {}

	virtual void SetTakedownState(ETakedownState Player_TakedownState) {}

	virtual void SetCombatState(ECombatState Player_CombatState){}

	virtual void SetCharacterRotationMode(bool bIsStrafing){}

	virtual void SetCollisionProfile(FName CapsuleCollisionProfile, FName MeshCollisionProfile, bool bSetCapsuleCollision = true, bool bSetMeshCollision = true){}

	virtual void SetCanMove_Look(bool bCanMove = true, bool bCanLook = true){}

	virtual void PushEnemy(){}
};
