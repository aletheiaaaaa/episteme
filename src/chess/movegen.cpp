#include "movegen.h"
#include <bit>
#include <random>
#include <iostream>
#include <cassert>

namespace valhalla {
    std::array<uint64_t, 64> fillKingAttacks() {
        std::array<uint64_t, 64> kingAttacks;
        kingAttacks.fill(0);
        for (int i = 0; i < 64; i++) {
            uint64_t square = (uint64_t)1 << i;
            uint64_t pattern = shiftNorth(square) | shiftNorth(shiftEast(square)) 
                | shiftEast(square) | shiftSouth(shiftEast(square)) 
                | shiftSouth(square)  | shiftSouth(shiftWest(square)) 
                | shiftWest(square)  | shiftNorth(shiftWest(square));
            kingAttacks[i] = pattern;
        }
        return kingAttacks;
    }

    std::array<uint64_t, 64> fillKnightAttacks() {
        std::array<uint64_t, 64> knightAttacks;
        knightAttacks.fill(0);
        for (int i = 0; i < 64; i++) {
            uint64_t square = (uint64_t)1 << i;
            uint64_t pattern = shiftWest(shiftNorth(shiftNorth(square))) | shiftEast(shiftNorth(shiftNorth(square)))
                | shiftNorth(shiftEast(shiftEast(square)))  | shiftSouth(shiftEast(shiftEast(square)))
                | shiftEast(shiftSouth(shiftSouth(square))) | shiftWest(shiftSouth(shiftSouth(square)))
                | shiftSouth(shiftWest(shiftWest(square)))  | shiftNorth(shiftWest(shiftWest(square)));
            knightAttacks[i] = pattern;
        }
        return knightAttacks;
    }

    const std::array<uint64_t, 64> KING_ATTACKS = fillKingAttacks();
    const std::array<uint64_t, 64> KNIGHT_ATTACKS = fillKnightAttacks();

    std::array<uint64_t, 64> fillRookMasks() {
        std::array<uint64_t, 64> rookMasks;
        rookMasks.fill(0);
        for (int i = 0; i < 64; i++) {
            uint64_t square = 0;
            square |= ((uint64_t)0x7E << 8 * (i / 8)) | ((uint64_t)0x1010101010100 << (i % 8));
            square &= ~((uint64_t)1 << i);
            rookMasks[i] = square;
        }
        return rookMasks;
    }

    std::array<uint64_t, 64> fillBishopMasks() {
        std::array<uint64_t, 64> bishopMasks;
        bishopMasks.fill(0); 
        for (int i = 0; i < 64; i++) {
            uint64_t diagonal = 0x8040201008040201;
            uint64_t antiDiagonal = 0x0102040810204080;
            uint64_t square = 0;
            int shift = (i / 8) - (i % 8);
            int antiShift = (i % 8) - (7 - (i / 8));
            diagonal = (shift > 0) ? (diagonal << (shift * 8)) : (diagonal >> -(shift * 8));
            antiDiagonal = (antiShift > 0) ? (antiDiagonal << (antiShift * 8)) : (antiDiagonal >> -(antiShift * 8));
            square |= (diagonal | antiDiagonal);
            square &= ~((uint64_t)1 << i);
            bishopMasks[i] = square & ~(0xFF818181818181FF);
        }
        return bishopMasks;
    }

    const std::array<uint64_t, 64> ROOK_MASKS = fillRookMasks();
    const std::array<uint64_t, 64> BISHOP_MASKS = fillBishopMasks();

    uint64_t slowRookAttacks(Square square, uint64_t blockers) {
        uint64_t rookAttacks = 0;
        size_t sq = sqIdx(square);
        uint64_t sqBB = (uint64_t)1 << sq;

        auto generateRay = [sqBB, &rookAttacks, &blockers](auto shiftDir) {
            uint64_t attackBB = sqBB;
            do {
                attackBB = shiftDir(attackBB);
                rookAttacks |= attackBB;
            } while (attackBB && !(attackBB & blockers));
        };

        generateRay(shiftNorth);
        generateRay(shiftEast);
        generateRay(shiftSouth);
        generateRay(shiftWest);

        return rookAttacks;
    }
    
    uint64_t slowBishopAttacks(Square square, uint64_t blockers) {
        uint64_t bishopAttacks = 0;
        size_t sq = sqIdx(square);
        uint64_t sqBB = (uint64_t)1 << sq;

        auto generateRay = [sqBB, &bishopAttacks, &blockers](auto shiftDir1, auto shiftDir2) {
            uint64_t attackBB = sqBB;
            do {
                attackBB = shiftDir2(shiftDir1(attackBB));
                bishopAttacks |= attackBB;
            } while (attackBB && !(attackBB & blockers));
        };

        generateRay(shiftNorth, shiftEast);
        generateRay(shiftSouth, shiftEast);
        generateRay(shiftSouth, shiftWest);
        generateRay(shiftNorth, shiftWest);

        return bishopAttacks;
    }
        
