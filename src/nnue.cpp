#include "nnue.h"

namespace episteme::nn {

    Accumulator NNUE::reset_accumulator(const std::array<Piece, 64>& mailbox) const {
        Accumulator out;

        for (uint8_t i = 0; i < 64; i++) {
            int stm = piecesquare(mailbox[i], sq_from_idx(i), false);
            int ntm = piecesquare(mailbox[i], sq_from_idx(i ^ 56), true);

            if (stm != -1) {
                for (int j = 0; j < 1024; j += 64) {
                    auto partial = [&](int offset) {
                        __m256i vec = _mm256_load_si256(reinterpret_cast<const __m256i*>(&out.stm[j + offset]));
                        __m256i l0w = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l0_weights[stm][j + offset]));
                        _mm256_storeu_si256((reinterpret_cast<__m256i*>(&out.stm[j + offset])) ,_mm256_add_epi16(vec, l0w));    
                    };

                    partial(0);
                    partial(16);
                    partial(32);
                    partial(48);
                }
            }

            if (ntm != -1) {
                for (int j = 0; j < 1024; j += 64) {
                    auto partial = [&](int offset) {
                        __m256i vec = _mm256_load_si256(reinterpret_cast<const __m256i*>(&out.ntm[j + offset]));
                        __m256i l0w = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l0_weights[ntm][j + offset]));
                        _mm256_storeu_si256((reinterpret_cast<__m256i*>(&out.ntm[j + offset])) ,_mm256_add_epi16(vec, l0w));    
                    };

                    partial(0);
                    partial(16);
                    partial(32);
                    partial(48);
                }
            }
        }

        for (int i = 0; i < 1024; i += 64) {
            auto partial = [&](int offset) {
                __m256i stm = _mm256_load_si256(reinterpret_cast<const __m256i*>(&out.stm[i + offset]));
                __m256i ntm = _mm256_load_si256(reinterpret_cast<const __m256i*>(&out.ntm[i + offset]));
                __m256i l0b = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l0_biases[i + offset]));

                _mm256_storeu_si256((reinterpret_cast<__m256i*>(&out.stm[i + offset])), _mm256_add_epi16(stm, l0b));
                _mm256_storeu_si256((reinterpret_cast<__m256i*>(&out.ntm[i + offset])), _mm256_add_epi16(ntm, l0b));
            };


            partial(0);
            partial(16);
            partial(32);
            partial(48);
        }

        return out;
    }

    int32_t NNUE::l1_forward(const Accumulator& accum) const {
        __m256i temp_0 = _mm256_setzero_si256();
        __m256i temp_1 = _mm256_setzero_si256();
        __m256i temp_2 = _mm256_setzero_si256();
        __m256i temp_3 = _mm256_setzero_si256();

        for (int i = 0; i < 1024; i += 64) {
            auto partial = [&](__m256i temp, int offset) {
                __m256i stm_pre = _mm256_load_si256(reinterpret_cast<const __m256i*>(&accum.stm[i + offset]));
                __m256i ntm_pre = _mm256_load_si256(reinterpret_cast<const __m256i*>(&accum.ntm[i + offset]));
                __m256i l1w0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l1_weights[0][i + offset]));
                __m256i l1w1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l1_weights[1][i + offset]));

                __m256i stm = _mm256_min_epi16(_mm256_max_epi16(stm_pre, _mm256_setzero_si256()), _mm256_set1_epi16(255));
                __m256i ntm = _mm256_min_epi16(_mm256_max_epi16(ntm_pre, _mm256_setzero_si256()), _mm256_set1_epi16(255));

                __m256i temp_stm = _mm256_mullo_epi16(stm, l1w0);
                __m256i temp_ntm = _mm256_mullo_epi16(ntm, l1w1);
    
                temp += _mm256_madd_epi16(stm, temp_stm); 
                temp += _mm256_madd_epi16(ntm, temp_ntm);    

                return temp;
            };

            temp_0 = partial(temp_0, 0);
            temp_1 = partial(temp_1, 16);
            temp_2 = partial(temp_2, 32);
            temp_3 = partial(temp_3, 48);
        }

        auto hadd = [&](__m256i temp) {
            __m128i temp_lo = _mm256_castsi256_si128(temp); 
            __m128i temp_hi = _mm256_extracti128_si256(temp, 1);

            __m128i out_pre = _mm_hadd_epi32(temp_lo, temp_hi);
            out_pre = _mm_hadd_epi32(out_pre, out_pre);
            out_pre = _mm_hadd_epi32(out_pre, out_pre);

            return _mm_cvtsi128_si32(out_pre);
        };

        int32_t out_0 = hadd(temp_0);
        int32_t out_1 = hadd(temp_1);
        int32_t out_2 = hadd(temp_2);
        int32_t out_3 = hadd(temp_3);

        int32_t out = out_0 + out_1 + out_2 + out_3;
        out += l1_bias;

        return out;
    }
}