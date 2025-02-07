// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectKeatsCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "MotionWarpingComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/DecalComponent.h"



#define COLLISION_BULLET ECC_GameTraceChannel2
#define COLLISION_VAULT ECC_GameTraceChannel3
#define COLLISION_ENEMY ECC_GameTraceChannel4
#define COLLISION_COVER ECC_GameTraceChannel5
//////////////////////////////////////////////////////////////////////////
// AProjectKeatsCharacter

AProjectKeatsCharacter::AProjectKeatsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	Pistol = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pistol"));
	Pistol->SetupAttachment(GetMesh(),TEXT("R_Hand_Pistol_Socket"));

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));

	bIsShootGateOpen = true;

	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AProjectKeatsCharacter::BeginPlay(){
	Super::BeginPlay();
	CurrentAmmo = DefaultAmmo;	
	AnimInstance_Character = GetMesh()->GetAnimInstance();
	AnimInstance_Pistol = Pistol->GetAnimInstance();
	if(GetWorld()){
		UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->ViewPitchMax = 60;
		UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->ViewPitchMin = -60;
	}
}


//////////////////////////////////////////////////////////////////////////
// Input


void AProjectKeatsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AProjectKeatsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AProjectKeatsCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &AProjectKeatsCharacter::TurnView);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AProjectKeatsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &AProjectKeatsCharacter::PitchView);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AProjectKeatsCharacter::LookUpAtRate);


	//Run
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AProjectKeatsCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AProjectKeatsCharacter::StopSprinting);

	//Aim
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this,&AProjectKeatsCharacter::StartAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this,&AProjectKeatsCharacter::StopAiming);

	//Shoot
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AProjectKeatsCharacter::Shoot);

	//Reload
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AProjectKeatsCharacter::Reload);

	//Takedown
	PlayerInputComponent->BindAction("Takedown", IE_Pressed, this, &AProjectKeatsCharacter::Takedown);

	//Vault
	PlayerInputComponent->BindAction("Vault", IE_Pressed, this, &AProjectKeatsCharacter::Vault);

	//Human Shield
	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &AProjectKeatsCharacter::HumanShield_Grab);
	PlayerInputComponent->BindAction("Human Shield Push", IE_Pressed, this, &AProjectKeatsCharacter::HumanShield_Push);

	//Cover
	PlayerInputComponent->BindAction("Cover", IE_Pressed, this, &AProjectKeatsCharacter::Cover);
}
//---------------------------INTERFACE_CHARACTER
void AProjectKeatsCharacter::SetCanAimDuringTakedown(ETakedownState Player_TakedownState)
{
	TakedownState = Player_TakedownState;
	if (TakedownState == ETakedownState::PerformingTakedown) {
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0);
	}
}

void AProjectKeatsCharacter::SetCanAimDuringVault(EVaultState Player_VaultState)
{
	VaultState= Player_VaultState;
	if (VaultState == EVaultState::Vaulting) {
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0);
	}
}

void AProjectKeatsCharacter::SetTakedownState(ETakedownState Player_TakedownState)
{
	TakedownState = Player_TakedownState;
}

void AProjectKeatsCharacter::SetCombatState(ECombatState Player_CombatState)
{
	CombatState = Player_CombatState;
}

void AProjectKeatsCharacter::SetCharacterRotationMode(bool bIsStrafing)
{
	bUseControllerRotationYaw = bIsStrafing;
	GetCharacterMovement()->bOrientRotationToMovement = !bIsStrafing;
}

void AProjectKeatsCharacter::SetCanShoot(bool bCanShoot)
{
	CanShoot = bCanShoot;
}


void AProjectKeatsCharacter::SetCanMove_Look(bool bCanMove, bool bCanLook)
{
	CanMove = bCanMove;
	CanLook = bCanLook;
}


//Sight perception target
bool AProjectKeatsCharacter::CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor, const bool* bWasVisible, int32* UserData) const
{
	static const FName Name_AILineOfSight = FName(TEXT("TestPawnLineOfSight"));
	FHitResult hitResult;
	FVector SightTargetLocation = GetMesh()->GetSocketLocation("spine_05");
	

	bool hit = GetWorld()->LineTraceSingleByChannel(hitResult, ObserverLocation, SightTargetLocation, ECollisionChannel::ECC_Visibility, FCollisionQueryParams(Name_AILineOfSight, false, IgnoreActor));

	if (hit && hitResult.GetActor()->IsOwnedBy(this)) {
		OutSeenLocation = SightTargetLocation;
		OutSightStrength = 1;
		return true;
	}
	
	OutSightStrength = 0;
	return false;
	
}


//Input Functions

