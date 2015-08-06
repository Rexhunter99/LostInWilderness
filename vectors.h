#pragma once

#ifndef __VECTORS_H__
#define __VECTORS_H__

// -- Include for the INT_MAX and etc definitions
#include <climits>


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

	vector3() { }

	vector3( T x, T y, T z )
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	vector3( T a )
	{
		this->x = a;
		this->y = a;
		this->z = a;
	}

	vector3( const vector2<T> &v, T z )
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

class vector3i
{
public:
	int64_t	x, y, z;

	vector3i( )
	{
		this->x = 0U;
		this->y = 0U;
		this->z = 0U;
	}

	vector3i( uint64_t x, uint64_t y, uint64_t z )
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	vector3i( uint64_t a )
	{
		this->x = a;
		this->y = a;
		this->z = a;
	}

	vector3i( const vector3i &v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	vector3i operator = ( const vector3i &v )
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		return *this;
	}

	bool operator == ( const vector3i & v ) const
	{
		return ( this->x == v.x && this->y == v.y && this->z == v.z );
	}

	bool operator < ( const vector3i & v ) const
	{
		return ( this->x < v.x && this->y < v.y && this->z < v.z );
	}

	bool operator > ( const vector3i & v ) const
	{
		return ( this->x > v.x && this->y > v.y && this->z > v.z );
	}

	bool operator <= ( const vector3i & v ) const
	{
		return ( this->x <= v.x && this->y <= v.y && this->z <= v.z );
	}

	bool operator >= ( const vector3i & v ) const
	{
		return ( this->x >= v.x && this->y >= v.y && this->z >= v.z );
	}

	vector3i operator + ( vector3i v )
	{
		vector3i r;
		r.x = this->x + v.x;
		r.y = this->y + v.y;
		r.z = this->z + v.z;
		return r;
	}

	vector3i operator - ( vector3i v )
	{
		vector3i r;
		r.x = this->x - v.x;
		r.y = this->y - v.y;
		r.z = this->z - v.z;
		return r;
	}

	vector3i operator * ( vector3i v )
	{
		vector3i r;
		r.x = this->x * v.x;
		r.y = this->y * v.y;
		r.z = this->z * v.z;
		return r;
	}

	vector3i operator / ( vector3i v )
	{
		vector3i r;
		r.x = this->x / v.x;
		r.y = this->y / v.y;
		r.z = this->z / v.z;
		return r;
	}

};

#include <unordered_map>

class vector3i_hash
{
public:

	size_t operator()( const vector3i &v ) const
	{
		return std::hash<int64_t>()(v.x) ^ std::hash<int64_t>()(v.y) ^ std::hash<int64_t>()(v.z);
	}

	bool operator()( const vector3i &lhs, const vector3i &rhs ) const
	{
		return ( lhs == rhs );
	}
};


#endif // __VECTORS_H__
