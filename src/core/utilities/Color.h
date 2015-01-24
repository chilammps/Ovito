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
 * \brief Contains the definitions of the Ovito::ColorT and Ovito::ColorAT class templates.
 */

#ifndef __OVITO_COLOR_H
#define __OVITO_COLOR_H

#include <core/Core.h>
#include <core/utilities/linalg/Vector3.h>
#include <core/utilities/io/LoadStream.h>
#include <core/utilities/io/SaveStream.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util)

/**
 * \brief A color value with red, blue, and green components.
 * 
 * This class stores three floating-point values in the range 0 to 1.
 * Note that the class derives from std::array, which provides additional methods and component-wise data access.
 * 
 * The typedef ::Color instantiates the ColorT class template with the default floating-point type ::FloatType.
 *
 * \tparam T The value type of the color components.
 *
 * \sa Color
 * \sa ColorAT
 */
template<typename T>
class ColorT : public std::array<T, 3>
{
public:

	/// Default constructs a color without initializing its components.
	/// The color components will therefore have an undefined value!
	ColorT() {}
	
	/// Initializes the color with the given red, green and blue values (in the range 0 to 1).
	Q_DECL_CONSTEXPR ColorT(T red, T green, T blue)
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
		: std::array<T, 3>{{red, green, blue}} {}
#else
		{ this->r() = red; this->g() = green; this->b() = blue; } 
#endif
	
	/// Converts a 3-vector to a color.
	/// The X, Y and Z vector components are used to initialize the red, green and blue components respectively.
	Q_DECL_CONSTEXPR explicit ColorT(const Vector_3<T>& v)
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
		: std::array<T, 3>{{v.x(), v.y(), v.z()}} {}
#else
		{ this->r() = v.x(); this->g() = v.y(); this->b() = v.z(); } 
#endif

	/// Initializes the color from an array with three values.
	Q_DECL_CONSTEXPR explicit ColorT(const std::array<T, 3>& c) : std::array<T, 3>(c) {}

	/// Conversion constructor from a Qt color.
	Q_DECL_CONSTEXPR ColorT(const QColor& c)
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
		: std::array<T, 3>{{T(c.redF()), T(c.greenF()), T(c.blueF())}} {}
#else
		{ this->r() = T(c.redF()); this->g() = T(c.greenF()); this->b() = T(c.blueF()); } 
#endif

	/// Sets all components of the color to zero.
	void setBlack() { r() = g() = b() = T(0); }

	/// Sets all components of the color to one.
	void setWhite() { r() = g() = b() = T(1); }

	/// Conversion operator to a Qt color.
	/// All components of the returned Qt color are clamped to the [0,1] range.
	operator QColor() const {
		return QColor::fromRgbF(
				qMin(qMax(r(), T(0)), T(1)),
				qMin(qMax(g(), T(0)), T(1)),
				qMin(qMax(b(), T(0)), T(1)));
	}
	
	/// Converts the RGB color to a 3-vector with XYZ components.
	explicit Q_DECL_CONSTEXPR operator const Vector_3<T>&() const {
		return reinterpret_cast<const Vector_3<T>&>(*this);
	}
	
	//////////////////////////// Component access //////////////////////////

	/// Returns the value of the red component of this color.
	Q_DECL_CONSTEXPR T r() const { return (*this)[0]; }

	/// Returns the value of the green component of this color.
	Q_DECL_CONSTEXPR T g() const { return (*this)[1]; }

	/// Returns the value of the blue component of this color.
	Q_DECL_CONSTEXPR T b() const { return (*this)[2]; }

	/// Returns a reference to the red component of this color.
	T& r() { return (*this)[0]; }

	/// Returns a reference to the green component of this color.
	T& g() { return (*this)[1]; }

