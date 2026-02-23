#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurretLookAtPlayerComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TURRET_API UTurretLookAtPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurretLookAtPlayerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float Range = 1500.0f;

	// Czy ma sprawdzać line-of-sight (trace)?
	UPROPERTY(EditAnywhere, Category = "Targeting")
	bool bRequireLineOfSight = true;

	// Obrót tylko w osi Z (typowe dla wieżyczki bazowej)
	UPROPERTY(EditAnywhere, Category = "Rotation")
	bool bYawOnly = true;

	// Prędkość obrotu w stopniach na sekundę (interpolacja)
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.0"))
	float RotationSpeedDegPerSec = 180.0f;

	// Martwa strefa w stopniach (żeby nie „drgało” przy prawie równym kierunku)
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.0"))
	float DeadZoneDeg = 1.0f;

private:
	AActor* GetPlayerActor() const;
	bool IsPlayerInRange(AActor* Player) const;
	bool HasLineOfSightTo(AActor* Player) const;

	void RotateOwnerToward(AActor* Player, float DeltaTime);
};