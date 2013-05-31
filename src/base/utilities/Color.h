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
 * \file Color.h 
 * \brief Contains the definitions of the Ovito::Color and Ovito::ColorA classes.
 */

#ifndef __OVITO_COLOR_H
#define __OVITO_COLOR_H

#include <base/Base.h>
#include <base/linalg/Vector3.h>
#include <base/io/LoadStream.h>
#include <base/io/SaveStream.h>

namespace Ovito {

/**
 * \brief A color class with floating-point RGB values.
 * 
 * The Color class stores three floating-point values, one for the red
 * component, one for the green component, and one for the blue component.
 * 
 * \author Alexander Stukowski
 * \sa ColorA
 */
template<typename T>
class ColorT : public std::array<T, 3>
{
public:

	/// \brief Constructs a color structure without initializing its component.
	/// \note All components are left uninitialized by this constructor and will therefore have an undefined value!
	ColorT() {}
	
	/// \brief Initializes the color width the given red, green and blue components.
	/// \param red The red value of the RGB color in the range 0 to 1.
	/// \param green The green value of the RGB color in the range 0 to 1.
	/// \param blue The blue value of the RGB color in the range 0 to 1.
	constexpr ColorT(T red, T green, T blue) : std::array<T, 3>({red, green, blue}) {}
	
	/// \brief Converts a vector to a color structure.
	/// \param v The input vector. Its X, Y and Z components are taken as red, green and blue components 
	///          respectively.
	constexpr explicit ColorT(const Vector_3<T>& v) : std::array<T, 3>(v) {}

	/// \brief Conversion from a Qt color object.
	/// \param c The Qt color to convert to a floating-point representation.
	constexpr explicit ColorT(const QColor& c) : std::array<T, 3>({c.redF(), c.greenF(), c.blueF()}) {}

	/// \brief Sets all components to zero.
	void setBlack() { r() = g() = b() = 0; }
	
	/// \brief Sets all components to one.
	void setWhite() { r() = g() = b() = 1; }

	/// \brief Converts this color to a Qt color object.
	/// \return A Qt color object. All color components are clamped to the [0,1] range before the conversion.
	explicit operator QColor() const {
		return QColor::fromRgbF(
				qMin(qMax(r(), T(0)), T(1)),
				qMin(qMax(g(), T(0)), T(1)),
				qMin(qMax(b(), T(0)), T(1)));
	}
	
	/// \brief Converts this color to a vector with three components.
	/// \return A vector.
	explicit constexpr operator const Vector_3<T>&() const { return reinterpret_cast<const Vector_3<T>&>(*this); }
	
	//////////////////////////// Component access //////////////////////////

	/// \brief Returns the value of the red component of this color.
	constexpr const T& r() const { return (*this)[0]; }

	/// \brief Returns the value of the green component of this color.
	constexpr const T& g() const { return (*this)[1]; }

	/// \brief Returns the value of the blue component of this color.
	constexpr const T& b() const { return (*this)[2]; }

	/// \brief Returns a reference to the red component of this color.
	T& r() { return (*this)[0]; }

	/// \brief Returns a reference to the green component of this color.
	T& g() { return (*this)[1]; }

