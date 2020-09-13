//__kernel void prefix_sum(global float *a, global float *c, uint N) {
//    uint global_X = get_global_id(0);
//    uint local_X = get_local_id(0);
//
//    local float temp[SIZE * 2];
//
//
//    if(global_X >= N) {
//        temp[local_X] = 0;
//        temp[SIZE + local_X] = 0;
//    }else{
//
//        temp[local_X] = a[global_X];
//        temp[SIZE + local_X] = temp[local_X];
//    }
//
//    barrier(CLK_LOCAL_MEM_FENCE);
//
//    int pos1 = 0;
//    int pos2 = SIZE;
//
//    for (int result = 1; result < N; result *= 2) {
//        temp[pos2 + local_X] = temp[pos1 + local_X];
//        if (result <= local_X) {
//            temp[pos2 + local_X] += temp[pos1 + local_X - result];
//        }
//
//        pos1 = pos2;
//        pos2 = SIZE - pos2;
//
//        barrier(CLK_LOCAL_MEM_FENCE);
//    }
//
//    a[global_X] = temp[pos1 + local_X];
//    if (local_X == SIZE - 1) {
////        printf("wtf1 %f %d\n", temp[pos1 + local_X], global_X / SIZE);
//        c[global_X / SIZE] = temp[pos1 + local_X];
//    }
//}


__kernel void prefix_sum(global float *a, global float *c, uint N) {
    uint global_X = get_global_id(0);
    uint local_X = get_local_id(0);

    local float temp[SIZE];
    if (global_X >= N) {
        temp[local_X] = 0;
    } else {
        temp[local_X] = a[global_X];
    }

    for (int j = 1; j < SIZE; j <<= 1) {
        barrier(CLK_LOCAL_MEM_FENCE);
        float val = 0;
        if (j <= local_X) {
            val = temp[local_X - j];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        temp[local_X] += val;
    }

    a[global_X] = temp[local_X];
    if (local_X == SIZE - 1) {
        c[global_X / SIZE] = temp[local_X];
    }
}


__kernel void add_to_blocks(global float *a, global float *c, uint N) {
    uint global_X = get_global_id(0);

    uint ind = get_group_id(0);

    if (ind > 0 && global_X < N) {
        float t = c[ind - 1];
//        printf("wtf %f\n", t);
        a[global_X] += t;
    }
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


