#ifndef __XPDRAW3D_H__
#define __XPDRAW3D_H__

extern "C" {
#include <lua.h>
}

#include <vector>
#include "xpsdk.h"

namespace xap3d {

/// register X-Plane objects functions in Lua
void exportDraw3dFunctions(lua_State *L);

/// called for each stage to draw objects
void draw3d(XPLMDrawingPhase phase);

void initDraw3d();

void frameFinished();

class Base3D {
public:
	Base3D() {}
	virtual ~Base3D() {}
public:
	virtual void draw_element() = 0;
public:
	float mR;
	float mG;
	float mB;
	float mAlpha;

	float mX1;
	float mY1;	
	float mZ1;
};

class Line3D: public Base3D {
public:
	Line3D() {}
	virtual ~Line3D() {}
public:
	void draw_element();
public:
	float mX2;
	float mY2;
	float mZ2;
};

class Circle3D: public Base3D {
public:
	Circle3D() {}
	virtual ~Circle3D() {}

	void setupVertices(std::vector<GLfloat>&, const GLsizei&);
public:
	void draw_element();
public:
	float mRadius;
	float mPitch;
	float mYaw;
	bool mFilled;
	bool mHasFixedOrientation;
};

class Angle3D: public Base3D {
public:
	Angle3D() {}
	virtual ~Angle3D() {}
public:
	void draw_element();
public:
	float mLenght;
	float mAngle;
	float mPitch;
	float mYaw;
	std::size_t mRays;
};

class StandingCone3D: public Base3D {
public:
	StandingCone3D() {}
	virtual ~StandingCone3D() {}
public:
	void draw_element();
public:
	float mHeight;
	float mRadius;
};

typedef std::vector<Base3D*> Objects3D;

};

#endif
