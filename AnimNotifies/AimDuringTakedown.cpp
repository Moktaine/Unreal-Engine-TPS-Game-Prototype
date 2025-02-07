// Fill out your copyright notice in the Description page of Project Settings.


#include "AimDuringTakedown.h"

void UAimDuringTakedown::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AActor* owner = MeshComp->GetOwner();
	if (owner) {
		if (owner->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
			Cast<IInterface_Character>(owner)->SetCanAimDuringTakedown(ETakedownState::CanAimDuringTakedown);
		}
	}
}

void UAimDuringTakedown::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* owner = MeshComp->GetOwner();

	if (owner) {
		if (owner->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
			Cast<IInterface_Character>(owner)->SetCanAimDuringTakedown(ETakedownState::PerformingTakedown);
		}
	}
}
