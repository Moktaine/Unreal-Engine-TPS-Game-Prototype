// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface_Character.h"
#include "Interface_EnemyABP.h"
#include "Enums_PlayerCharacter.h"
#include "Enemy_Base.generated.h"


class UNiagaraSystem;
class USoundBase;




UCLASS(config=Game)
class PROJECTKEATS_API AEnemy_Base : public ACharacter, public IInterface_Character, public IInterface_EnemyABP
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pistol, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Pistol;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UMotionWarpingComponent* MotionWarping;

public:
	// Sets default values for this character's properties
	AEnemy_Base();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
		UNiagaraSystem* NS_Blood_Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
		UNiagaraSystem* NS_Blood_Head;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_HitReaction_Shoulder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_HitReaction_Stomach;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_HitReaction_Leg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_HumanShield_Push;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_HumanShield_Push_Secondary;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Effects")
		USoundBase* SB_BulletHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Effects")
		USoundBase* SB_HardStunned;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
	float DefaultHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
	float ForceMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECombatState_Enemy CombatState_Enemy {ECombatState_Enemy::Default };
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float Health;

	void Died(FVector ImpactPoint, FVector ForceDirection, FName BoneName);

	void PushTrace();

	TArray<AActor*, FDefaultAllocator> IgnoredActorsPush;

	void EndPush(UAnimMontage* animMontage, bool bInterrupted);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void GotShot(FVector ImpactPoint, FVector ForceDirection, FName BoneName) override;

	virtual void DealDamage(float DamageAmount) override;

	virtual ECombatState_Enemy GetCombatState_Enemy() override;

	virtual void Enemy_MotionWarp(FName WarpTarget, FVector Location, FRotator Rotation) override;

	virtual void SetCombatState_Enemy(ECombatState_Enemy EnemyState) override;

	virtual void Enemy_PlayMontage(UAnimMontage* Montage_Enemy) override;

	virtual void SetCollisionProfile(FName CapsuleCollisionProfile, FName MeshCollisionProfile, bool bSetCapsuleCollision, bool bSetMeshCollision) override;

	virtual void PushEnemy() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
};
