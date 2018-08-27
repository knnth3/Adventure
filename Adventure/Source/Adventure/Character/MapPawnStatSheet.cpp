// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "MapPawnStatSheet.h"

#include "Adventure.h"

void AMapPawnAttack::Attack_Implementation(const FMapPawnStatSheet& Stats, const FVector& StartLocation, const FVector& EndLocation) const
{
}

void AMapPawnAttack_Basic::Attack_Implementation(const FMapPawnStatSheet& Stats, const FVector& StartLocation, const FVector& EndLocation) const
{
	UE_LOG(LogNotice, Warning, TEXT("Move used: Basic Attack!"));
}
