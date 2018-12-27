// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "HeldObject.h"


// Sets default values
AHeldObject::AHeldObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHeldObject::BeginPlay()
{
	Super::BeginPlay();
	
}

WEAPON_TYPE AHeldObject::GetWeaponStance_Implementation() const
{
	return WEAPON_TYPE::SWORD_1H;
}

// Called every frame
void AHeldObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

