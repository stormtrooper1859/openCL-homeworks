__kernel void matrix_mul(global const float *a, global const float *b, global float *c, uint N, uint K, uint M) {
    uint global_X = get_global_id(0);
    uint global_Y = get_global_id(1);

    uint local_X = get_local_id(0);
    uint local_Y = get_local_id(1);

    uint cache_poisiton_A = global_Y * K * WPT + local_X;
    uint cache_poisiton_B = global_X + local_Y * M * WPT;

    local float at[TILE_W][TILE_H];
    local float bt[TILE_W][TILE_H];

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
        //        at[x * TILE_W + y] = a[cache0 + i * TILE_W + y];
        //        bt[y * TILE_W + x] = b[cache1 + i * TILE_W + x];
        //        at[x][y] = a[cache0 + i * TILE_W];
        //        bt[x][y] = b[cache1 + i * TILE_W];
//        at[local_X][local_Y] = a[cache_poisiton_A + i * TILE_W];
//        at[local_X][local_Y] = a[cache_poisiton_A + i * TILE_H];
//        at[local_Y][local_X] = a[cache0 + i * TILE_W];
//        bt[local_Y][local_X] = b[cache1 + i * TILE_W];

//        at[local_X][local_Y] = a[cache_poisiton_A + i * TILE_W];
//        bt[local_X][local_Y] = b[cache_poisiton_B + i * M * TILE_W];

//+
//        at[local_Y][local_X] = a[cache_poisiton_A + i * TILE_W];
//        bt[local_Y][local_X] = b[cache_poisiton_B + i * M * TILE_W];

        barrier(CLK_LOCAL_MEM_FENCE);

#pragma unroll
        for (uint k = 0; k < TILE_W; k++) {
            //            temp += at[x * TILE_W + k] * bt[y * TILE_W + k];
            //            temp += at[x][k] * bt[k][y];

#pragma unroll
            for (uint j = 0; j < WPT; ++j) {
//                at[local_Y][local_X * WPT + j] = a[cache_poisiton_A + i * TILE_W + j];
//                bt[local_Y][local_X * WPT + j] = b[cache_poisiton_B + i * M * TILE_W + j];
                temp[j] += at[local_Y * WPT + j][k] * bt[k][local_X];
            }
//            temp += at[local_Y][k] * bt[k][local_X];
//            temp += at[k][local_Y] * bt[local_X][k];
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    for (int i = 0; i < WPT; ++i) {
        c[global_Y * M * WPT + global_X + i * M] = temp[i];
    }

//    c[global_Y * M + global_X] = temp;
}
