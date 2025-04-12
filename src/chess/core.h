#pragma once

#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <array>

namespace valhalla {
    enum class Rank : uint16_t {
        Rank1, Rank2, Rank3, Rank4, Rank5, Rank6, Rank7, Rank8, 
        None
    };

    enum class File : uint16_t {
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH, 
        None
    };

    const uint64_t FILE_A = 0x0101010101010101;
    const uint64_t FILE_B = 0x0202020202020202;
    const uint64_t FILE_C = 0x0404040404040404;
    const uint64_t FILE_D = 0x0808080808080808;
    const uint64_t FILE_E = 0x1010101010101010;
    const uint64_t FILE_F = 0x2020202020202020;
    const uint64_t FILE_G = 0x4040404040404040;
    const uint64_t FILE_H = 0x8080808080808080;

    const uint64_t RANK_1 = 0x00000000000000FF;
    const uint64_t RANK_2 = 0x000000000000FF00;
    const uint64_t RANK_3 = 0x0000000000FF0000;
    const uint64_t RANK_4 = 0x00000000FF000000;
    const uint64_t RANK_5 = 0x000000FF00000000;
    const uint64_t RANK_6 = 0x0000FF0000000000;
    const uint64_t RANK_7 = 0x00FF000000000000;
    const uint64_t RANK_8 = 0xFF00000000000000;

    const size_t DOUBLE_PUSH = 16;

    enum class Square : uint16_t {
        A1, B1, C1, D1, E1, F1, G1, H1, 
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,
        None
    };

    enum class Piece : uint16_t {
        WhitePawn,   BlackPawn, 
        WhiteKnight, BlackKnight,
        WhiteBishop, BlackBishop,
        WhiteRook,   BlackRook, 
        WhiteQueen,  BlackQueen,
        WhiteKing,   BlackKing,
        None
    };

    enum class PieceType : uint16_t {
        Pawn, Knight, Bishop, Rook, Queen, King, 
        None
    };

    enum class Color : uint16_t {
        White, Black, 
        None
    };

    enum class BBIndex : uint16_t {
        Pawn, Knight, Bishop, Rook, Queen, King, White, Black,
        None
    };

    struct AllowedCastles {
        class RookPair {
            public:
                Square kingside{Square::None};
                Square queenside{Square::None};

                [[nodiscard]] inline bool isKingsideSet() const {
                    if (kingside != Square::None) {
                        return true;
                    }
                    return false;            
                };

                [[nodiscard]] inline bool isQueensideSet() const {
                    if (queenside != Square::None) {
                        return true;
                    }
                    return false;
                };

                inline void clear() {
                    kingside = Square::None; 
                    queenside = Square::None;            
                };

                inline void unset(bool isKingside) {
                    if (isKingside) {
                        kingside = Square::None;
                    } else {
                        queenside = Square::None;
                    }            
                };
        };
        std::array<RookPair, 2> rooks{};
    };

    static const std::unordered_map<char, std::pair<PieceType, Color>> pieceMap = {
        {'P', {PieceType::Pawn, Color::White}}, {'N', {PieceType::Knight, Color::White}}, 
        {'B', {PieceType::Bishop, Color::White}}, {'R', {PieceType::Rook, Color::White}}, 
        {'Q', {PieceType::Queen, Color::White}}, {'K', {PieceType::King, Color::White}}, 
        {'p', {PieceType::Pawn, Color::Black}}, {'n', {PieceType::Knight, Color::Black}}, 
        {'b', {PieceType::Bishop, Color::Black}}, {'r', {PieceType::Rook, Color::Black}}, 
        {'q', {PieceType::Queen, Color::Black}}, {'k', {PieceType::King, Color::Black}}
    };
    
    [[nodiscard]] inline PieceType pieceType(Piece piece) {
        return static_cast<PieceType>(static_cast<uint16_t>(piece) >> 1);
    };

    [[nodiscard]] inline Color color(Piece piece) {
        return static_cast<Color>(static_cast<uint16_t>(piece) & 0b1);
    };

    [[nodiscard]] inline Color flipColor(Color color) {
        return static_cast<Color>(!static_cast<bool>(color));
    }

    [[nodiscard]] inline Piece pieceTypeWithColor(PieceType pieceType, Color color) {
        return static_cast<Piece>(2 * static_cast<uint16_t>(pieceType) + static_cast<uint16_t>(color));
    }

    [[nodiscard]] inline Square sqFromIdx(size_t index) {
        return static_cast<Square>(index);
    }

    [[nodiscard]] inline uint16_t sqIdx(Square square) {
        return static_cast<uint16_t>(square);
    }

    [[nodiscard]] inline uint16_t pieceTypeIdx(PieceType pieceType) {
        return static_cast<uint16_t>(pieceType);
    }

    [[nodiscard]] inline uint16_t colorIdx(Color color) {
        return static_cast<uint16_t>(color);
    }

    [[nodiscard]] inline uint16_t rankIdx(Rank rank) {
        return static_cast<uint16_t>(rank);
    }

    [[nodiscard]] inline uint16_t fileIdx(File file) {
        return static_cast<uint16_t>(file);
    }

    [[nodiscard]] inline File file(Square square) {
        return static_cast<File>(sqIdx(square) % 8);
    }

    [[nodiscard]] inline Rank rank(Square square) {
        return static_cast<Rank>(sqIdx(square) / 8);
    }

    [[nodiscard]] inline uint64_t shiftWest(uint64_t bitboard) {
        return (bitboard & ~FILE_A) >> 1; 
    }

    [[nodiscard]] inline uint64_t shiftEast(uint64_t bitboard) {
        return (bitboard & ~FILE_H) << 1;
    }

    [[nodiscard]] inline uint64_t shiftNorth(uint64_t bitboard) {
        return (bitboard & ~RANK_8) << 8;
    }

    [[nodiscard]] inline uint64_t shiftSouth(uint64_t bitboard) {
        return (bitboard & ~RANK_1) >> 8;
    }
}