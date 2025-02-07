// Fill out your copyright notice in the Description page of Project Settings.


#include "SetCollisionProfile.h"


void USetCollisionProfile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* owner = MeshComp->GetOwner();

	if (owner) {
		if (owner->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
			Cast<IInterface_Character>(owner)->SetCollisionProfile(CapsuleCollisionProfileName, MeshCollisionProfileName, bSetCapsuleCollision, bSetMeshCollision);
		}
	}
}
