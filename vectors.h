#pragma once

#ifndef __VECTORS_H__
#define __VECTORS_H__


template <typename T>
class vector2
{
public:
	T	x, y;

	vector2( T x, T y )
	{
		this->x = x;
		this->y = y;
	}

	vector2( const vector2<T> &v )
	{
		this->x = v.x;
		this->y = v.y;
	}

	vector2<T> operator = ( const vector2<T> &v )
	{
		this->x = v.x;
		this->y = v.y;
		return *this;
	}

	bool operator == ( const vector2<T> & v )
	{
		if ( this->x == v.x && this->y == v.y )
			return true;
		else
			return false;
	}

	vector2<T> operator + ( vector2<T> v )
	{
		vector2<T> r;
		r.x = this->x + v.x;
		r.y = this->y + v.y;
		return r;
	}

	vector2<T> operator - ( vector2<T> v )
	{
		vector2<T> r;
		r.x = this->x - v.x;
		r.y = this->y - v.y;
		return r;
	}

	vector2<T> operator * ( vector2<T> v )
	{
		vector2<T> r;
		r.x = this->x * v.x;
		r.y = this->y * v.y;
		return r;
	}

	vector2<T> operator / ( vector2<T> v )
	{
		vector2<T> r;
		r.x = this->x / v.x;
		r.y = this->y / v.y;
		return r;
	}

};

typedef vector2<float>				v2f32;
typedef vector2<double>				v2f64;
typedef vector2<char>				v2s8;
typedef vector2<unsigned char>		v2u8;
typedef vector2<short>				v2s16;
typedef vector2<unsigned short>		v2u16;
typedef vector2<long>				v2s32;
typedef vector2<unsigned long>		v2u32;
typedef vector2<long long>			v2s64;
typedef vector2<long unsigned long>	v2u64;

#endif // __VECTORS_H__
