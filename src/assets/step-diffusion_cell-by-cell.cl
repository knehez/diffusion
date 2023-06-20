#ifdef cl_khr_fp64
	#warning "Using Khronos-style double precision floating points"
	#pragma OPENCL EXTENSION cl_khr_fp64 : enable
	typedef double real;
	typedef double2 real2;
	typedef double3 real3;
#elif defined cl_amd_fp64
	#warning "Using AMD-style double precision floating points"
	#pragma OPENCL EXTENSION cl_amd_fp64 : enable
	typedef double real;
	typedef double2 real2;
	typedef double3 real3;
#else
	#warning "Double precision floating points not supported. Defaulting to single precision mode."
	typedef float real;
	typedef float2 real2;
	typedef float3 real3;
#endif

__kernel void calc_cell_1D(__global const real *before, __global real *after, real coeff,
		uint size, long2 neighbours) {
    const size_t index = get_global_id(0);
	__global const real *old_cell = before + index;
	after[index] = (1 - 2 * coeff) * *old_cell + coeff * (old_cell[neighbours.even] + old_cell[neighbours.odd]);
	//after[index] = old_cell[neighbours.even];
	//after[index] = *old_cell + 1.0;
}

__kernel void calc_cell_2D(__global const real *before, __global real *after, real2 coeffs,
		uint2 size, long4 neighbours) {
    const size_t index = get_global_id(0) + size.x * get_global_id(1);
	__global const real *old_cell = before + index;
	after[index] = (1 - 2 * (coeffs.x + coeffs.y)) * *old_cell
		+ coeffs.x * (old_cell[neighbours.even.x] + old_cell[neighbours.odd.x])
		+ coeffs.y * (old_cell[neighbours.even.y] + old_cell[neighbours.odd.y]);
	/*after[index] = *old_cell
		+ coeffs.x * (old_cell[neighbours.even.x] - 2 * old_cell[0] + old_cell[neighbours.odd.x])
		+ coeffs.y * (old_cell[neighbours.even.y] - 2 * old_cell[0] + old_cell[neighbours.odd.y]);*/
}

__kernel void calc_cell_3D(__global const real *before, __global real *after, real3 coeffs,
		uint3 size, long8 neighbours) {
    const size_t index = get_global_id(0)
		+ size.x * (get_global_id(1)
			+ size.y * get_global_id(2));
	__global const real *old_cell = before + index;
	after[index] = (1 - 2 * (coeffs.x + coeffs.y + coeffs.z)) * *old_cell
		+ coeffs.x * (old_cell[neighbours.even.x] + old_cell[neighbours.odd.x])
		+ coeffs.y * (old_cell[neighbours.even.y] + old_cell[neighbours.odd.y])
		+ coeffs.z * (old_cell[neighbours.even.z] + old_cell[neighbours.odd.z]);
}