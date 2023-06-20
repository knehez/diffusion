    __kernel void vectorAdd(__global int* v1, __global int* v2) 
    {
        __global const float coeff = 0.0078408;
        int i = get_global_id(0);
        if (i == 0 || i == get_global_size(0)) return;  // vagy helyette az enqueue-nál offset és kisebb range
        v2[i] = (1 - 2 * coeff) * v1[i] + coeff * (v1[i-1] + v1[i+1]);
    }