    template<size_t NUM_BITS, typename F>
    std::array<uint64_t, (1 << NUM_BITS)> fillSqAttack (Square square, std::array<uint64_t, 64> MASKS, F slowAttacks) {
        constexpr size_t ARR_SIZE = 1 << NUM_BITS;
        std::array<uint64_t, ARR_SIZE> attacks; 
        uint64_t mask = MASKS[sqIdx(square)];

        uint64_t submask = 0;
        size_t numMoves = 0;
        do {
            attacks[numMoves] = slowAttacks(square, submask);
            submask = (submask - mask) & mask;
            numMoves++;
        } while (submask);

        while (numMoves < ARR_SIZE) {
            attacks[numMoves] = 0;
            numMoves++; 
        }

        return attacks;
    }

    template<size_t NUM_BITS, typename F>
    std::pair<uint64_t, std::array<uint64_t, 1 << NUM_BITS>> findMagics(Square square, std::array<uint64_t, 64> MASKS, F slowAttacks) {
        constexpr size_t ARR_SIZE = 1 << NUM_BITS;
        auto attacks = fillSqAttack<NUM_BITS>(square, MASKS, slowAttacks);
        std::array<uint64_t, ARR_SIZE> submasks;
        std::array<uint64_t, ARR_SIZE> usedIndices;
        uint64_t mask = MASKS[sqIdx(square)];

        uint64_t submask = 0;
        int numMoves = 0;
        do {
            submasks[numMoves] = submask;
            submask = (submask - mask) & mask;
            numMoves++;
        } while (submask);

        std::mt19937 gen(42);
        std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
        bool fail;
        uint64_t magic;
        do {
            usedIndices.fill(0);
            magic = dist(gen) & dist(gen) & dist(gen);
            fail = false;
            for (int i = 0; (!fail) && (i < numMoves); i++) {
                const auto magicIdx = (submasks[i] * magic) >> (64 - NUM_BITS);
                if (usedIndices[magicIdx] == 0) {
                    usedIndices[magicIdx] = attacks[i];
                } else if (usedIndices[magicIdx] != attacks[i]) {
                    fail = true;
                }
            }
        } while (fail);

        return {magic, usedIndices};
    }

    template<size_t NUM_BITS, typename F>
    std::array<std::array<uint64_t, (1 << NUM_BITS)>, 64> fillAttacks(const std::array<uint64_t, 64>& MAGICS, const std::array<uint64_t, 64>& MASKS, F slowAttacks) {
        constexpr size_t ARR_SIZE = 1 << NUM_BITS;
        std::array<std::array<uint64_t, ARR_SIZE>, 64> attackTables{};
    
        for (int sq = 0; sq < 64; ++sq) {
            const uint64_t mask = MASKS[sq];
            std::array<uint64_t, ARR_SIZE> table{};
            uint64_t submask = 0;
            do {
                const uint64_t index = (submask * MAGICS[sq]) >> (64 - NUM_BITS);
                table[index] = slowAttacks(sqFromIdx(sq), submask);
                submask = (submask - mask) & mask;
            } while (submask);
            attackTables[sq] = table;
        }
    
        return attackTables;
    }
    
    std::array<std::array<uint64_t, 4096>, 64> fillRookAttacks() {
        return fillAttacks<12>(ROOK_MAGICS, ROOK_MASKS, slowRookAttacks);
    }

    std::array<std::array<uint64_t, 512>, 64> fillBishopAttacks() {
        return fillAttacks<9>(BISHOP_MAGICS, BISHOP_MASKS, slowBishopAttacks);
    }

    const std::array<std::array<uint64_t, 4096>, 64> ROOK_ATTACKS = fillRookAttacks();
    const std::array<std::array<uint64_t, 512>, 64> BISHOP_ATTACKS = fillBishopAttacks();

