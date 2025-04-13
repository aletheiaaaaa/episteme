#pragma once

#include "position.h"
#include <unordered_set>
#include <bit>
#include <random>

namespace valhalla {
    class MoveList {
        public:
            inline void addMove(const Move& move) {
                list[count] = move;
                count++;
            }
        
            inline void clearMoves() {
                count = 0;
            }
        
            [[nodiscard]] inline uint32_t getCount() const {
                return count;
            }
        
            [[nodiscard]] inline const std::array<Move, 256>& getList() const {
                return list;
            }
        private:
        std::array<Move, 256> list;
        uint32_t count = 0;
    };

    struct PawnAttacks {
        uint64_t push1st, push2nd, leftCaptures, rightCaptures;
    };

    constexpr std::array<uint64_t, 64> BISHOP_MAGICS = {
        0x3a00802502504e0,
        0x81c0890900020a,
        0x81c0890900020a,
        0x880602004450158,
        0x880804204004140,
        0x880602004450158,
        0x3a00802502504e0,
        0xa80042100c010,
        0x880602004450158,
        0x3a00802502504e0,
        0x880804204004140,
        0x880602004450158,
        0x81c0890900020a,
        0x3a00802502504e0,
        0x240001902028088,
        0x880804204004140,
        0x880804204004140,
        0x880602004450158,
        0x6062240086001204,
        0x81c0890900020a,
        0x220102200a48,
        0x1a084010048048,
        0x880804204004140,
        0x880804204004140,
        0x880602004450158,
        0x880602004450158,
        0x2020134002042200,
        0x8080200820002,
        0x101008800400a400,
        0x2020134002042200,
        0x880602004450158,
        0x880602004450158,
        0x4402010000a000,
        0x880602004450158,
        0x2000402914c0800,
        0x902020080480080,
        0x1002410041040140,
        0x3a00802502504e0,
        0x880602004450158,
        0x880602004450158,
        0x7710512080d0c,
        0x7710512080d0c,
        0x4020244008000090,
        0x2000402914c0800,
        0x21200380410,
        0x3a00802502504e0,
        0x81c0890900020a,
        0x880602004450158,
        0x3a00802502504e0,
        0x1000c0100130001,
        0x880602004450158,
        0x880602004450158,
        0x1360004002801402,
        0x880804204004140,
        0x3a00802502504e0,
        0x81c0890900020a,
        0x7710512080d0c,
        0x880804204004140,
        0x1360004002801402,
        0x880602004450158,
        0x140000010020010,
        0x880804204004140,
        0x880602004450158,
        0x3a00802502504e0,
    };

    constexpr std::array<uint64_t, 64> ROOK_MAGICS = {
        0x80024000802910,
        0x100100821004040,
        0x80a000a004840008,
        0x2c08000420802008,
        0x220004a108140200,
        0x220004a108140200,
        0x220004a108140200,
        0x1000080644a0100,
        0x491101020804000,
        0x2000402914c0800,
        0x8001201800402260,
        0x100888002330004,
        0x1012482010080080,
        0xa80042100c010,
        0x880804204004140,
        0x8a0800080004229,
        0x2040a0a001001140,
        0x4000600a002000,
        0x880602004450158,
        0x4001248009000,
        0x4001120011300a00,
        0x100888002330004,
        0x5020018004044122,
        0x940801001020,
        0x240010c8400400e0,
        0x240001902028088,
        0x101020005408,
        0x1800022444002100,
        0x101022002004810,
        0x1820080050400,
        0x90104012800a0003,
        0x800804c0026480,
        0x80802010002009a0,
        0x120005644284000,
        0x2040842000411,
        0x2040842000411,
        0x2040842000411,
        0x2040842000411,
        0x100888002330004,
        0x2e508c480c020402,
        0x8002004000860800,
        0x2020134002042200,
        0x5200109200208,
        0x4001248009000,
        0xc222200100202008,
        0x41004004424a0802,
        0x100888002330004,
        0x120400050a80500,
        0x24802290410100,
        0x3a00802502504e0,
        0x420010a11820c420,
        0x880804204004140,
        0x880804204004140,
        0x204000282190220,
        0x21200380410,
        0x404020940082c020,
        0x22100884288220a,
        0x2801209200205642,
        0x104420008208012,
        0x42002824121042,
        0x8001004010059,
        0x1300800cc000101,
        0x10010028105420c,
        0x1000010020c28412,
    };

