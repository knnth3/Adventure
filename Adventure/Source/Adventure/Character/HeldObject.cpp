// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

