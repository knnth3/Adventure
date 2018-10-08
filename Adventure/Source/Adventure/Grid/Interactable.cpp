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
	PrimaryActorTick.bCanEverTick = false;
	m_ObjectID = -1;
	bIsTraversable = true;
	bHasMovementPenalty = false;

	// Create a static mesh component
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Focus"));
	RootComponent = Scene;

	// Create a box collision
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(Scene);
	BoxCollision->SetWorldScale3D(FVector(1.524));
	BoxCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	BoxCollision->ComponentTags.Add(FName("PathBlock"));

	// Create a component for the pawns body
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Scene);
	Visual->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Block);
	Visual->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel4, ECollisionResponse::ECR_Block);
	bReplicates = true;
}

// Called when the game starts or when spawned
void AInteractable::BeginPlay()
{
	Super::BeginPlay();
	
}

void AInteractable::SetStaticMesh(UStaticMesh * StaticMesh)
{
	if (Visual)
	{
		Visual->SetStaticMesh(StaticMesh);
	}
}

void AInteractable::ServerOnly_SetObjectID(int ID)
{
	m_ObjectID = ID;
}

void AInteractable::ServerOnly_SetClassIndex(int Index)
{
	m_ClassIndex = Index;
}

int AInteractable::GetClassIndex_Implementation() const
{
	return m_ClassIndex;
}

int AInteractable::GetObjectID_Implementation() const
{
	return m_ObjectID;
}

bool AInteractable::IsNonTraversable_Implementation() const
{
	return bIsTraversable;
}

bool AInteractable::IsBlockingSpace_Implementation() const
{
	return bHasMovementPenalty;
}

