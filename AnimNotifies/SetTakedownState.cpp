// Fill out your copyright notice in the Description page of Project Settings.

#include "SetTakedownState.h"

void USetTakedownState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* owner = MeshComp->GetOwner();
	if (!owner) { return; }

	if (owner->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
		Cast<IInterface_Character>(owner)->SetTakedownState(TakedownState);
	}
}
