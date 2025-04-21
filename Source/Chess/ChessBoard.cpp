#include "ChessBoard.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "EngineUtils.h"

AChessBoard::AChessBoard()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AChessBoard::BeginPlay()
{
	Super::BeginPlay();
	// Call once per second
	GetWorld()->GetTimerManager().SetTimer(BoardUpdateTimer, this, &AChessBoard::RequestBoardUpdate, 1.0f, true);
	// Set up recurring timer to fetch new board updates every 1 second
	// Store all pieces in TMap
	// StorePiecesInTMap();
}

void AChessBoard::StorePiecesInTMap()
{
	// Iterate through all actors and store them in the map based on their row/col
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor->ActorHasTag(TEXT("Piece"))) // Ensure it has the "Piece" tag
		{
			// Create the key based on Row and Col, e.g., "Piece_0_0"
			FString ActorName = Actor->GetName();
			PieceActors.Add(ActorName, Actor);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Stored %d pieces in TMap"), PieceActors.Num());
}

void AChessBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Optional: Periodically request board updates
}

void AChessBoard::RequestBoardUpdate()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(TEXT("http://127.0.0.1:8000/move"));
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->OnProcessRequestComplete().BindUObject(this, &AChessBoard::OnBoardDataReceived);
	Request->ProcessRequest();
	DestroyCall();
	SendAcknowledgementToServer();
}

void AChessBoard::SendAcknowledgementToServer()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> AckRequest = FHttpModule::Get().CreateRequest();
	AckRequest->SetURL(TEXT("http://127.0.0.1:8000/ack")); // Make sure this endpoint exists in Python
	AckRequest->SetVerb("POST");
	AckRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	AckRequest->SetContentAsString(JsonString);
	AckRequest->ProcessRequest();
}




void AChessBoard::OnBoardDataReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get board data"));
		return;
	}

	JsonString = Response->GetContentAsString();

	TSharedPtr<FJsonValue> JsonParsed;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonParsed) || !JsonParsed.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON"));
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* OuterArray;
	if (!JsonParsed->TryGetArray(OuterArray)) return;

	TArray<TArray<TArray<int32>>> BoardArray;

	for (const auto& ChannelValue : *OuterArray)
	{
		const TArray<TSharedPtr<FJsonValue>>* Rows;
		if (!ChannelValue->TryGetArray(Rows)) continue;

		TArray<TArray<int32>> Layer;
		for (const auto& RowValue : *Rows)
		{
			const TArray<TSharedPtr<FJsonValue>>* Cols;
			if (!RowValue->TryGetArray(Cols)) continue;

			TArray<int32> Row;
			for (const auto& CellValue : *Cols)
			{
				Row.Add(static_cast<int32>(CellValue->AsNumber()));
			}
			Layer.Add(Row);
		}
		BoardArray.Add(Layer);
	}

	UpdateBoardFromArray(BoardArray);
}

void AChessBoard::UpdateBoardFromArray(const TArray<TArray<TArray<int32>>>& BoardArray)
{
	UE_LOG(LogTemp, Warning, TEXT("Received board update with %d channels"), BoardArray.Num());

	if (BoardArray.Num() == 0) return;

	const auto& Board = BoardArray[0]; // First channel assumed to hold piece positions

	for (int Row = 0; Row < Board.Num(); ++Row)
	{
		for (int Col = 0; Col < Board[Row].Num(); ++Col)
		{
			int32 PieceCode = Board[Row][Col];

			// Skip empty cells (e.g., 0 or custom empty value)
			if (PieceCode == 0) continue;

			// Calculate the position based on Row and Col
			FVector NewLocation = FVector((Col * 100.f) + X, (Row * 100.f) + Y, 0); // Adjust spacing as needed

			// Decide the mesh and spawn the piece based on PieceCode
			AActor* SpawnedPiece = nullptr;
			UStaticMesh* PieceMesh = nullptr;

			bool bIsWhite = (PieceCode >= 1 && PieceCode <= 6);
			switch (PieceCode)
			{
			case 1: case 7: // Pawn
				SpawnedPiece = GetWorld()->SpawnActor<AActor>(PawnActorClass, NewLocation, FRotator::ZeroRotator);
				PieceMesh = bIsWhite ? WhitePawnMesh : BlackPawnMesh;
				break;
			case 2: case 8: // Knight
				SpawnedPiece = GetWorld()->SpawnActor<AActor>(KnightActorClass, NewLocation, FRotator::ZeroRotator);
				PieceMesh = bIsWhite ? WhiteKnightMesh : BlackKnightMesh;
				break;
			case 3: case 9: // Bishop
				SpawnedPiece = GetWorld()->SpawnActor<AActor>(BishopActorClass, NewLocation, FRotator::ZeroRotator);
				PieceMesh = bIsWhite ? WhiteBishopMesh : BlackBishopMesh;
				break;
			case 4: case 10: // Rook
				SpawnedPiece = GetWorld()->SpawnActor<AActor>(RookActorClass, NewLocation, FRotator::ZeroRotator);
				PieceMesh = bIsWhite ? WhiteRookMesh : BlackRookMesh;
				break;
			case 5: case 11: // Queen
				SpawnedPiece = GetWorld()->SpawnActor<AActor>(QueenActorClass, NewLocation, FRotator::ZeroRotator);
				PieceMesh = bIsWhite ? WhiteQueenMesh : BlackQueenMesh;
				break;
			case 6: case 12: // King
				SpawnedPiece = GetWorld()->SpawnActor<AActor>(KingActorClass, NewLocation, FRotator::ZeroRotator);
				PieceMesh = bIsWhite ? WhiteKingMesh : BlackKingMesh;
				break;
			}

			// Set the mesh for the spawned piece
			if (SpawnedPiece)
			{
				UStaticMeshComponent* MeshComponent = SpawnedPiece->FindComponentByClass<UStaticMeshComponent>();
				if (MeshComponent && PieceMesh)
				{
					MeshComponent->SetStaticMesh(PieceMesh);
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Board updated with newly spawned pieces."));
}
