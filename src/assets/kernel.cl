    __kernel void vectorAdd(__global int* v1) 
    {
        int i = get_global_id(0);
        v1[i] = 1;
    }