	/// Returns a reference to the blue component of this color.
	T& b() { return (*this)[2]; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// Compares this color with another color for equality.
	/// \param c The other color.
	/// \return \c true if all three color components are exactly equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const ColorT& c) const { return (r() == c.r() && g() == c.g() && b() == c.b()); }
	
	/// \brief Compares this color with another color for inequality.
	/// \param c The second color.
	/// \return \c true if any of the color components are not equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator!=(const ColorT& c) const { return (r() != c.r() || g() != c.g() || b() != c.b()); }

	/// Adds another color to this color in a component-wise manner.
	ColorT& operator+=(const ColorT& c) { r() += c.r(); g() += c.g(); b() += c.b(); return *this; }
	
	/// Multiplies the components of another color with the components of this color.
	ColorT& operator*=(const ColorT& c) { r() *= c.r(); g() *= c.g(); b() *= c.b(); return *this; }

	/// Assigns the XYZ components of the given vector to a RGB components of this color.
	ColorT& operator=(const Vector_3<T>& v) { r() = v.x(); g() = v.y(); b() = v.z(); return *this; }
	
	/// Ensures that none of the color components is greater than 1.
	/// Any component greater than 1 is changed to 1.
	/// \sa clampMin(), clampMinMax()
	void clampMax() { if(r() > T(1)) r() = T(1); if(g() > T(1)) g() = T(1); if(b() > T(1)) b() = T(1); }
	
	/// Ensures that none of the color components is less than 0.
	/// Any color component less than 0 is set to 0.
	/// \sa clampMax(), clampMinMax()
	void clampMin() { if(r() < T(0)) r() = T(0); if(g() < T(0)) g() = T(0); if(b() < T(0)) b() = T(0); }
	
	/// Ensures that all color components between 0 and 1.
	/// \sa clampMin(), clampMax()
	void clampMinMax() {
		for(typename std::array<T, 3>::size_type i = 0; i < std::array<T, 3>::size(); i++) {
			if((*this)[i] > T(1)) (*this)[i] = T(1);
			else if((*this)[i] < T(0)) (*this)[i] = T(0);
		}
	}

	/// \brief Creates a RGB color from a hue-saturation-value representation.
	/// \param hue The hue value between 0 and 1.
	/// \param saturation The saturation value between 0 and 1.
	/// \param value The value of the color between 0 and 1.
	/// \return The RGB representation of the color.
	static ColorT fromHSV(T hue, T saturation, T value) {
		if(saturation == 0) {
			return ColorT(value, value, value);
		}
		else {
			T f, p, q, t;
			int i;
			if(hue >= 1.0 || hue < T(0)) hue = T(0);
			hue *= T(6);
			i = (int)floor(hue);
			f = hue - (T)i;
			p = value * (T(1) - saturation);
			q = value * (T(1) - (saturation * f));
			t = value * (T(1) - (saturation * (T(1) - f)));
			switch(i) {
				case 0: return ColorT(value, t, p);
				case 1: return ColorT(q, value, p);
				case 2: return ColorT(p, value, t);
				case 3: return ColorT(p, q, value);
				case 4: return ColorT(t, p, value);
				case 5: return ColorT(value, p, q);
				default:
					return ColorT(value, value, value);
			}
		}
	}

	/// Produces a string representation of the color.
	QString toString() const { return QString("(%1 %2 %3)").arg(r()).arg(g()).arg(b()); }
};

/// Multiplies the three components of a color \a c with a scalar value \a s.
/// \relates ColorT
template<typename T>
Q_DECL_CONSTEXPR inline ColorT<T> operator*(T s, const ColorT<T>& c) {
	return ColorT<T>(c.r()*s, c.g()*s, c.b()*s);
}

/// Multiplies the three components of a color \a c with a scalar value \a s.
/// \relates ColorT
template<typename T>
Q_DECL_CONSTEXPR inline ColorT<T> operator*(const ColorT<T>& c, T s) {
	return ColorT<T>(c.r()*s, c.g()*s, c.b()*s);
}

/// Computes the component-wise sum of two colors \a c1 and \a c2.
/// \relates ColorT
template<typename T>
Q_DECL_CONSTEXPR inline ColorT<T> operator+(const ColorT<T>& c1, const ColorT<T>& c2) {
	return { c1.r()+c2.r(), c1.g()+c2.g(), c1.b()+c2.b() };
}

/// Computes the component-wise product of two colors \a c1 and \a c2.
/// \relates ColorT
template<typename T>
Q_DECL_CONSTEXPR inline ColorT<T> operator*(const ColorT<T>& c1, const ColorT<T>& c2) {
	return { c1.r()*c2.r(), c1.g()*c2.g(), c1.b()*c2.b() };
}

/// Prints a color to a text output stream.
/// \relates ColorT
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const ColorT<T>& c) {
	return os << c.r() << ' ' << c.g()  << ' ' << c.b();
}

