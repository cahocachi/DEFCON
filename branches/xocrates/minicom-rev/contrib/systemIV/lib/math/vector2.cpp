#include "lib/universal_include.h"

#include "vector2.h"
#include "vector3.h"

#include <math.h>
#include <float.h>
#include "math_utils.h"
#include "lib/math/fixed.h"


template <>
bool Vector2<float>::Compare(Vector2<float> const &b) const
{
	return ( NearlyEquals(x, b.x) &&
			 NearlyEquals(y, b.y) );
}

template <>
bool Vector2<Fixed>::Compare(Vector2<Fixed> const &b) const
{
	return ( x == b.x && y == b.y );
}

template <> template <>
Vector2<float>::Vector2(Vector2<Fixed> const &_v)
{
	x = _v.x.DoubleValue();
	y = _v.y.DoubleValue();
}

template <> template <>
Vector2<Fixed>::Vector2(Vector2<float> const &_v)
: x(Fixed::FromDouble(_v.x)), y(Fixed::FromDouble(_v.y))
{
}

// *******************
//  Private Functions
// *******************

// *** Compare
bool Vector2Float::Compare(Vector2Float const &b) const
{
	return ( (fabsf(x - b.x) < FLT_EPSILON) &&
			 (fabsf(y - b.y) < FLT_EPSILON) );
}


// ******************
//  Public Functions
// ******************

// Constructor
Vector2Float::Vector2Float()
:	x(0.0f),
	y(0.0f) 
{
}


// Constructor
Vector2Float::Vector2Float(Vector3<float> const &v)
:	x(v.x),
	y(v.z)
{
}


// Constructor
Vector2Float::Vector2Float(float _x, float _y)
:	x(_x),
	y(_y) 
{
}


void Vector2Float::Zero()
{
	x = y = 0.0f;
}


void Vector2Float::Set(float _x, float _y)
{
	x = _x;
	y = _y;
}


// Cross Product
float Vector2Float::operator ^ (Vector2Float const &b) const
{
	return x*b.y - y*b.x;
}


// Dot Product
float Vector2Float::operator * (Vector2Float const &b) const
{
	return (x * b.x + y * b.y);
}


// Negate
Vector2Float Vector2Float::operator - () const
{
	return Vector2Float(-x, -y);
}


Vector2Float Vector2Float::operator + (Vector2Float const &b) const
{
	return Vector2Float(x + b.x, y + b.y);
}


Vector2Float Vector2Float::operator - (Vector2Float const &b) const
{
	return Vector2Float(x - b.x, y - b.y);
}


Vector2Float Vector2Float::operator * (float const b) const
{
	return Vector2Float(x * b, y * b);
}


Vector2Float Vector2Float::operator / (float const b) const
{
	float multiplier = 1.0f / b;
	return Vector2Float(x * multiplier, y * multiplier);
}


void Vector2Float::operator = (Vector2Float const &b)
{
	x = b.x;
	y = b.y;
}


// Assign from a Vector3 - throws away Y value of Vector3
void Vector2Float::operator = (Vector3<float> const &b)
{
	x = b.x;
	y = b.z;
}


// Scale
void Vector2Float::operator *= (float const b)
{
	x *= b;
	y *= b;
}


// Scale
void Vector2Float::operator /= (float const b)
{
	float multiplier = 1.0f / b;
	x *= multiplier;
	y *= multiplier;
}


void Vector2Float::operator += (Vector2Float const &b)
{
	x += b.x;
	y += b.y;
}


void Vector2Float::operator -= (Vector2Float const &b)
{
	x -= b.x;
	y -= b.y;
}


Vector2Float const &Vector2Float::Normalise()
{
	float lenSqrd = x*x + y*y;
	if (lenSqrd > 0.0f)
	{
		float invLen = 1.0f / sqrtf(lenSqrd);
		x *= invLen;
		y *= invLen;
	}
	else
	{
		x = 0.0f;
        y = 1.0f;
	}

	return *this;
}


void Vector2Float::SetLength(float _len)
{
	float scaler = _len / Mag();
	x *= scaler;
	y *= scaler;
}


float Vector2Float::Mag() const
{
    return sqrtf(x * x + y * y);
}


float Vector2Float::MagSquared() const
{
    return x * x + y * y;
}


bool Vector2Float::operator == (Vector2Float const &b) const
{
	return Compare(b);
}


bool Vector2Float::operator != (Vector2Float const &b) const
{
	return !Compare(b);
}


float *Vector2Float::GetData()
{
	return &x;
}
