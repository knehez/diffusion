#pragma once

#define DIFFUSIONLIB_API

#include <math.h>
#include <string.h>

class Scaling;
struct Datagrid;
inline bool operator==(const Scaling &lhs, const Scaling &rhs);
inline bool operator!=(const Scaling &lhs, const Scaling &rhs);

DIFFUSIONLIB_API bool allocate(Datagrid&, const Scaling&);
DIFFUSIONLIB_API void release(Datagrid&);
DIFFUSIONLIB_API int load(const char *file, Datagrid&);
DIFFUSIONLIB_API int save(const char *file, const Datagrid&);


class DIFFUSIONLIB_API Scaling {
	size_t _dim = 0;
	size_t _nr[3] = { 1, 1, 1 };
	double _r_min[3] = { NAN, NAN, NAN };
	double _r_max[3] = { NAN, NAN, NAN };

public:
	const size_t &DIM = _dim;
	const size_t &Nx = _nr[0], &Ny = _nr[1], &Nz = _nr[2];
	const double &Xmin = _r_min[0], &Xmax = _r_max[0];
	const double &Ymin = _r_min[1], &Ymax = _r_max[1];
	const double &Zmin = _r_min[2], &Zmax = _r_max[2];

	inline Scaling() {}

	inline Scaling(const Scaling &other) {
		_dim = other._dim;
		memcpy(_nr, other._nr, sizeof(_nr));
		memcpy(_r_min, other._r_min, sizeof(_r_min));
		memcpy(_r_max, other._r_max, sizeof(_r_max));
	}

	inline size_t nr_tot() const {
		/*return	(_dim > 0 ? _nr[0] : 1)
			*	(_dim > 1 ? _nr[1] : 1)
			-*	(_dim > 2 ? _nr[2] : 1);*/
		return _nr[0] * _nr[1] * _nr[2];
	}

	inline double dx() const {
		return (_r_max[0] - _r_min[0]) / (_nr[0] - 1);
	}

	inline double dy() const {
		return (_r_max[1] - _r_min[1]) / (_nr[1] - 1);
	}

	inline double dz() const {
		return (_r_max[2] - _r_min[2]) / (_nr[2] - 1);
	}

	inline void clear() {
		_dim = 0;
		_nr[0] = 1, _nr[1] = 1, _nr[2] = 1;
		_r_min[0] = NAN, _r_min[1] = NAN, _r_min[2] = NAN;
		_r_max[0] = NAN, _r_max[1] = NAN, _r_max[2] = NAN;
	}

	inline Scaling& X(double min, size_t n, double max) {
		_dim = _dim > 1 ? _dim : 1;
		_nr[0] = n;
		_r_min[0] = min;
		_r_max[0] = max;
		return *this;
	}

	inline Scaling& Y(double min, size_t n, double max) {
		_dim = _dim > 2 ? _dim : 2;
		_nr[1] = n;
		_r_min[1] = min;
		_r_max[1] = max;
		return *this;
	}

	inline Scaling& Z(double min, size_t n, double max) {
		_dim = 3;
		_nr[2] = n;
		_r_min[2] = min;
		_r_max[2] = max;
		return *this;
	}

	inline Scaling& operator=(const Scaling& rhs) {
		_dim = rhs._dim;
		memcpy(_nr, rhs._nr, sizeof(_nr));
		memcpy(_r_min, rhs._r_min, sizeof(_r_min));
		memcpy(_r_max, rhs._r_max, sizeof(_r_max));
		return *this;
	}

	inline double& lookup(double *data, size_t i, size_t j = 0, size_t k = 0) const {
		return data[i + (j + k * _nr[1]) * _nr[0]];
	}

	inline const double& lookup(const double *data, size_t i, size_t j = 0, size_t k = 0) const {
		return data[i + (j + k * _nr[1]) * _nr[0]];
	}

	inline double* lookupRow(double *data, size_t j, size_t k = 0) const {
		return data + (j + k * _nr[1]) * _nr[0];
	}

	inline const double* lookupRow(const double *data, size_t j, size_t k = 0) const {
		return data + (j + k * _nr[1]) * _nr[0];
	}

	inline double* lookupPage(double *data, size_t k) const {
		return data + k * _nr[1] * _nr[0];
	}

	inline const double* lookupPage(const double *data, size_t k) const {
		return data + k * _nr[1] * _nr[0];
	}

	friend inline bool operator==(const Scaling &lhs, const Scaling &rhs);
	friend inline bool operator!=(const Scaling &lhs, const Scaling &rhs);
	friend DIFFUSIONLIB_API int load(const char *file, Datagrid&);
	friend DIFFUSIONLIB_API int save(const char *file, const Datagrid&);
};

inline bool operator==(const Scaling &lhs, const Scaling &rhs) {
	return lhs._dim == rhs._dim
		&& (lhs._dim > 0 ? (lhs._nr[0] == rhs._nr[0] && lhs._r_min[0] == lhs._r_min[0] && lhs._r_max[0] == rhs._r_max[0]) : true)
		&& (lhs._dim > 1 ? (lhs._nr[1] == rhs._nr[1] && lhs._r_min[1] == lhs._r_min[1] && lhs._r_max[1] == rhs._r_max[1]) : true)
		&& (lhs._dim > 2 ? (lhs._nr[2] == rhs._nr[2] && lhs._r_min[2] == lhs._r_min[2] && lhs._r_max[2] == rhs._r_max[2]) : true);
}

inline bool operator!=(const Scaling &lhs, const Scaling &rhs) {
	return lhs._dim != rhs._dim
		|| (lhs._dim > 0 ? (lhs._nr[0] != rhs._nr[0] || lhs._r_min[0] != lhs._r_min[0] || lhs._r_max[0] != rhs._r_max[0]) : false)
		|| (lhs._dim > 1 ? (lhs._nr[1] != rhs._nr[1] || lhs._r_min[1] != lhs._r_min[1] || lhs._r_max[1] != rhs._r_max[1]) : false)
		|| (lhs._dim > 2 ? (lhs._nr[2] != rhs._nr[2] || lhs._r_min[2] != lhs._r_min[2] || lhs._r_max[2] != rhs._r_max[2]) : false);
}


struct DIFFUSIONLIB_API Datagrid {
	Scaling scaling;
	double *data = NULL;

	inline double& operator()(size_t i, size_t j = 0, size_t k = 0) const {
		return scaling.lookup(data, i, j, k);
	}

	inline Datagrid row(size_t j, size_t k = 0) const {
		Datagrid subgrid;
		subgrid.scaling.X(scaling.Xmin, scaling.Nx, scaling.Xmax);
		subgrid.data = scaling.lookupRow(data, j, k);
		return subgrid;
	}

	inline Datagrid page(size_t k) const {
		Datagrid subgrid;
		subgrid.scaling.X(scaling.Xmin, scaling.Nx, scaling.Xmax)
			.Y(scaling.Ymin, scaling.Ny, scaling.Ymax);
		subgrid.data = scaling.lookupPage(data, k);
		return subgrid;
	}
};