    PawnAttacks getPawnAttacksHelper(const Position& position, Color stm, bool isPseudo) {
        uint64_t usBB = position.bitboard(colorIdx(stm) + position.COLOR_OFFSET);
        uint64_t themBB = position.bitboard(colorIdx(flipColor(stm)) + position.COLOR_OFFSET);
        uint64_t pawnBB = position.bitboard(pieceTypeIdx(PieceType::Pawn)) & usBB;
        
        uint64_t occupied = usBB | (isPseudo ? 0ULL : themBB);  // only filter themBB if not pseudo
        uint64_t capturesMask = isPseudo ? ~0ULL : themBB;
    
        PawnAttacks attacks;
    
        if (stm == Color::White) {
            attacks.push1st = (pawnBB << 8) & ~occupied;
            attacks.push2nd = ((attacks.push1st & RANK_3) << 8) & ~occupied;
            attacks.leftCaptures = ((pawnBB & ~FILE_A) << 7) & capturesMask;
            attacks.rightCaptures = ((pawnBB & ~FILE_H) << 9) & capturesMask;
        } else {
            attacks.push1st = (pawnBB >> 8) & ~occupied;
            attacks.push2nd = ((attacks.push1st & RANK_6) >> 8) & ~occupied;
            attacks.leftCaptures = ((pawnBB & ~FILE_A) >> 9) & capturesMask;
            attacks.rightCaptures = ((pawnBB & ~FILE_H) >> 7) & capturesMask;
        }
    
        return attacks;
    }

    bool isSquareAttacked(Square square, const Position& position, Color nstm) {
        uint64_t squareBB = uint64_t(1) << sqIdx(square);
    
        uint64_t themBB = position.bitboard(colorIdx(nstm) + position.COLOR_OFFSET);
        
        uint64_t knights = position.bitboard(pieceTypeIdx(PieceType::Knight));
        uint64_t knightAttacks = getKnightAttacks(square) & knights & themBB;

        uint64_t queens = position.bitboard(pieceTypeIdx(PieceType::Queen));
        uint64_t bishopsAndQueens = position.bitboard(pieceTypeIdx(PieceType::Bishop)) | queens;
        uint64_t bishopAttacks = getBishopAttacks(square, position) & bishopsAndQueens & themBB;
        uint64_t rooksAndQueens = position.bitboard(pieceTypeIdx(PieceType::Rook)) | queens;
        uint64_t rookAttacks = getRookAttacks(square, position) & rooksAndQueens & themBB;
        
        uint64_t kings = position.bitboard(pieceTypeIdx(PieceType::King));
        uint64_t kingAttacks = getKingAttacks(square) & kings & themBB;
    
        if ((knightAttacks | bishopAttacks | rookAttacks | kingAttacks) != 0) {
            return true;
        }
    
        PawnAttacks pawnAttacks = getPawnPseudoAttacks(position, nstm);
        if (((pawnAttacks.leftCaptures | pawnAttacks.rightCaptures) & squareBB) != 0) {
            return true;
        }

        return false;
    }
    
    template<PieceType PT, typename F>
    void generatePieceMoves(MoveList& moveList, const Position& position, F getAttacks) {
        uint64_t usBB = position.bitboard(colorIdx(position.STM()) + position.COLOR_OFFSET);
        uint64_t pieceBB = position.bitboard(pieceTypeIdx(PT)) & usBB;
        while (pieceBB != 0) {
            Square fromSq = sqFromIdx(std::countr_zero(pieceBB));
            uint64_t attacksBB;
            if constexpr (std::is_invocable_r<uint64_t, F, Square, const Position&>::value) {
                attacksBB = getAttacks(fromSq, position) & ~usBB;
            } else {
                attacksBB = getAttacks(fromSq) & ~usBB;
            }    
            while (attacksBB != 0) {
                Square toSq = sqFromIdx(std::countr_zero(attacksBB));
                moveList.addMove({fromSq, toSq});
                attacksBB &= attacksBB - 1;
            }
            pieceBB &= pieceBB - 1;
        }
    }

    void generatePawnMoves(MoveList& moveList, const Position& position) {
        Color stm = position.STM();
        PawnAttacks attacks = getPawnAttacks(position, stm);
        uint64_t promoRank = (stm == Color::White) ? (RANK_8) : (RANK_1);
        constexpr std::array<size_t, 2> leftCaptureShift{ 7, 9 };
        constexpr std::array<size_t, 2> rightCaptureShift{ 9, 7 };
        constexpr size_t push1stShift = 8;
        constexpr size_t push2ndShift = 16;

        auto getFromSq = [stm](uint64_t attacksBB, size_t shift) {
            if (stm == Color::White) {
                return sqFromIdx(std::countr_zero(attacksBB) - shift);
            } else {
                return sqFromIdx(std::countr_zero(attacksBB) + shift);
            }
        };

        auto addMove = [promoRank, &moveList](Square fromSq, Square toSq) {
            if ((((uint64_t)1 << sqIdx(toSq)) & promoRank) != 0) {
                for (auto promoPiece : {PromoPiece::Knight, PromoPiece::Bishop, PromoPiece::Rook, PromoPiece::Queen}) {
                    moveList.addMove({fromSq, toSq, MoveType::Promotion, promoPiece});
                }
            } else {
                moveList.addMove({fromSq, toSq});
            };
        };

        auto attacks2Moves = [&](uint64_t attacksBB, size_t shift) {
            while (attacksBB != 0) {
                Square fromSq = getFromSq(attacksBB, shift);
                Square toSq = sqFromIdx(std::countr_zero(attacksBB));
                addMove(fromSq, toSq);
                attacksBB &= attacksBB - 1;
            }
        };

        attacks2Moves(attacks.push1st, push1stShift);
        attacks2Moves(attacks.push2nd, push2ndShift);
        attacks2Moves(attacks.leftCaptures, leftCaptureShift[colorIdx(stm)]);
        attacks2Moves(attacks.rightCaptures, rightCaptureShift[colorIdx(stm)]);
    }

