// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "Enums_PlayerCharacter.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ECombatState :uint8 {
	Default			UMETA(DisplayName = "Default"),
	Reloading		UMETA(DisplayName = "Reloading"),
	Aim				UMETA(DisplayName = "Aim"),
	HipFire			UMETA(DisplayName = "HipFire"),
	Cover			UMETA(DisplayName = "Cover"),
	Takedown		UMETA(DisplayName = "Takedown"),
	HumanShield		UMETA(DisplayName = "HumanShield"),
	Vault			UMETA(DisplayName = "Vault")
};


UENUM(BlueprintType)
enum class ECombatState_Enemy :uint8 {
	Default			UMETA(DisplayName = "Default"),
	Dead			UMETA(DisplayName = "Dead"),
	Stunned			UMETA(DisplayName = "Stunned"),
	HardStunned		UMETA(DisplayName = "Hard Stunned"),
	HumanShield		UMETA(DisplayName = "HumanShield")
};



UENUM(BlueprintType)
enum class ETakedownState :uint8 {
	PerformingTakedown			UMETA(DisplayName = "Performing Takedown"),
	CanAimDuringTakedown		UMETA(DisplayName = "Can Aim During Takedown"),
	AimingDuringTakedown		UMETA(DisplayName = "Aiming During Takedown"),
	ControllingEnemy			UMETA(DisplayName = "Controlling Enemy"),
	ControllingEnemyAiming		UMETA(DisplayName = "Controlling Enemy Aiming"),
	ControllingEnemyReloading	UMETA(DisplayName = "Controlling Enemy Reloading")
};

UENUM(BlueprintType)
enum class ETakedownType :uint8 {
	Soft		UMETA(DisplayName = "Soft Takedown"),
	Hard		UMETA(DisplayName = "Hard Takedown"),
	Wall		UMETA(DisplayName = "Wall Takedown")
};


UENUM(BlueprintType)
enum class EVaultState :uint8 {
	Vaulting				UMETA(DisplayName = "Vaulting"),
	CanAimDuringVault		UMETA(DisplayName = "CanAimDuringVault"),
	AimingDuringVault		UMETA(DisplayName = "AimingDuringVault")
};

UENUM(BlueprintType)
enum class EHumanShieldState :uint8 {
	HumanShieldBase			UMETA(DisplayName = "HumanShield Base"),
	HumanShieldAim			UMETA(DisplayName = "HumanShield Aim")
};

UENUM(BlueprintType)
enum class ECoverTypes :uint8 {
	HighCover			UMETA(DisplayName = "High Cover"),
	LowCover			UMETA(DisplayName = "Low Cover")
};

UCLASS()
class PROJECTKEATS_API UEnums_PlayerCharacter : public UUserDefinedEnum
{
	GENERATED_BODY()
	
};
