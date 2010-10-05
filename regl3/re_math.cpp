/*****************************************************************************
 * reMath contains vector and matrix class
 *
 * Author: rewolf & Juzz Wuzz 2010
 * andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <cmath>
using namespace std;

#include "re_math.h"


namespace reMath{

	/**************************************************************************
	 * vector2 
	 **************************************************************************/

	//-------------------------------------------------------------------------
	// Constructs a zero vector
	vector2::vector2(){
		memset((void*)v, 0, 2*sizeof(float));
	}

	//-------------------------------------------------------------------------
	// Constructs a vector from the given components
	vector2::vector2(float _x, float _y){
		x=_x; y=_y;
	}

	//-------------------------------------------------------------------------
	// Constructs a vector from the given components
	vector2::vector2(const float elems[2]){
		memcpy((void*)v, (void*)elems, sizeof(v));
	}

	//-------------------------------------------------------------------------
	// Constructs a copy of the given vector
	vector2::vector2(const vector2& copy){
		memcpy((void*)v, (void*)copy.v, sizeof(v));
	}

	//-------------------------------------------------------------------------
	// OPERATOR+ performs vector addition
	vector2 
	vector2::operator +(const vector2 &vec) const{
		vector2 out;
		out.x = x + vec.x;
		out.y = y + vec.y;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR- performs vector subtraction
	vector2 
	vector2::operator -(const vector2 &vec) const{
		vector2 out;
		out.x = x - vec.x;
		out.y = y - vec.y;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR- performs vector negation.
	vector2 
	vector2::operator -() const{
		vector2 out;
		out.x = - x;
		out.y = - y;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs vector-scalar multiplication
	vector2 
	vector2::operator *(float scalar) const{
		vector2 out;
		out.x = x * scalar;
		out.y = y * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs scalar-vector multiplication
	vector2 
	operator *(float scalar, vector2 &vec){
		vector2 out;
		out.x = vec.x * scalar;
		out.y = vec.y * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR/ performs vector-scalar division
	vector2 
	vector2::operator /(float scalar) const{
		float div = (1.0f / scalar);
		vector2 out;
		out.x = x * div;
		out.y = y * div;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR/ performs scalar-vector division
	vector2 
	operator /(float scalar, vector2 &vec){
		float div = (1.0f / scalar);
		vector2 out;
		out.x = vec.x * div;
		out.y = vec.y * div;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR+= performs (in-place) vector addition
	vector2& 
	vector2::operator +=(const vector2 &vec){
		x += vec.x;
		y += vec.y;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR-= performs (in-place) vector subtraction
	vector2& 
	vector2::operator -=(const vector2 &vec){
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR*= performs (in-place) scalar multiplication
	vector2& 
	vector2::operator *=(float scalar){
		x *= scalar;
		y *= scalar;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR/= performs (in-place) scalar division
	vector2& 
	vector2::operator /=(float scalar){
		float div = (1.0f / scalar);
		x *= div;
		y *= div;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR== tests whether this vector is equal to another
	bool 
	vector2::operator ==(const vector2& vec) const{
		if (x!=vec.x || y!=vec.y)
			return false;
		return true;
	}

	//-------------------------------------------------------------------------
	// OPERATOR!= tests whether this vector is not equal to another
	bool
	vector2::operator !=(const vector2& vec) const{
		if (x!=vec.x || y!=vec.y)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR< tests whether this vector is less than another
	bool
	vector2::operator <(const vector2& vec) const{
		if (x < vec.x && y < vec.y)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR<= tests whether this vector is less than or equal to another
	bool
	vector2::operator <=(const vector2& vec) const{
		if (x <= vec.x && y <= vec.y)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR> tests whether this vector is greater than another
	bool
	vector2::operator >(const vector2& vec) const{
		if (x > vec.x && y > vec.y)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR>= tests whether this vector is greater than or equal to another
	bool
	vector2::operator >=(const vector2& vec) const{
		if (x >= vec.x && y >= vec.y)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR[] returns a reference to the element at the given index
	float& 
	vector2::operator [](int idx){
		return v[idx];
	}

	//-------------------------------------------------------------------------
	// STR returns a formatted string representation of the vector
	string 
	vector2::str(){
		char out[64];
		sprintf(out,"<%.2f, %.2f>",x,y);
		return string(out);
	}

	//-------------------------------------------------------------------------
	// SET sets the components of the vector to those given
	void
	vector2::set(float _x, float _y){
		x=_x; y=_y;
	}

	//-------------------------------------------------------------------------
	// MAG returns the length/magnitude of the vector
	float 
	vector2::Mag()const{
		return sqrt(x*x+y*y);
	}

	//-------------------------------------------------------------------------
	// MAG2 returns the magnitude squared
	float 
	vector2::Mag2()const{
		return x*x+y*y;
	}

	//-------------------------------------------------------------------------
	// DOT returns the dot product with the given vector
	float 
	vector2::Dot(const vector2 & v) const{
		return x*v.x + y*v.y;
	}

	//-------------------------------------------------------------------------
	// NORMALIZE scales this vector to magnitude 1
	void 
	vector2::Normalize(){
		float magInv = 1.0f/Mag();
		x*=magInv;
		y*=magInv;
	}

	//-------------------------------------------------------------------------
	// Returns a normalized version of this vector
	vector2 
	vector2::GetUnit()const{
		float magInv = 1.0f/Mag();
		return vector2(x*magInv, y*magInv);
	}

	//-------------------------------------------------------------------------
	// SETFLOOR floors the values in the vector
	void 
	vector2::SetFloor(){
		x=floorf(x);
		y=floorf(y);
	}

	//-------------------------------------------------------------------------
	// SETABS sets the values to be their absolute form
	void 
	vector2::SetAbs(){
		x=abs(x);
		y=abs(y);
	}



	/**************************************************************************
	 * vector3 
	 **************************************************************************/

	//-------------------------------------------------------------------------
	// Constructs a zero vector
	vector3::vector3(){
		memset((void*)v, 0, 3*sizeof(float));
	}

	//-------------------------------------------------------------------------
	// Constructs a vector from the given components
	vector3::vector3(float _x, float _y, float _z){
		x=_x; y=_y; z=_z;
	}

	//-------------------------------------------------------------------------
	// Constructs a vector from the given components
	vector3::vector3(const float elems[3]){
		memcpy((void*)v, (void*)elems, sizeof(v));
	}

	//-------------------------------------------------------------------------
	// Constructs a copy of the given vector
	vector3::vector3(const vector3& copy){
		memcpy((void*)v, (void*)copy.v, sizeof(v));
	}

	//-------------------------------------------------------------------------
	// OPERATOR+ performs vector addition
	vector3 
	vector3::operator +(const vector3 &vec) const{
		vector3 out;
		out.x = x + vec.x;
		out.y = y + vec.y;
		out.z = z + vec.z;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR- performs vector subtraction
	vector3 
	vector3::operator -(const vector3 &vec) const{
		vector3 out;
		out.x = x - vec.x;
		out.y = y - vec.y;
		out.z = z - vec.z;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR- performs vector negation.
	vector3 
	vector3::operator -() const{
		vector3 out;
		out.x = - x;
		out.y = - y;
		out.z = - z;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs vector-scalar multiplication
	vector3 
	vector3::operator *(float scalar) const{
		vector3 out;
		out.x = x * scalar;
		out.y = y * scalar;
		out.z = z * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs scalar-vector multiplication
	vector3 
	operator *(float scalar, vector3& vec){
		vector3 out;
		out.x = vec.x * scalar;
		out.y = vec.y * scalar;
		out.z = vec.z * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR/ performs vector-scalar division
	vector3 
	vector3::operator /(float scalar) const{
		float div = (1.0f / scalar);
		vector3 out;
		out.x = x * div;
		out.y = y * div;
		out.z = z * div;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR/ performs scalar-vector division
	vector3 
	operator /(float scalar, vector3& vec){
		float div = (1.0f / scalar);
		vector3 out;
		out.x = vec.x * div;
		out.y = vec.y * div;
		out.z = vec.z * div;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR+= performs (in-place) vector addition
	vector3& 
	vector3::operator +=(const vector3 &vec){
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR-= performs (in-place) vector subtraction
	vector3& 
	vector3::operator -=(const vector3 &vec){
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR*= performs (in-place) scalar multiplication
	vector3& 
	vector3::operator *=(float scalar){
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR/= performs (in-place) scalar division
	vector3& 
	vector3::operator /=(float scalar){
		float div = (1.0f / scalar);
		x *= div;
		y *= div;
		z *= div;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR== tests whether this vector is equal to another
	bool 
	vector3::operator ==(const vector3& vec) const{
		if (x!=vec.x || y!=vec.y || z!=vec.z)
			return false;
		return true;
	}

	//-------------------------------------------------------------------------
	// OPERATOR!= tests whether this vector is not equal to another
	bool
	vector3::operator !=(const vector3& vec) const{
		if (x!=vec.x || y!=vec.y || z!=vec.z)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR< tests whether this vector is less than another
	bool
	vector3::operator <(const vector3& vec) const{
		if (x < vec.x && y < vec.y && z < vec.z)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR<= tests whether this vector is less than or equal to another
	bool
	vector3::operator <=(const vector3& vec) const{
		if (x <= vec.x && y <= vec.y && z <= vec.z)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR> tests whether this vector is greater than another
	bool
	vector3::operator >(const vector3& vec) const{
		if (x > vec.x && y > vec.y && z > vec.z)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR>= tests whether this vector is greater than or equal to another
	bool
	vector3::operator >=(const vector3& vec) const{
		if (x >= vec.x && y >= vec.y && z >= vec.z)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR[] returns a reference to the element at the given index
	float& 
	vector3::operator [](int idx){
		return v[idx];
	}

	//-------------------------------------------------------------------------
	// STR returns a formatted string representation of the vector
	string 
	vector3::str(){
		char out[64];
		sprintf(out,"<%.2f, %.2f, %.2f>",x,y,z);
		return string(out);
	}

	//-------------------------------------------------------------------------
	// SET sets the components of the vector to those given
	void
	vector3::set(float _x, float _y, float _z){
		x=_x; y=_y; z=_z;
	}

	//-------------------------------------------------------------------------
	// MAG returns the length/magnitude of the vector
	float 
	vector3::Mag()const{
		return sqrt(x*x+y*y+z*z);
	}

	//-------------------------------------------------------------------------
	// MAG2 returns the magnitude squared
	float 
	vector3::Mag2()const{
		return x*x+y*y+z*z;
	}

	//-------------------------------------------------------------------------
	// DOT returns the dot product with the given vector
	float 
	vector3::Dot(const vector3 & v) const{
		return x*v.x + y*v.y + z*v.z;
	}

	//-------------------------------------------------------------------------
	// CROSS returns the cross product with the given vector
	vector3 
	vector3::Cross(const vector3 &v) const{
		return vector3(y*v.z-z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}

	//-------------------------------------------------------------------------
	// NORMALIZE scales this vector to magnitude 1
	void 
	vector3::Normalize(){
		float magInv = 1.0f/Mag();
		x*=magInv;
		y*=magInv;
		z*=magInv;
	}

	//-------------------------------------------------------------------------
	// Returns a normalized version of this vector
	vector3 
	vector3::GetUnit()const{
		float magInv = 1.0f/Mag();
		return vector3(x*magInv, y*magInv, z*magInv);
	}

	//-------------------------------------------------------------------------
	// SETFLOOR floors the values in the vector
	void 
	vector3::SetFloor(){
		x=floorf(x);
		y=floorf(y);
		z=floorf(z);
	}

	//-------------------------------------------------------------------------
	// SETABS sets the values to be their absolute form
	void 
	vector3::SetAbs(){
		x=abs(x);
		y=abs(y);
		z=abs(z);
	}



	/**************************************************************************
	 * vector4 - not fully functional
	 **************************************************************************/

	//-------------------------------------------------------------------------
	// Constructs a zero vector with w = 1
	vector4::vector4(){
		memset((void*)v, 0, 3*sizeof(float));
		w=1.0f;
	}

	//-------------------------------------------------------------------------
	// Constructs a vector from the given components
	vector4::vector4(float _x, float _y, float _z, float _w){
		x=_x; y=_y; z=_z; w=_w;
	}

	//-------------------------------------------------------------------------
	// Constructs a vector from the given elements
	vector4::vector4(const float elems[4]){
		memcpy((void*)v, (void*)elems, sizeof(v));
	}

	//-------------------------------------------------------------------------
	// Constructs a copy of the given vector
	vector4::vector4(const vector4& copy){
		memcpy((void*)v, (void*)copy.v, sizeof(v));
	}

	//-------------------------------------------------------------------------
	// OPERATOR+ performs vector addition, returning 
	vector3 
	vector4::operator +(const vector4 &pt) const{
		vector3 out;
		out.x = x + pt.x;
		out.y = y + pt.y;
		out.z = z + pt.z;
		return out;
	}

	//-------------------------------------------------------------------------
	vector3 
	vector4::operator -(const vector4 &pt) const{
		vector3 out;
		out.x = x - pt.x;
		out.y = y - pt.y;
		out.z = z - pt.z;
		return out;
	}

	//-------------------------------------------------------------------------
	vector4 vector4::operator +(const vector3 &vec) const{
		vector4 out;
		out.x = x + vec.x;
		out.y = y + vec.y;
		out.z = z + vec.z;
		out.w = 1.0f;
		return out;
	}

	//-------------------------------------------------------------------------
	vector4 vector4::operator -(const vector3 &vec) const{
		vector4 out;
		out.x = x - vec.x;
		out.y = y - vec.y;
		out.z = z - vec.z;
		out.w = 1.0f;
		return out;
	}

	//-------------------------------------------------------------------------
	vector4 vector4::operator *(float scalar) const{
		vector4 out;
		out.x = x * scalar;
		out.y = y * scalar;
		out.z = z * scalar;
		out.w = 1.0f;
		return out;
	}

	//-------------------------------------------------------------------------
	vector4 operator *(float scalar, vector4& pt){
		vector4 out;
		out.x = pt.x * scalar;
		out.y = pt.y * scalar;
		out.z = pt.z * scalar;
		out.w = 1.0f;
		return out;
	}

	//-------------------------------------------------------------------------
	vector4& vector4::operator +=(const vector4 &pt){
		x += pt.x;
		y += pt.y;
		z += pt.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	vector4& vector4::operator -=(const vector4 &pt){
		x -= pt.x;
		y -= pt.y;
		z -= pt.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	vector4& vector4::operator +=(const vector3 &vec) {
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	vector4& vector4::operator -=(const vector3 &vec) {
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	bool vector4::operator ==(const vector4& pt) const{
		if (x!=pt.x || y!=pt.y || z!=pt.z)
			return false;
		return true;
	}

	//-------------------------------------------------------------------------
	bool vector4::operator !=(const vector4& pt) const{
		if (x!=pt.x || y!=pt.y || z!=pt.z)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	float& vector4::operator [](int idx){
		return v[idx];
	}

	//-------------------------------------------------------------------------
	string vector4::str(){
		char out[64];
		sprintf(out,"[%.2f, %.2f, %.2f, %.2f]",x,y,z,w);
		return string(out);
	}



	/**************************************************************************
	 * matrix3 
	 **************************************************************************/

	//-------------------------------------------------------------------------
	// Constructs an identity matrix
	matrix3::matrix3(){
		SetIdentity();
	}

	//-------------------------------------------------------------------------
	// Constructs a copy of the given matrix
	matrix3::matrix3(const matrix3& copy){
		memcpy((void*)m, (void*)copy.m, sizeof(m));
	}

	//-------------------------------------------------------------------------
	// Constructs a matrix with the given elements
	matrix3::matrix3(const float elems[9]){
		memcpy((void*)m, (void*)elems, sizeof(m));
	}

	//-------------------------------------------------------------------------
	// STR returns a formatted string representation of the matrix
	string 
	matrix3::str( void ){
		char out[256];
		// column-major
		sprintf(out,"\n|%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f|\n",
			m[0],m[3],m[6],m[1],m[4],m[7],m[2],m[5],m[8]);
		return string(out);
	}

	//-------------------------------------------------------------------------
	// OPERATOR+ performs matrix addition
	matrix3 
	matrix3::operator +(const matrix3 &mat)const{
		matrix3 out;
		for (int i = 0; i < 9; i++)
			out[i] = m[i]+mat.m[i];
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR- performs matrix subtraction
	matrix3 
	matrix3::operator -(const matrix3 &mat)const{
		matrix3 out;
		for (int i = 0; i < 9; i++)
			out[i] = m[i]-mat.m[i];
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix-scalar multiplication
	matrix3 
	matrix3::operator *(float scalar)const{
		matrix3 out;
		for (int i = 0; i < 9; i++)
			out[i] = m[i] * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix multiplication
	matrix3 
	matrix3::operator *(const matrix3 &mat)const{
		matrix3 out;

		int col;
		for (int i = 0; i < 3; i++){
			col=0;
			for (int j = 0; j < 3; j++){
				out.m[i+col] = 	m[i] 		* 	mat.m[col] 		+
								m[i+3]		*	mat.m[col+1] 	+
								m[i+6]		*	mat.m[col+2];

				col+=3;
			}
		}
		return out;
	}
	
	//-------------------------------------------------------------------------
	// OPERATOR*= performs (inplace) matrix multiplications
	void 
	matrix3::operator *=(const matrix3 &mat){
		float temp[9];

		int col;
		for (int i = 0; i < 3; i++){
			col=0;
			for (int j = 0; j < 3; j++){
				temp[i+col] = 	m[i] 		* 	mat.m[col]	 	+
								m[i+3]		*	mat.m[col+1] 	+
								m[i+6]		*	mat.m[col+2];
				col+=3;
			}
		}
		memcpy(this->m, temp, sizeof(float)*9);
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix-vector3 multiplication
	vector3
	matrix3::operator *(const vector3& vec)const{
		vector3 product;
		product.x = m[0]*vec.x+m[3]*vec.y+m[6]*vec.z;
		product.y = m[1]*vec.x+m[4]*vec.y+m[7]*vec.z;
		product.z = m[2]*vec.x+m[5]*vec.y+m[8]*vec.z;
		return product;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix-vector2 multiplication
	vector2
	matrix3::operator *(const vector2& vec)const {
		vector2 product;
		product.x = m[0]*vec.x+m[3]*vec.y;
		product.y = m[1]*vec.x+m[4]*vec.y;
		return product;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs scalar-matrix multiplication
	matrix3 
	operator*(float scalar, const matrix3& mat){
		matrix3 out;
		for (int i = 0; i < 9; i++)
			out[i] = mat.m[i] * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR+= performs (in-place) matrix addition
	matrix3& 
	matrix3::operator +=(const matrix3 &mat){
		for (int i = 0; i < 9; i++)
			m[i]+=mat.m[i];
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR-= performs (in-place) matrix subtraction
	matrix3& 
	matrix3::operator -=(const matrix3 &mat){
		for (int i = 0; i < 9; i++)
			m[i]-=mat.m[i];
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR== performs a per-element equality test
	bool 
	matrix3::operator ==(const matrix3 &mat)const{
		for (int i = 0; i < 9; i++)
			if (m[i]!=mat.m[i])
				return false;
		return true;
	}

	//-------------------------------------------------------------------------
	// OPERATOR!= performs a per-element equality test, return true if any fail
	bool 
	matrix3::operator !=(const matrix3 &mat)const{
		for (int i = 0; i < 9; i++)
			if (m[i]!=mat.m[i])
				return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR[] returns a reference to the element at the given index.
	float& 
	matrix3::operator[](int idx){
		return m[idx];
	}

	//-------------------------------------------------------------------------
	// SETIDENTITY Resets the matrix to an identity matrix
	void 
	matrix3::SetIdentity(){
		memset((void*)m, 0, sizeof(m));
		m[0] = m[5]= m[8] = 1.0f;
	}

	//-------------------------------------------------------------------------
	// INVERSE Returns the inverse of the matrix.
	matrix3
	matrix3::Inverse(){
		matrix3 out;
		float det = Determinant();

		if (close_enough(det, 0.0f)){
			out.SetIdentity();
		} else{
			det = 1.0f / det;

			out.m[0] = det * (m[4] * m[8] - m[5] * m[7]);
			out.m[1] = det * (m[2] * m[7] - m[1] * m[8]);
			out.m[2] = det * (m[1] * m[5] - m[2] * m[4]);

			out.m[3] = det * (m[5] * m[6] - m[3] * m[8]);
			out.m[4] = det * (m[0] * m[8] - m[2] * m[6]);
			out.m[5] = det * (m[2] * m[3] - m[0] * m[5]);

			out.m[6] = det * (m[3] * m[7] - m[4] * m[6]);
			out.m[7] = det * (m[1] * m[6] - m[0] * m[7]);
			out.m[8] = det * (m[0] * m[4] - m[1] * m[3]);
		}

		return out;
	}

	//-------------------------------------------------------------------------
	// TRANSPOSE Returns the transpose of the matrix.
	matrix3 
	matrix3::Transpose(){
		matrix3 out;
		for (int i = 0; i < 3; i ++){
			for (int j = 0; j < 3; j++){
				out.m[j*3+i] = m[i*3+j];
			}
		}
		return out;
	}

	//-------------------------------------------------------------------------
	// DETERMINANT Returns the determinant of the matrix.
	float 
	matrix3::Determinant(){
		return (m[0] * (m[4] * m[8] - m[7] * m[5]))
			 - (m[3] * (m[1] * m[8] - m[7] * m[2]))
			 + (m[6] * (m[1] * m[5] - m[4] * m[2]));
	}



	/**************************************************************************
	 * matrix4 
	 **************************************************************************/

	//-------------------------------------------------------------------------
	// Constructs an identity matrix
	matrix4::matrix4(){
		SetIdentity();
	}

	//-------------------------------------------------------------------------
	// Constructs a copy of the given matrix
	matrix4::matrix4(const matrix4& copy){
		memcpy((void*)m, (void*)copy.m, sizeof(m));
	}

	//-------------------------------------------------------------------------
	// Constructs a matrix with the given elements
	matrix4::matrix4(const float elems[16]){
		memcpy((void*)m, (void*)elems, sizeof(m));
	}

	//-------------------------------------------------------------------------
	// STR returns a formatted string representation of the matrix
	string 
	matrix4::str( void ){
		char out[256];
		// column-major
		sprintf(out,"\n|%.2f\t%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f\t%.2f|\n",
			m[0],m[4],m[8],m[12],m[1],m[5],m[9],m[13],m[2],m[6],m[10],m[14],m[3],m[7],m[11],m[15]);
		return string(out);
	}

	//-------------------------------------------------------------------------
	// OPERATOR+ performs matrix addition
	matrix4 
	matrix4::operator +(const matrix4 &mat)const{
		matrix4 out;
		for (int i = 0; i < 16; i++)
			out[i] = m[i]+mat.m[i];
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR- performs matrix subtraction
	matrix4 
	matrix4::operator -(const matrix4 &mat)const{
		matrix4 out;
		for (int i = 0; i < 16; i++)
			out[i] = m[i]-mat.m[i];
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix-scalar multiplication
	matrix4 
	matrix4::operator *(float scalar)const{
		matrix4 out;
		for (int i = 0; i < 16; i++)
			out[i] = m[i] * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix multiplication
	matrix4 
	matrix4::operator *(const matrix4 &mat)const{
		matrix4 out;

		int col;
		for (int i = 0; i < 4; i++){
			col=0;
			for (int j = 0; j < 4; j++){
				out.m[i+col] = 	m[i] 		* 	mat.m[col] 		+
								m[i+4]		*	mat.m[col+1] 	+
								m[i+8]		*	mat.m[col+2] 	+
								m[i+12]		* 	mat.m[col+3];

				col+=4;
			}
		}
		return out;
	}
	
	//-------------------------------------------------------------------------
	// OPERATOR*= performs (inplace) matrix multiplications
	void 
	matrix4::operator *=(const matrix4 &mat){
		float temp[16];

		int col;
		for (int i = 0; i < 4; i++){
			col=0;
			for (int j = 0; j < 4; j++){
				temp[i+col] = 	m[i] 		* 	mat.m[col]	 	+
								m[i+4]		*	mat.m[col+1] 	+
								m[i+8]		*	mat.m[col+2] 	+
								m[i+12]		* 	mat.m[col+3];
				col+=4;
			}
		}
		memcpy(this->m, temp, sizeof(float)*16);
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix-vector4 multiplication
	vector4
	matrix4::operator *(const vector4& vec)const{
		vector4 product;
		product.x = m[0]*vec.x+m[4]*vec.y+m[8]*vec.z+m[12]*vec.w;
		product.y = m[1]*vec.x+m[5]*vec.y+m[9]*vec.z+m[13]*vec.w;
		product.z = m[2]*vec.x+m[6]*vec.y+m[10]*vec.z+m[14]*vec.w;
		product.w = m[3]*vec.x+m[7]*vec.y+m[11]*vec.z+m[15]*vec.w;
		return product;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix-vector3 multiplication
	vector3
	matrix4::operator *(const vector3& vec)const {
		vector3 product;
		product.x = m[0]*vec.x+m[4]*vec.y+m[8]*vec.z;
		product.y = m[1]*vec.x+m[5]*vec.y+m[9]*vec.z;
		product.z = m[2]*vec.x+m[6]*vec.y+m[10]*vec.z;
		return product;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs scalar-matrix multiplication
	matrix4 
	operator*(float scalar, const matrix4& mat){
		matrix4 out;
		for (int i = 0; i < 16; i++)
			out[i] = mat.m[i] * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR+= performs (in-place) matrix addition
	matrix4& 
	matrix4::operator +=(const matrix4 &mat){
		for (int i = 0; i < 16; i++)
			m[i]+=mat.m[i];
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR-= performs (in-place) matrix subtraction
	matrix4& 
	matrix4::operator -=(const matrix4 &mat){
		for (int i = 0; i < 16; i++)
			m[i]-=mat.m[i];
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR== performs a per-element equality test
	bool 
	matrix4::operator ==(const matrix4 &mat)const{
		for (int i = 0; i < 16; i++)
			if (m[i]!=mat.m[i])
				return false;
		return true;
	}

	//-------------------------------------------------------------------------
	// OPERATOR!= performs a per-element equality test, return true if any fail
	bool 
	matrix4::operator !=(const matrix4 &mat)const{
		for (int i = 0; i < 16; i++)
			if (m[i]!=mat.m[i])
				return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR[] returns a reference to the element at the given index.
	float& 
	matrix4::operator[](int idx){
		return m[idx];
	}

	//-------------------------------------------------------------------------
	// SETIDENTITY Resets the matrix to an identity matrix
	void 
	matrix4::SetIdentity(){
		memset((void*)m, 0, sizeof(m));
		m[0] = m[5]= m[10] = m[15] = 1.0f;
	}

	//-------------------------------------------------------------------------
	// TRANSPOSE Returns the transpose of the matrix.
	matrix4 
	matrix4::Transpose(){
		matrix4 out;
		for (int i = 0; i < 4; i ++){
			for (int j = 0; j < 4; j++){
				out.m[j*4+i] = m[i*4+j];
			}
		}
		return out;
	}

	//-------------------------------------------------------------------------
	// DETERMINANT Returns the determinant of the matrix.
	float 
	matrix4::Determinant(){
		// calculate the common 2x2 determinants
		float m_10_14 = m[10]*m[15] - m[14]*m[11]; // m10*m15 - m14m11
		float m_6_14  = m[6] *m[15] - m[14]*m[7];
		float m_6_10  = m[6] *m[11] - m[10]*m[7];

		float m_2_14  = m[2] *m[15] - m[14]*m[3];
		float m_2_10  = m[2] *m[11] - m[10]*m[3];
		float m_2_6   = m[2] *m[7]  - m[6] *m[3];

		// return the main determinant (4 * 3x3 dets)
		return 	+ m[0] * (m[5]*m_10_14 	- m[9]*m_6_14	+ m[13]*m_6_10)
				- m[4] * (m[1]*m_10_14	- m[9]*m_2_14	+ m[13]*m_2_10)
				+ m[8] * (m[1]*m_6_14	- m[5]*m_2_14	+ m[13]*m_2_6)
				- m[12]* (m[1]*m_6_10	- m[5]*m_2_10	+ m[9] *m_2_6);
	}

	//-------------------------------------------------------------------------
	// GETMATRIX3 Returns the matrix3 stored from the top left of the matrix.
	matrix3
	matrix4::GetMatrix3(){
		matrix3 out;

		out[0] = m[0];
		out[1] = m[1];
		out[2] = m[2];

		out[3] = m[4];
		out[4] = m[5];
		out[5] = m[6];

		out[6] = m[8];
		out[7] = m[9];
		out[8] = m[10];

		return out;
	}

	//-------------------------------------------------------------------------
	// GETTRANSLATION Returns the vector3 for the translation stored here
	vector3
	matrix4::GetTranslation(){
		vector3 out;

		out.x = m[12];
		out.y = m[13];
		out.z = m[14];

		return out;
	}



	/******************************************************************************
	 * Projection Transform Functions
	 ******************************************************************************/

	//-------------------------------------------------------------------------
	// FRUSTUM_PROJ Returns a matrix that projects a vector to the screen with 
	// perspective.  Allows for assymmetric frustums.
	matrix4
	frustum_proj (float left, float right, float bottom, float top, float near, float far){
		matrix4 mat4;
		float* mat = mat4.m;
		// first column
		mat[0] = 2*near / (right - left);
		// second 
		mat[5] = 2*near / (top - bottom);
		// third 
		mat[8] = (right+left) / (right-left);
		mat[9] = (top+bottom)/(top-bottom);
		mat[10]= -(far+near)/(far-near);
		mat[11]= -1.0f;
		// last column
		mat[14]= -(2*far*near)/(far-near);
		mat[15]= .0f;

		return mat4;
	}

	//-------------------------------------------------------------------------
	// PERSPECTIVE_PROJ Returns a matrix that projects a vector to the screen 
	// using a symmetric frustum to create the perspective effect.  The 
	// vertical Field-Of-View fovy, is in RADIANS
	matrix4
	perspective_proj(float fovy, float aspect, float near, float far){
		matrix4 mat4;
		float* mat = mat4.m;
		float f = 1/tan(fovy*.5f);
		// first column
		mat[0] = f/aspect;
		// second
		mat[5] = f;
		// third
		mat[10] = (far+near)/(near-far);
		mat[11] = -1.0f;
		// last column
		mat[14] = 2*far*near/(near - far);
		mat[15] = .0f;

		return mat4;
	}

	//-------------------------------------------------------------------------
	// ORTHO_PROJ projects a vector to the screen orthogonally
	matrix4
	ortho_proj(float left, float right, float bottom, float top, float near, float far){
		matrix4 mat4;
		float* mat = mat4.m;

		// first column
		mat[0] = 2.0f/(right-left);
		// second
		mat[5] = 2.0f/(top-bottom);
		// third
		mat[10] = -2.0f/(far-near);
		// last column
		mat[12] = -(right+left)/(right-left);
		mat[13] = -(top+bottom)/(top-bottom);
		mat[14] = -(far+near)/(far-near);
		mat[15] = 1.0f;

		return mat4;
	}

	//-------------------------------------------------------------------------
	// ORTHO_PROJ projects a vector to the screen orthogonally but assumes
	// a near and far plane of -1 and 1 respectively.
	matrix4 
	ortho2d_proj(float left, float right, float bottom, float top){
		matrix4 mat4;
		float* mat = mat4.m;

		// first column
		mat[0] = 2.0f/(right-left);
		// second
		mat[5] = 2.0f/(top-bottom);
		// third
		mat[10] = -1.0f;
		// last column
		mat[12] = -(right+left)/(right-left);
		mat[13] = -(top+bottom)/(top-bottom);
		mat[14] = .0f;
		mat[15] = 1.0f;

		return mat4;
	}

	//-------------------------------------------------------------------------
	// CREATE LOOK AT creates a look-at matrix for use as a view matrix
	matrix4
	create_look_at(vector3 &camera, vector3 &target, vector3 &up){
		// Builds a look-at style view matrix.
		// This is essentially the same matrix used by gluLookAt().

		matrix4 out;

		vector3 zAxis = camera - target;
		zAxis.Normalize();

		vector3 xAxis = up.Cross(zAxis);
		xAxis.Normalize();

		vector3 yAxis = zAxis.Cross(xAxis);
		yAxis.Normalize();

		out[0] = xAxis.x;
		out[4] = xAxis.y;
		out[8] = xAxis.z;
		out[12] = -xAxis.Dot(camera);

		out[1] = yAxis.x;
		out[5] = yAxis.y;
		out[9] = yAxis.z;
		out[13] = -yAxis.Dot(camera);

		out[2] = zAxis.x;
		out[6] = zAxis.y;
		out[10] = zAxis.z;
		out[14] = -zAxis.Dot(camera);

		out[3] = 0.0f;
		out[7] = 0.0f;
		out[11] = 0.0f;
		out[15] = 1.0f;

		return out;
	}

	//-------------------------------------------------------------------------
	// PERSPECTIVE_UNPROJ_WORLD, given a mouse x,y coordinate along with the z-buffer
	// value, returns the world-space coordinate displayed at that location in the viewport.
	// The fragment coordinate assumes positive y to be upwards. tan_fovy is the tangent
	// of HALF the field-of-view => tan_fovy = tan(fov/2)
	vector3
	perspective_unproj_world (vector3 fragment, float w, float h, float near, float far,
			float tan_fovy, matrix4& inverseView)
	{
		float x,y,z,aspect;

		aspect = w/h;
		x = fragment.x;
		y = fragment.y;
		z = fragment.z;

		// Convert to Normalized Device Coordinates
		x = (x*2.0f/w)-1.0f;
		y = (y*2.0f/-h)+1.0f;
		z = z*2 - 1.0f;
		
		// to eye space, first need to get z
		z = (2 * far * near) / 
			((z - (far + near) / (far - near)) * (far - near));

		y = y * (-z) * tan_fovy;
		x = x * (-z) * aspect * tan_fovy;

		// reverse the view transformation to acquire world-space coordinates
		return inverseView * vector3(x,y,z);
	}

	//-------------------------------------------------------------------------
	// PERSPECTIVE_UNPROJ_VIEW, given a mouse x,y coordinate along with the z-buffer
	// value, returns the eye-space coordinate displayed at that location in the viewport.
	vector3
	perspective_unproj_view (vector3 fragment, float w, float h, float near, float far,
			float tan_fovy)
	{
		float x,y,z,aspect;

		aspect = w/h;
		x = fragment.x;
		y = fragment.y;
		z = fragment.z;

		// Convert to Normalized Device Coordinates
		x = (x*2.0f/w)-1.0f;
		y = (y*2.0f/-h)+1.0f;
		z = z*2 - 1.0f;
		
		// to eye space, first need to get z
		z = (2 * far * near) / 
			((z - (far + near) / (far - near)) * (far - near));

		y = y * (-z) * tan_fovy;
		x = x * (-z) * aspect * tan_fovy;
		return vector3(x,y,z);
	}

	//-------------------------------------------------------------------------
	// CLOSE_ENOUGH returns a bool on the difference of f1 and f2 if they
	// are close enough to each other.
	bool
	close_enough(float f1, float f2){
		return fabsf((f1 - f2) / ((f2 == 0.0f) ? 1.0f : f2)) < 1e-6f;
	}



	/******************************************************************************
	 * Transform Functions
	 * post-fix mult
	 ******************************************************************************/

	//-------------------------------------------------------------------------
	// TRANSLATE_TR returns a matrix that may be used for vector translation
	// if the vector is pre-multiplied by this matrix.
	matrix4
	translate_tr (float x, float y, float z){
		matrix4 mat;
		mat[12] =x;
		mat[13] =y;
		mat[14] =z;
		return mat;
	}

	//-------------------------------------------------------------------------
	// ROTATE_TR
	// Overridden form to accept a vector3 as input
	matrix4
	translate_tr (vector3 vec){
		matrix4 mat;
		mat[12] = vec.x;
		mat[13] = vec.y;
		mat[14] = vec.z;
		return mat;
	}

	//-------------------------------------------------------------------------
	// ROTATE_TR returns a rotation transform matrix for pre-multiplication
	// of vectors. The angle must be specified in RADIANS
	matrix4
	rotate_tr (float angle, float x, float y, float z){
		matrix4 mat4;
		float* mat = mat4.m;
		float c = cosf(angle);
		float s = sinf(angle);

		// first column
		mat[0] = x*x*(1-c)+c; 
		mat[1] = y*x*(1-c)+z*s;
		mat[2] = x*z*(1-c)-y*s;
		mat[3] = .0f;
		// second
		mat[4] = x*y*(1-c)-z*s;
		mat[5] = y*y*(1-c)+c;
		mat[6] = y*z*(1-c)+x*s;
		mat[7] = .0f;
		// third
		mat[8] = x*z*(1-c)+y*s;
		mat[9] = y*z*(1-c)-x*s;
		mat[10] = z*z*(1-c)+c;
		mat[11] = .0f;
		// last column
		mat[12] = .0f;
		mat[13] = .0f;
		mat[14] = .0f;
		mat[15] = 1.0f;
		return mat4;
	}

	//-------------------------------------------------------------------------
	// ROTATE_TR
	// Overridden form to accept a vector3 as input
	matrix4
	rotate_tr (float angle, vector3 vec){
		matrix4 mat4;
		float* mat = mat4.m;
		float c = cosf(angle);
		float s = sinf(angle);

		int x = vec.x;
		int y = vec.y;
		int z = vec.z;

		// first column
		mat[0] = x*x*(1-c)+c; 
		mat[1] = y*x*(1-c)+z*s;
		mat[2] = x*z*(1-c)-y*s;
		mat[3] = .0f;
		// second
		mat[4] = x*y*(1-c)-z*s;
		mat[5] = y*y*(1-c)+c;
		mat[6] = y*z*(1-c)+x*s;
		mat[7] = .0f;
		// third
		mat[8] = x*z*(1-c)+y*s;
		mat[9] = y*z*(1-c)-x*s;
		mat[10] = z*z*(1-c)+c;
		mat[11] = .0f;
		// last column
		mat[12] = .0f;
		mat[13] = .0f;
		mat[14] = .0f;
		mat[15] = 1.0f;
		return mat4;
	}

	//-------------------------------------------------------------------------
	// SCALE_TR returns a scaling transformation matrix that can scale vectors
	// via pre-multiplication of the vector.
	matrix4
	scale_tr (float x, float y, float z){
		matrix4 mat4;

		mat4.m[0]=x;
		mat4.m[5]=y;
		mat4.m[10]=z;
		
		return mat4;
	}

	//-------------------------------------------------------------------------
	// SCALE_TR
	// Overridden form to accept a vector3 as input
	matrix4
	scale_tr (vector3 vec){
		matrix4 mat4;

		mat4.m[0]=vec.x;
		mat4.m[5]=vec.y;
		mat4.m[10]=vec.z;
		
		return mat4;
	}

	//-------------------------------------------------------------------------
	// SCALE_TR
	// Overridden form to accept a single float as input
	matrix4
	scale_tr (float scale){
		matrix4 mat4;

		mat4.m[0]=scale;
		mat4.m[5]=scale;
		mat4.m[10]=scale;
		
		return mat4;
	}
}

