kernel void add(global const float *a, global const float *b, global float *c, const uint m, const uint p) {
	uint id0 = get_global_id(0);
	uint id1 = get_global_id(1);

	float temp = 0;

	c[id0 * p + id1] = temp;
}