void AProjectKeatsCharacter::TurnAtRate(float Rate)
{
	if (!CanLook) { return; }
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AProjectKeatsCharacter::LookUpAtRate(float Rate)
{
	if (!CanLook) { return; }
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}


//-----------------------MOVE------------------------
void AProjectKeatsCharacter::MoveForward(float Value)
{
	if (!CanMove) { return; }
	//Character can't move while on certain takedown states
	if (CombatState == ECombatState::Takedown &&
		!(TakedownState == ETakedownState::CanAimDuringTakedown || TakedownState == ETakedownState::AimingDuringTakedown || TakedownState == ETakedownState::PerformingTakedown) ||
		CombatState == ECombatState::Cover) 
	{
		return;
	}

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AProjectKeatsCharacter::MoveRight(float Value)
{
	if (!CanMove) { return; }
	//Character can't move while on certain takedown states
	if (CombatState == ECombatState::Takedown &&
		!(TakedownState == ETakedownState::CanAimDuringTakedown || TakedownState == ETakedownState::AimingDuringTakedown || TakedownState == ETakedownState::PerformingTakedown))
	{
		return;
	}


	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction

		if (CombatState == ECombatState::Cover) {
			MoveOnCover(Value);
		}
		else {
			AddMovementInput(Direction, Value);
		}
	}
}
//---------------------SPRINT--------------------------
void AProjectKeatsCharacter::StartSprinting()
{
	if (CombatState == ECombatState::Default || CombatState == ECombatState::Aim) {
		StopAiming();
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed_Sprinting;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void AProjectKeatsCharacter::StopSprinting()
{
	if (CombatState == ECombatState::Default || CombatState == ECombatState::HipFire) {
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed_Base;
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}


//--------------------CAMERA LOOK-----------------------
void AProjectKeatsCharacter::PitchView(float Val) 
{
	AddControllerPitchInput(Val * MouseSensitivity);
}

void AProjectKeatsCharacter::TurnView(float Val)
{
	AddControllerYawInput(Val * MouseSensitivity);
}


//-----------------AIMING------------------
void AProjectKeatsCharacter::StartAiming()
{
	switch (CombatState)
	{
	case ECombatState::Default:
	case ECombatState::Aim:
	case ECombatState::HipFire:
		CombatState = ECombatState::Aim;
		FollowCamera->SetFieldOfView(50);
		CameraBoom->CameraLagSpeed = 15;
		bUseControllerRotationYaw = true;
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientStartCameraShake(CS_Bob);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed_Aiming;
		break;

	case ECombatState::Takedown:
		if (TakedownState == ETakedownState::CanAimDuringTakedown) {
			TakedownState = ETakedownState::AimingDuringTakedown;
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1);
		}
		if (TakedownState == ETakedownState::ControllingEnemy) {
			TakedownState = ETakedownState::ControllingEnemyAiming;
			UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientStartCameraShake(CS_Bob);
			FollowCamera->SetFieldOfView(50);
		}
		break;

	case ECombatState::HumanShield:
		HumanShieldState = EHumanShieldState::HumanShieldAim;
		FollowCamera->SetFieldOfView(50);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientStartCameraShake(CS_Bob);
		break;

	case ECombatState::Vault:
		if (VaultState == EVaultState::CanAimDuringVault) {
			VaultState = EVaultState::AimingDuringVault;
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1);
		}
		break;
	default:
		break;
	}
}

void AProjectKeatsCharacter::StopAiming()
{
	if(CombatState == ECombatState::Aim){ CombatState = ECombatState::Default;}
	if (CombatState == ECombatState::Takedown && TakedownState == ETakedownState::ControllingEnemyAiming) { TakedownState = ETakedownState::ControllingEnemy; }
	if (CombatState == ECombatState::HumanShield && HumanShieldState == EHumanShieldState::HumanShieldAim) { HumanShieldState = EHumanShieldState::HumanShieldBase; }

	FollowCamera->SetFieldOfView(90);
	CameraBoom->CameraLagSpeed = 7;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed_Base;
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientStopCameraShake(CS_Bob);
}


//----------------------SHOOTING------------------------
void AProjectKeatsCharacter::OpenShootGate()
{
	bIsShootGateOpen = true;
}

void AProjectKeatsCharacter::Shot()
{
	FVector MuzzleLocation = Pistol->GetSocketLocation(TEXT("Muzzle"));
	//Spawn Muzzle Flash
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NS_MuzzleFlash, MuzzleLocation);
	UNiagaraFunctionLibrary::SpawnSystemAttached(NS_MuzzleSmoke, Pistol, TEXT("Muzzle"), FVector(0,0,0), FRotator(0,0,0), EAttachLocation::SnapToTarget, true);
			
	//Play Pistol Shot Sound
	if(SB_PistolShot){
		UGameplayStatics::PlaySound2D(GetWorld(), SB_PistolShot);
	}else{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Pistol Shot Sounds Array is NULL!"));
	}

	//Play Pistol Shoot Montage
	if(AnimInstance_Pistol){
		AnimInstance_Pistol->Montage_Play(Montage_Pistol_Shoot);
	}

	CurrentAmmo--;
	
}


void AProjectKeatsCharacter::Shoot()
{
	if (!CanShoot) {
		return;
	}

	if(CombatState == ECombatState::Aim || CombatState == ECombatState::Default ||
		(CombatState == ECombatState::Takedown && (TakedownState == ETakedownState::AimingDuringTakedown) || TakedownState == ETakedownState::ControllingEnemyAiming) || 
		(CombatState == ECombatState::Vault && VaultState == EVaultState::AimingDuringVault) ||
		(CombatState == ECombatState::HumanShield && HumanShieldState == EHumanShieldState::HumanShieldAim))
	{

		if (!bIsShootGateOpen) { return; }

		if (CurrentAmmo <= 0) {
			//If no ammo left play empty gun sound and animation
			UGameplayStatics::PlaySound2D(GetWorld(), SB_PistolEmpty, .4);
			AnimInstance_Pistol->Montage_Play(Montage_Pistol_Empty);
			return;
		}

		Shot();

		FVector ShootTraceEnd = FollowCamera->GetComponentLocation() + (UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetActorForwardVector() * 10000);

		if (CombatState == ECombatState::Default) {
			//Spread bullet if HipFiring
			if (GetCharacterMovement()->Velocity.Size() > 450) {
				CombatState = ECombatState::HipFire;
				FRotator VectorRotator = FRotator(FMath::RandRange(-3, 3), FMath::RandRange(-3, 3), 0.0);
				ShootTraceEnd = UKismetMathLibrary::Quat_RotateVector(VectorRotator.Quaternion(), ShootTraceEnd);
			}
			else {
				if (AnimInstance_Character) {
					AnimInstance_Character->Montage_Play(Montage_Character_Shoot);
				}
			}
		}

		if (CombatState == ECombatState::Aim) {
			if (AnimInstance_Character) {
				AnimInstance_Character->Montage_Play(Montage_Character_Shoot);
			}
		}


		FCollisionQueryParams ShootTraceParams;
		ShootTraceParams.AddIgnoredActor(this);
		//Pistol Shot Line Trace
		FHitResult hit_Initial;
		bool IsHit = GetWorld()->LineTraceSingleByChannel(hit_Initial, FollowCamera->GetComponentLocation(), ShootTraceEnd, ECollisionChannel::COLLISION_BULLET, ShootTraceParams);

		if (!IsHit) { return; }

		//Trace from muzzle to check if any enemy is infront of the muzzle
		FHitResult hit_Secondary;
		FVector hit_Secondary_Start = Pistol->GetSocketLocation(TEXT("Muzzle"));
		FVector hit_Secondary_End = hit_Secondary_Start + UKismetMathLibrary::GetDirectionUnitVector(Pistol->GetSocketLocation(TEXT("Muzzle")), hit_Initial.Location) * 200;
		bool IsHit_Secondary = GetWorld()->LineTraceSingleByChannel(hit_Secondary, hit_Secondary_Start, hit_Secondary_End, ECollisionChannel::COLLISION_BULLET);

		//If hit actor is enemy spawn BLOOD else spawn DUST particles
		if (hit_Initial.Component->ComponentHasTag(TEXT("Enemy")) || (IsHit_Secondary && hit_Secondary.Component->ComponentHasTag(TEXT("Enemy")))) {

			FHitResult hit_Final;
			if (IsHit_Secondary && hit_Secondary.Component->ComponentHasTag(TEXT("Enemy"))) {
				hit_Final = hit_Secondary;
			}
			else {
				hit_Final = hit_Initial;
			}

			if (hit_Final.GetActor()->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
				Cast<IInterface_Character>(hit_Final.GetActor())->GotShot(hit_Final.ImpactPoint, UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), hit_Final.GetActor()->GetActorLocation()), hit_Final.BoneName);
			}
		}
		else {
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NS_BulletImpact, hit_Initial.Location, UKismetMathLibrary::MakeRotFromX(hit_Initial.Normal));

			//Spawn bullet hole decal
			UDecalComponent* BulletHole = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), Decal_BulletHole, FVector(5, 10, 10), hit_Initial.Location, UKismetMathLibrary::MakeRotFromX(hit_Initial.Normal));
			BulletHole->SetFadeScreenSize(0.0f);
		}
		//Spawn Bullet Trail Particle
		UNiagaraComponent* BulletTrail = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NS_BulletTrail, Pistol->GetSocketLocation(TEXT("Muzzle")));
		if (BulletTrail) { BulletTrail->SetNiagaraVariableVec3(TEXT("Trail End"), hit_Initial.Location); }


		GetWorld()->SpawnActor<AActor>(BP_Shell, Pistol->GetSocketTransform(TEXT("Chamber")));

		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientStartCameraShake(CS_Shoot);

		bIsShootGateOpen = false;
		//Wait for determined amount beforce can shoot again
		GetWorld()->GetTimerManager().SetTimer(ShootTimer, this, &AProjectKeatsCharacter::OpenShootGate, FireRate * UGameplayStatics::GetGlobalTimeDilation(GetWorld()), false);


	
	}
	else if (CombatState == ECombatState::Takedown && TakedownState == ETakedownState::ControllingEnemy) {
		if (CurrentAmmo <= 0) {
			//If no ammo left play empty gun sound and animation
			UGameplayStatics::PlaySound2D(GetWorld(), SB_PistolEmpty, .4);
			AnimInstance_Pistol->Montage_Play(Montage_Pistol_Empty);
			return; 
		}
		
		Shot();

		ECombatState_Enemy EnemyCombatState = Cast<IInterface_Character>(TargetEnemy)->GetCombatState_Enemy();

		if (TakedownType == ETakedownType::Soft) {
			PerformSyncedAnim(TargetEnemy, Montage_Character_Kill, Montage_Enemy_Takedown_Killed);
		}
		if (TakedownType == ETakedownType::Hard) {
			PerformSyncedAnim(TargetEnemy, Montage_Character_Kill_Hard, Montage_Enemy_Takedown_Killed_Hard);
		}
		if (TakedownType == ETakedownType::Wall) {
			PerformSyncedAnim(TargetEnemy, Montage_Character_Kill_Wall, Montage_Enemy_Takedown_Killed_Wall);
		}
		
		GetWorld()->GetTimerManager().ClearTimer(FreeTimer);
		
		if (TargetEnemy->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())){
			Cast<IInterface_Character>(TargetEnemy)->DealDamage(1000);
		}
	}
	else if (CombatState == ECombatState::HumanShield && HumanShieldState == EHumanShieldState::HumanShieldBase) {
		if (CurrentAmmo <= 0) {
			//If no ammo left play empty gun sound and animation
			UGameplayStatics::PlaySound2D(GetWorld(), SB_PistolEmpty, .4);
			AnimInstance_Pistol->Montage_Play(Montage_Pistol_Empty);
			return;
		}

		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed_Base;
		PerformSyncedAnim(TargetEnemy, Montage_Character_HumanShield_Shoot, Montage_Enemy_HumanShield_Killed);
		Shot();
		FDetachmentTransformRules DetachmentRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, false);
		TargetEnemy->DetachFromActor(DetachmentRules);
		GetWorld()->GetTimerManager().ClearTimer(FreeTimer);

		if (TargetEnemy->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
			Cast<IInterface_Character>(TargetEnemy)->SetCombatState_Enemy(ECombatState_Enemy::Default);
			Cast<IInterface_Character>(TargetEnemy)->DealDamage(100000);
		}
	}
	
}


