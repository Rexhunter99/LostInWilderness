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

typedef vector2<float>				vector2f;
typedef vector2<double>				vector2d;
typedef vector2<char>				vector2c;
typedef vector2<unsigned char>		vector2ub;
typedef vector2<short>				vector2s;
typedef vector2<unsigned short>		vector2us;
typedef vector2<long>				vector2i;
typedef vector2<unsigned long>		vector2ui;

template <typename T>
class vector3
{
public:
	T	x, y, z;

	vector3( T x, T y, T z )
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	vector3( float a )
	{
		this->x = a;
		this->y = a;
		this->z = a;
	}

	vector3( const vector2<T> &v, float z )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = z;
	}

	vector3( const vector3<T> &v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	vector3<T> operator = ( const vector3<T> &v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		return *this;
	}

	bool operator == ( const vector3<T> & v )
	{
		if ( this->x == v.x && this->y == v.y && this->z == v.z )
			return true;
		else
			return false;
	}

	vector3<T> operator + ( vector3<T> v )
	{
		vector3<T> r;
		r.x = this->x + v.x;
		r.y = this->y + v.y;
		r.z = this->z + v.z;
		return r;
	}

	vector3<T> operator - ( vector3<T> v )
	{
		vector3<T> r;
		r.x = this->x - v.x;
		r.y = this->y - v.y;
		r.z = this->z - v.z;
		return r;
	}

	vector3<T> operator * ( vector3<T> v )
	{
		vector3<T> r;
		r.x = this->x * v.x;
		r.y = this->y * v.y;
		r.z = this->z * v.z;
		return r;
	}

	vector3<T> operator / ( vector3<T> v )
	{
		vector3<T> r;
		r.x = this->x / v.x;
		r.y = this->y / v.y;
		r.z = this->z / v.z;
		return r;
	}

};

typedef vector3<float>				vector3f;
typedef vector3<double>				vector3d;
typedef vector3<char>				vector3c;
typedef vector3<unsigned char>		vector3ub;
typedef vector3<short>				vector3s;
typedef vector3<unsigned short>		vector3us;
typedef vector3<long>				vector3i;
typedef vector3<unsigned long>		vector3ui;

#endif // __VECTORS_H__
