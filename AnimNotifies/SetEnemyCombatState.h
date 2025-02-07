// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Enums_PlayerCharacter.h"
#include "Interface_Character.h"
#include "SetEnemyCombatState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTKEATS_API USetEnemyCombatState : public UAnimNotify, public IInterface_Character
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		ECombatState_Enemy Enemy_CombatState {
		ECombatState_Enemy::Default
	};

private:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
