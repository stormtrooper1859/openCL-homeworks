/*
 * Перемножение матриц A и B
 * TILE_W - размер локальной группы. локальная группа должна быть квадратная
 * WPT - сколько элементов обрабатывает один поток. TILE_W должен делиться на WPT
 *
 * Матрица A имеет размерность NxK
 * Матрица B имеет размерность KxM
 * Матрица C имеет размерность NxM
 *
 * Первое измерение идет по столбцам С, второе по строчкам
 * # - пример элементов одноверменно обрабатываемых одним ядром
 *
 *              B------
 *              -.....-
 *              -.....-
 *              -------
 *
 *              --dim0->
 *
 *  A----   |   C------
 *  -...-   |   -..#..-
 *  -...- dim1  -..#..-
 *  -...-   |   -.....-
 *  -----   ˅   -------
 *
 */
__kernel void matrix_mul(global const float *a, global const float *b, global float *c, uint N, uint K, uint M) {
    uint global_X = get_global_id(0);
    uint global_Y = get_global_id(1);

    uint local_X = get_local_id(0);
    uint local_Y = get_local_id(1);

    uint cache_poisiton_A = global_Y * K * WPT + local_X;
    uint cache_poisiton_B = global_X + local_Y * M * WPT;

    local float at[TILE_W][TILE_W];
    local float bt[TILE_W][TILE_W];

    float temp[WPT];
    for (int i = 0; i < WPT; ++i) {
        temp[i] = 0;
    }

    uint title_count = K / TILE_W;
    for (uint i = 0; i < title_count; i++) {

        for (uint j = 0; j < WPT; ++j) {
            at[local_Y * WPT + j][local_X] = a[cache_poisiton_A + i * TILE_W + K * j];
            bt[local_Y * WPT + j][local_X] = b[cache_poisiton_B + i * M * TILE_W + M * j];
        }

        barrier(CLK_LOCAL_MEM_FENCE);

#pragma unroll
        for (uint k = 0; k < TILE_W; k++) {

#pragma unroll
            for (uint j = 0; j < WPT; ++j) {
                temp[j] += at[local_Y * WPT + j][k] * bt[k][local_X];
            }
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    for (int i = 0; i < WPT; ++i) {
        c[global_Y * M * WPT + global_X + i * M] = temp[i];
    }
}
