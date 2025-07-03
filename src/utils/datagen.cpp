#include     "datagen.h"

#include <cassert>

namespace episteme::datagen {
    using namespace std::chrono;

    void play_random(Position& position, int32_t num_moves) {
        for (int i = 0; i < num_moves; i++) {
            MoveList move_list;
            generate_all_moves(move_list, position); 
            move_list.shuffle();

            bool found_legal = false;

            for (size_t j = 0; j < move_list.count; j++) {
                position.make_move(move_list.list[j]);
                if (!in_check(position, position.NTM())) {
                    found_legal = true;
                    break;
                }

                position.unmake_move();
            }

            if (!found_legal) break;
        }
    }

    void game_loop(const Parameters& params, std::ostream& stream, uint32_t id) {
        Position position;
        position.from_startpos();

        Format formatter{};

        search::Parameters search_params{
            .nodes = params.hard_limit,
            .soft_nodes = params.soft_limit,
        };

        search::Config cfg{
            .params = search_params,
            .hash_size = params.hash_size,
            .num_threads = params.num_threads,
        };

        search::Engine engine(cfg);
        time_point start = steady_clock::now();
        size_t positions = 0;
        size_t games = 0;

        for (int i = 0; i < (params.num_games / params.num_threads) && !stop; i++) {
            engine.reset_game();
            play_random(position, 8);

            formatter.init(position);

            search::ScoredMove initial = engine.search(position);
            if (initial.score >= INITIAL_MAX) {
                i--;
                position.from_startpos();
                continue;
            };

            uint64_t win_plies = 0, draw_plies = 0, loss_plies = 0;
            std::optional<double> wdl;

            while (!stop) {
                const search::ScoredMove scored_move = engine.search(position);

                if (!scored_move.move.data()) {
                    wdl = in_check(position, position.STM()) ? position.STM() == Color::Black : 0.5;
                    break;
                }
                // } else {
                //     if (std::abs(scored_move.score) >= search::MATE - search::MAX_SEARCH_PLY) wdl = scored_move.score > 0;
                //     else {
                //         if (scored_move.score >= WIN_SCORE_MIN) win_plies++, draw_plies = loss_plies = 0;
                //         else if (scored_move.score <= -WIN_SCORE_MIN) loss_plies++, win_plies = draw_plies = 0;
                //         else if (std::abs(scored_move.score) <= DRAW_SCORE_MAX && position.half_move_clock() >= 100) draw_plies++, win_plies = loss_plies = 0;

                //         if (win_plies >= WIN_PLIES_MIN) wdl = 1.0;
                //         else if (loss_plies >= WIN_PLIES_MIN) wdl = 0.0;
                //         else if (draw_plies >= DRAW_PLIES_MIN) wdl = 0.5;    
                //     }
                // }

                position.make_move(scored_move.move);

                if (position.is_threefold() || position.is_insufficient()) {
                    wdl = 0.5;
                    break;
                }

                formatter.push(scored_move.move, scored_move.score);
                engine.reset_go();

                if (wdl) break;
            }

            if (wdl) {
                games++;
                positions += formatter.write(stream, static_cast<uint8_t>(*wdl * 2));
            }

            position.from_startpos();

            if ((i + 1) % 10 == 0) {
                time_point end = steady_clock::now();
                int32_t elapsed = duration_cast<milliseconds>(end - start).count() / 1000;
                std::cout << std::format(
                    "Wrote {} positions from {} games on thread {} in {} seconds ({} pos/sec)",
                    positions,
                    games,
                    id,
                    elapsed,
                    positions / (elapsed > 0 ? elapsed : 1)
                ) << std::endl;
                start = steady_clock::now();
                positions = games = 0;
            }
        }
    }

    void run(Parameters& params) {
        std::cout << "Beginning datagen" << std::endl;

        std::signal(SIGINT, []([[maybe_unused]] int signum){
            stop = true;
        });

        params.num_games = params.num_threads * (params.num_games / params.num_threads);

        std::vector<std::thread> threads;
        std::vector<std::string> files;

        for (size_t i = 0; i < params.num_threads; i++) {
            const auto file = std::filesystem::path(std::format("{}/temp_{}.{}", params.out_dir, i, Format::EXTENSION));
            files.push_back(file);

            threads.emplace_back(
                [&params, file = std::move(file), i]() {
                    std::ofstream stream(file, std::ios::binary | std::ios::app);
                    if (!stream) {
                        std::cout << std::format("Failed to open file {} for thread {}", file.string(), i) << std::endl;
                    }

                    game_loop(params, stream, i);

                    if (!stream.good()) {
                        std::cout << std::format("Error encountered on thread {} when closing", i) << std::endl;
                    }
                }
            );
        }

        if (stop.load()) {
            std::cout << "Datagen interrupted" << std::endl;
            return;
        }

        for (auto& thread : threads) thread.join();
        std::cout << "Datagen complete" << std::endl;
    }
}