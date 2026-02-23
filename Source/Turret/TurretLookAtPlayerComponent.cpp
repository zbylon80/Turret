#include "TurretLookAtPlayerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

UTurretLookAtPlayerComponent::UTurretLookAtPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTurretLookAtPlayerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTurretLookAtPlayerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner) return;

	AActor* Player = GetPlayerActor();
	if (!Player) return;

	if (!IsPlayerInRange(Player)) return;
	if (bRequireLineOfSight && !HasLineOfSightTo(Player)) return;

	RotateOwnerToward(Player, DeltaTime);
}

AActor* UTurretLookAtPlayerComponent::GetPlayerActor() const
{
	// Prosto: bierzemy pawn gracza 0
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	return PlayerPawn;
}

bool UTurretLookAtPlayerComponent::IsPlayerInRange(AActor* Player) const
{
	const AActor* Owner = GetOwner();
	if (!Owner || !Player) return false;

	const float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Player->GetActorLocation());
	return DistSq <= FMath::Square(Range);
}

bool UTurretLookAtPlayerComponent::HasLineOfSightTo(AActor* Player) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !Player) return false;

	// Punkt startowy: właściciel (możesz później podmienić na socket/muzzle)
	const FVector Start = Owner->GetActorLocation();
	// Cel: środek gracza (możesz podmienić na Head socket / Capsule)
	const FVector End = Player->GetActorLocation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	Params.AddIgnoredActor(Player); // opcjonalnie: ignorer player, ale wtedy trzeba sprawdzać trafienia inaczej
	Params.bTraceComplex = true;

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	// Jeśli coś uderzyło po drodze, to brak LOS
	// UWAGA: ponieważ ignorujemy Playera, to "czysty" brak trafienia = LOS OK
	return !bHit;

	// Jeśli wolisz NIE ignorować Playera:
	// - usuń Params.AddIgnoredActor(Player)
	// - i wtedy zwracaj: (!bHit) ? false : Hit.GetActor() == Player
}

void UTurretLookAtPlayerComponent::RotateOwnerToward(AActor* Player, float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Player) return;

	const FVector OwnerLoc = Owner->GetActorLocation();
	const FVector TargetLoc = Player->GetActorLocation();

	const FVector ToTarget = (TargetLoc - OwnerLoc).GetSafeNormal();
	FRotator DesiredRot = ToTarget.Rotation();

	if (bYawOnly)
	{
		// zachowaj tylko yaw, zeruj pitch/roll
		DesiredRot.Pitch = 0.0f;
		DesiredRot.Roll = 0.0f;
	}

	const FRotator CurrentRot = Owner->GetActorRotation();

	// Dead-zone: jeśli różnica w yaw (lub full rot) jest mała, nie ruszaj
	const float AngleDiff = FMath::Abs(FRotator::NormalizeAxis(DesiredRot.Yaw - CurrentRot.Yaw));
	if (bYawOnly && AngleDiff <= DeadZoneDeg) return;

	// Interpolacja: stała prędkość w deg/s
	const float InterpSpeed = (RotationSpeedDegPerSec > 0.0f) ? (RotationSpeedDegPerSec / 90.0f) : 0.0f;
	// Uwaga: RInterpTo używa "speed" w jednostkach 1/s, więc trik: dobrać to pod feeling.
	// Alternatywa (bardziej przewidywalna): obrót o max RotationSpeedDegPerSec * DeltaTime.

	// Bardziej przewidywalne rozwiązanie: clamp delta yaw
	if (bYawOnly)
	{
		const float MaxStep = RotationSpeedDegPerSec * DeltaTime;
		const float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, DesiredRot.Yaw);
		const float StepYaw = FMath::Clamp(DeltaYaw, -MaxStep, MaxStep);

		FRotator NewRot = CurrentRot;
		NewRot.Yaw = CurrentRot.Yaw + StepYaw;
		Owner->SetActorRotation(NewRot);
		return;
	}

	// Full rotation (pitch/yaw) – mniej typowe dla turret base
	const FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, DesiredRot, DeltaTime, RotationSpeedDegPerSec);
	Owner->SetActorRotation(NewRot);
}