    void generateEnPassant(MoveList& moveList, const Position& position) {
        Square epSq = position.epSquare();
        Color stm = position.STM();
        uint64_t epBB = (uint64_t)1 << sqIdx(epSq);
        uint64_t pawnBB = position.bitboard(pieceTypeIdx(PieceType::Pawn)) & position.bitboard(colorIdx(stm) + position.COLOR_OFFSET);

        uint64_t leftAttacks = (stm == Color::White) ? (((epBB & ~FILE_A) >> 9) & pawnBB) : (((epBB & ~FILE_A) << 7) & pawnBB);
        uint64_t rightAttacks = (stm == Color::White) ? (((epBB & ~FILE_H) >> 7) & pawnBB) : (((epBB & ~FILE_H) << 9) & pawnBB);

        if (leftAttacks != 0) {
            Square fromSq = sqFromIdx(std::countr_zero(leftAttacks));
            moveList.addMove({fromSq, epSq, MoveType::EnPassant});
        }
        if (rightAttacks != 0) {
            Square fromSq = sqFromIdx(std::countr_zero(rightAttacks));
            moveList.addMove({fromSq, epSq, MoveType::EnPassant});
        }
    }

    void generateCastles(MoveList& moveList, const Position& position, bool isKingside) {
        Color stm = position.STM();
        uint64_t kingBB = position.bitboard(pieceTypeIdx(PieceType::King)) & position.bitboard(colorIdx(stm) + position.COLOR_OFFSET);
        constexpr std::array<std::array<Square, 2>, 2> kingEnds = {{
            {Square::C1, Square::G1},
            {Square::C8, Square::G8}
        }};

        Square kingSrc = sqFromIdx(std::countr_zero(kingBB));
        Square kingDst = kingEnds[colorIdx(stm)][isKingside];

        Square rookSrc = isKingside ? position.castlingRights().rooks[colorIdx(stm)].kingside : position.castlingRights().rooks[colorIdx(stm)].queenside;

        if (isSquareAttacked(kingSrc, position, position.nSTM())) {
            return;
        }

        size_t kingStart = std::min(sqIdx(kingSrc), sqIdx(kingDst));
        size_t kingEnd = std::max(sqIdx(kingSrc), sqIdx(kingDst));

        size_t start = std::min(sqIdx(rookSrc), sqIdx(kingSrc));
        size_t end = std::max(sqIdx(rookSrc), sqIdx(kingSrc));

        for (size_t sq = (start + 1); sq <= (end - 1); sq++) {
            Piece piece = position.mailbox()[sq];
            if (piece != Piece::None) {
                return;
            }
        }

        for (size_t sq = kingStart; sq <= kingEnd; sq++) {
            if ((sq != sqIdx(kingSrc)) && isSquareAttacked(sqFromIdx(sq), position, position.nSTM())) {
                return;
            }
        }

        moveList.addMove({kingSrc, kingDst, MoveType::Castling});
    }

    void generateAllMoves(MoveList& moveList, const Position& position) {

        generatePawnMoves(moveList, position);
        generateKnightMoves(moveList, position);
        generateBishopMoves(moveList, position);
        generateRookMoves(moveList, position);
        generateQueenMoves(moveList, position);
        generateKingMoves(moveList, position);

        if (position.epSquare() != Square::None) {
            generateEnPassant(moveList, position);
        }

        Square kingsideCastle = position.castlingRights().rooks[colorIdx(position.STM())].kingside;
        Square queensideCastle = position.castlingRights().rooks[colorIdx(position.STM())].queenside;

        if (kingsideCastle != Square::None) {
            generateCastles(moveList, position, true);
        }
        if (queensideCastle != Square::None) {
            generateCastles(moveList, position, false);
        }
    }
}