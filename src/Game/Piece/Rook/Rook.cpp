//
// Created by Adis Veletanlic on 11/2/23.
//

#include "Rook.h"
#include <iostream>
#include "../../Board/Board.h"

Rook::Rook(sf::Vector2i position, PlayerColor color, PieceType type) : Piece(position, color, type) {
    if (color == PlayerColor::Black) {
        LoadTexture("assets/sprites/Rook-black.png");
    } else {
        LoadTexture("assets/sprites/Rook-white.png");
    }
}

std::vector<Move> Rook::AvailableMoves(const Board& board, const std::optional<Move>& lastMove) const {
    std::vector<Move> moves;
    std::vector<std::pair<int, int>> directions = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };
    for (const auto& dir : directions) {
        std::vector<Move> directionMoves = FindMovesInDirectionForPiece(dir.first, dir.second, board);
        moves.insert(moves.end(), directionMoves.begin(), directionMoves.end());
    }
    return moves;
}

const sf::Texture& Rook::GetTexture() const {
    return texture;
}
