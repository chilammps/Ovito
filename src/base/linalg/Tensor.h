///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __OVITO_TENSOR_H
#define __OVITO_TENSOR_H

#include <base/Base.h>
#include "Matrix3.h"

namespace Ovito {

/// A first order tensor is just a one-dimensional vector.
typedef Vector3 Tensor1;

/// A second order tensor is just a two-dimensional matrix.
typedef Matrix3 Tensor2;

/*
 * \brief A symmetric 2nd order tensor.
 *
 * Stores only the lower left part of the 3x3 matrix.
 */
template<typename T>
class SymmetricTensor2T : public std::array<T, 6>
{
public:

	struct Zero {};
	struct Identity {};

	typedef T value_type;
	typedef std::size_t size_type;

public:

	/// \brief Constructs a tensor without initializing its components.
	/// \note All components are left uninitialized by this constructor and will therefore have an undefined value!
	SymmetricTensor2T() {}

	/// \brief Constructor that initializes all tensor components to the same value.
	explicit SymmetricTensor2T(T val) : std::array<T, 6>{{val,val,val,val,val,val}} {}

	/// \brief Initializes the tensor to the null tensor. All components are set to zero.
	constexpr SymmetricTensor2T(Zero) : std::array<T, 6>{{T(0), T(0), T(0), T(0), T(0), T(0)}} {}

	/// \brief Initializes the tensor to the identity tensor.
	constexpr SymmetricTensor2T(Identity) : std::array<T, 6>{{T(1), T(1), T(1), T(0), T(0), T(0)}} {}

	/// \brief Returns the number of rows in this matrix.
	static constexpr size_type row_count() { return 3; }

	/// \brief Returns the columns of rows in this matrix.
	static constexpr size_type col_count() { return 3; }

	/// \brief Tensor element access.
	inline const T& operator()(size_type row, size_type col) const {
		OVITO_ASSERT(row < row_count() && col < col_count());
		if(row < col) std::swap(row, col);
		switch(row - col) {
			case 0: return (*this)[row];
			case 1: return (*this)[row+2];
			case 2: return (*this)[5];
			default: OVITO_ASSERT(false); return (*this)[0];
		}
	}

	/// \brief Tensor element access.
	inline T& operator()(size_type row, size_type col) {
		OVITO_ASSERT(row < row_count() && col < col_count());
		if(row < col) std::swap(row, col);
		switch(row - col) {
			case 0: return (*this)[row];
			case 1: return (*this)[row+2];
			case 2: return (*this)[5];
			default: OVITO_ASSERT(false); return (*this)[0];
		}
	}
};

/// Writes the symmetric tensor to an output stream.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const SymmetricTensor2T<T>& m) {
	for(typename SymmetricTensor2T<T>::size_type row = 0; row < m.row_count(); row++) {
		for(typename SymmetricTensor2T<T>::size_type col = 0; col < m.col_count(); col++) {
			os << m(row, col) << " ";
		}
		os << std::endl;
	}
	return os;
}

/// Writes a symmetric tensor to an output stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const SymmetricTensor2T<T>& m)
{
	for(typename SymmetricTensor2T<T>::size_type i = 0; i < m.size(); i++)
		stream << m[i];
	return stream;
}

/// Reads a symmetric tensor from an input stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, SymmetricTensor2T<T>& m)
{
	for(typename SymmetricTensor2T<T>::size_type i = 0; i < m.size(); i++)
		stream >> m[i];
	return stream;
}

/**
 * \fn typedef SymmetricTensor2
 * \brief Template class instance of the SymmetricTensor2T class.
 */
typedef SymmetricTensor2T<FloatType> SymmetricTensor2;

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::SymmetricTensor2)
Q_DECLARE_TYPEINFO(Ovito::SymmetricTensor2, Q_PRIMITIVE_TYPE);

#endif // __OVITO_TENSOR_H
