    __kernel void vectorAdd(__global float* v1, __global float* v2) 
    {
        const float coeff = 0.0078408;
        int i = get_global_id(0);
        if (i == 0 || i == get_global_size(0)) return;  // vagy helyette az enqueue-nál offset és kisebb range
        v2[i] = (1 - 2 * coeff) * v1[i] + coeff * (v1[i-1] + v1[i+1]);
        //v2[i] = v1[i] + 1;
    }