/// Prints a color to a Qt debug stream.
/// \relates ColorT
template<typename T>
inline QDebug operator<<(QDebug dbg, const ColorT<T>& c) {
    dbg.nospace() << "(" << c.r() << " " << c.g() << " " << c.b() << ")";
    return dbg.space();
}

/// Writes a color to a binary file stream.
/// \relates ColorT
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const ColorT<T>& c) {
	return stream << c.r() << c.g() << c.b();
}

/// Reads a color from a binary file stream.
/// \relates ColorT
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, ColorT<T>& c) {
	return stream >> c.r() >> c.g() >> c.b();
}

/// Writes a color to a Qt data stream.
/// \relates ColorT
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const ColorT<T>& c) {
	return stream << c.r() << c.g() << c.b();
}

/// Reads a color from a Qt data stream.
/// \relates ColorT
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, ColorT<T>& c) {
	return stream >> c.r() >> c.g() >> c.b();
}

/**
 * \brief A color value with red, blue, green, and alpha components.
 *
 * This class stores four floating-point values in the range 0 to 1. The fourth component (alpha) controls
 * the opacity of the color.
 * Note that the class derives from std::array, which provides additional methods and component-wise data access.
 *
 * The typedef ::ColorA instantiates the ColorAT class template with the default floating-point type ::FloatType.
 *
 * \tparam T The value type of the color components.
 *
 * \sa ColorA
 * \sa ColorT
 */
template<typename T>
class ColorAT : public std::array<T, 4>
{
public:

	/// Default constructs a color without initializing its components.
	/// The components will therefore have an undefined value!
	ColorAT() {}

	/// Initializes the color with the given red, green, blue, and alpha value.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR ColorAT(T red, T green, T blue, T alpha = T(1))
		: std::array<T, 4>{{red, green, blue, alpha}} {}
#else
	ColorAT(T red, T green, T blue, T alpha = T(1))
		{ this->r() = red; this->g() = green; this->b() = blue; this->a() = alpha; } 
#endif

	/// Converts a 4-vector to a color. The X, Y, Z, and W vector components are used to initialize the red, green, blue, and alpha components respectively.
	Q_DECL_CONSTEXPR explicit ColorAT(const Vector_4<T>& v) : std::array<T, 4>(v) {}

	/// Conversion constructor from a Qt color value.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR ColorAT(const QColor& c)
		: std::array<T, 4>{{T(c.redF()), T(c.greenF()), T(c.blueF()), T(c.alphaF())}} {}
#else
	ColorAT(const QColor& c)
		{ this->r() = T(c.redF()); this->g() = T(c.greenF()); this->b() = T(c.blueF()); this->a() = T(c.alphaF()); } 
#endif
	/// Converts a color without alpha component to a color with alpha component.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR ColorAT(const ColorT<T>& c, T alpha = T(1))
		: std::array<T, 4>{{c.r(), c.g(), c.b(), alpha}} {}
#else
	ColorAT(const ColorT<T>& c, T alpha = T(1))
		{ this->r() = c.r(); this->g() = c.g(); this->b() = c.b(); this->a() = alpha; } 
#endif

	/// Initializes the color components from an array of four values.
	Q_DECL_CONSTEXPR explicit ColorAT(const std::array<T, 4>& c) : std::array<T, 4>(c) {}

