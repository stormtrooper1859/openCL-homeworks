//__kernel void prefix_sum(global float *data, global float *blocks_sum, uint N) {
//    uint global_id = get_global_id(0);
//    uint local_id = get_local_id(0);
//
//    local float temp[LOCAL_GROUP_SIZE];
//    temp[local_id] = 0;
//    if (global_id < N) {
//        temp[local_id] = data[global_id];
//    }
//
//    barrier(CLK_LOCAL_MEM_FENCE);
//
////    printf("%d: %f\n", local_id, temp[local_id]);
//
//
//    int index1 = (local_id + 1) * 1 - 1;
//    for (int i = 1; i < LOCAL_GROUP_SIZE; i *= 2) {
////        int index = local_id * 2 * i - 1;
//        index1 += (local_id + 1) * i;
////        int index = (local_id + 1) * i * 2 - 1;
//        if (index1 < LOCAL_GROUP_SIZE) {
//
////            if(index - i < 0) {
////                printf("index: %d %d %d %d\n", local_id, i, index, index - i);
////            }
//            temp[index1] += temp[index1 - i];
//        }
//        barrier(CLK_LOCAL_MEM_FENCE);
//    }
//
////    printf("_ %d: %f\n", local_id, temp[local_id]);
//
//    for (int i = LOCAL_GROUP_SIZE / 4; i >= 1; i /= 2) {
//        int index = (local_id + 1) * i * 2 - 1;
//        if (index + i < LOCAL_GROUP_SIZE) {
////            if(index + i >= LOCAL_GROUP_SIZE) {
////                printf("index: %d %d %d %d\n", local_id, i, index, index - i);
////            }
//
//            temp[index + i] += temp[index];
//        }
//        barrier(CLK_LOCAL_MEM_FENCE);
//    }
//
////    printf("__ %d: %f\n", local_id, temp[local_id]);
//
//    data[global_id] = temp[local_id];
//
//    if (local_id == LOCAL_GROUP_SIZE - 1) {
//        blocks_sum[get_group_id(0)] = temp[local_id];
//    }
//}

__kernel void prefix_sum(global float *data, global float *blocks_sum, uint N) {
    uint global_id = get_global_id(0);
    uint local_id = get_local_id(0);

    local float temp[LOCAL_GROUP_SIZE];
    temp[local_id] = 0;
    if (global_id < N) {
        temp[local_id] = data[global_id];
    }

    for (int i = 1; i < LOCAL_GROUP_SIZE; i *= 2) {
        barrier(CLK_LOCAL_MEM_FENCE);

        float stored = 0;
        if (i <= local_id) {
            stored = temp[local_id - i];
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        temp[local_id] += stored;
    }

    data[global_id] = temp[local_id];

    if (local_id == LOCAL_GROUP_SIZE - 1) {
        blocks_sum[get_group_id(0)] = temp[local_id];
    }
}


__kernel void add_to_blocks(global float *data, global float *blocks_sum, uint N) {
    uint global_id = get_global_id(0);
    uint group_id = get_group_id(0);

    if (group_id > 0 && global_id < N) {
        float t = blocks_sum[group_id - 1];
        data[global_id] += t;
    }
}
