// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Interface_Character.h"
#include "SetCanMoveLook.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTKEATS_API USetCanMoveLook : public UAnimNotify, public IInterface_Character
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		bool CanMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		bool CanLook = true;
private:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