	/// Sets the red, green, and blue components to zero and alpha to one.
	void setBlack() { r() = g() = b() = T(0); a() = T(1); }

	/// Sets all color components to one.
	void setWhite() { r() = g() = b() = a() = T(1); }

	/// Converts this RGBA color to a XYZW vector.
	explicit Q_DECL_CONSTEXPR operator const Vector_4<T>&() const { return reinterpret_cast<const Vector_4<T>&>(*this); }

	/// Conversion operator to a Qt color.
	/// All components of the returned Qt color are clamped to the [0,1] range.
	operator QColor() const {
		return QColor::fromRgbF(
				qMin(qMax(r(), T(0)), T(1)),
				qMin(qMax(g(), T(0)), T(1)),
				qMin(qMax(b(), T(0)), T(1)),
				qMin(qMax(a(), T(0)), T(1)));
	}

	//////////////////////////// Component access //////////////////////////

	/// Returns the value of the red component of this color.
	Q_DECL_CONSTEXPR T r() const { return (*this)[0]; }

	/// Returns the value of the green component of this color.
	Q_DECL_CONSTEXPR T g() const { return (*this)[1]; }

	/// Returns the value of the blue component of this color.
	Q_DECL_CONSTEXPR T b() const { return (*this)[2]; }

	/// Returns the value of the alpha component of this color.
	Q_DECL_CONSTEXPR T a() const { return (*this)[3]; }

	/// Returns a reference to the red component of this color.
	T& r() { return (*this)[0]; }

	/// Returns a reference to the green component of this color.
	T& g() { return (*this)[1]; }

	/// Returns a reference to the blue component of this color.
	T& b() { return (*this)[2]; }

	/// Returns a reference to the alpha component of this color.
	T& a() { return (*this)[3]; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// Compares this color with another color for equality.
	/// \param c The other color.
	/// \return \c true if all four color components are exactly equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const ColorAT& c) const { return (r() == c.r() && g() == c.g() && b() == c.b() && a() == c.a()); }
	
	/// \brief Compares this color with another color for inequality.
	/// \param c The second color.
	/// \return \c true if any of the color components are not equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator!=(const ColorAT& c) const { return (r() != c.r() || g() != c.g() || b() != c.b() || a() != c.a()); }

	/// Adds the components of another color to this color.
	ColorAT& operator+=(const ColorAT& c) { r() += c.r(); g() += c.g(); b() += c.b(); a() += c.a(); return *this; }

	/// Multiplies the components of another color with the components of this color.
	ColorAT& operator*=(const ColorAT& c) { r() *= c.r(); g() *= c.g(); b() *= c.b(); a() *= c.a(); return *this; }

	/// Converts a vector to a color assigns it to this object.
	ColorAT& operator=(const Vector_4<T>& v) { r() = v.x(); g() = v.y(); b() = v.z(); a() = v.w(); return *this; }

	/// Ensures that none of the color components is greater than 1.
	/// Any component greater than 1 is changed to 1.
	/// \sa clampMin(), clampMinMax()
	void clampMax() { if(r() > T(1)) r() = T(1); if(g() > T(1)) g() = T(1); if(b() > T(1)) b() = T(1); if(a() > T(1)) a() = T(1); }

	/// Ensures that none of the color components is less than 0.
	/// Any color component less than 0 is set to 0.
	/// \sa clampMax(), clampMinMax()
	void clampMin() { if(r() < T(0)) r() = T(0); if(g() < T(0)) g() = T(0); if(b() < T(0)) b() = T(0); if(a() < T(0)) a() = T(0); }

	/// Ensures that all color components between 0 and 1.
	/// \sa clampMin(), clampMax()
	void clampMinMax() {
		for(typename std::array<T, 4>::size_type i = 0; i < std::array<T, 4>::size(); i++) {
			if((*this)[i] > T(1)) (*this)[i] = T(1);
			else if((*this)[i] < T(0)) (*this)[i] = T(0);
		}
	}

	/// Produces a string representation of the color.
	QString toString() const { return QString("(%1 %2 %3 %4)").arg(r()).arg(g()).arg(b()).arg(a()); }
};

/// Multiplies the four components of the color \a c with a scalar value \a s.
/// \relates ColorAT
template<typename T>
Q_DECL_CONSTEXPR inline ColorAT<T> operator*(T s, const ColorAT<T>& c) {
	return ColorAT<T>(c.r()*s, c.g()*s, c.b()*s, c.a()*s);
}

/// Multiplies the four components of the color \a c with a scalar value \a s.
/// \relates ColorAT
template<typename T>
Q_DECL_CONSTEXPR inline ColorAT<T> operator*(const ColorAT<T>& c, T s) {
	return ColorAT<T>(c.r()*s, c.g()*s, c.b()*s, c.a()*s);
}

/// Computes the component-wise sum of two colors.
/// \relates ColorAT
template<typename T>
Q_DECL_CONSTEXPR inline ColorAT<T> operator+(const ColorAT<T>& c1, const ColorAT<T>& c2) {
	return { c1.r()+c2.r(), c1.g()+c2.g(), c1.b()+c2.b(), c1.a()+c2.a() };
}

/// Computes the component-wise product of two colors.
/// \relates ColorAT
template<typename T>
Q_DECL_CONSTEXPR inline ColorAT<T> operator*(const ColorAT<T>& c1, const ColorAT<T>& c2) {
	return { c1.r()*c2.r(), c1.g()*c2.g(), c1.b()*c2.b(), c1.a()*c2.a() };
}

/// Prints a color to a text output stream.
/// \relates ColorAT
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const ColorAT<T>& c) {
	return os << c.r() << ' ' << c.g()  << ' ' << c.b() << ' ' << c.a();
}