//---------------------RELOAD---------------------------
void AProjectKeatsCharacter::Reload(){
	if (CurrentAmmo < DefaultAmmo) {
		switch (CombatState)
		{
		case ECombatState::Default:
		case ECombatState::Aim:
		case ECombatState::HipFire:
			CanShoot = false;
			StopAiming();
			CurrentAmmo = DefaultAmmo;
			CombatState = ECombatState::Reloading;
			break;
		case ECombatState::Cover:
			break;
		case ECombatState::Takedown:
			if (TakedownState == ETakedownState::ControllingEnemy || TakedownState == ETakedownState::ControllingEnemyAiming) {
				CanShoot = false;
				StopAiming();
				CurrentAmmo = DefaultAmmo;
				TakedownState = ETakedownState::ControllingEnemyReloading;
			}
			break;
		}
	}
	
}


//---------------------TAKEDOWN--------------------------
void AProjectKeatsCharacter::Takedown()
{
	if (CombatState == ECombatState::Default || CombatState == ECombatState::Aim || CombatState == ECombatState::HipFire || CombatState == ECombatState::Reloading) {

		FRotator BoxTraceOrientation = FRotator(0, GetControlRotation().Yaw, GetControlRotation().Roll);
		FVector BoxTraceStart = GetActorLocation() + (UKismetMathLibrary::GetForwardVector(BoxTraceOrientation) * 30);
		FVector BoxTraceEnd = BoxTraceStart + (UKismetMathLibrary::GetForwardVector(BoxTraceOrientation) * 130);

		TArray<FHitResult> hits;

		UKismetSystemLibrary::BoxTraceMulti(GetWorld(), BoxTraceStart, BoxTraceEnd, FVector(10, 50, 100), BoxTraceOrientation, UEngineTypes::ConvertToTraceType(COLLISION_ENEMY), 
			false, {}, EDrawDebugTrace::ForDuration, hits, true);

		for (FHitResult hit : hits) {

			if (!hit.GetActor()->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) { continue; }

			ECombatState_Enemy EnemyCombatState = Cast<IInterface_Character>(hit.GetActor())->GetCombatState_Enemy();

			if (EnemyCombatState != ECombatState_Enemy::Stunned && EnemyCombatState != ECombatState_Enemy::HardStunned) { continue; }

			//Check if there is a wall to perform wall takedown
			FHitResult hit_Wall;
			FVector WallTraceStart = hit.GetActor()->GetActorLocation();
			FVector WallTraceEnd = WallTraceStart + (UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), WallTraceStart) * 200);
			bool IsWallHit = GetWorld()->LineTraceSingleByChannel(hit_Wall, WallTraceStart, WallTraceEnd, ECollisionChannel::ECC_Camera);

			DrawDebugLine(GetWorld(), WallTraceStart, WallTraceEnd, FColor::Cyan, false, 3, 0, .5);
			DrawDebugLine(GetWorld(), hit_Wall.ImpactPoint, hit_Wall.ImpactPoint + (hit_Wall.ImpactNormal * 100), FColor::Red, false, 3, 0, .5);
			DrawDebugSphere(GetWorld(), hit_Wall.ImpactPoint, 20, 10, FColor::Emerald, false, 3, 0, .3);

	
			FVector Location_MoveTo;
			FRotator Rotation_MoveTo;

			FVector WallDirection = UKismetMathLibrary::GetDirectionUnitVector(hit_Wall.TraceEnd, hit_Wall.TraceStart);
			float dot = UKismetMathLibrary::Dot_VectorVector(WallDirection, hit_Wall.ImpactNormal);

			if (IsWallHit) {
				if (dot > 0.7) {
					//Set MoveTo Location and Rotation
					Location_MoveTo = hit_Wall.ImpactPoint + (hit_Wall.ImpactNormal * 215);
					Rotation_MoveTo = UKismetMathLibrary::FindLookAtRotation(Location_MoveTo, hit_Wall.ImpactPoint);

					//Set Enemy MoveTo Location and Rotation
					FVector Location_MoveTo_Enemy = hit_Wall.ImpactPoint + (hit_Wall.ImpactNormal * 100);
					FRotator Rotation_MoveTo_Enemy = UKismetMathLibrary::FindLookAtRotation(hit_Wall.ImpactPoint, Location_MoveTo);

					DrawDebugSphere(GetWorld(), Location_MoveTo_Enemy, 20, 10, FColor::Green, false, 3, 0, 1);

					TakedownType = ETakedownType::Wall;
					PerformSyncedAnim(hit.GetActor(), Montage_Character_Takedown_Wall, Montage_Enemy_Takedown_Wall);
					Cast<IInterface_Character>(hit.GetActor())->Enemy_MotionWarp(FName(TEXT("Synced Anim Warp")), Location_MoveTo_Enemy, Rotation_MoveTo_Enemy);
				}
			}

			if (!IsWallHit || dot < 0.7) {
				//Set MoveTo Location and Rotation
				FRotator EnemyRotation = UKismetMathLibrary::FindLookAtRotation(hit.GetActor()->GetActorLocation(), GetActorLocation());
				
				if (EnemyCombatState == ECombatState_Enemy::Stunned) {
					TakedownType = ETakedownType::Soft;
					TakedownDistance = 80;
					PerformSyncedAnim(hit.GetActor(), Montage_Character_Takedown, Montage_Enemy_Takedown, true, EnemyRotation);
				}
				if (EnemyCombatState == ECombatState_Enemy::HardStunned) {
					TakedownType = ETakedownType::Hard;
					TakedownDistance = 40;
					PerformSyncedAnim(hit.GetActor(), Montage_Character_Takedown_Hard, Montage_Enemy_Takedown_Hard, true, EnemyRotation);
				}

				Location_MoveTo = hit.GetActor()->GetActorLocation() + (UKismetMathLibrary::GetForwardVector(EnemyRotation)) * TakedownDistance;
				Rotation_MoveTo = UKismetMathLibrary::FindLookAtRotation(Location_MoveTo, hit.GetActor()->GetActorLocation());
			}
			

			DrawDebugSphere(GetWorld(), Location_MoveTo, 20, 10, FColor::Yellow, false, 3, 0, 1);

			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName(TEXT("Synced Anim Warp")), Location_MoveTo, FRotator(0,Rotation_MoveTo.Yaw, 0));

			CombatState = ECombatState::Takedown;

			GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed_Base;
				
			TargetEnemy = hit.GetActor();

			
			GetWorld()->GetTimerManager().SetTimer(FreeTimer, this, &AProjectKeatsCharacter::Takedown_FreeEnemy, FreeTime_Takedown, false);

			break;
		}
	}
	else if (CombatState == ECombatState::Takedown && TakedownState == ETakedownState::ControllingEnemy) {
		TakedownState = ETakedownState::PerformingTakedown;
		if (TakedownType == ETakedownType::Soft) {
			PerformSyncedAnim(TargetEnemy, Montage_Character_KnockOut, Montage_Enemy_Takedown_Knocked);
		}
		if (TakedownType == ETakedownType::Hard) {
			PerformSyncedAnim(TargetEnemy, Montage_Character_KnockOut_Hard, Montage_Enemy_Takedown_Knocked_Hard);
		}
		if (TakedownType == ETakedownType::Wall) {
			PerformSyncedAnim(TargetEnemy, Montage_Character_KnockOut_Wall, Montage_Enemy_Takedown_Knocked_Wall);
		}
		
		GetWorld()->GetTimerManager().ClearTimer(FreeTimer);

		if (TargetEnemy->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
			Cast<IInterface_Character>(TargetEnemy)->DealDamage(1000);
		}
	}
}

