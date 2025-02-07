// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/Perception/AISightTargetInterface.h" 
#include "GameFramework/Character.h"
#include "Enums_PlayerCharacter.h"
#include "Interface_Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectKeatsCharacter.generated.h"


class UNiagaraSystem;
class USoundBase;

UCLASS(config=Game)
class AProjectKeatsCharacter : public ACharacter, public IInterface_Character, public IAISightTargetInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Pistol;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UMotionWarpingComponent* MotionWarping;
	
public:
	AProjectKeatsCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
		float TurnRateGamepad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
		float MouseSensitivity = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ECombatState CombatState {ECombatState::Default};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETakedownState TakedownState {ETakedownState::PerformingTakedown};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETakedownType TakedownType {ETakedownType::Soft};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EVaultState VaultState {EVaultState::Vaulting};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EHumanShieldState HumanShieldState {EHumanShieldState::HumanShieldBase};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ECoverTypes CoverType {ECoverTypes::HighCover};

	//----------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Character Attributes")
		float DefaultWalkSpeed_Base;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Character Attributes")
		float DefaultWalkSpeed_Aiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Character Attributes")
		float DefaultWalkSpeed_Sprinting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Character Attributes")
		float TakedownDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Character Attributes")
		float FreeTime_Takedown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Character Attributes")
		float FreeTime_HumanShield;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debuging")
		TEnumAsByte<EDrawDebugTrace::Type> VaultDebug;
	

	//----------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pistol Attributes")
		float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pistol Attributes")
		float DefaultAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool CanShoot;

	//----------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
		UNiagaraSystem* NS_MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
		UNiagaraSystem* NS_MuzzleSmoke;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
		UNiagaraSystem* NS_BulletTrail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
		UNiagaraSystem* NS_BulletImpact;

	//----------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundFX")
		USoundBase* SB_PistolShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundFX")
		USoundBase* SB_PistolEmpty;

	//----------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decals")
		UMaterialInterface* Decal_BulletHole;

	//----------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Classes")
		TSubclassOf<AActor> BP_Shell;

	//----------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shakes")
		TSubclassOf<UCameraShakeBase> CS_Shoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shakes")
		TSubclassOf<UCameraShakeBase> CS_Bob;

	//----------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_Shoot; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_Takedown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_Kill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_KnockOut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_FreeEnemy;


	//HARD Takedown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_Takedown_Hard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_Kill_Hard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_KnockOut_Hard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_FreeEnemy_Hard;


	//WALL Takedown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_Takedown_Wall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_Kill_Wall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_KnockOut_Wall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_FreeEnemy_Wall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_Vault;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_HumanShield_Grab;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_HumanShield_Shoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_HumanShield_KnockOut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_HumanShield_Free;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Character_HumanShield_Push;

	//Enemy Montages
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_Takedown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_Takedown_Killed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_Takedown_Knocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
	UAnimMontage* Montage_Enemy_FreeEnemy;

	//HARD Takedown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_Takedown_Hard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_Takedown_Killed_Hard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_Takedown_Knocked_Hard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_FreeEnemy_Hard;

	//WALL Takedown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_Takedown_Wall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_Takedown_Killed_Wall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_Takedown_Knocked_Wall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_FreeEnemy_Wall;


	//HumanShield
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_HumanShield_Grabbed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_HumanShield_Killed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_HumanShield_Knocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages-Enemy")
		UAnimMontage* Montage_Enemy_HumanShield_Free;

	
	//Pistol Montages
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Pistol_Shoot; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
		UAnimMontage* Montage_Pistol_Empty; 

	//----------------------------------------

	UAnimInstance* AnimInstance_Character;

	UAnimInstance* AnimInstance_Pistol;

protected:

	virtual void BeginPlay() override;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	bool CanMove = true;

	bool CanLook = true;

	void StartSprinting();

	void StopSprinting();

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	void PitchView(float Val);

	void TurnView(float Val);

	//Aim
	void StartAiming();

	void StopAiming();

	//Shoot
	void Shoot();

	void Shot();

	FTimerHandle ShootTimer;

	void OpenShootGate();
	bool bIsShootGateOpen;

	UPROPERTY(BlueprintReadOnly)
		float CurrentAmmo;

	//Reload
	void Reload();
	
	//Takedown
	void Takedown();

	void Takedown_FreeEnemy();

	FTimerHandle FreeTimer;

	AActor* TargetEnemy;

	void PerformSyncedAnim(AActor* TargetActor, UAnimMontage* Montage_Player, UAnimMontage* Montage_Enemy, bool bSetEnemyRotation = false, FRotator EnemyRotation = FRotator::ZeroRotator);

	//Vault
	void Vault();

	void OnVaultBlendOut(UAnimMontage* animMontage, bool bInterrupted);

	//Human Shield
	void HumanShield_Grab();

	void HumanShield_Free();

	void HumanShield_Push();

	//Cover
	void Cover();

	void MoveOnCover(float MovementValue);


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	//Character interface
	virtual void SetCanAimDuringTakedown(ETakedownState Player_TakedownState) override;

	virtual void SetCanAimDuringVault(EVaultState Player_VaultState) override;

	virtual void SetTakedownState(ETakedownState Player_TakedownState) override;

	virtual void SetCombatState(ECombatState Player_CombatState) override;

	virtual void SetCharacterRotationMode(bool bIsStrafing) override;

	virtual void SetCanShoot(bool bCanShoot) override;

	virtual void SetCanMove_Look(bool bCanMove = true, bool bCanLook = true) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	//Sight perception target
	virtual bool CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor = nullptr, const bool* bWasVisible = nullptr, int32* UserData = nullptr) const;

};

