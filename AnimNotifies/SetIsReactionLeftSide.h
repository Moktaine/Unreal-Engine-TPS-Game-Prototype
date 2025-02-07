// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Interface_EnemyABP.h"
#include "SetIsReactionLeftSide.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTKEATS_API USetIsReactionLeftSide : public UAnimNotify, public IInterface_EnemyABP
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		bool IsReactionLeftSide;
private:

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