	/// \brief Returns a reference to the blue component of this color.
	T& b() { return (*this)[2]; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Compares this color with another color for equality.
	/// \param rgb The second color.
	/// \return \c true if all three color components are exactly equal; \c false otherwise.
	constexpr bool operator==(const ColorT& rgb) const { return (r() == rgb.r() && g() == rgb.g() && b() == rgb.b()); }
	
	/// \brief Compares this color with another color for inequality.
	/// \param rgb The second color.
	/// \return \c true if any of the three color components are not equal; \c false otherwise.		
	constexpr bool operator!=(const ColorT& rgb) const { return (r() != rgb.r() || g() != rgb.g() || b() != rgb.b()); }

	/// Adds the components of another color to this color.
	ColorT& operator+=(const ColorT& c) { r() += c.r(); g() += c.g(); b() += c.b(); return *this; }
	
	/// Multiplies the components of another color with the components of this color.
	ColorT& operator*=(const ColorT& c) { r() *= c.r(); g() *= c.g(); b() *= c.b(); return *this; }

	/// Converts a vector to a color assigns it to this object.
	ColorT& operator=(const Vector_3<T>& v) { r() = v.x(); g() = v.y(); b() = v.z(); return *this; }
	
	/// \brief Ensures that all components are not greater than one.
	/// 
	/// If any of the R, G and B components is greater than 1 then it is set to 1.
	/// \sa clampMin(), clampMinMax()
	void clampMax() { if(r() > 1.0) r() = 1.0; if(g() > 1.0) g() = 1.0; if(b() > 1.0) b() = 1.0; }
	
	/// \brief Ensures that all components are not less than zero.
	/// 
	/// If any of the R, G and B components is less than 0 then it is set to 0.
	/// \sa clampMax(), clampMinMax()
	void clampMin() { if(r() < 0.0) r() = 0.0; if(g() < 0.0) g() = 0.0; if(b() < 0.0) b() = 0.0; }
	
	/// \brief Ensures that all components are not greater than one and not less than zero.
	/// \sa clampMin(), clampMax()
	void clampMinMax() {
		for(typename std::array<T, 3>::size_type i = 0; i < std::array<T, 3>::size(); i++) {
			if((*this)[i] > 1.0) (*this)[i] = 1;
			else if((*this)[i] < 0.0) (*this)[i] = 0;
		}
	}

	/// \brief Converts a color from hue, saturation, value representation to RGB representation.
	/// \param hue The hue value between zero and one.
	/// \param saturation The saturation value between zero and one.
	/// \param value The value of the color between zero and one.
	/// \return The color in RGB representation.
	static ColorT fromHSV(T hue, T saturation, T value) {
		if(saturation == 0.0) {
			return ColorT(value, value, value);
		}
		else {
			T f, p, q, t;
			int i;
			if(hue >= 1.0 || hue < 0.0) hue = 0.0;
			hue *= 6.0;
			i = (int)floor(hue);
			f = hue - (T)i;
			p = value * (1.0 - saturation);
			q = value * (1.0 - (saturation * f));
			t = value * (1.0 - (saturation * (1.0 - f)));
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

	///////////////////////////////// Information ////////////////////////////////

	/// \brief Gives a string representation of this color.
	/// \return A string that contains the components of the color. 
	QString toString() const { return QString("(%1 %2 %3)").arg(r()).arg(g()).arg(b()); }
};

/// Multiplies all three components of a color with a scalar value.
template<typename T>
constexpr inline ColorT<T> operator*(T f, const ColorT<T>& c) {
	return ColorT<T>(c.r()*f, c.g()*f, c.b()*f);
}

/// Multiplies all three components of a color with a scalar value.
template<typename T>
constexpr inline ColorT<T> operator*(const ColorT<T>& c, T f) {
	return ColorT<T>(c.r()*f, c.g()*f, c.b()*f);
}

/// Computes the sum of each color component.
template<typename T>
constexpr inline ColorT<T> operator+(const ColorT<T>& c1, const ColorT<T>& c2) {
	return { c1.r()+c2.r(), c1.g()+c2.g(), c1.b()+c2.b() };
}

/// Computes the product of each color component.
template<typename T>
constexpr inline ColorT<T> operator*(const ColorT<T>& c1, const ColorT<T>& c2) {
	return { c1.r()*c2.r(), c1.g()*c2.g(), c1.b()*c2.b() };
}

/// \brief Writes a color to a text output stream.
/// \param os The destination stream.
/// \param c The color to write.
/// \return The destination stream \a os.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const ColorT<T>& c) {
	return os << c.r() << ' ' << c.g()  << ' ' << c.b();
}

/// \brief Writes a color to a binary output stream.
/// \param stream The destination stream.
/// \param c The color to write.
/// \return The destination \a stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const ColorT<T>& c) {
	return stream << c.r() << c.g() << c.b();
}

/// \brief Reads a color from an input stream.
/// \param stream The source stream.
/// \param c[out] The destination variable.
/// \return The source \a stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, ColorT<T>& c) {
	return stream >> c.r() >> c.g() >> c.b();
}

/**
 * \brief A color class with floating-point RGBA values.
 * 
 * The ColorA class stores four floating-point values, one for the red
 * component, one for the green component, one for the blue component and
 * one for the alpha component which controls the opacity of the color.
 * 
 * \author Alexander Stukowski
 * \sa Color
 */
template<typename T>
class ColorAT : public std::array<T, 4>
{
public:

