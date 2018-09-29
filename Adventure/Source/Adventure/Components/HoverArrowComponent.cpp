// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "HoverArrowComponent.h"
#include "GameFramework/Actor.h"
#include "Adventure.h"

// Sets default values for this component's properties
UHoverArrowComponent::UHoverArrowComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bActive = false;
	ElapsedTime = 0.0f;
	Frequency = 5.0f;
	Amplitude = 300.0f;
	SetVisibility(bActive, true);
}

void UHoverArrowComponent::ShowCursor()
{
	bActive = true;
	SetVisibility(bActive, true);
}

void UHoverArrowComponent::HideCursor()
{
	bActive = false;
	SetVisibility(bActive, true);
}

// Called when the game starts
void UHoverArrowComponent::BeginPlay()
{
	Super::BeginPlay();

	BaseHeight = RelativeLocation.Z;
}

// Called every frame
void UHoverArrowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bActive)
	{
		// Set Height
		ElapsedTime = FMath::Fmod((ElapsedTime + DeltaTime), Frequency);

		float angularFrequency = (2.0f*PI) / Frequency;
		float newHeight = 0.5f* (Amplitude * FMath::Cos(angularFrequency * ElapsedTime + PI) + Amplitude);

		FVector newLocation = FVector(RelativeLocation.X, RelativeLocation.Y, newHeight + BaseHeight);
		SetRelativeLocation(newLocation);

		// Set Rotation
		SetWorldRotation(FRotator(0.0f, FMath::RadiansToDegrees(angularFrequency * ElapsedTime), 90.0f));
	}
}