void AProjectKeatsCharacter::Takedown_FreeEnemy()
{
	
	ECombatState_Enemy EnemyCombatState = Cast<IInterface_Character>(TargetEnemy)->GetCombatState_Enemy();

	if (TakedownType == ETakedownType::Soft) {
		PerformSyncedAnim(TargetEnemy, Montage_Character_FreeEnemy, Montage_Enemy_FreeEnemy);
	}
	if (TakedownType == ETakedownType::Hard) {
		PerformSyncedAnim(TargetEnemy, Montage_Character_FreeEnemy_Hard, Montage_Enemy_FreeEnemy_Hard);
	}
	if (TakedownType == ETakedownType::Wall) {
		PerformSyncedAnim(TargetEnemy, Montage_Character_FreeEnemy_Wall, Montage_Enemy_FreeEnemy_Wall);
	}
	GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, TEXT("Enemy Freed"));
	CanShoot = true;
}

//----------------------PERFORM SYNCED ANIM-----------------------
void AProjectKeatsCharacter::PerformSyncedAnim(AActor* TargetActor, UAnimMontage* Montage_Player, UAnimMontage* Montage_Enemy, bool bSetEnemyRotation, FRotator EnemyRotation)
{
	//Set Rotation Mode
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	//Set Enemy Rotation to LookAt Player
	if(bSetEnemyRotation){ TargetActor->SetActorRotation(FRotator(0, EnemyRotation.Yaw, 0)); }

	//Play Montages
	AnimInstance_Character->Montage_Play(Montage_Player);
	
	if(TargetActor->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())){
		Cast<IInterface_Character>(TargetActor)->Enemy_PlayMontage(Montage_Enemy);
	}
}

