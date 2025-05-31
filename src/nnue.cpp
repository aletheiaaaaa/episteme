#include "nnue.h"

namespace episteme::nn {
    Accumulator NNUE::l0_forward(const std::array<Piece, 64>& mailbox) const {
        Accumulator out;

        for (uint8_t i = 0; i < 64; i++) {
            int stm = piecesquare(mailbox[i], sq_from_idx(i), false);
            int ntm = piecesquare(mailbox[i], sq_from_idx(i ^ 56), true);

            if (stm != -1) {
                for (int j = 0; j < 1024; j++) {
                    out.stm[j] += l0_weights[stm][j];
                }
            }
            if (ntm != -1) {
                for (int j = 0; j < 1024; j++) {
                    out.ntm[j] += l0_weights[stm][j];
                }
            }
        }
        
        for (int i = 0; i < 1024; i++) {
            out.stm[i] += l0_biases[i];
            out.ntm[i] += l0_biases[i];
        }

        for (int i = 0; i < 1024; i++) {
            int stmClipped = std::clamp(static_cast<int>(out.stm[i]), 0, 255);
            int ntmClipped = std::clamp(static_cast<int>(out.ntm[i]), 0, 255);

            out.stm[i] = stmClipped * stmClipped;
            out.ntm[i] = ntmClipped * ntmClipped;
        }

        return out;
    }

    int32_t NNUE::l1_forward(const Accumulator& accum) const {
        __m256i temp_0 = _mm256_setzero_si256();
        __m256i temp_1 = _mm256_setzero_si256();
        __m256i temp_2 = _mm256_setzero_si256();
        __m256i temp_3 = _mm256_setzero_si256();

        for (int i = 0; i < 1024; i += 64) {
            __m256i stm_0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&accum.stm[i]));
            __m256i ntm_0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&accum.ntm[i]));
            __m256i l1w0_0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l1_weights[0][i]));
            __m256i l1w1_0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l1_weights[1][i]));

            temp_0 += _mm256_madd_epi16(stm_0, l1w0_0); 
            temp_0 += _mm256_madd_epi16(ntm_0, l1w1_0);

            __m256i stm_1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&accum.stm[i + 16]));
            __m256i ntm_1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&accum.ntm[i + 16]));
            __m256i l1w0_1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l1_weights[0][i + 16]));
            __m256i l1w1_1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l1_weights[1][i + 16]));

            temp_1 += _mm256_madd_epi16(stm_1, l1w0_1); 
            temp_1 += _mm256_madd_epi16(ntm_1, l1w1_1);

            __m256i stm_2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&accum.stm[i + 32]));
            __m256i ntm_2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&accum.ntm[i + 32]));
            __m256i l1w0_2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l1_weights[0][i + 32]));
            __m256i l1w1_2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l1_weights[1][i + 32]));

            temp_2 += _mm256_madd_epi16(stm_2, l1w0_2); 
            temp_2 += _mm256_madd_epi16(ntm_2, l1w1_2);

            __m256i stm_3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&accum.stm[i + 48]));
            __m256i ntm_3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&accum.ntm[i + 48]));
            __m256i l1w0_3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l1_weights[0][i + 48]));
            __m256i l1w1_3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&l1_weights[1][i + 48]));

            temp_3 += _mm256_madd_epi16(stm_3, l1w0_3); 
            temp_3 += _mm256_madd_epi16(ntm_3, l1w1_3);
        }

        __m128i temp_lo_0 = _mm256_castsi256_si128(temp_0); 
        __m128i temp_hi_0 = _mm256_extracti128_si256(temp_0, 1);

        __m128i out_pre_0 = _mm_hadd_epi32(temp_lo_0, temp_hi_0);
        out_pre_0 = _mm_hadd_epi32(out_pre_0, out_pre_0);
        out_pre_0 = _mm_hadd_epi32(out_pre_0, out_pre_0);

        int32_t out_0 = _mm_cvtsi128_si32(out_pre_0);

        __m128i temp_lo_1 = _mm256_castsi256_si128(temp_1); 
        __m128i temp_hi_1 = _mm256_extracti128_si256(temp_1, 1);

        __m128i out_pre_1 = _mm_hadd_epi32(temp_lo_1, temp_hi_1);
        out_pre_1 = _mm_hadd_epi32(out_pre_1, out_pre_1);
        out_pre_1 = _mm_hadd_epi32(out_pre_1, out_pre_1);

        int32_t out_1 = _mm_cvtsi128_si32(out_pre_1);

        __m128i temp_lo_2 = _mm256_castsi256_si128(temp_2); 
        __m128i temp_hi_2 = _mm256_extracti128_si256(temp_2, 1);

        __m128i out_pre_2 = _mm_hadd_epi32(temp_lo_2, temp_hi_2);
        out_pre_2 = _mm_hadd_epi32(out_pre_2, out_pre_2);
        out_pre_2 = _mm_hadd_epi32(out_pre_2, out_pre_2);

        int32_t out_2 = _mm_cvtsi128_si32(out_pre_2);

        __m128i temp_lo_3 = _mm256_castsi256_si128(temp_3); 
        __m128i temp_hi_3 = _mm256_extracti128_si256(temp_3, 1);

        __m128i out_pre_3 = _mm_hadd_epi32(temp_lo_3, temp_hi_3);
        out_pre_3 = _mm_hadd_epi32(out_pre_3, out_pre_3);
        out_pre_3 = _mm_hadd_epi32(out_pre_3, out_pre_3);

        int32_t out_3 = _mm_cvtsi128_si32(out_pre_3);

        int32_t out = out_0 + out_1 + out_2 + out_3;
        out += l1_bias;

        return out;
    }
}