	/// \brief Constructs a color structure without initializing its component.
	/// \note All components are left uninitialized by this constructor and will therefore have an undefined value!
	ColorAT() {}

	/// \brief Initializes the color width the given red, green and blue components.
	/// \param red The red value of the RGB color in the range 0 to 1.
	/// \param green The green value of the RGB color in the range 0 to 1.
	/// \param blue The blue value of the RGB color in the range 0 to 1.
	constexpr ColorAT(T red, T green, T blue, T alpha = 1) : std::array<T, 4>({red, green, blue, alpha}) {}

	/// \brief Converts a vector to a color structure.
	/// \param v The input vector. Its X, Y and Z components are taken as red, green and blue components
	///          respectively.
	constexpr explicit ColorAT(const Vector_4<T>& v) : std::array<T, 4>(v) {}

	/// \brief Conversion from a Qt color object.
	/// \param c The Qt color to convert to a floating-point representation.
	constexpr explicit ColorAT(const QColor& c) : std::array<T, 4>({c.redF(), c.greenF(), c.blueF(), 1}) {}

	/// \brief Sets all components to zero.
	void setBlack() { r() = g() = b() = 0; a() = 1; }

	/// \brief Sets all components to one.
	void setWhite() { r() = g() = b() = a() = 1; }

	/// \brief Converts this color to a vector with three components.
	/// \return A vector.
	explicit constexpr operator const Vector_4<T>&() const { return reinterpret_cast<const Vector_4<T>&>(*this); }

	//////////////////////////// Component access //////////////////////////

	/// \brief Returns the value of the red component of this color.
	constexpr const T& r() const { return (*this)[0]; }

	/// \brief Returns the value of the green component of this color.
	constexpr const T& g() const { return (*this)[1]; }

	/// \brief Returns the value of the blue component of this color.
	constexpr const T& b() const { return (*this)[2]; }

	/// \brief Returns the value of the alpha component of this color.
	constexpr const T& a() const { return (*this)[3]; }

	/// \brief Returns a reference to the red component of this color.
	T& r() { return (*this)[0]; }

	/// \brief Returns a reference to the green component of this color.
	T& g() { return (*this)[1]; }

	/// \brief Returns a reference to the blue component of this color.
	T& b() { return (*this)[2]; }