void AProjectKeatsCharacter::Vault()
{
	if (CombatState != ECombatState::Default && CombatState != ECombatState::Aim && CombatState != ECombatState::HipFire) { return; }

	
	FVector VaultStartPoint, VaultEndPoint, VaultLandPoint;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	//Object Height Check
	int8 height = 5;
	int8 depth = 10;
	FVector LastHitPoint;
	int8 LastHitIndex = -1;

	for (int i = 0; i < height; i++) {
		FHitResult hit;
		FVector TraceStart = GetActorLocation() + FVector(0,0,i*10);
		FVector TraceEnd = TraceStart + (GetActorForwardVector() * 150);

		bool IsHit = GetWorld()->LineTraceSingleByChannel(hit, TraceStart, TraceEnd, ECollisionChannel::COLLISION_VAULT, params);
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false, 5, 0, .3);

		if (IsHit) {
			LastHitPoint = hit.ImpactPoint;
			LastHitIndex = i;
		}
	}

	if (LastHitIndex == height-1 || LastHitIndex < 0) { 
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, TEXT("Can't vault! Too High or Too Low"));
		return; 
	}
	GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, FString::Printf(TEXT("Last Hit Index: %d"), LastHitIndex));
	//Set Vault Start Point
	FHitResult hit_VaultStart;

	bool IsHit_VaultStart = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), LastHitPoint + FVector(0, 0, 50), LastHitPoint, 10, UEngineTypes::ConvertToTraceType(COLLISION_VAULT),
		false, {}, VaultDebug, hit_VaultStart, true, FColor::Yellow);

	if (IsHit_VaultStart) {
		VaultStartPoint = hit_VaultStart.ImpactPoint;
		DrawDebugSphere(GetWorld(), VaultStartPoint, 10, 12, FColor::Yellow, false, 3, 0, .2);
	}

	//Object Depth Check
	LastHitIndex = -1;
	for (int i = 1; i < depth; i++) {
		FHitResult hit;
		FVector vector = GetActorForwardVector() * i * 20;
		FVector TraceStart = VaultStartPoint + FVector(vector.X, vector.Y, 50);
		FVector TraceEnd = TraceStart - FVector(0,0,70);

		bool IsHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, 10, UEngineTypes::ConvertToTraceType(COLLISION_VAULT),
			false, {}, VaultDebug, hit, true, FColor::Purple);

		if (IsHit) {
			LastHitPoint = hit.ImpactPoint;
			LastHitIndex = i;
		}
	}

	if (LastHitIndex == depth-1 || LastHitIndex < 0) {
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, TEXT("Can't vault! Too Far or Too Close."));
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, FString::Printf(TEXT("Last Hit Index: %d"), LastHitIndex));
	
	//Set Vault End Point
	VaultEndPoint = LastHitPoint;
	DrawDebugSphere(GetWorld(), VaultEndPoint, 10, 12, FColor::Purple, false, 3, 0, .2);


	//Set Vault Land Point
	FVector TraceStart = VaultEndPoint + (GetActorForwardVector() * 150);
	FVector TraceEnd = TraceStart - FVector(0,0,500);

	FHitResult hit_VaultLand;

	bool IsHit_VaultLand = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, 20, UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false, {}, VaultDebug, hit_VaultLand, true, FColor::Emerald);

	if (!IsHit_VaultLand) {
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, TEXT("Can't vault! No Available Landing Point."));
		return;
	}

	VaultLandPoint = hit_VaultLand.ImpactPoint;
	DrawDebugSphere(GetWorld(), VaultLandPoint, 10, 12, FColor::Emerald, false, 3, 0, .2);


	//Start Vaulting

	GetCapsuleComponent()->SetCapsuleHalfHeight(45);
	CameraBoom->AddRelativeLocation(FVector(0, 0, -60));
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	CombatState = ECombatState::Vault;
	AnimInstance_Character->Montage_Play(Montage_Character_Vault);
	MotionWarping->AddOrUpdateWarpTargetFromLocation(FName("Vault Start"), VaultStartPoint);
	MotionWarping->AddOrUpdateWarpTargetFromLocation(FName("Vault End"), VaultEndPoint);
	MotionWarping->AddOrUpdateWarpTargetFromLocation(FName("Vault Land"), VaultLandPoint);
	
	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &AProjectKeatsCharacter::OnVaultBlendOut);
	AnimInstance_Character->Montage_SetBlendingOutDelegate(BlendOutDelegate, Montage_Character_Vault);
	
}


