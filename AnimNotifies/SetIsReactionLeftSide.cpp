// Fill out your copyright notice in the Description page of Project Settings.


#include "SetIsReactionLeftSide.h"

void USetIsReactionLeftSide::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetAnimInstance()->GetClass()->ImplementsInterface(UInterface_EnemyABP::StaticClass())) {
		IInterface_EnemyABP::Execute_SetIsReactionLeftSide(MeshComp->GetAnimInstance(), IsReactionLeftSide);
	}

}
