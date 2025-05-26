#include "position.h"

namespace episteme {

    Position::Position() {
        state.bitboard.fill(0);
        state.mailbox.fill(Piece::None);
        positionHistory.reserve(1024);

        for (int c = 0; c < 2; ++c) {
            state.allowedCastles.rooks[c].kingside = Square::None;
            state.allowedCastles.rooks[c].queenside = Square::None;
        }

        state.stm = colorIdx(Color::White);
        state.halfClock = 0;
        state.fullNumber = 0;
        state.enPassant = Square::None;
    }

    void Position::fromFEN(std::string_view FEN) {
        std::array<std::string, 6> tokens;
        size_t i = 0;

        for (auto&& token : FEN | std::views::split(' ')) {
            if (i >= tokens.size()) break;
            tokens[i++] = std::string(token.begin(), token.end());
        }

        size_t squareIdx = 56;
        for (char c : tokens[0]) {
            if (c == '/') {
                squareIdx -= 16;
            } else if (std::isdigit(c)) {
                squareIdx += c - '0';
            } else {
                auto it = pieceMap.find(c);
                if (it != pieceMap.end()) {
                    PieceType type = it->second.first;
                    Color color = it->second.second;
                    Piece piece = pieceTypeWithColor(type, color);
                    uint64_t sq = (uint64_t)1 << squareIdx;

                    state.bitboard[pieceTypeIdx(type)] ^= sq;
                    state.bitboard[colorIdx(color) + COLOR_OFFSET] ^= sq;
                    state.mailbox[squareIdx] = piece;
                }
                ++squareIdx;
            }
        }

        state.stm = (tokens[1] == "w") ? 0 : 1;

        if (tokens[2] != "-") {
            for (char c : tokens[2]) {
                switch (c) {
                    case 'K': state.allowedCastles.rooks[colorIdx(Color::White)].kingside = Square::H1; break;
                    case 'Q': state.allowedCastles.rooks[colorIdx(Color::White)].queenside = Square::A1; break;
                    case 'k': state.allowedCastles.rooks[colorIdx(Color::Black)].kingside = Square::H8; break;
                    case 'q': state.allowedCastles.rooks[colorIdx(Color::Black)].queenside = Square::A8; break;
                }
            }
        }

        if (tokens[3] != "-") {
            state.enPassant = static_cast<Square>((tokens[3][0] - 'a') * 8 + (tokens[3][1] - '1'));
        }

        state.halfClock = std::stoi(tokens[4]);
        state.fullNumber = std::stoi(tokens[5]);

        positionHistory.push_back(state);
    }

    std::string Position::toFEN() const {
        std::string fen;
    
        for (int rank = 7; rank >= 0; --rank) {
            int empty = 0;
            for (int file = 0; file < 8; ++file) {
                Piece piece = state.mailbox[rank * 8 + file];
                if (piece == Piece::None) {
                    ++empty;
                } else {
                    if (empty != 0) {
                        fen += std::to_string(empty);
                        empty = 0;
                    }
                    char c;
                    PieceType pt = pieceType(piece);
                    Color col = color(piece);
                    switch (pt) {
                        case PieceType::Pawn:   c = 'p'; break;
                        case PieceType::Knight: c = 'n'; break;
                        case PieceType::Bishop: c = 'b'; break;
                        case PieceType::Rook:   c = 'r'; break;
                        case PieceType::Queen:  c = 'q'; break;
                        case PieceType::King:   c = 'k'; break;
                        default: c = '?';
                    }
                    fen += (col == Color::White) ? std::toupper(c) : c;
                }
            }
            if (empty != 0)
                fen += std::to_string(empty);
            if (rank != 0)
                fen += '/';
        }
    
        fen += (state.stm == static_cast<bool>(Color::White)) ? " w " : " b ";
    
        std::string castling;
        if (state.allowedCastles.rooks[colorIdx(Color::White)].isKingsideSet()) castling += 'K';
        if (state.allowedCastles.rooks[colorIdx(Color::White)].isQueensideSet()) castling += 'Q';
        if (state.allowedCastles.rooks[colorIdx(Color::Black)].isKingsideSet()) castling += 'k';
        if (state.allowedCastles.rooks[colorIdx(Color::Black)].isQueensideSet()) castling += 'q';
        fen += (castling.empty() ? "-" : castling) + " ";
    
        fen += (state.enPassant == Square::None ? "-" : Move(state.enPassant, state.enPassant).toString().substr(2)) + " ";
    
        fen += std::to_string(state.halfClock) + " " + std::to_string(state.fullNumber);
    
        return fen;
    }    

