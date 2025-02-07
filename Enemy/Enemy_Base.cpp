// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Base.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"


#define COLLISION_ENEMY ECC_GameTraceChannel1
// Sets default values
AEnemy_Base::AEnemy_Base()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Pistol = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pistol"));
	Pistol->SetupAttachment(GetMesh(), TEXT("R_Hand_Pistol_Socket"));

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
}

// Called when the game starts or when spawned
void AEnemy_Base::BeginPlay()
{
	Super::BeginPlay();
	Health = DefaultHealth;
}

void AEnemy_Base::Died(FVector ImpactPoint, FVector ForceDirection, FName BoneName)
{
	CombatState_Enemy = ECombatState_Enemy::Dead;
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->AddImpulseAtLocation(ForceDirection * ForceMultiplier, ImpactPoint, BoneName);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	
}




// Called every frame
void AEnemy_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy_Base::GotShot(FVector ImpactPoint, FVector ForceDirection, FName BoneName)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("Shot Bone: %s"), *BoneName.ToString()));
	//DrawDebugPoint(GetWorld(), ImpactPoint, 3, FColor::Red, false, 5, 0);
	//DrawDebugLine(GetWorld(), ImpactPoint, ImpactPoint + (-1 * ForceDirection * 100), FColor::Yellow, false, 5, 0, .5);

	FRotator HitRotation = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation());
	this->SetActorRotation(FRotator(0, HitRotation.Yaw, 0));


	if (SB_BulletHit) {
		UGameplayStatics::PlaySound2D(GetWorld(), SB_BulletHit);
	}

	if (Health <= 0) {
		Died(ImpactPoint, ForceDirection, BoneName);
	}

	UNiagaraSystem* SystemToSpawn = NS_Blood_Body;

	//-------------Hit Reaction Animations--------------

	//Head
	if (BoneName == FName(TEXT("head")) || BoneName == FName(TEXT("neck_01")) || BoneName == FName(TEXT("neck_02"))) {
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, TEXT("Head"));
		
		SystemToSpawn = NS_Blood_Head;

		Health -= DefaultHealth;
	}
	//Right Shoulder
	if (BoneName == FName(TEXT("clavicle_r")) || BoneName == FName(TEXT("upperarm_r")) || BoneName == FName(TEXT("lowerarm_r")) || BoneName == FName(TEXT("hand_r")) || BoneName == FName(TEXT("spine_05"))) {
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, TEXT("Right Shoulder"));
		GetMesh()->GetAnimInstance()->Montage_Play(Montage_Character_HitReaction_Shoulder);

		if (GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UInterface_EnemyABP::StaticClass())) {
			IInterface_EnemyABP::Execute_SetIsReactionLeftSide(GetMesh()->GetAnimInstance(), false);
		}
		
		Health -= FMath::FRandRange(20.0, 30.0);
	}
	//Left Shoulder
	if (BoneName == FName(TEXT("clavicle_l")) || BoneName == FName(TEXT("upperarm_l")) || BoneName == FName(TEXT("lowerarm_l")) || BoneName == FName(TEXT("hand_l"))) {
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, TEXT("Left Shoulder"));
		GetMesh()->GetAnimInstance()->Montage_Play(Montage_Character_HitReaction_Shoulder);

		if (GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UInterface_EnemyABP::StaticClass())) {
			IInterface_EnemyABP::Execute_SetIsReactionLeftSide(GetMesh()->GetAnimInstance(), true);
		}

		Health -= FMath::FRandRange(20.0, 30.0);
	}
	//Right Leg
	if (BoneName == FName(TEXT("thigh_r")) || BoneName == FName(TEXT("calf_r")) || BoneName == FName(TEXT("foot_r"))) {
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, TEXT("Right Leg"));
		GetMesh()->GetAnimInstance()->Montage_Play(Montage_Character_HitReaction_Leg);

		if (GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UInterface_EnemyABP::StaticClass())) {
			IInterface_EnemyABP::Execute_SetIsReactionLeftSide(GetMesh()->GetAnimInstance(), false);
		}

		Health -= FMath::FRandRange(20.0, 30.0);
	}
	//Left Leg
	if (BoneName == FName(TEXT("thigh_l")) || BoneName == FName(TEXT("calf_l")) || BoneName == FName(TEXT("foot_l"))) {
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, TEXT("Left Leg"));
		GetMesh()->GetAnimInstance()->Montage_Play(Montage_Character_HitReaction_Leg);
		
		if (GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UInterface_EnemyABP::StaticClass())) {
			IInterface_EnemyABP::Execute_SetIsReactionLeftSide(GetMesh()->GetAnimInstance(), true);
		}

		Health -= FMath::FRandRange(20.0, 30.0);
	}
	//Stomach
	if (BoneName == FName(TEXT("pelvis")) || BoneName == FName(TEXT("spine_02")) || BoneName == FName(TEXT("spine_04"))) {
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, TEXT("Stomach"));
		GetMesh()->GetAnimInstance()->Montage_Play(Montage_Character_HitReaction_Stomach);

		if (GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UInterface_EnemyABP::StaticClass())) {
			IInterface_EnemyABP::Execute_SetIsReactionLeftSide(GetMesh()->GetAnimInstance(), false);
		}

		Health -= FMath::FRandRange(20.0, 30.0);
	}

	//Spawn Blood
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SystemToSpawn, ImpactPoint, UKismetMathLibrary::MakeRotFromX(ForceDirection));

	if (Health <= 0) {
		Died(ImpactPoint, ForceDirection, BoneName);
	}
	else if(Health <= 30){
		CombatState_Enemy = ECombatState_Enemy::HardStunned;
		if (SB_HardStunned) {UGameplayStatics::PlaySound2D(GetWorld(), SB_HardStunned);}
	}
	else {
		CombatState_Enemy = ECombatState_Enemy::Stunned;
	}


}

