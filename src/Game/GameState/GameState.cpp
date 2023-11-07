//
// Created by Adis Veletanlic on 2023-11-06.
//

#include "GameState.h"
#include "../Piece/Queen/Queen.h"
#include "../Piece/Rook/Rook.h"
#include "../Piece/Bishop/Bishop.h"
#include "../Piece/Knight/Knight.h"
#include "../Piece/Pawn/Pawn.h"

Position GameState::GetKingPosition(PlayerColor color) const {
    return color == PlayerColor::White ? whiteKingPosition : blackKingPosition;
}

void GameState::ChangePlayerTurn() {
    this->playerTurn = this->playerTurn == PlayerColor::White ? PlayerColor::Black : PlayerColor::White;
}

void GameState::UpdateBoard(sf::RenderWindow& window) {
    this->board->DrawBoard(window);
}

void GameState::RenderMovesAndAttacks(sf::RenderWindow& window, const std::vector<Move>& availableMoves, const std::optional<std::shared_ptr<Piece>>& selectedPiece) {
    if (availableMoves.empty() || !selectedPiece) {
        return;
    }
    this->board->DrawAvailableMoves(window, availableMoves, selectedPiece.value());
}

bool GameState::IsKingInCheck() const {
    sf::Vector2i kingPosition = GetKingPosition(playerTurn);

    // Pawn Threats
    int direction = (playerTurn == PlayerColor::White) ? -1 : 1;
    std::vector<sf::Vector2i> pawnThreats = {
            {kingPosition.x - 1, kingPosition.y + direction},
            {kingPosition.x + 1, kingPosition.y + direction}
    };
    for (const auto& move : pawnThreats) {
        if (Board::IsWithinBounds(move)) {
            auto piece = board->GetPieceAt(move);
            if (piece && piece->GetType() == PieceType::Pawn && piece->GetColor() != playerTurn) {
                return true;
            }
        }
    }

    // Knight Threats
    std::vector<sf::Vector2i> knightThreats = {
            {kingPosition.x - 1, kingPosition.y - 2}, {kingPosition.x + 1, kingPosition.y - 2},
            {kingPosition.x + 2, kingPosition.y - 1}, {kingPosition.x + 2, kingPosition.y + 1},
            {kingPosition.x + 1, kingPosition.y + 2}, {kingPosition.x - 1, kingPosition.y + 2},
            {kingPosition.x - 2, kingPosition.y + 1}, {kingPosition.x - 2, kingPosition.y - 1}
    };
    for (const auto& move : knightThreats) {
        if (Board::IsWithinBounds(move)) {
            auto piece = board->GetPieceAt(move);
            if (piece && piece->GetType() == PieceType::Knight && piece->GetColor() != playerTurn) {
                return true;
            }
        }
    }

    // Rook, Bishop, and Queen Threats
    std::vector<sf::Vector2i> directions = {
            {0, -1}, {0, 1}, {-1, 0}, {1, 0},   // Rook and Queen
            {-1, -1}, {1, -1}, {-1, 1}, {1, 1}  // Bishop and Queen
    };
    for (const auto& dir : directions) {
        sf::Vector2i currentPos = kingPosition;
        while (Board::IsWithinBounds(currentPos += dir)) {
            auto piece = board->GetPieceAt(currentPos);
            if (piece) {
                PieceType type = piece->GetType();
                if ((type == PieceType::Rook || type == PieceType::Bishop || type == PieceType::Queen) && piece->GetColor() != playerTurn) {
                    if (type == PieceType::Queen) return true; // Queen threat
                    if (dir.x == 0 || dir.y == 0) { // Horizontal or vertical
                        if (type == PieceType::Rook) return true; // Rook threat
                    } else { // Diagonal
                        if (type == PieceType::Bishop) return true; // Bishop threat
                    }
                }
                break;
            }
        }
    }
    return false;
}

bool GameState::IsCheckmate(const Player &player) {
    return false;
}

std::shared_ptr<Board> GameState::GetBoard() {
    return this->board;
}

void GameState::PromotePawnAt(const sf::Vector2i& position, PieceType type) {
    auto pawn = this->board->GetPieceAt(position);
    switch (type) {
        case PieceType::Queen:
            this->board->SetPieceAt(position, std::make_shared<Queen>(position, playerTurn, type));
            break;
        case PieceType::Rook:
            this->board->SetPieceAt(position, std::make_shared<Rook>(position, playerTurn, type));
            break;
        case PieceType::Bishop:
            this->board->SetPieceAt(position, std::make_shared<Bishop>(position, playerTurn, type));
            break;
        case PieceType::Knight:
            this->board->SetPieceAt(position, std::make_shared<Knight>(position, playerTurn, type));
            break;
        case PieceType::Pawn:
        case PieceType::King:
            break;
    }
}

bool GameState::CanMoveTo(const sf::Vector2i &move, const std::vector<Move>& availableMoves) {
    return std::any_of(availableMoves.begin(), availableMoves.end(),
                       [&move](const Move& availableMove) {
                           return availableMove.moveToDirection == move;
                       });
}

bool GameState::IsValidMove(const sf::Vector2i &move, const Player& currentPlayer) {
    const auto& tempMovedPiecePointer = currentPlayer.GetSelectedPiece(*this->board);

    if (tempMovedPiecePointer) {
        const auto tempMovedPiece = tempMovedPiecePointer.value();
        sf::Vector2i originalPosition = tempMovedPiece->GetPosition();
        auto pieceAtDestination = board->GetPieceAt(move);

        board->SetPieceAt(originalPosition, nullptr);
        board->SetPieceAt(move, tempMovedPiece);
        tempMovedPiece->SetPosition(move);

        //CheckUpdateKingPosition(move);

        bool movePutsKingInCheck = IsKingInCheck();

        board->SetPieceAt(move, pieceAtDestination);
        board->SetPieceAt(originalPosition, tempMovedPiece);
        tempMovedPiece->SetPosition(originalPosition);

        //CheckUpdateKingPosition(originalPosition);

        return !movePutsKingInCheck;
    }
    return false;
}

void GameState::MoveSelectedPieceTo(const Position& moveTo, const Position& moveFrom) {

    auto movingPiece = this->board->GetPieceAt(moveFrom);

    this->board->SetPieceAt(moveTo, this->board->GetPieceAt(moveFrom));
    movingPiece->SetPosition(moveTo);
    this->board->SetPieceAt(moveFrom, nullptr);

    if (movingPiece->GetType() == PieceType::Pawn) {
        auto pawn = std::dynamic_pointer_cast<Pawn>(movingPiece);
        pawn->SetHasMoved();
    }
}

void GameState::UpdateKingPosition(const Position& moveTo, const std::shared_ptr<Piece>& piece, Position& kingPosition) {
    if (this->board->GetPieceAt(moveTo)->GetType() == PieceType::King) {
        if (this->board->GetPieceAt(moveTo)->GetColor() == PlayerColor::White) {
            this->whiteKingPosition = moveTo;
        } else {
            this->blackKingPosition = moveTo;
        }
    }
}

void GameState::InitializeBoard() {
    this->board->Populate();
}

std::shared_ptr<Piece> GameState::GetPieceOnBoard(Position position) {
    return this->board->GetPieceAt(position);
}

PlayerColor GameState::GetPlayerTurn() {
    return this->playerTurn;
}