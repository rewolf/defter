
#ifndef _REMATH_H_
#define _REMATH_H_

/*
 * Some Notes:
 *  - Angles are specified in Radians
 *  - Matrices are stored in column major ordering
 *  - Transformations should pre-multiply vectors. eg. V2 = M * V1
 *	- The vector4 homogeneous coordinates are not implemented correctly yet, use 
 *	  vector3 instead.
 */

namespace reMath{
	// Forward declarations
	class vector2;
	class vector3;
	class vector4;
	class matrix3;
	class matrix4;

	/******************************************************************************
	 * TRANSFORMS AND MATH
	 ******************************************************************************/

	/******************************************************************************
	 * vector2
	 * A class to handle 2D vectors and their operations
	 ******************************************************************************/
	class vector2{
	public:
		vector2();
		vector2(float x, float y);
		vector2(const vector2& copy);
		vector2(const float elems[2]);

		vector2		operator+		(const vector2& vec) const;
		vector2		operator-		(const vector2& vec) const;
		vector2		operator-		() const;
		vector2		operator*		(float scalar) const;
		friend vector2 operator*	(float scalar, const vector2& vec);
		vector2		operator/		(float scalar) const;
		friend vector2 operator/	(float scalar, const vector2& vec);

		vector2&	operator+=		(const vector2& vec);
		vector2&	operator-=		(const vector2& vec);
		vector2&	operator*=		(float scalar);
		vector2&	operator/=		(float scalar);

		bool		operator==		(const vector2& vec) const;
		bool		operator!=		(const vector2& vec) const;

		float&		operator[]		(int idx) ;
		string		str				(void);

		void		set				(float x, float y);

		float		Mag				(void) const;
		float		Mag2			(void) const;

		float		Dot				(const vector2&)const;

		void		Normalize		(void);
		vector2		GetUnit			(void) const;


	public:
		union{
			struct {float x,y;};
			float v[2];
		};
	};
	vector2 operator*	(float scalar, const vector2& vec);

	/******************************************************************************
	 * vector3
	 * A class to handle 3D vectors and their operations
	 ******************************************************************************/
	class vector3{
	public:
		vector3();
		vector3(float x, float y, float z);
		vector3(const vector3& copy);
		vector3(const float elems[3]);

		vector3		operator+		(const vector3& vec) const;
		vector3		operator-		(const vector3& vec) const;
		vector3		operator-		() const;
		vector3		operator*		(float scalar) const;
		friend vector3 operator*	(float scalar, const vector3& vec);
		vector3		operator/		(float scalar) const;
		friend vector3 operator/	(float scalar, const vector3& vec);

		vector3&	operator+=		(const vector3& vec);
		vector3&	operator-=		(const vector3& vec);
		vector3&	operator*=		(float scalar);
		vector3&	operator/=		(float scalar);

		bool		operator==		(const vector3& vec) const;
		bool		operator!=		(const vector3& vec) const;

		float&		operator[]		(int idx) ;
		string		str				(void);

		void		set				(float x, float y, float z);

		float		Mag				(void) const;
		float		Mag2			(void) const;

		float		Dot				(const vector3&)const;
		vector3		Cross			(const vector3&)const;

		void		Normalize		(void);
		vector3		GetUnit			(void) const;


	public:
		union{
			struct {float x,y,z;};
			float v[3];
		};
	};
	vector3 operator*	(float scalar, const vector3& vec);

	/******************************************************************************
	 * vector4
	 * A Class to represent homogeneous coordinates - not fully functional
	 ******************************************************************************/
	class vector4{
	public:
		vector4();
		vector4(const vector4& copy);
		vector4(const float elems[4]);

		vector3		operator+		(const vector4& vec) const;
		vector3		operator-		(const vector4& vec) const;
		vector4		operator+		(const vector3& vec) const;
		vector4		operator-		(const vector3& vec) const;
		vector4		operator*		(float scalar) const;
		friend vector4 operator*		(float scalar, const vector4& pt);

		vector4&	operator+=		(const vector4& vec);
		vector4&	operator-=		(const vector4& vec);
		vector4&	operator+=		(const vector3& vec);
		vector4&	operator-=		(const vector3& vec);

