//By: Eric Marquez. 
//All information and code provided is free to use and can be used comercially.
//Use of such examples indicates no fault to the author for any damages caused by them. 
//The author must be credited.
#include "Interactable.h"

#include "Adventure.h"

// Sets default values
AInteractable::AInteractable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TargetVisibility = 1.0f;
	OpacityTransitionValue = 0.05;
	VisibilityValue = 1.0f;
	bCanBeTransparent = true;

	//Create a static mesh component
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Focus"));
	RootComponent = Scene;

	//Create a component for the pawns body
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Scene);
	Visual->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel4, ECollisionResponse::ECR_Block);
	bReplicates = true;
}

void AInteractable::MakeTransparent()
{
	if (bCanBeTransparent)
		TargetVisibility = 0.0f;
}

// Called when the game starts or when spawned
void AInteractable::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Modify opacity based on bMakeTransparent set by camera owner
	if (bCanBeTransparent && TargetVisibility != VisibilityValue)
	{
		VisibilityValue = FMath::Clamp(FMath::Lerp(VisibilityValue, TargetVisibility, OpacityTransitionValue), 0.0f, 1.0f);
		float Distance = FMath::Abs(VisibilityValue - TargetVisibility);
		if (Distance <= 0.1f)
		{
			VisibilityValue = TargetVisibility;
		}
		OnVisibilityValueChanged(VisibilityValue);
	}
	TargetVisibility = 1.0f;
}

void AInteractable::SetStaticMesh(UStaticMesh * StaticMesh)
{
	if (Visual)
	{
		Visual->SetStaticMesh(StaticMesh);
	}
}

