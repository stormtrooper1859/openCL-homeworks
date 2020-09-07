__kernel void matrix_mul(global const float *a, global const float *b, global float *c, uint N, uint K, uint M) {
    uint global_X = get_global_id(0);
    uint global_Y = get_global_id(1);

    uint local_X = get_local_id(0);
    uint local_Y = get_local_id(1);

//    uint cache0 = global_X * K + local_Y;
//    uint cache1 = global_Y * K + local_X;

//    uint cache0 = global_X * K + local_Y;
//    uint cache1 = global_Y * K + local_X;
    float temp = 0;

    uint cache_poisiton_A = global_Y * K + local_X;
    uint cache_poisiton_B = global_X + local_Y * M;

    //    local float at[TILE_W * TILE_W];
    //    local float bt[TILE_W * TILE_W];
    local float at[TILE_W][TILE_H];
    local float bt[TILE_W][TILE_H];

    uint title_count = K / TILE_W;
    for (uint i = 0; i < title_count; i++) {
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
        at[local_Y][local_X] = a[cache_poisiton_A + i * TILE_W];
        bt[local_Y][local_X] = b[cache_poisiton_B + i * M * TILE_W];

        barrier(CLK_LOCAL_MEM_FENCE);

//#pragma unroll
        for (uint k = 0; k < TILE_W; k++) {
            //            temp += at[x * TILE_W + k] * bt[y * TILE_W + k];
            //            temp += at[x][k] * bt[k][y];

            temp += at[local_Y][k] * bt[k][local_X];
//            temp += at[k][local_Y] * bt[local_X][k];
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }
    c[global_Y * M + global_X] = temp;
}
