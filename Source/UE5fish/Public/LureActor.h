// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LureActor.generated.h"

UCLASS()
class UE5FISH_API ALureActor : public AActor
{
    GENERATED_BODY()

public:
    ALureActor();

    UFUNCTION()
    void AddImpulse(const FVector& Force);

protected:
    virtual void BeginPlay() override;

    /** ルアーメッシュ（スケルタルメッシュ） */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lure")
    class USkeletalMeshComponent* Mesh;

};