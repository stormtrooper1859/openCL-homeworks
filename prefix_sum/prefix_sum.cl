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

        float t = 0;
        if (i <= local_id) {
            t = temp[local_id - i];
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        temp[local_id] += t;
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