/// Prints a color to a Qt debug stream.
/// \relates ColorAT
template<typename T>
inline QDebug operator<<(QDebug dbg, const ColorAT<T>& c) {
    dbg.nospace() << "(" << c.r() << " " << c.g() << " " << c.b() << " " << c.a() << ")";
    return dbg.space();
}

/// Writes a color to a binary output stream.
/// \relates ColorAT
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const ColorAT<T>& c) {
	return stream << c.r() << c.g() << c.b() << c.a();
}

/// Reads a color from a binary input stream.
/// \relates ColorAT
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, ColorAT<T>& c) {
	return stream >> c.r() >> c.g() >> c.b() >> c.a();
}

/// Writes a color to a Qt data stream.
/// \relates ColorAT
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const ColorAT<T>& c) {
	return stream << c.r() << c.g() << c.b() << c.a();
}

/// Reads a color from a Qt data stream.
/// \relates ColorAT
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, ColorAT<T>& c) {
	return stream >> c.r() >> c.g() >> c.b() >> c.a();
}

/**
 * \brief Instantiation of the ColorT class template with the default floating-point type.
 * \relates ColorT
 */
typedef ColorT<FloatType>	Color;

/**
 * \brief Instantiation of the ColorAT class template with the default floating-point type.
 * \relates ColorAT
 */
typedef ColorAT<FloatType>	ColorA;

// Type-specific OpenGL functions:
inline void glColor3(const ColorT<GLdouble>& c) { glColor3dv(c.data()); }
inline void glColor3(const ColorT<GLfloat>& c) { glColor3fv(c.data()); }
inline void glColor4(const ColorAT<GLdouble>& c) { glColor4dv(c.data()); }
inline void glColor4(const ColorAT<GLfloat>& c) { glColor4fv(c.data()); }

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Color);
Q_DECLARE_METATYPE(Ovito::ColorA);
Q_DECLARE_METATYPE(Ovito::Color*);
Q_DECLARE_METATYPE(Ovito::ColorA*);
Q_DECLARE_TYPEINFO(Ovito::Color, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ColorA, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Color*, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ColorA*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_COLOR_H
