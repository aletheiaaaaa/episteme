#include "datagen.h"

namespace episteme::datagen {
    namespace fs = std::filesystem;

    void play_random(Position& position, int32_t num_moves) {
        MoveList move_list;
        generate_all_moves(move_list, position);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution dist(1, 100);

        if (!move_list.count || num_moves == 0) {
            std::cout << position.to_FEN() << std::endl;
            return;
        };

        Move move{};
        while (!move.data()) {
            int r = dist(gen);
            PieceType random_type = (
                r < 25 ? PieceType::Pawn : r < 40 ? PieceType::Knight : r < 55 ? PieceType::Bishop : r < 70 ? PieceType::Queen: r < 85 ? PieceType::King : PieceType::Rook
            );

            MoveList random_moves;
            for (size_t i = 0; i < move_list.count; i++) {
                position.make_move(move_list.list[i]);
                if (in_check(position, position.STM())) {
                    position.unmake_move();
                    continue;
                }
                position.unmake_move();

                if (piece_type(position.mailbox(move_list.list[i].from_square())) == random_type) {
                    random_moves.add(move_list.list[i]);
                }
            }

            if (random_moves.count) {
                move = random_moves.list[dist(gen) % random_moves.count];
            }
        }

        position.make_move(move);
        play_random(position, num_moves - 1);
    }

    void game_loop(const Parameters& params, std::ostream& stream) {
        Position position;
        position.from_startpos();

        Format formatter{};

        search::Parameters search_params{
            .nodes = params.hard_limit,
            .soft_nodes = params.soft_limit,
            .position = position,
        };

        search::Config cfg{
            .params = search_params,
            .hash_size = params.hash_size,
            .num_threads = params.num_threads,
        };

        search::Instance instance(cfg);

        for (int i = 0; i < (params.num_games / params.num_threads) && !stop; i++) {
            play_random(position, 8);
            formatter.init(position);
            instance.reset_game();

            const search::ScoredMove initial = instance.datagen();
            if (initial.score >= INITIAL_MAX) continue;

            uint64_t win_plies = 0, draw_plies = 0, loss_plies = 0;
            std::optional<double> wdl;

            while (!stop) {
                const search::ScoredMove scored_move = instance.datagen();

                if (!scored_move.move.data()) {
                    wdl = in_check(position, position.STM()) ? position.STM() == Color::Black : 0.5;
                    break;
                } else {
                    if (std::abs(scored_move.score) >= search::MATE - search::MAX_SEARCH_PLY) wdl = scored_move.score > 0;
                    else {
                        if (scored_move.score >= WIN_SCORE_MIN) win_plies++, draw_plies = loss_plies = 0;
                        else if (scored_move.score <= -WIN_SCORE_MIN) loss_plies++, win_plies = draw_plies = 0;
                        else if (std::abs(scored_move.score) <= DRAW_SCORE_MAX && position.half_move_clock() >= 100) draw_plies++, win_plies = loss_plies = 0;

                        if (win_plies >= WIN_PLIES_MIN) wdl = 1.0;
                        else if (loss_plies >= WIN_PLIES_MIN) wdl = 0.0;
                        else if (draw_plies >= DRAW_PLIES_MIN) wdl = 0.5;    
                    }
                }

                position.make_move(scored_move.move);
                search_params.position = position;
                instance.update_params(search_params);

                if (position.is_threefold() || position.is_insufficient()) {
                    wdl = 0.5;
                    break;
                }

                formatter.push(scored_move.move, scored_move.score);
                instance.reset_go();

                if (wdl) break;
            }

            if (wdl) {
                formatter.write(stream, static_cast<uint8_t>(*wdl * 2));
            }

            position.from_startpos();
            instance.reset_game();  
        }
    }

    void run(Parameters& params) {
        std::cout << "Beginning datagen" << std::endl;

        std::signal(SIGINT, []([[maybe_unused]] int signum){stop = true;});

        params.num_games = params.num_threads * (params.num_games / params.num_threads);

        std::vector<std::thread> threads;
        std::vector<std::string> files;

        for (size_t i = 0; i < params.num_threads; i++) {
            const auto file = fs::path(std::format("{}/temp_{}.{}", params.out_dir, i, Format::EXTENSION));
            files.push_back(file);

            threads.emplace_back(
                [params, file = std::move(file), i]() {
                    std::ofstream stream(file, std::ios::binary | std::ios::app);
                    if (!stream) {
                        std::cout << std::format("Failed to open file {} for thread {}", file.string(), i) << std::endl;
                    }

                    game_loop(params, stream);
                    stream.close();

                    if (!stream.good()) {
                        std::cout << std::format("Error encountered on thread {} when closing", i) << std::endl;
                    }
                }
            );
        }

        for (auto& thread : threads) thread.join();
    }
}