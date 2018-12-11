// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "MapPawnComponent_Head.h"


// Sets default values
UMapPawnComponent_Head::UMapPawnComponent_Head()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void UMapPawnComponent_Head::BeginPlay()
{
	Super::BeginPlay();
	
}