    [[nodiscard]] std::array<uint64_t, 64> fillKingAttacks();
    [[nodiscard]] std::array<uint64_t, 64> fillKnightAttacks();
    [[nodiscard]] std::array<uint64_t, 64> fillBishopMasks();
    [[nodiscard]] std::array<uint64_t, 64> fillRookMasks();

    [[nodiscard]] uint64_t slowBishopAttacks(Square square, uint64_t blockers);
    [[nodiscard]] uint64_t slowRookAttacks(Square square, uint64_t blockers);
    
    [[nodiscard]] bool isSquareAttacked(Square square, const Position& position, Color stm);

    extern const std::array<uint64_t, 64> KING_ATTACKS;
    extern const std::array<uint64_t, 64> KNIGHT_ATTACKS;

    [[nodiscard]] inline uint64_t getKingAttacks(Square square) {
        return KING_ATTACKS[sqIdx(square)];
    }

    [[nodiscard]] inline uint64_t getKnightAttacks(Square square) {
        return KNIGHT_ATTACKS[sqIdx(square)];
    }

    extern const std::array<uint64_t, 64> ROOK_MASKS;
    extern const std::array<uint64_t, 64> BISHOP_MASKS;

    extern const std::array<std::array<uint64_t, 4096>, 64> ROOK_ATTACKS;
    extern const std::array<std::array<uint64_t, 512>, 64> BISHOP_ATTACKS;

    [[nodiscard]] inline uint64_t getRookAttacks(Square square, const Position& position) {
        size_t sq = sqIdx(square);
        uint64_t blockers = (position.bitboard(colorIdx(Color::White) + position.COLOR_OFFSET) | position.bitboard(colorIdx(Color::Black) + position.COLOR_OFFSET));
        uint64_t relBlockers = (blockers & ROOK_MASKS[sq]);
        uint64_t data = relBlockers * ROOK_MAGICS[sq];
        uint64_t idx = data >> (64 - 12);
        return ROOK_ATTACKS[sq][idx];
    }

    [[nodiscard]] inline uint64_t getBishopAttacks(Square square, const Position& position) {
        size_t sq = sqIdx(square);
        uint64_t blockers = (position.bitboard(colorIdx(Color::White) + position.COLOR_OFFSET) | position.bitboard(colorIdx(Color::Black) + position.COLOR_OFFSET));
        uint64_t relBlockers = (blockers & BISHOP_MASKS[sq]);
        uint64_t data = relBlockers * BISHOP_MAGICS[sq];
        uint64_t idx = data >> (64 - 9);
        return BISHOP_ATTACKS[sq][idx];
    }

    [[nodiscard]] inline uint64_t getQueenAttacks(Square square, const Position& position) {
        return (getBishopAttacks(square, position) | getRookAttacks(square, position));
    }

    extern PawnAttacks getPawnAttacksHelper(const Position& position, Color stm, bool isPseudo);

    [[nodiscard]] inline PawnAttacks getPawnAttacks(const Position& position, Color stm) {
        return getPawnAttacksHelper(position, stm, false);
    }
    
    [[nodiscard]] inline PawnAttacks getPawnPseudoAttacks(const Position& position, Color stm) {
        return getPawnAttacksHelper(position, stm, true);
    }    

    void generatePawnMoves(MoveList &moveList, const Position &position);

    template<PieceType PT, typename F>
    extern void generatePieceMoves(MoveList& moveList, const Position& position, F getAttacks);

    inline void generateKnightMoves(MoveList& moveList, const Position& position) {
        generatePieceMoves<PieceType::Knight>(moveList, position, getKnightAttacks);
    }

    inline void generateBishopMoves(MoveList& moveList, const Position& position) {
        generatePieceMoves<PieceType::Bishop>(moveList, position, getBishopAttacks);
    }

    inline void generateRookMoves(MoveList& moveList, const Position& position) {
        generatePieceMoves<PieceType::Rook>(moveList, position, getRookAttacks);
    }

    inline void generateQueenMoves(MoveList& moveList, const Position& position) {
        generatePieceMoves<PieceType::Queen>(moveList, position, getQueenAttacks);
    }

    inline void generateKingMoves(MoveList& moveList, const Position& position) {
        generatePieceMoves<PieceType::King>(moveList, position, getKingAttacks);
    }

    void generateEnPassant(MoveList &moveList, const Position &position);
    void generateCastles(MoveList &moveList, const Position &position, bool isKingside);

    void generateAllMoves(MoveList &moveList, const Position &position);
}