void AProjectKeatsCharacter::OnVaultBlendOut(UAnimMontage* animMontage, bool bInterrupted)
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	CombatState = ECombatState::Default;
	VaultState = EVaultState::Vaulting;
	GetCapsuleComponent()->SetCapsuleHalfHeight(90);
	CameraBoom->AddRelativeLocation(FVector(0, 0, 60));

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}


void AProjectKeatsCharacter::HumanShield_Grab() {
	if (CombatState == ECombatState::Default || CombatState == ECombatState::Aim || CombatState == ECombatState::HipFire || CombatState == ECombatState::Reloading) {
		FRotator BoxTraceOrientation = FRotator(0, GetControlRotation().Yaw, GetControlRotation().Roll);
		FVector BoxTraceStart = GetActorLocation() + (UKismetMathLibrary::GetForwardVector(BoxTraceOrientation) * 30);
		FVector BoxTraceEnd = BoxTraceStart + (UKismetMathLibrary::GetForwardVector(BoxTraceOrientation) * 130);

		TArray<FHitResult> hits;

		UKismetSystemLibrary::BoxTraceMulti(GetWorld(), BoxTraceStart, BoxTraceEnd, FVector(10, 50, 100), BoxTraceOrientation, UEngineTypes::ConvertToTraceType(COLLISION_ENEMY),
			false, {}, EDrawDebugTrace::ForDuration, hits, true);

		for (FHitResult hit : hits) {
			if (!hit.GetActor()->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) { return; }

			ECombatState_Enemy EnemyCombatState = Cast<IInterface_Character>(hit.GetActor())->GetCombatState_Enemy();

			if (EnemyCombatState != ECombatState_Enemy::Stunned) { return; }
			HumanShieldState = EHumanShieldState::HumanShieldBase;
			DrawDebugSphere(GetWorld(), hit.GetActor()->GetActorLocation(), 20, 10, FColor::Green, false, 3, 0, 1);

			//Set MoveTo Location and Rotation
			FRotator EnemyRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), hit.GetActor()->GetActorLocation());
			FVector Location_MoveTo = hit.GetActor()->GetActorLocation() - ((UKismetMathLibrary::GetForwardVector(EnemyRotation)) * 10) + 
				(UKismetMathLibrary::GetRightVector(EnemyRotation) * 10);

			FRotator Rotation_MoveTo = UKismetMathLibrary::FindLookAtRotation(Location_MoveTo, hit.GetActor()->GetActorLocation() + 
				(UKismetMathLibrary::GetRightVector(EnemyRotation) * 10));

			DrawDebugSphere(GetWorld(), Location_MoveTo, 20, 10, FColor::Yellow, false, 3, 0, 1);

			PerformSyncedAnim(hit.GetActor(), Montage_Character_HumanShield_Grab, Montage_Enemy_HumanShield_Grabbed, true, EnemyRotation);
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName(TEXT("Synced Anim Warp")), Location_MoveTo, Rotation_MoveTo);

			CombatState = ECombatState::HumanShield;

			if (hit.GetActor()->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
				Cast<IInterface_Character>(hit.GetActor())->SetCombatState_Enemy(ECombatState_Enemy::HumanShield);
			}

			GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed_Aiming;

			TargetEnemy = hit.GetActor();


			GetWorld()->GetTimerManager().SetTimer(FreeTimer, this, &AProjectKeatsCharacter::HumanShield_Free, FreeTime_HumanShield, false);

			break;
		}
	}
	else if (CombatState == ECombatState::HumanShield && HumanShieldState == EHumanShieldState::HumanShieldBase) {
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed_Base;
		PerformSyncedAnim(TargetEnemy, Montage_Character_HumanShield_KnockOut, Montage_Enemy_HumanShield_Knocked);
		FDetachmentTransformRules DetachmentRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, false);
		TargetEnemy->DetachFromActor(DetachmentRules);
		GetWorld()->GetTimerManager().ClearTimer(FreeTimer);

		if (TargetEnemy->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
			Cast<IInterface_Character>(TargetEnemy)->SetCombatState_Enemy(ECombatState_Enemy::Default); 
			Cast<IInterface_Character>(TargetEnemy)->DealDamage(100000);
		}
	}
}

