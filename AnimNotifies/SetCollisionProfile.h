// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Interface_Character.h"
#include "SetCollisionProfile.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTKEATS_API USetCollisionProfile : public UAnimNotify, public IInterface_Character
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		FName CapsuleCollisionProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		FName MeshCollisionProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		bool bSetCapsuleCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		bool bSetMeshCollision;
private: 

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