	/// \brief Returns a reference to the alpha component of this color.
	T& a() { return (*this)[3]; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Compares this color with another color for equality.
	/// \param rgb The second color.
	/// \return \c true if all three color components are exactly equal; \c false otherwise.
	constexpr bool operator==(const ColorAT& rgb) const { return (r() == rgb.r() && g() == rgb.g() && b() == rgb.b() && a() == rgb.a()); }
	
	/// \brief Compares this color with another color for inequality.
	/// \param rgb The second color.
	/// \return \c true if any of the three color components are not equal; \c false otherwise.
	constexpr bool operator!=(const ColorAT& rgb) const { return (r() != rgb.r() || g() != rgb.g() || b() != rgb.b() || a() != rgb.a()); }

	/// Adds the components of another color to this color.
	ColorAT& operator+=(const ColorAT& c) { r() += c.r(); g() += c.g(); b() += c.b(); a() += c.a(); return *this; }

	/// Multiplies the components of another color with the components of this color.
	ColorAT& operator*=(const ColorAT& c) { r() *= c.r(); g() *= c.g(); b() *= c.b(); a() *= c.a(); return *this; }

	/// Converts a vector to a color assigns it to this object.
	ColorAT& operator=(const Vector_4<T>& v) { r() = v.x(); g() = v.y(); b() = v.z(); a() = v.w(); return *this; }

	/// \brief Ensures that all components are not greater than one.
	/// 
	/// If any of the R, G and B components is greater than 1 then it is set to 1.
	/// \sa clampMin(), clampMinMax()
	void clampMax() { if(r() > 1.0) r() = 1.0; if(g() > 1.0) g() = 1.0; if(b() > 1.0) b() = 1.0; if(a() > 1.0) a() = 1.0; }

	/// \brief Ensures that all components are not less than zero.
	/// 
	/// If any of the R, G and B components is less than 0 then it is set to 0.
	/// \sa clampMax(), clampMinMax()
	void clampMin() { if(r() < 0.0) r() = 0.0; if(g() < 0.0) g() = 0.0; if(b() < 0.0) b() = 0.0; if(a() < 0.0) a() = 0.0; }

	/// \brief Ensures that all components are not greater than one and not less than zero.
	/// \sa clampMin(), clampMax()
	void clampMinMax() {
		for(typename std::array<T, 4>::size_type i = 0; i < std::array<T, 4>::size(); i++) {
			if((*this)[i] > 1.0) (*this)[i] = 1;
			else if((*this)[i] < 0.0) (*this)[i] = 0;
		}
	}

	///////////////////////////////// Information ////////////////////////////////

	/// \brief Gives a string representation of this color.
	/// \return A string that contains the components of the color. 
	QString toString() const { return QString("(%1 %2 %3 %4)").arg(r()).arg(g()).arg(b()).arg(a()); }
};

/// Multiplies all three components of a color with a scalar value.
template<typename T>
constexpr inline ColorAT<T> operator*(T f, const ColorAT<T>& c) {
	return ColorAT<T>(c.r()*f, c.g()*f, c.b()*f, c.a()*f);
}

/// Multiplies all three components of a color with a scalar value.
template<typename T>
constexpr inline ColorAT<T> operator*(const ColorAT<T>& c, T f) {
	return ColorAT<T>(c.r()*f, c.g()*f, c.b()*f, c.a()*f);
}

/// Computes the sum of each color component.
template<typename T>
constexpr inline ColorAT<T> operator+(const ColorAT<T>& c1, const ColorAT<T>& c2) {
	return { c1.r()+c2.r(), c1.g()+c2.g(), c1.b()+c2.b(), c1.a()+c2.a() };
}

/// Computes the product of each color component.
template<typename T>
constexpr inline ColorAT<T> operator*(const ColorAT<T>& c1, const ColorAT<T>& c2) {
	return { c1.r()*c2.r(), c1.g()*c2.g(), c1.b()*c2.b(), c1.a()*c2.a() };
}

/// \brief Writes a color to a text output stream.
/// \param os The destination stream.
/// \param c The color to write.
/// \return The destination stream \a os.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const ColorAT<T>& c) {
	return os << c.r() << ' ' << c.g()  << ' ' << c.b() << ' ' << c.a();
}

/// \brief Writes a color to a binary output stream.
/// \param stream The destination stream.
/// \param c The color to write.
/// \return The destination \a stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const ColorAT<T>& c) {
	return stream << c.r() << c.g() << c.b() << c.a();
}

/// \brief Reads a color from an input stream.
/// \param stream The source stream.
/// \param c[out] The destination variable.
/// \return The source \a stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, ColorAT<T>& c) {
	return stream >> c.r() >> c.g() >> c.b() >> c.a();
}

/**
 * \fn typedef Color
 * \brief Template class instance of the ColorT template.
 */
typedef ColorT<FloatType>	Color;

/**
 * \fn typedef ColorA
 * \brief Template class instance of the ColorAT template.
 */
typedef ColorAT<FloatType>	ColorA;

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Color)
Q_DECLARE_METATYPE(Ovito::ColorA)
Q_DECLARE_TYPEINFO(Ovito::Color, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ColorA, Q_PRIMITIVE_TYPE);

#endif // __OVITO_COLOR_H