		bool		operator==		(const vector4& vec) const;
		bool		operator!=		(const vector4& vec) const;

		float&		operator[]		(int idx) ;
		string		str				(void);


	public:
		union{
			struct {float x,y,z,w;};
			float v[4];
		};
	};
	vector4 operator*	(float scalar, const vector4& pt);

	/******************************************************************************
	 * matrix3
	 * A class to represent 3x3 matrices that are often used in vector
	 * transformations. Note column-major indices
	 ******************************************************************************/
	class matrix3{
	public:
		matrix3();
		matrix3(const matrix3& copy);
		matrix3(const float elems[9]);

		matrix3		operator+		(const matrix3& mat) const;
		matrix3		operator-		(const matrix3& mat) const;
		matrix3		operator*		(float scalar) const;
		friend matrix3 operator*	(float scalar, const matrix3& mat);
		matrix3		operator*		(const matrix3& mat) const;
		vector3		operator*		(const vector3& vec) const;
		vector2		operator*		(const vector2& vec) const;
		void		operator*=		(const matrix3& mat);
		matrix3&	operator+=		(const matrix3& mat);
		matrix3&	operator-=		(const matrix3& mat);

		bool		operator==		(const matrix3& mat) const;
		bool		operator!=		(const matrix3& mat) const;

		float&		operator[]		(int idx);

		string		str				(void);
		
		void		SetIdentity		(void);
		matrix3		Inverse			(void);
		matrix3		Transpose		(void);
		float		Determinant		(void);

	public:
		float	m[9];
	};
	matrix4 operator*	(float scalar, const matrix4&);

	/******************************************************************************
	 * matrix4
	 * A class to represent 4x4 matrices that are often used in vector
	 * transformations. Note column-major indices
	 ******************************************************************************/
	class matrix4{
	public:
		matrix4();
		matrix4(const matrix4& copy);
		matrix4(const float elems[16]);

		matrix4		operator+		(const matrix4& mat) const;
		matrix4		operator-		(const matrix4& mat) const;
		matrix4		operator*		(float scalar) const;
		friend matrix4 operator*	(float scalar, const matrix4& mat);
		matrix4		operator*		(const matrix4& mat) const;
		vector4		operator*		(const vector4& vec) const;
		vector3		operator*		(const vector3& vec) const;
		void		operator*=		(const matrix4& mat);
		matrix4&	operator+=		(const matrix4& mat);
		matrix4&	operator-=		(const matrix4& mat);

		bool		operator==		(const matrix4& mat) const;
		bool		operator!=		(const matrix4& mat) const;

		float&		operator[]		(int idx);

		string		str				(void);
		
		void		SetIdentity		(void);
		matrix4		Transpose		(void);
		float		Determinant		(void);
		matrix3		GetMatrix3		(void);
		vector3		GetTranslation	(void);

	public:
		float	m[16];
	};
	matrix4 operator*	(float scalar, const matrix4&);


	/******************************************************************************
	 * Transform Matrices & Utility Functions
	 ******************************************************************************/

	matrix4 frustum_proj		(float left, float right, float bottom, float top, float near, float far);
	matrix4 perspective_proj	(float fovy, float aspect, float near, float far);
	matrix4 ortho_proj			(float left, float right, float bottom, float top, float near, float far);
	matrix4 ortho2d_proj		(float left, float right, float bottom, float top);
	matrix4 create_look_at		(vector3 &camera, vector3 &target, vector3 &up);
	vector3 perspective_unproj_world	(vector3 fragment, float w, float h, float near, float far, 
											float tan_fovy, matrix4& inverseView);
	vector3 perspective_unproj_view		(vector3 fragment, float w, float h, float near, float far, 
											float tan_fovy);

	bool close_enough(float f1, float f2);

	matrix4 translate_tr	(float x, float y, float z);
	matrix4 translate_tr	(vector3 vec);
	matrix4 rotate_tr		(float angle, float x, float y, float z);
	matrix4 rotate_tr		(float angle, vector3 vec);
	matrix4 scale_tr		(float x, float y, float z);
	matrix4 scale_tr		(vector3 vec);
	matrix4 scale_tr		(float scale);
}
#endif