    void Position::fromStartPos() {
        state.bitboard[pieceTypeIdx(PieceType::Pawn)]   = 0x00FF00000000FF00;
        state.bitboard[pieceTypeIdx(PieceType::Knight)] = 0x4200000000000042;
        state.bitboard[pieceTypeIdx(PieceType::Bishop)] = 0x2400000000000024;
        state.bitboard[pieceTypeIdx(PieceType::Rook)]   = 0x8100000000000081;
        state.bitboard[pieceTypeIdx(PieceType::Queen)]  = 0x0800000000000008;
        state.bitboard[pieceTypeIdx(PieceType::King)]   = 0x1000000000000010;

        state.bitboard[colorIdx(Color::White) + COLOR_OFFSET] = 0x000000000000FFFF;
        state.bitboard[colorIdx(Color::Black) + COLOR_OFFSET] = 0xFFFF000000000000;

        auto setupRank = [&](int rank, Color color) {
            state.mailbox[sqIdx(static_cast<Square>(rank * 8 + 0))] = pieceTypeWithColor(PieceType::Rook, color);
            state.mailbox[sqIdx(static_cast<Square>(rank * 8 + 1))] = pieceTypeWithColor(PieceType::Knight, color);
            state.mailbox[sqIdx(static_cast<Square>(rank * 8 + 2))] = pieceTypeWithColor(PieceType::Bishop, color);
            state.mailbox[sqIdx(static_cast<Square>(rank * 8 + 3))] = pieceTypeWithColor(PieceType::Queen, color);
            state.mailbox[sqIdx(static_cast<Square>(rank * 8 + 4))] = pieceTypeWithColor(PieceType::King, color);
            state.mailbox[sqIdx(static_cast<Square>(rank * 8 + 5))] = pieceTypeWithColor(PieceType::Bishop, color);
            state.mailbox[sqIdx(static_cast<Square>(rank * 8 + 6))] = pieceTypeWithColor(PieceType::Knight, color);
            state.mailbox[sqIdx(static_cast<Square>(rank * 8 + 7))] = pieceTypeWithColor(PieceType::Rook, color);
        };

        setupRank(0, Color::White);
        setupRank(7, Color::Black);

        for (int file = 0; file < 8; ++file) {
            state.mailbox[sqIdx(static_cast<Square>(8 + file))] = Piece::WhitePawn;
            state.mailbox[sqIdx(static_cast<Square>(48 + file))] = Piece::BlackPawn;
        }

        state.halfClock = 0;
        state.fullNumber = 1;

        state.allowedCastles.rooks[colorIdx(Color::White)].kingside  = Square::H1;
        state.allowedCastles.rooks[colorIdx(Color::White)].queenside = Square::A1;
        state.allowedCastles.rooks[colorIdx(Color::Black)].kingside  = Square::H8;
        state.allowedCastles.rooks[colorIdx(Color::Black)].queenside = Square::A8;

        positionHistory.push_back(state);
    }

