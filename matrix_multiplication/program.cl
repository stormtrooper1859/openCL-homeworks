__kernel void matrix_mul(global const float *a, global const float *b, global float *c, uint N, uint K, uint M) {
    uint id0 = get_global_id(0);
    uint id1 = get_global_id(1);

    uint x = get_local_id(0);
    uint y = get_local_id(1);

    //    uint ls0 = get_local_size(0);
    //    uint ls1 = get_local_size(1);

    uint cache0 = id0 * K + y;
    uint cache1 = id1 * K + x;
    float temp = 0;

    //    local float at[TILE_W * TILE_W];
    //    local float bt[TILE_W * TILE_W];
    local float at[TILE_W][TILE_W];
    local float bt[TILE_W][TILE_W];

    uint title_count = K / TILE_W;
    for (uint i = 0; i < title_count; i++) {
        //        at[x * TILE_W + y] = a[cache0 + i * TILE_W + y];
        //        bt[y * TILE_W + x] = b[cache1 + i * TILE_W + x];
        //        at[x][y] = a[cache0 + i * TILE_W];
        //        bt[x][y] = b[cache1 + i * TILE_W];
        at[y][x] = a[cache0 + i * TILE_W];
        bt[y][x] = b[cache1 + i * TILE_W];

        barrier(CLK_LOCAL_MEM_FENCE);

        for (uint k = 0; k < TILE_W; k++) {
            //            temp += at[x * TILE_W + k] * bt[y * TILE_W + k];
            //            temp += at[x][k] * bt[k][y];
            temp += at[k][x] * bt[y][k];
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }
    c[id0 * M + id1] = temp;
}