void AProjectKeatsCharacter::HumanShield_Free()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed_Base;
	PerformSyncedAnim(TargetEnemy, Montage_Character_HumanShield_Free, Montage_Enemy_HumanShield_Free);
	FDetachmentTransformRules DetachmentRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, false);
	TargetEnemy->DetachFromActor(DetachmentRules);

	if (TargetEnemy->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {Cast<IInterface_Character>(TargetEnemy)->SetCombatState_Enemy(ECombatState_Enemy::Default);}
}

void AProjectKeatsCharacter::HumanShield_Push()
{
	if (CombatState != ECombatState::HumanShield) { return; }
	if (!TargetEnemy) { return; }

	FDetachmentTransformRules rules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, false);
	TargetEnemy->DetachFromActor(rules);
	GetWorld()->GetTimerManager().ClearTimer(FreeTimer);

	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed_Base;

	AnimInstance_Character->Montage_Play(Montage_Character_HumanShield_Push);

	if (TargetEnemy->GetClass()->ImplementsInterface(UInterface_Character::StaticClass())) {
		Cast<IInterface_Character>(TargetEnemy)->PushEnemy();
	}


}

void AProjectKeatsCharacter::Cover()
{
	if (!(CombatState == ECombatState::Default || CombatState == ECombatState::Reloading || CombatState == ECombatState::Aim || CombatState == ECombatState::HipFire)) { return; }

	CoverType = ECoverTypes::HighCover;
	FHitResult coverHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = TraceStart + (GetActorForwardVector() * 100);
	bool isHit = GetWorld()->LineTraceSingleByChannel(coverHit, TraceStart, TraceEnd, ECollisionChannel::COLLISION_COVER, params);
	
	FHitResult coverHit_High;
	bool isHit_High = GetWorld()->LineTraceSingleByChannel(coverHit_High, TraceStart + FVector(0, 0, 120), TraceEnd + FVector(0, 0, 120), ECollisionChannel::COLLISION_COVER, params);

	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false, 5, 0, .5);
	DrawDebugLine(GetWorld(), TraceStart + FVector(0, 0, 120), TraceEnd + FVector(0, 0, 120), FColor::Blue, false, 5, 0, .5);
	if (!isHit) { return; }
	if (!isHit_High) { CoverType = ECoverTypes::LowCover; }

	GetCharacterMovement()->SetPlaneConstraintEnabled(true);
	GetCharacterMovement()->SetPlaneConstraintNormal(coverHit.ImpactNormal);

	CombatState = ECombatState::Cover;
	bUseControllerRotationYaw = false;

	FVector Location = coverHit.ImpactPoint + (coverHit.ImpactNormal * 30);
	FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Location, Location + (coverHit.ImpactNormal * -50));
	FLatentActionInfo ActionInfo;
	ActionInfo.CallbackTarget = this;

	DrawDebugSphere(GetWorld(), Location, 10, 10, FColor::Purple, false, 5, 0, .3);

	UKismetSystemLibrary::MoveComponentTo(RootComponent, Location, Rotation, false, false, .1, true, EMoveComponentAction::Type::Move, ActionInfo);
}

