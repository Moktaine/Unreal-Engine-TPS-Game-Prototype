// Fill out your copyright notice in the Description page of Project Settings.


#include "SetEnemyCombatState.h"

void USetEnemyCombatState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* EnemyActor = MeshComp->GetOwner();
	if (!EnemyActor) { return; }
	if (EnemyActor->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
		Cast<IInterface_Character>(EnemyActor)->SetCombatState_Enemy(Enemy_CombatState);
	}
}
