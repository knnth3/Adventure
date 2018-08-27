// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "MapPawnComponent_Head.h"


// Sets default values
AMapPawnComponent_Head::AMapPawnComponent_Head()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMapPawnComponent_Head::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMapPawnComponent_Head::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