void AProjectKeatsCharacter::MoveOnCover(float MovementValue)
{
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	//Trace Right
	FRotator rotator = UKismetMathLibrary::MakeRotFromX(GetCharacterMovement()->GetPlaneConstraintNormal() * -1);
	FVector TraceStart = GetActorLocation() + (UKismetMathLibrary::GetRightVector(rotator) * 45);
	FVector TraceEnd = TraceStart + (GetCharacterMovement()->GetPlaneConstraintNormal() * -1 * 200);
	bool IsRightHit = GetWorld()->LineTraceSingleByChannel(hit, TraceStart, TraceEnd, ECollisionChannel::COLLISION_COVER, params);

	//DebugLine Right
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Yellow, false, 0, 0, 1);

	//Trace High Right
	bool isHit_High_Right = GetWorld()->LineTraceSingleByChannel(hit, TraceStart + FVector(0, 0, 80),TraceEnd + FVector(0, 0, 80), ECollisionChannel::COLLISION_COVER, params);

	//DebugLine High Right
	DrawDebugLine(GetWorld(), TraceStart + FVector(0, 0, 80), TraceEnd + FVector(0, 0, 80), FColor::Yellow, false, 0, 0, 1);


	//Trace Left
	rotator = UKismetMathLibrary::MakeRotFromX(GetCharacterMovement()->GetPlaneConstraintNormal());
	TraceStart = GetActorLocation() + (UKismetMathLibrary::GetRightVector(rotator) * 45);
	TraceEnd = TraceStart + (GetCharacterMovement()->GetPlaneConstraintNormal() * -1 * 200);
	bool IsLeftHit = GetWorld()->LineTraceSingleByChannel(hit, TraceStart, TraceEnd, ECollisionChannel::COLLISION_COVER, params);

	//DebugLine Left
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Yellow, false, 0, 0, 1);

	//Trace High Left
	bool isHit_High_Left = GetWorld()->LineTraceSingleByChannel(hit, TraceStart + FVector(0, 0, 80), TraceEnd + FVector(0, 0, 80), ECollisionChannel::COLLISION_COVER, params);

	//DebugLine High Left
	DrawDebugLine(GetWorld(), TraceStart + FVector(0, 0, 80), TraceEnd + FVector(0, 0, 80), FColor::Yellow, false, 0, 0, 1);

	//Set Cover Type
	//CoverType = ((isHit_High_Right && isHit_High_Left) || !(IsRightHit && IsLeftHit)) ? ECoverTypes::HighCover : ECoverTypes::LowCover;

	if (isHit_High_Right && isHit_High_Left) {
		CoverType = ECoverTypes::HighCover;
	}
	else {
		if (IsRightHit && IsLeftHit) {CoverType = ECoverTypes::LowCover;}
	}

	FVector ForwardVector = UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0));
	FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), GetActorLocation() + (GetCharacterMovement()->GetPlaneConstraintNormal() * -1 * 200));

	DrawDebugDirectionalArrow(GetWorld(), GetActorLocation() + FVector(0, 0, 90), GetActorLocation() + FVector(0, 0, 90) + (ForwardVector * 100), 100, FColor::Emerald, false, 0, 0, 2);
	DrawDebugDirectionalArrow(GetWorld(), GetActorLocation() + FVector(0, 0, 90), GetActorLocation() + FVector(0, 0, 90) + (Direction * 100), 100, FColor::Orange, false, 0, 0, 2);

	if (IsRightHit && IsLeftHit) {
		if (MovementValue == 0) { return; }
		TraceStart = GetActorLocation();
		TraceEnd = TraceStart + (GetCharacterMovement()->GetPlaneConstraintNormal() * -1 * 200);
		bool IsHit = GetWorld()->LineTraceSingleByChannel(hit, TraceStart, TraceEnd, ECollisionChannel::COLLISION_COVER, params);

		if (!IsHit) { return; }
		
		GetCharacterMovement()->SetPlaneConstraintNormal(hit.ImpactNormal);

		FVector WorldDirection = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotator(0, 0, GetControlRotation().Yaw));
		AddMovementInput(WorldDirection, MovementValue);
	}
	else {
		float dot = UKismetMathLibrary::Dot_VectorVector(ForwardVector, Direction);
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Purple, FString::Printf(TEXT("Dot Product: %.2f"), dot));

		bool select;
		if (UKismetMathLibrary::SignOfFloat(dot) == 1) {
			select = (UKismetMathLibrary::SignOfFloat(MovementValue) == 1) ? IsRightHit : IsLeftHit;
		}
		else {
			select = (UKismetMathLibrary::SignOfFloat(MovementValue) == 1) ? IsLeftHit : IsRightHit;
		}

		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FString::Printf(TEXT("Select Bool is: %d"), (select) ? 1 : 0));

		float value = (UKismetMathLibrary::SignOfFloat(MovementValue) != 0 && select) ? MovementValue : 0;

		FVector WorldDirection = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotator(0, 0, GetControlRotation().Yaw));
		AddMovementInput(WorldDirection, value);
	}
}
