kernel void matrix_mul(global const float *a, global const float *b, global float *c, uint N, uint K, uint M) {
    uint id0 = get_global_id(0);
    uint id1 = get_global_id(1);

    uint cache1 = id0 * K;
    uint cache2 = id1 * K;

    float temp = 0;
    for (uint k = 0; k < K; k++) {
        temp += a[cache1 + k] * b[cache2 + k];
    }
    c[id0 * M + id1] = temp;
}
