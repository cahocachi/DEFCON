#ifndef INCLUDED_VECTOR2_H
#define INCLUDED_VECTOR2_H

#include <math.h>

// #include "lib/math/vector3.h"
#include "lib/math/math_utils.h"
#include "lib/math/fixed.h"

template <class T> class Vector3;

template <class T> class Vector2
{
private:
	bool Compare(Vector2 const &b) const;

public:
	T x, y;
	
	// *** Class methods ***
	static Vector2 const & ZeroVector()
	{
        static Vector2 zero(0, 0);
        return zero;
	}

	// *** Constructors ***
	Vector2()
	:	x(0), y(0)
	{
	}

	inline Vector2(T _x, T _y)
	:	x(_x), y(_y)
	{
	}

    explicit Vector2(Vector3<T> const &_b)
	:	x(_b.x), y(_b.z)
	{
	}

	template <class T2>
    explicit
	Vector2(Vector2<T2> const &_v);

	void Zero()
	{
		x = y = 0;
	}

	void Set(float _x, float _y)
	{
		x = _x;
		y = _y;
	}
	
	T operator ^ (Vector2<T> const &b) const
	{
		return x*b.y - y*b.x;
	}

	T operator * (Vector2<T> const &b) const
	{
		return x*b.x + y*b.y;
	}

	Vector2<T> operator - () const
	{
		return Vector2(-x, -y);
	}

	Vector2<T> operator + (Vector2<T> const &b) const
	{
		return Vector2(x + b.x, y + b.y);
	}

	Vector2<T> operator - (Vector2<T> const &b) const
	{
		return Vector2(x - b.x, y - b.y);
	}

	Vector2<T> operator * (T const b) const
	{
		return Vector2(x * b, y * b);
	}

	Vector2<T> operator / (T const b) const
	{
		T multiplier = 1 / b;
		return Vector2(x * multiplier, y * multiplier);
	}

	Vector2<T> const &operator *= (T const b)
	{
		x *= b;
		y *= b;
		return *this;
	}

	Vector2<T> const &operator /= (T const b)
	{
		T multiplier = 1 / b;
		x *= multiplier;
		y *= multiplier;
		return *this;
	}

	Vector2<T> const &operator += (Vector2<T> const &b)
	{
		x += b.x;
		y += b.y;
		return *this;
	}

	Vector2<T> const &operator -= (Vector2<T> const &b)
	{
		x -= b.x;
		y -= b.y;
		return *this;
	}

	Vector2<T> const &Normalise()
	{
		T lenSqrd = x*x + y*y;
		if (lenSqrd > 0)
		{
			T invLen = 1 / sqrt(lenSqrd);
			x *= invLen;
			y *= invLen;
		}
		else
		{
			x = 0;
            y = 1;
		}

		return *this;
	}

	Vector2 const &SetLength(T _len)
	{
		T mag = Mag();
		if (NearlyEquals(mag, 0))
		{	
			x = _len;
			return *this;
		}

		T scaler = _len / Mag();
		*this *= scaler;
		return *this;
	}


	bool operator == (Vector2 const &b) const // Uses FLT_EPSILON for floats
	{
		return Compare(b);
	}
	
	bool operator != (Vector2 const &b) const // Uses FLT_EPSILON for floats
	{
		return !Compare(b);
	}

	void RotateAroundZ (T _angle)
	{
		T s = sin( _angle );
		T c = cos( _angle );
        
        Vector2 result( c*x - s*y, c*y + s*x );
        *this = result;
	}
	
	T Mag() const
	{
        T magSqd = MagSquared();     
        return sqrt(magSqd);		
	}

	T MagSquared() const
	{
		return x * x + y * y;
	}

	float *GetData()
	{
		return &x;
	}

	float const *GetDataConst() const
	{
		return &x;
	}
};

// Operator * between float and Vector2
template <class T>
inline Vector2<T> operator * (	T _scale, Vector2<T> const &_v )
{
	return _v * _scale;
}

// the old, non-template class
class Vector2Float
{
private:
	bool Compare(Vector2Float const &b) const;

public:
	float x, y;

	Vector2Float();
	Vector2Float(Vector3<float> const &);
	Vector2Float(float _x, float _y);

	void	Zero();
    void	Set	(float _x, float _y);

	float	operator ^ (Vector2Float const &b) const;	// Cross product
	float   operator * (Vector2Float const &b) const;	// Dot product

	Vector2Float operator - () const;					// Negate
	Vector2Float operator + (Vector2Float const &b) const;
	Vector2Float operator - (Vector2Float const &b) const;
	Vector2Float operator * (float const b) const;		// Scale
	Vector2Float operator / (float const b) const;

    void	operator = (Vector2Float const &b);
    void	operator = (Vector3<float> const &b);
	void	operator *= (float const b);
	void	operator /= (float const b);
	void	operator += (Vector2Float const &b);
	void	operator -= (Vector2Float const &b);

	bool operator == (Vector2Float const &b) const;		// Uses FLT_EPSILON
	bool operator != (Vector2Float const &b) const;		// Uses FLT_EPSILON

	Vector2Float const &Normalise();
	void	SetLength		(float _len);
	
    float	Mag				() const;
	float	MagSquared		() const;

	float  *GetData			();
};


// Operator * between float and Vector2Float
inline Vector2Float operator * (	float _scale, Vector2Float const &_v )
{
	return _v * _scale;
}


#endif
