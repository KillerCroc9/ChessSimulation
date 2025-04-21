#pragma once

#include <Interfaces/IHttpRequest.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessBoard.generated.h"


UCLASS()
class CHESS_API AChessBoard : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	AChessBoard();

protected:
	virtual void BeginPlay() override;

	void StorePiecesInTMap();

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	TMap<FString, AActor*> PieceActors;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* WhitePawnMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* BlackPawnMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* WhiteKnightMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* BlackKnightMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* WhiteBishopMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* BlackBishopMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* WhiteRookMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* BlackRookMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* WhiteQueenMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* BlackQueenMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* WhiteKingMesh;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	UStaticMesh* BlackKingMesh;

	// Actor class to spawn for chess pieces
	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	TSubclassOf<AActor> PawnActorClass;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	TSubclassOf<AActor> KnightActorClass;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	TSubclassOf<AActor> BishopActorClass;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	TSubclassOf<AActor> RookActorClass;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	TSubclassOf<AActor> QueenActorClass;

	UPROPERTY(EditAnywhere, Category = "Chess Pieces")
	TSubclassOf<AActor> KingActorClass;

	UFUNCTION(BlueprintImplementableEvent)
	void DestroyCall();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	float X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	float Y;


private:
	void RequestBoardUpdate();
	void SendAcknowledgementToServer();
	void OnAckResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnBoardDataReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void UpdateBoardFromArray(const TArray<TArray<TArray<int32>>>& BoardArray);
	FTimerHandle BoardUpdateTimer;
	FString JsonString;
};
