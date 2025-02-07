// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Interface_Character.h"
#include "Enums_PlayerCharacter.h"
#include "SetTakedownState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTKEATS_API USetTakedownState : public UAnimNotify, public IInterface_Character
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		ETakedownState TakedownState;

private:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
