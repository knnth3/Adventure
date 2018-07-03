// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "ChatManager.h"
#include "Adventure.h"


// Sets default values
AChatManager::AChatManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChatManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AChatManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChatManager::AddText(FString text)
{
	Server_AddText(text);
}

bool AChatManager::HasNewEntries()
{
	bool value = m_bHasNewItems;
	m_bHasNewItems = false;
	return value;
}

void AChatManager::Server_AddText_Implementation(const FString& text)
{
	m_Log.Push(text);
}

bool AChatManager::Server_AddText_Validate(const FString& text)
{
	return true;
}

void AChatManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AChatManager, m_Log);
	DOREPLIFETIME(AChatManager, m_bHasNewItems);
}
