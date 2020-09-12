__kernel void prefix_sum(global const float *a, global float *c, uint N) {
    uint global_X = get_global_id(0);
//    uint local_X = get_local_id(0);

    local float temp[SIZE * 2];

    temp[global_X] = a[global_X];
    temp[SIZE + global_X] = temp[global_X];

    barrier(CLK_LOCAL_MEM_FENCE);

    int pos1 = 0;
    int pos2 = SIZE;

    for (int result = 1; result < N; result *= 2) {
        temp[pos2 + global_X] = temp[pos1 + global_X];
        if (result <= global_X) {
            temp[pos2 + global_X] += temp[pos1 + global_X - result];
        }

        pos1 = pos2;
        pos2 = SIZE - pos2;

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    c[global_X] = temp[pos1 + global_X];
}




//__kernel void matrix_mul(global const float *a, global float *c, uint N) {
//    uint global_X = get_global_id(0);
//
//    uint local_X = get_local_id(0);
//
////    local float c[n];
//
//
//    c[0] = a[0];
//    for (uint i = 1; i < N; i++) {
//        c[i] = c[i-1] + a[i];
//    }
//}


