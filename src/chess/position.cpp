#include "position.h"

namespace valhalla {

    Position::Position() {
        bitboards.fill(0);
        theMailbox.fill(Piece::None);
        positionHistory.reserve(1024);

        for (int c = 0; c < 2; ++c) {
            allowedCastles.rooks[c].kingside = Square::None;
            allowedCastles.rooks[c].queenside = Square::None;
        }

        stm = false;
        halfClock = 0;
        fullNumber = 0;
        enPassant = Square::None;
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

                    bitboards[pieceTypeIdx(type)] ^= sq;
                    bitboards[colorIdx(color) + COLOR_OFFSET] ^= sq;
                    theMailbox[squareIdx] = piece;
                }
                ++squareIdx;
            }
        }

        stm = (tokens[1] == "w") ? 0 : 1;

        if (tokens[2] != "-") {
            for (char c : tokens[2]) {
                switch (c) {
                    case 'K': allowedCastles.rooks[colorIdx(Color::White)].kingside = Square::H1; break;
                    case 'Q': allowedCastles.rooks[colorIdx(Color::White)].queenside = Square::A1; break;
                    case 'k': allowedCastles.rooks[colorIdx(Color::Black)].kingside = Square::H8; break;
                    case 'q': allowedCastles.rooks[colorIdx(Color::Black)].queenside = Square::A8; break;
                }
            }
        }

        if (tokens[3] != "-") {
            enPassant = static_cast<Square>((tokens[3][0] - 'a') * 8 + (tokens[3][1] - '1'));
        }

        halfClock = std::stoi(tokens[4]);
        fullNumber = std::stoi(tokens[5]);

        PositionState state = {
            .bitboards = bitboards,
            .theMailbox = theMailbox,
            .allowedCastles = allowedCastles,
            .stm = stm,
            .halfClock = halfClock,
            .fullNumber = fullNumber,
            .enPassant = enPassant
        };
        positionHistory.push_back(state);
    }

    std::string Position::toFEN() const {
        std::string fen;
    
        for (int rank = 7; rank >= 0; --rank) {
            int empty = 0;
            for (int file = 0; file < 8; ++file) {
                Piece piece = theMailbox[rank * 8 + file];
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
    
        fen += (stm == static_cast<bool>(Color::White)) ? " w " : " b ";
    
        std::string castling;
        if (allowedCastles.rooks[colorIdx(Color::White)].isKingsideSet()) castling += 'K';
        if (allowedCastles.rooks[colorIdx(Color::White)].isQueensideSet()) castling += 'Q';
        if (allowedCastles.rooks[colorIdx(Color::Black)].isKingsideSet()) castling += 'k';
        if (allowedCastles.rooks[colorIdx(Color::Black)].isQueensideSet()) castling += 'q';
        fen += (castling.empty() ? "-" : castling) + " ";
    
        fen += (enPassant == Square::None ? "-" : Move(enPassant, enPassant).toString().substr(2)) + " ";
    
        fen += std::to_string(halfClock) + " " + std::to_string(fullNumber);
    
        return fen;
    }    

    void Position::fromStartPos() {
        PositionState state;

        bitboards[pieceTypeIdx(PieceType::Pawn)]   = 0x00FF00000000FF00;
        bitboards[pieceTypeIdx(PieceType::Knight)] = 0x4200000000000042;
        bitboards[pieceTypeIdx(PieceType::Bishop)] = 0x2400000000000024;
        bitboards[pieceTypeIdx(PieceType::Rook)]   = 0x8100000000000081;
        bitboards[pieceTypeIdx(PieceType::Queen)]  = 0x0800000000000008;
        bitboards[pieceTypeIdx(PieceType::King)]   = 0x1000000000000010;

        bitboards[colorIdx(Color::White) + COLOR_OFFSET] = 0x000000000000FFFF;
        bitboards[colorIdx(Color::Black) + COLOR_OFFSET] = 0xFFFF000000000000;

        auto setupRank = [&](int rank, Color color) {
            theMailbox[sqIdx(static_cast<Square>(rank * 8 + 0))] = pieceTypeWithColor(PieceType::Rook, color);
            theMailbox[sqIdx(static_cast<Square>(rank * 8 + 1))] = pieceTypeWithColor(PieceType::Knight, color);
            theMailbox[sqIdx(static_cast<Square>(rank * 8 + 2))] = pieceTypeWithColor(PieceType::Bishop, color);
            theMailbox[sqIdx(static_cast<Square>(rank * 8 + 3))] = pieceTypeWithColor(PieceType::Queen, color);
            theMailbox[sqIdx(static_cast<Square>(rank * 8 + 4))] = pieceTypeWithColor(PieceType::King, color);
            theMailbox[sqIdx(static_cast<Square>(rank * 8 + 5))] = pieceTypeWithColor(PieceType::Bishop, color);
            theMailbox[sqIdx(static_cast<Square>(rank * 8 + 6))] = pieceTypeWithColor(PieceType::Knight, color);
            theMailbox[sqIdx(static_cast<Square>(rank * 8 + 7))] = pieceTypeWithColor(PieceType::Rook, color);
        };

        setupRank(0, Color::White);
        setupRank(7, Color::Black);

        for (int file = 0; file < 8; ++file) {
            theMailbox[sqIdx(static_cast<Square>(8 + file))] = Piece::WhitePawn;
            theMailbox[sqIdx(static_cast<Square>(48 + file))] = Piece::BlackPawn;
        }

        halfClock = 0;
        fullNumber = 1;

        allowedCastles.rooks[colorIdx(Color::White)].kingside  = Square::H1;
        allowedCastles.rooks[colorIdx(Color::White)].queenside = Square::A1;
        allowedCastles.rooks[colorIdx(Color::Black)].kingside  = Square::H8;
        allowedCastles.rooks[colorIdx(Color::Black)].queenside = Square::A8;

        state = {
            .bitboards = bitboards,
            .theMailbox = theMailbox,
            .allowedCastles = allowedCastles,
            .stm = stm,
            .halfClock = halfClock,
            .fullNumber = fullNumber,
            .enPassant = enPassant
        };
        positionHistory.push_back(state);
    }

    void Position::makeMove(const Move& move) {
        Piece& src = theMailbox[sqIdx(move.fromSquare())];
        Piece& dst = theMailbox[sqIdx(move.toSquare())];
        uint64_t bbSrc = (uint64_t)1 << sqIdx(move.fromSquare());
        uint64_t bbDst = (uint64_t)1 << sqIdx(move.toSquare());
        Color side = STM();
        auto us = colorIdx(side);
        auto them = colorIdx(flipColor(side));

        enPassant = Square::None;

        if (pieceType(src) == PieceType::Pawn || dst != Piece::None) {
            halfClock = 0;
        } else {
            halfClock++;
        }

        if (side == Color::Black) {
            fullNumber++;
        }

        switch (move.moveType()) {
            case MoveType::Normal: {
                if (dst != Piece::None) {
                    bitboards[pieceTypeIdx(pieceType(dst))] ^= bbDst;
                    bitboards[them + COLOR_OFFSET] ^= bbDst;
                    if (pieceType(dst) == PieceType::Rook) {
                        auto& rooks = allowedCastles.rooks[them];
                        if (move.toSquare() == rooks.kingside) {
                            rooks.unset(true);
                        } else if (move.toSquare() == rooks.queenside) {
                            rooks.unset(false);
                        }
                    }
                }

                if (pieceType(src) == PieceType::King) {
                    auto& rooks = allowedCastles.rooks[us];
                    rooks.clear();
                } else if (pieceType(src) == PieceType::Rook) {
                    auto& rooks = allowedCastles.rooks[us];
                    if (move.fromSquare() == rooks.kingside) {
                        rooks.unset(true);
                    } else if (move.fromSquare() == rooks.queenside) {
                        rooks.unset(false);
                    }
                }

                if (pieceType(src) == PieceType::Pawn &&
                    std::abs(sqIdx(move.fromSquare()) - sqIdx(move.toSquare())) == DOUBLE_PUSH) {
                    int epOffset = (side == Color::White) ? -8 : 8;
                    enPassant = sqFromIdx(sqIdx(move.toSquare()) + epOffset);
                }

                bitboards[pieceTypeIdx(pieceType(src))] ^= bbSrc ^ bbDst;
                bitboards[us + COLOR_OFFSET] ^= bbSrc ^ bbDst;
                dst = src;
                break;
            }

            case MoveType::Castling: {
                bool kingSide = bbDst > bbSrc;
                Square rookSrc = kingSide ? allowedCastles.rooks[us].kingside : allowedCastles.rooks[us].queenside;
                Square rookDst = (side == Color::White)
                    ? (kingSide ? Square::F1 : Square::D1)
                    : (kingSide ? Square::F8 : Square::D8);
                uint64_t bbRookSrc = (uint64_t)1 << sqIdx(rookSrc);
                uint64_t bbRookDst = (uint64_t)1 << sqIdx(rookDst);

                bitboards[pieceTypeIdx(PieceType::Rook)] ^= bbRookSrc ^ bbRookDst;
                bitboards[us + COLOR_OFFSET] ^= bbRookSrc ^ bbRookDst;

                theMailbox[sqIdx(rookSrc)] = Piece::None;
                theMailbox[sqIdx(rookDst)] = pieceTypeWithColor(PieceType::Rook, side);

                dst = src;

                bitboards[pieceTypeIdx(PieceType::King)] ^= bbSrc ^ bbDst;
                bitboards[us + COLOR_OFFSET] ^= bbSrc ^ bbDst;
                allowedCastles.rooks[us].clear();
                break;
            }

            case MoveType::EnPassant: {
                int epOffset = (side == Color::White) ? -8 : 8;
                int captureIdx = sqIdx(move.toSquare()) + epOffset;
                uint64_t bbCap = (uint64_t)1 << captureIdx;

                bitboards[pieceTypeIdx(PieceType::Pawn)] ^= bbSrc ^ bbDst ^ bbCap;
                bitboards[us + COLOR_OFFSET] ^= bbSrc ^ bbDst;
                bitboards[them + COLOR_OFFSET] ^= bbCap;

                theMailbox[captureIdx] = Piece::None;
                dst = src;
                break;
            }

            case MoveType::Promotion: {
                if (dst != Piece::None) {
                    bitboards[pieceTypeIdx(pieceType(dst))] ^= bbDst;
                    bitboards[them + COLOR_OFFSET] ^= bbDst;
                }

                PieceType promo = move.promoPieceType();
                bitboards[pieceTypeIdx(promo)] ^= bbDst;
                bitboards[pieceTypeIdx(PieceType::Pawn)] ^= bbSrc;

                bitboards[us + COLOR_OFFSET] ^= bbSrc ^ bbDst;

                dst = pieceTypeWithColor(promo, side);
                break;
            }
        }

        src = Piece::None;
        stm = !stm;

        positionHistory.emplace_back(PositionState{
            bitboards,
            theMailbox,
            allowedCastles,
            stm,
            halfClock,
            fullNumber,
            enPassant
        });
    }

    void Position::unmakeMove() {
        positionHistory.pop_back();
        const PositionState& prev = positionHistory.back();

        bitboards = prev.bitboards;
        theMailbox = prev.theMailbox;
        allowedCastles = prev.allowedCastles;
        stm = prev.stm;
        halfClock = prev.halfClock;
        fullNumber = prev.fullNumber;
        enPassant = prev.enPassant;
    }

}