void AEnemy_Base::DealDamage(float DamageAmount)
{
	Health -= DamageAmount;
}


ECombatState_Enemy AEnemy_Base::GetCombatState_Enemy()
{
	return CombatState_Enemy;
}

void AEnemy_Base::Enemy_MotionWarp(FName WarpTarget, FVector Location, FRotator Rotation)
{
	MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(WarpTarget, Location, Rotation);
}

void AEnemy_Base::SetCombatState_Enemy(ECombatState_Enemy EnemyState)
{
	CombatState_Enemy = EnemyState;
}

void AEnemy_Base::Enemy_PlayMontage(UAnimMontage* Montage_Enemy)
{
	GetMesh()->GetAnimInstance()->Montage_Play(Montage_Enemy);
	GetCapsuleComponent()->SetCollisionProfileName(FName(TEXT("SyncedAnim")));
}

void AEnemy_Base::SetCollisionProfile(FName CapsuleCollisionProfile, FName MeshCollisionProfile, bool bSetCapsuleCollision, bool bSetMeshCollision)
{
	if (bSetCapsuleCollision) { GetCapsuleComponent()->SetCollisionProfileName(CapsuleCollisionProfile); }
	if (bSetMeshCollision) { GetMesh()->SetCollisionProfileName(MeshCollisionProfile); }
}

void AEnemy_Base::PushEnemy()
{
	GetMesh()->GetAnimInstance()->Montage_Play(Montage_Character_HumanShield_Push);
	FOnMontageEnded MontageEndDelegate;
	MontageEndDelegate.BindUObject(this, &AEnemy_Base::EndPush);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(MontageEndDelegate, Montage_Character_HumanShield_Push);
	PushTrace();
}


void AEnemy_Base::PushTrace()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_ENEMY));
	IgnoredActorsPush.Add(this);
	TArray<FHitResult> Hits;
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), GetActorLocation(), GetActorLocation(), 
		100, ObjectTypes, false, IgnoredActorsPush, EDrawDebugTrace::Type::ForDuration, Hits, true);

	for (FHitResult hit : Hits) {
		AActor* hitActor = hit.GetActor();
		IgnoredActorsPush.Add(hitActor);

		FRotator rotation = UKismetMathLibrary::FindLookAtRotation(hitActor->GetActorLocation(), GetActorLocation());
		hitActor->SetActorRotation(rotation);

		if (hitActor->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
			Cast<IInterface_Character>(hitActor)->Enemy_PlayMontage(Montage_Character_HumanShield_Push_Secondary);
		}
	}

	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(handle, this, &AEnemy_Base::PushTrace, 0.1, false);
}

void AEnemy_Base::EndPush(UAnimMontage* animMontage, bool bInterrupted)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, TEXT("Push Timer Cleared!"));
	IgnoredActorsPush.Empty();
}


// Called to bind functionality to input
void AEnemy_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}




