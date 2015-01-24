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

/**
 * \file
 * \brief Contains the definition of the Ovito::ScalingT class template.
 */

#ifndef __OVITO_SCALING_H
#define __OVITO_SCALING_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>
#include "Vector3.h"
#include "Quaternion.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief A transformation that describes a non-uniform scaling in an arbitrary axis system.
 *
 * The Vector3 #S specifies the scaling factors along the x, y, and z axes, and the quaternion #Q
 * defines the axis system in which the scaling is applied.
 */
template<typename T>
class ScalingT
{
public:

	struct Identity {};

public:

	/// \brief The scaling factors in x, y and z directions of the axis system specified by Q.
	Vector_3<T> S;

	/// \brief The orientation of the axis system the scaling is applied in.
	QuaternionT<T> Q;

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief Constructs a Scaling object without initializing its components.
	/// \note The components are left uninitialized by this constructor and will therefore have a random value!
	ScalingT() {}

	/// \brief Initializes a Scaling with the scaling factors and the coordinate system.
	/// \param scaling The scaling factors in x, y and z directions of the axis system specified by \a orientation.
	/// \param orientation The orientation of the axis system the scaling is applied in.
	Q_DECL_CONSTEXPR ScalingT(const Vector_3<T>& scaling, const QuaternionT<T>& orientation) : S(scaling), Q(orientation) {}

	/// \brief Initializes the object to the identity.
	/// The Scaling structure is initialized with the scaling factors (1,1,1), i.e. no scaling at all.
	Q_DECL_CONSTEXPR ScalingT(Identity) : S(T(1)), Q(typename QuaternionT<T>::Identity()) {}

	/////////////////////////////// Unary operators //////////////////////////////

	/// \brief Returns the inverse of this scaling.
	/// \return The inverse scaling that exactly compensates this scaling.
	ScalingT inverse() const {
		OVITO_ASSERT_MSG(S != typename Vector_3<T>::Zero(), "Scaling::inverse()", "Cannot invert a singular scaling value.");
		return { Vector_3<T>(T(1) / S.x(), T(1) / S.y(), T(1) / S.z()), Q.inverse().normalized() };
	}

	/////////////////////////////// Binary operators /////////////////////////////

	/// \brief Performs the multiplication of two scaling structures.
	/// \param s2 The second scaling.
	/// \return A scaling structure that is equal to first applying scaling \a s2 and then \c this scaling.
	ScalingT operator*(const ScalingT& s2) const {
		if(Q == s2.Q) {
			return ScalingT(Vector_3<T>(S.x() * s2.S.x(), S.y() * s2.S.y(), S.z() * s2.S.z()), Q);
		}
		else {
			//AffineDecomposition decomp(AffineTransformation::scaling(*this) * AffineTransformation::scaling(s2));
			//return decomp.scaling;
			OVITO_ASSERT_MSG(false, "Scaling product", "Product of two Scaling values is not implemented yet.");
			return ScalingT(Identity());
		}
	}

	/// \brief Adds the given scaling to this scaling.
	/// \param s2 The scaling to add to this scaling.
	/// \return This resulting scaling which is equal to \c s2*(*this).
	ScalingT& operator+=(const ScalingT& s2) { *this = s2 * (*this); return *this; }

	/// \brief Adds the inverse of another scaling to this scaling.
	/// \param s2 The scaling to subtract from this scaling.
	/// \return This resulting scaling which is equal to \c (*this)*s2.inverse().
	ScalingT& operator-=(const ScalingT& s2) { *this = *this * s2.inverse(); return *this; }

	/// \brief Sets the scaling to the identity scaling.
	ScalingT& setIdentity() {
		S = Vector_3<T>(T(1));
		Q.setIdentity();
		return *this;
	}

	/// \brief Sets the scaling to the identity scaling.
	ScalingT& operator=(Identity) { return setIdentity(); }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Compares two scaling structures for equality.
	/// \param s The scaling to compare with.
	/// \return \c true if each of the components are equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const ScalingT& s) const { return (s.S==S) && (s.Q==Q); }

	/// \brief Compares two scaling structures for inequality.
	/// \param s The scaling to compare with.
	/// \return \c true if any of the components are not equal; \c false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const ScalingT& s) const { return (s.S!=S) || (s.Q!=Q); }