    void Position::makeMove(const Move& move) {
        Piece& src = state.mailbox[sqIdx(move.fromSquare())];
        Piece& dst = state.mailbox[sqIdx(move.toSquare())];
        uint64_t bbSrc = (uint64_t)1 << sqIdx(move.fromSquare());
        uint64_t bbDst = (uint64_t)1 << sqIdx(move.toSquare());
        Color side = STM();
        auto us = colorIdx(side);
        auto them = colorIdx(flipColor(side));

        state.enPassant = Square::None;

        if (pieceType(src) == PieceType::Pawn || dst != Piece::None) {
            state.halfClock = 0;
        } else {
            state.halfClock++;
        }

        if (side == Color::Black) {
            state.fullNumber++;
        }

        switch (move.moveType()) {
            case MoveType::Normal: {
                if (dst != Piece::None) {
                    state.bitboard[pieceTypeIdx(pieceType(dst))] ^= bbDst;
                    state.bitboard[them + COLOR_OFFSET] ^= bbDst;
                    if (pieceType(dst) == PieceType::Rook) {
                        auto& rooks = state.allowedCastles.rooks[them];
                        if (move.toSquare() == rooks.kingside) {
                            rooks.unset(true);
                        } else if (move.toSquare() == rooks.queenside) {
                            rooks.unset(false);
                        }
                    }
                }

                if (pieceType(src) == PieceType::King) {
                    auto& rooks = state.allowedCastles.rooks[us];
                    rooks.clear();
                } else if (pieceType(src) == PieceType::Rook) {
                    auto& rooks = state.allowedCastles.rooks[us];
                    if (move.fromSquare() == rooks.kingside) {
                        rooks.unset(true);
                    } else if (move.fromSquare() == rooks.queenside) {
                        rooks.unset(false);
                    }
                }

                if (pieceType(src) == PieceType::Pawn &&
                    std::abs(sqIdx(move.fromSquare()) - sqIdx(move.toSquare())) == DOUBLE_PUSH) {
                    int epOffset = (side == Color::White) ? -8 : 8;
                    state.enPassant = sqFromIdx(sqIdx(move.toSquare()) + epOffset);
                }

                state.bitboard[pieceTypeIdx(pieceType(src))] ^= bbSrc ^ bbDst;
                state.bitboard[us + COLOR_OFFSET] ^= bbSrc ^ bbDst;
                dst = src;
                break;
            }

            case MoveType::Castling: {
                bool kingSide = bbDst > bbSrc;
                Square rookSrc = kingSide ? state.allowedCastles.rooks[us].kingside : state.allowedCastles.rooks[us].queenside;
                Square rookDst = (side == Color::White)
                    ? (kingSide ? Square::F1 : Square::D1)
                    : (kingSide ? Square::F8 : Square::D8);
                uint64_t bbRookSrc = (uint64_t)1 << sqIdx(rookSrc);
                uint64_t bbRookDst = (uint64_t)1 << sqIdx(rookDst);

                state.bitboard[pieceTypeIdx(PieceType::Rook)] ^= bbRookSrc ^ bbRookDst;
                state.bitboard[us + COLOR_OFFSET] ^= bbRookSrc ^ bbRookDst;

                state.mailbox[sqIdx(rookSrc)] = Piece::None;
                state.mailbox[sqIdx(rookDst)] = pieceTypeWithColor(PieceType::Rook, side);

                dst = src;

                state.bitboard[pieceTypeIdx(PieceType::King)] ^= bbSrc ^ bbDst;
                state.bitboard[us + COLOR_OFFSET] ^= bbSrc ^ bbDst;
                state.allowedCastles.rooks[us].clear();
                break;
            }

            case MoveType::EnPassant: {
                int epOffset = (side == Color::White) ? -8 : 8;
                int captureIdx = sqIdx(move.toSquare()) + epOffset;
                uint64_t bbCap = (uint64_t)1 << captureIdx;

                state.bitboard[pieceTypeIdx(PieceType::Pawn)] ^= bbSrc ^ bbDst ^ bbCap;
                state.bitboard[us + COLOR_OFFSET] ^= bbSrc ^ bbDst;
                state.bitboard[them + COLOR_OFFSET] ^= bbCap;

                state.mailbox[captureIdx] = Piece::None;
                dst = src;
                break;
            }

            case MoveType::Promotion: {
                if (dst != Piece::None) {
                    state.bitboard[pieceTypeIdx(pieceType(dst))] ^= bbDst;
                    state.bitboard[them + COLOR_OFFSET] ^= bbDst;
                }

                PieceType promo = move.promoPieceType();
                state.bitboard[pieceTypeIdx(promo)] ^= bbDst;
                state.bitboard[pieceTypeIdx(PieceType::Pawn)] ^= bbSrc;

                state.bitboard[us + COLOR_OFFSET] ^= bbSrc ^ bbDst;

                dst = pieceTypeWithColor(promo, side);
                break;
            }
        }

        src = Piece::None;
        state.stm = !state.stm;

        positionHistory.emplace_back(state);
    }

    void Position::unmakeMove() {
        positionHistory.pop_back();
        const PositionState& prev = positionHistory.back();
        state = prev;
    }

    Move fromUCI(const Position& position, const std::string& move) {
        std::string srcStr = move.substr(0, 2);
        std::string dstStr = move.substr(2, 2);

        auto str2Sq = [](std::string square) {
            int file = square[0] - 'a';
            int rank = square[1] - '1';
            return static_cast<Square>(rank * 8 + file);
        };

        Square src = str2Sq(srcStr);
        Square dst = str2Sq(dstStr);

        auto isCastling = [&]() {
            Color stm = position.STM();
            if (pieceType(position.mailbox(sqIdx(src))) != PieceType::King) return false;
        
            Square kingside = (stm == Color::White) ? Square::G1 : Square::G8;
            Square queenside = (stm == Color::White) ? Square::C1 : Square::C8;
        
            bool kingsideCastle = (dst == kingside) && position.castlingRights(stm).isKingsideSet();
            bool queensideCastle = (dst == queenside) && position.castlingRights(stm).isQueensideSet();
        
            return (kingsideCastle || queensideCastle);
        };

        bool isPromo = (move.length() == 5);
        bool isEnPassant = (pieceType(position.mailbox(sqIdx(src))) == PieceType::Pawn) && (dst == position.epSquare());

        auto char2Piece = [&](char promo) {
            switch (promo) {
                case ('q'): return PromoPiece::Queen;
                case ('r'): return PromoPiece::Rook;
                case ('b'): return PromoPiece::Bishop;
                case ('n'): return PromoPiece::Knight;
                default: break;
            }
        };

        if (isCastling()) return Move(src, dst, MoveType::Castling);
        else if (isEnPassant) return Move(src, dst, MoveType::EnPassant);
        else if (isPromo) return Move(src, dst, MoveType::Promotion, char2Piece(move.at(4)));
        else return Move(src, dst);
    }
}