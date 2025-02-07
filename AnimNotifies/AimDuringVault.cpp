// Fill out your copyright notice in the Description page of Project Settings.


#include "AimDuringVault.h"

void UAimDuringVault::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AActor* owner = MeshComp->GetOwner();

	if (owner) {
		if (owner->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
			Cast<IInterface_Character>(owner)->SetCanAimDuringVault(EVaultState::CanAimDuringVault);
		}
	}
}

void UAimDuringVault::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* owner = MeshComp->GetOwner();

	if (owner) {
		if (owner->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
			Cast<IInterface_Character>(owner)->SetCanAimDuringVault(EVaultState::Vaulting);
		}
	}
}