	/// \brief Returns whether this is the identity.
	/// \return \c true if the scaling in each of the three spatial directions is 1;
	///         \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(Identity) const { return (S == Vector_3<T>(1)); }

	/// \brief Returns whether this is not the identity.
	/// \return \c true if the scaling in any of the three spatial directions is not 1;
	///         \c false otherwise.
	Q_DECL_CONSTEXPR bool operator!=(Identity) const { return (S != Vector_3<T>(1)); }

	///////////////////////////////// Interpolation //////////////////////////////

	/// \brief Computes a linear interpolation between two scaling structures.
	/// \param s1 The first scaling.
	/// \param s2 The second scaling.
	/// \param t The parameter for the linear interpolation in the range [0,1].
	/// \return A linear interpolation between \a s1 and \a s2.
    static ScalingT interpolate(const ScalingT& s1, const ScalingT& s2, T t) {
    	return ScalingT(t * s2.S + (1 - t) * s1.S, QuaternionT<T>::interpolate(s1.Q, s2.Q, t));
    }

	/// \brief Computes a quadratic interpolation between two scaling structures.
	/// \param s1 The first scaling (at t==0.0).
	/// \param s2 The second scaling (at t==1.0).
	/// \param out Controls the tangential direction at \a s1.
	/// \param in Controls the tangential direction at \a s2.
	/// \param t The parameter for the linear interpolation in the range [0,1].
	/// \return A linear interpolation between \a s1 and \a s2.
    static ScalingT interpolateQuad(const ScalingT& s1, const ScalingT& s2, const ScalingT& out, const ScalingT& in, T t) {
    	T Ti = 1 - t;
    	T U2 = t*t, T2 = Ti*Ti;
    	T U3 = U2 * t, T3 = T2 * Ti;
    	Vector_3<T> s = s1.S * T3 + out.S * (3 * t * T2) + in.S * (3 * U2 * Ti) + s2.S * U3;
    	return ScalingT(s, QuaternionT<T>::interpolateQuad(s1.Q, s2.Q, in.Q, out.Q, t));
    }

    ////////////////////////////////// Utilities /////////////////////////////////

	/// Returns a string representation of this scaling.
	QString toString() const { return QString("[Scaling: %1 Orientation: %2]").arg(S.toString(), Q.toString()); }
};

/// \brief Writes the Scaling to a text output stream.
/// \param os The output stream.
/// \param s The scaling to write to the output stream \a os.
/// \return The output stream \a os.
/// \relates ScalingT
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const ScalingT<T>& s) {
	return os << '[' << s.S << "], " << s.Q;
}

/// \brief Writes the scaling to the Qt debug stream.
/// \relates ScalingT
template<typename T>
inline QDebug operator<<(QDebug dbg, const ScalingT<T>& s) {
    dbg.nospace() << "[" << s.S << "]";
    return dbg.space();
}

/// \brief Writes a Scaling to a binary output stream.
/// \param stream The output stream.
/// \param s The scaling to write to the output stream \a stream.
/// \return The output stream \a stream.
/// \relates ScalingT
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const ScalingT<T>& s)
{
	return stream << s.S << s.Q;
}

/// \brief Reads a Scaling from a binary input stream.
/// \param stream The input stream.
/// \param s Reference to a scaling variable where the parsed data will be stored.
/// \return The input stream \a stream.
/// \relates ScalingT
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, ScalingT<T>& s)
{
	return stream >> s.S >> s.Q;
}

/// \brief Writes a scaling to a Qt data stream.
/// \relates ScalingT
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const ScalingT<T>& s) {
	return stream << s.S << s.Q;
}

/// \brief Reads a scaling from a Qt data stream.
/// \relates ScalingT
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, ScalingT<T>& s) {
	return stream >> s.S >> s.Q;
}

/**
 * \brief Template class instance of the ScalingT template.
 * \relates ScalingT
 */
typedef ScalingT<FloatType>		Scaling;

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Scaling);
Q_DECLARE_METATYPE(Ovito::Scaling*);
Q_DECLARE_TYPEINFO(Ovito::Scaling, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Scaling*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_SCALING_H
