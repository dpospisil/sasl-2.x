#include "xpdraw3d.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <SOIL.h>
#include "glheaders.h"

#include <vector>
#include <string.h>

#include "math3d.h"
#include "xpsdk.h"

#include "avionics.h"

using namespace xa;
using namespace xap;
using namespace xap3d;

static XPLMDataRef viewX;
static XPLMDataRef viewY;
static XPLMDataRef viewZ;
static XPLMDataRef viewPitch;
static XPLMDataRef viewRoll;
static XPLMDataRef viewHeading;

static XPLMDataRef viewIsExternal;
static XPLMDataRef localx;
static XPLMDataRef localy;
static XPLMDataRef localz;
static XPLMDataRef quaternion;

/*
static XPLMDataRef pilotX;
static XPLMDataRef pilotY;
static XPLMDataRef pilotZ;
static XPLMDataRef pilotPitch;
static XPLMDataRef pilotRoll;
static XPLMDataRef pilotHeading;
*/
//static XPLMDataRef viewType;

static double lastViewX;
static double lastViewY;
static double lastViewZ;
static double lastViewPitch;
static double lastViewRoll;
static double lastViewHeading;
//int lastViewType;

static double lastLocalX;
static double lastLocalY;
static double lastLocalZ;
static float lastQuat[4];

//static int viewType3dCockpit = 1026;

// current OpenGL matrixes
static float projectionMatrix[16];
static float modelMatrix[16];
static float clipMatrix[16];
static float frustum[6][4];

static int hdr_pass;
static XPLMDataRef hdr_pass_dr;

void xap3d::initDraw3d()
{
    viewX = XPLMFindDataRef("sim/graphics/view/view_x");
    viewY = XPLMFindDataRef("sim/graphics/view/view_y");
    viewZ = XPLMFindDataRef("sim/graphics/view/view_z");
    viewPitch = XPLMFindDataRef("sim/graphics/view/view_pitch");
    viewRoll = XPLMFindDataRef("sim/graphics/view/view_roll");
    viewHeading = XPLMFindDataRef("sim/graphics/view/view_heading");
    hdr_pass_dr = XPLMFindDataRef("sim/graphics/view/plane_render_type");   //! This dataref will exist from X-Plane 10.30 onwards so you don't need the hack anymore
	
	viewIsExternal = XPLMFindDataRef("sim/graphics/view/view_is_external");

	localx = XPLMFindDataRef("sim/flightmodel/position/local_x");
	localy = XPLMFindDataRef("sim/flightmodel/position/local_y");
	localz = XPLMFindDataRef("sim/flightmodel/position/local_z");
	quaternion = XPLMFindDataRef("sim/flightmodel/position/q");
	
    /*
    pilotX = XPLMFindDataRef("sim/graphics/view/pilots_head_x");
    pilotY = XPLMFindDataRef("sim/graphics/view/pilots_head_y");
    pilotZ = XPLMFindDataRef("sim/graphics/view/pilots_head_z");
    pilotPitch = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
    pilotRoll = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    pilotHeading = XPLMFindDataRef("sim/graphics/view/cockpit_heading");

    viewType = XPLMFindDataRef("sim/graphics/view/view_type");
    */
}

/*static double radians(double degrees)
{
    double radians = 0;
    radians = degrees * (M_PI/180);
    return radians;
}*/

static Vector crossProduct(Vector v1, Vector v2)
{
    Vector vec;
    vec.x = v1.y * v2.z - v2.y * v1.z;
    vec.y = v2.x * v1.z - v1.x * v2.z;
    vec.z = v1.x * v2.y - v1.y * v2.x;

    return vec;
}

static double dotProduct(Vector v1, Vector v2) {
	double dot;
    dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    return dot;
 }

// start texturing
static void setTexture(int texId)
{
    GLint currentTexture[1];
    glGetIntegerv(GL_TEXTURE_BINDING_2D,currentTexture);

    if (currentTexture[0] != texId) {
        if (! currentTexture[0])
            glEnable(GL_TEXTURE_2D);

        XPLMBindTexture2d(texId, 0);
    }
}


static void extractMatrixes()
{
    float   t;

    /* Get the current PROJECTION matrix from OpenGL */
    glGetFloatv( GL_PROJECTION_MATRIX, projectionMatrix );

    /* Get the current MODELVIEW matrix from OpenGL */
    glGetFloatv( GL_MODELVIEW_MATRIX, modelMatrix );

    /* Combine the two matrices (multiply projection by modelview) */
    clipMatrix[ 0] = modelMatrix[ 0] * projectionMatrix[ 0] + modelMatrix[ 1] * projectionMatrix[ 4] + modelMatrix[ 2] * projectionMatrix[ 8] + modelMatrix[ 3] * projectionMatrix[12];
    clipMatrix[ 1] = modelMatrix[ 0] * projectionMatrix[ 1] + modelMatrix[ 1] * projectionMatrix[ 5] + modelMatrix[ 2] * projectionMatrix[ 9] + modelMatrix[ 3] * projectionMatrix[13];
    clipMatrix[ 2] = modelMatrix[ 0] * projectionMatrix[ 2] + modelMatrix[ 1] * projectionMatrix[ 6] + modelMatrix[ 2] * projectionMatrix[10] + modelMatrix[ 3] * projectionMatrix[14];
    clipMatrix[ 3] = modelMatrix[ 0] * projectionMatrix[ 3] + modelMatrix[ 1] * projectionMatrix[ 7] + modelMatrix[ 2] * projectionMatrix[11] + modelMatrix[ 3] * projectionMatrix[15];

    clipMatrix[ 4] = modelMatrix[ 4] * projectionMatrix[ 0] + modelMatrix[ 5] * projectionMatrix[ 4] + modelMatrix[ 6] * projectionMatrix[ 8] + modelMatrix[ 7] * projectionMatrix[12];
    clipMatrix[ 5] = modelMatrix[ 4] * projectionMatrix[ 1] + modelMatrix[ 5] * projectionMatrix[ 5] + modelMatrix[ 6] * projectionMatrix[ 9] + modelMatrix[ 7] * projectionMatrix[13];
    clipMatrix[ 6] = modelMatrix[ 4] * projectionMatrix[ 2] + modelMatrix[ 5] * projectionMatrix[ 6] + modelMatrix[ 6] * projectionMatrix[10] + modelMatrix[ 7] * projectionMatrix[14];
    clipMatrix[ 7] = modelMatrix[ 4] * projectionMatrix[ 3] + modelMatrix[ 5] * projectionMatrix[ 7] + modelMatrix[ 6] * projectionMatrix[11] + modelMatrix[ 7] * projectionMatrix[15];

    clipMatrix[ 8] = modelMatrix[ 8] * projectionMatrix[ 0] + modelMatrix[ 9] * projectionMatrix[ 4] + modelMatrix[10] * projectionMatrix[ 8] + modelMatrix[11] * projectionMatrix[12];
    clipMatrix[ 9] = modelMatrix[ 8] * projectionMatrix[ 1] + modelMatrix[ 9] * projectionMatrix[ 5] + modelMatrix[10] * projectionMatrix[ 9] + modelMatrix[11] * projectionMatrix[13];
    clipMatrix[10] = modelMatrix[ 8] * projectionMatrix[ 2] + modelMatrix[ 9] * projectionMatrix[ 6] + modelMatrix[10] * projectionMatrix[10] + modelMatrix[11] * projectionMatrix[14];
    clipMatrix[11] = modelMatrix[ 8] * projectionMatrix[ 3] + modelMatrix[ 9] * projectionMatrix[ 7] + modelMatrix[10] * projectionMatrix[11] + modelMatrix[11] * projectionMatrix[15];

    clipMatrix[12] = modelMatrix[12] * projectionMatrix[ 0] + modelMatrix[13] * projectionMatrix[ 4] + modelMatrix[14] * projectionMatrix[ 8] + modelMatrix[15] * projectionMatrix[12];
    clipMatrix[13] = modelMatrix[12] * projectionMatrix[ 1] + modelMatrix[13] * projectionMatrix[ 5] + modelMatrix[14] * projectionMatrix[ 9] + modelMatrix[15] * projectionMatrix[13];
    clipMatrix[14] = modelMatrix[12] * projectionMatrix[ 2] + modelMatrix[13] * projectionMatrix[ 6] + modelMatrix[14] * projectionMatrix[10] + modelMatrix[15] * projectionMatrix[14];
    clipMatrix[15] = modelMatrix[12] * projectionMatrix[ 3] + modelMatrix[13] * projectionMatrix[ 7] + modelMatrix[14] * projectionMatrix[11] + modelMatrix[15] * projectionMatrix[15];

    /* Extract the numbers for the RIGHT plane */
    frustum[0][0] = clipMatrix[ 3] - clipMatrix[ 0];
    frustum[0][1] = clipMatrix[ 7] - clipMatrix[ 4];
    frustum[0][2] = clipMatrix[11] - clipMatrix[ 8];
    frustum[0][3] = clipMatrix[15] - clipMatrix[12];

    /* Normalize the result */
    t = sqrt( frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2] );
    frustum[0][0] /= t;
    frustum[0][1] /= t;
    frustum[0][2] /= t;
    frustum[0][3] /= t;

    /* Extract the numbers for the LEFT plane */
    frustum[1][0] = clipMatrix[ 3] + clipMatrix[ 0];
    frustum[1][1] = clipMatrix[ 7] + clipMatrix[ 4];
    frustum[1][2] = clipMatrix[11] + clipMatrix[ 8];
    frustum[1][3] = clipMatrix[15] + clipMatrix[12];

    /* Normalize the result */
    t = sqrt( frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2] );
    frustum[1][0] /= t;
    frustum[1][1] /= t;
    frustum[1][2] /= t;
    frustum[1][3] /= t;

    /* Extract the BOTTOM plane */
    frustum[2][0] = clipMatrix[ 3] + clipMatrix[ 1];
    frustum[2][1] = clipMatrix[ 7] + clipMatrix[ 5];
    frustum[2][2] = clipMatrix[11] + clipMatrix[ 9];
    frustum[2][3] = clipMatrix[15] + clipMatrix[13];

    /* Normalize the result */
    t = sqrt( frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2] );
    frustum[2][0] /= t;
    frustum[2][1] /= t;
    frustum[2][2] /= t;
    frustum[2][3] /= t;

    /* Extract the TOP plane */
    frustum[3][0] = clipMatrix[ 3] - clipMatrix[ 1];
    frustum[3][1] = clipMatrix[ 7] - clipMatrix[ 5];
    frustum[3][2] = clipMatrix[11] - clipMatrix[ 9];
    frustum[3][3] = clipMatrix[15] - clipMatrix[13];

    /* Normalize the result */
    t = sqrt( frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2] );
    frustum[3][0] /= t;
    frustum[3][1] /= t;
    frustum[3][2] /= t;
    frustum[3][3] /= t;

    /* Extract the FAR plane */
    frustum[4][0] = clipMatrix[ 3] - clipMatrix[ 2];
    frustum[4][1] = clipMatrix[ 7] - clipMatrix[ 6];
    frustum[4][2] = clipMatrix[11] - clipMatrix[10];
    frustum[4][3] = clipMatrix[15] - clipMatrix[14];

    /* Normalize the result */
    t = sqrt( frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2] );
    frustum[4][0] /= t;
    frustum[4][1] /= t;
    frustum[4][2] /= t;
    frustum[4][3] /= t;

    /* Extract the NEAR plane */
    frustum[5][0] = clipMatrix[ 3] + clipMatrix[ 2];
    frustum[5][1] = clipMatrix[ 7] + clipMatrix[ 6];
    frustum[5][2] = clipMatrix[11] + clipMatrix[10];
    frustum[5][3] = clipMatrix[15] + clipMatrix[14];

    /* Normalize the result */
    t = sqrt( frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2] );
    frustum[5][0] /= t;
    frustum[5][1] /= t;
    frustum[5][2] /= t;
    frustum[5][3] /= t;
}

static bool isSphereInFrustum(double x, double y, double z, double r)
{
    int p;

    for( p = 0; p < 6; p++) {
        if ( frustum[p][0] * x + frustum[p][1] * y + frustum[p][2] * z + frustum[p][3] <= -r) {
            return false;
        }
    }
    return true;
}

/*static void drawTexturedQuad(int texId, float size,
                        double x, double y, double z,
                        float angleX, float angleY, float angleZ,
                        float u1, float v1, float u2, float v2)
{
    float size_half = size / 2;

    // exit if this quad is not in viewing frustum
    if (!isSphereInFrustum(x,y,z, size_half)) {
        return;
    }

    setTexture(texId);

    glPushMatrix();
    glTranslatef(x,y,z);
    glRotatef((GLfloat)angleX,(GLfloat)1.0f,(GLfloat)0.0f,(GLfloat)0.0f);
    glRotatef((GLfloat)angleY,(GLfloat)0.0f,(GLfloat)1.0f,(GLfloat)0.0f);
    glRotatef((GLfloat)angleZ,(GLfloat)0.0f,(GLfloat)0.0f,(GLfloat)1.0f);

    glBegin(GL_QUADS);
        glColor4f(1.0f,1.0f,1.0f,0.5f);
        glTexCoord2f(u1, v1);
        glVertex3f( -size_half, size_half, 0.0f);
        glTexCoord2f(u2, v1);
        glVertex3f( size_half, size_half, 0.0f);
        glTexCoord2f(u2, v2);
        glVertex3f( size_half, -size_half, 0.0f);
        glTexCoord2f(u1, v2);
        glVertex3f( -size_half, -size_half, 0.0f);
    glEnd();

    glPopMatrix();
}*/

// billboarding code taken from http://www.lighthouse3d.com/opengl/billboarding/

static void drawTexturedBillboard(int texId, float width, float height,
                           double x, double y, double z,
                           float r, float g, float b, float alpha,
                           float u1, float v1, float u2, float v2)
{
    float width_half = width / 2;
    float height_half = height / 2;
    Vector objToCamProj,lookAt,upAux,objToCam;
    double angleCosine;

    // exit if this billboard is not in viewing frustum
    if (!isSphereInFrustum(x,y,z, width_half > height_half ? width_half : height_half)) {
        return;
    }

    setTexture(texId);

    glPushMatrix();
    glTranslatef(x,y,z);

    // objToCamProj is the vector in world coordinates from the
    // local origin to the camera projected in the XZ plane
    objToCamProj.x = lastViewX - x;
    objToCamProj.y = 0.0f;
    objToCamProj.z = lastViewZ - z;

    // This is the original lookAt vector for the object
    // in world coordinates
    lookAt.x = 0.0f;
    lookAt.y = 0.0f;
    lookAt.z = 1.0f;

    // normalize both vectors to get the cosine directly afterwards
    objToCamProj.normalize();

    // easy fix to determine wether the angle is negative or positive
    // for positive angles upAux will be a vector pointing in the
    // positive y direction, otherwise upAux will point downwards
    // effectively reversing the rotation.
    upAux = crossProduct(lookAt,objToCamProj);

    // compute the angle
    angleCosine = dotProduct(lookAt,objToCamProj);

    // perform the rotation. The if statement is used for stability reasons
    // if the lookAt and objToCamProj vectors are too close together then
    // |angleCosine| could be bigger than 1 due to lack of precision
    //if ((angleCosine < 0.99990) && (angleCosine > -0.9999)) {
        glRotatef(acos(angleCosine)*180/M_PI,upAux.x, upAux.y, upAux.z);
    //}

    // so far it is just like the cylindrical billboard. The code for the
    // second rotation comes now
    // The second part tilts the object so that it faces the camera

    // objToCam is the vector in world coordinates from
    // the local origin to the camera
    objToCam.x = lastViewX - x;
    objToCam.y = lastViewY - y;
    objToCam.z = lastViewZ - z;

    // Normalize to get the cosine afterwards
    objToCam.normalize();

    // Compute the angle between objToCamProj and objToCam,
    //i.e. compute the required angle for the lookup vector

    angleCosine = dotProduct(objToCamProj,objToCam);


    // Tilt the object. The test is done to prevent instability
    // when objToCam and objToCamProj have a very small
    // angle between them

    //if ((angleCosine < 0.99990) && (angleCosine > -0.9999)) {
        if (objToCam[1] < 0)
            glRotatef(acos(angleCosine)*180/M_PI,1,0,0);
        else
            glRotatef(acos(angleCosine)*180/M_PI,-1,0,0);
    //}

    glBegin(GL_QUADS);
        glColor4f(r,g,b,alpha);
        glTexCoord2f(u1, v1);
        glVertex3f( -width_half, height_half, 0.0f);
        glTexCoord2f(u2, v1);
        glVertex3f( width_half, height_half, 0.0f);
        glTexCoord2f(u2, v2);
        glVertex3f( width_half, -height_half, 0.0f);
        glTexCoord2f(u1, v2);
        glVertex3f( -width_half, -height_half, 0.0f);
    glEnd();

    glPopMatrix();
}

// delayed draw call
struct BillboardCommand
{
    // texture
    int texId;

    // size
    float width;
    float height;

    // position of object
    double x;
    double y;
    double z;

    // alpha and color
    float r;
    float g;
    float b;
    float alpha;

    // uv coordinates
    float u1;
    float v1;
    float u2;
    float v2;
};

// list of billboards to draw
static std::map<int, std::vector<BillboardCommand> > billboardsToDraw;

// draw billboard
static int luaDrawBillboard(lua_State *L)
{
    double tx,ty,tw,th;

    if (6 != lua_gettop(L) && 10 != lua_gettop(L) && 14 != lua_gettop(L))
        return 0;

    BillboardCommand c;
    if ((! lua_islightuserdata(L, 1) || lua_isnil(L, 1)))
        return 0;

    TexturePart *tex = (TexturePart*)lua_touserdata(L, 1);
    c.texId = tex->getTexture()->getId();

    if (! c.texId)
        return 0;

    if (billboardsToDraw.count(c.texId) == 0) {
        std::vector<BillboardCommand> vector;
        billboardsToDraw[c.texId] = vector;
    }

    int texWidth = tex->getTexture()->getWidth();
    int texHeight = tex->getTexture()->getHeight();
    double fw = 1/(float)texWidth;
    double fh = 1/(float)texHeight;

    c.width = lua_tonumber(L, 2);
    c.height = lua_tonumber(L, 3);
    c.x = lua_tonumber(L, 4);
    c.y = lua_tonumber(L, 5);
    c.z = lua_tonumber(L, 6);

    if (lua_gettop(L) >= 10)
    {
        c.r = lua_tonumber(L, 7);
        c.g = lua_tonumber(L, 8);
        c.b = lua_tonumber(L, 9);
        c.alpha = lua_tonumber(L, 10);
    } else {
        c.r = 1.0f;
        c.g = 1.0f;
        c.b = 1.0f;
        c.alpha = 1.0f;
    }

    if (14 == lua_gettop(L))
    {
        tx = lua_tonumber(L, 11);
        ty = lua_tonumber(L, 12);
        tw = lua_tonumber(L, 13);
        th = lua_tonumber(L, 14);
    } else {
        tx = 0.0;
        ty = 0.0;
        tw = texWidth;
        th = texHeight;
    }

    double tx1 = tex->getX1() + (tx * fw);
    double ty1 = tex->getY1() + (ty * fh);
    double tx2 = tx1 + (tw * fw);
    double ty2 = ty1 + (fh * th);

    c.u1 = tx1;
    c.v1 = ty1;
    c.u2 = tx2;
    c.v2 = ty2;

    billboardsToDraw[c.texId].push_back(c);

    return 0;
}

// TODO: store billboards in a vertex buffers, sorted by texture before rendering
static void drawBillboards()
{
    if (billboardsToDraw.size()) {
        // set correct graphic states
        XPLMSetGraphicsState(1,1,1,0,1,1,0);
        glEnable(GL_TEXTURE_2D);
        glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /*glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glEnableClientState( GL_COLOR_ARRAY );*/

        for (std::map<int, std::vector<BillboardCommand> >::iterator iter = billboardsToDraw.begin(); iter != billboardsToDraw.end(); ++iter) {
            std::vector <BillboardCommand> vector = (*iter).second;

            for (std::vector<BillboardCommand>::iterator i = vector.begin();
                    i != vector.end(); i++)
            {
                BillboardCommand &c = *i;

                drawTexturedBillboard(c.texId, c.width, c.height,
                                      c.x, c.y, c.z,
                                      c.r, c.g, c.b, c.alpha,
                                      c.u1, c.v1, c.u2, c.v2);
            }

            /*glVertexPointer( 3, GL_FLOAT, sizeof(Vertex), &(m_VertexBuffer[0].m_Pos) );
            glTexCoordPointer( 2, GL_FLOAT, sizeof(Vertex), &(m_VertexBuffer[0].m_Tex0) );
            glColorPointer( 4, GL_FLOAT, sizeof(Vertex), &(m_VertexBuffer[0].m_Diffuse) );*/

            //glDrawArrays( GL_QUADS, 0, m_VertexBuffer.size() );
        }

        /*glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        glDisableClientState( GL_COLOR_ARRAY );*/

        glDepthMask( GL_TRUE );
        glPopAttrib();
    }
}

static xap3d::Objects3D ObjectsToDraw;

static int luaDraw3DLine(lua_State* L) {
	if (lua_isnil(L, 1) || (lua_gettop(L) != 6 && lua_gettop(L) != 10)) {
		return 0;
	}

	Line3D* new_line = new Line3D;

	if (lua_gettop(L) == 10) {
		new_line->mR = (float)lua_tonumber(L, 7);
		new_line->mG = (float)lua_tonumber(L, 8);
		new_line->mB = (float)lua_tonumber(L, 9);
		new_line->mAlpha = (float)lua_tonumber(L, 10);
	} else {
		new_line->mR = 1.0f;
		new_line->mG = 1.0f;
		new_line->mB = 1.0f;
		new_line->mAlpha = 1.0f;
	}

	new_line->mX1 = (float)lua_tonumber(L, 1);
	new_line->mY1 = (float)lua_tonumber(L, 2);
	new_line->mZ1 = (float)lua_tonumber(L, 3);
	new_line->mX2 = (float)lua_tonumber(L, 4);
	new_line->mY2 = (float)lua_tonumber(L, 5);
	new_line->mZ2 = (float)lua_tonumber(L, 6);

	ObjectsToDraw.push_back(new_line);

	return 0;
}

static int luaDraw3DCircle(lua_State* L) {
	if (lua_isnil(L, 1) || (lua_gettop(L) != 11 && lua_gettop(L) != 9 && lua_gettop(L) != 5)) {
		return 0;
	}

	Circle3D* new_circle = new Circle3D;

	if (lua_gettop(L) > 5) {
		new_circle->mR = (float)lua_tonumber(L, 6);
		new_circle->mG = (float)lua_tonumber(L, 7);
		new_circle->mB = (float)lua_tonumber(L, 8);
		new_circle->mAlpha = (float)lua_tonumber(L, 9);
	} else {
		new_circle->mR = 1.0f;
		new_circle->mG = 1.0f;
		new_circle->mB = 1.0f;
		new_circle->mAlpha = 1.0f;
	}

	new_circle->mX1 = (float)lua_tonumber(L, 1);
	new_circle->mY1 = (float)lua_tonumber(L, 2);
	new_circle->mZ1 = (float)lua_tonumber(L, 3);
	new_circle->mRadius = (float)lua_tonumber(L, 4);
	new_circle->mFilled = (int)lua_tonumber(L, 5);

	if (lua_gettop(L) == 11) {
		new_circle->mPitch = (float)lua_tonumber(L, 10);
		new_circle->mYaw = (float)lua_tonumber(L, 11);
		new_circle->mHasFixedOrientation = true;
	} else {
		new_circle->mHasFixedOrientation = false;
	}

	ObjectsToDraw.push_back(new_circle);

	return 0;
}

static int luaDraw3DAngle(lua_State* L) {
	if (lua_isnil(L, 1) || (lua_gettop(L) != 6 && lua_gettop(L) != 10 && lua_gettop(L) != 12)) {
		return 0;
	}

	Angle3D* new_angle = new Angle3D;

	if (lua_gettop(L) > 6) {
		new_angle->mR = (float)lua_tonumber(L, 7);
		new_angle->mG = (float)lua_tonumber(L, 8);
		new_angle->mB = (float)lua_tonumber(L, 9);
		new_angle->mAlpha = (float)lua_tonumber(L, 10);
	} else {
		new_angle->mR = 1.0f;
		new_angle->mG = 1.0f;
		new_angle->mB = 1.0f;
		new_angle->mAlpha = 1.0f;
	}

	if (lua_gettop(L) > 10) {
		new_angle->mPitch = (float)lua_tonumber(L, 11);
		new_angle->mYaw = (float)lua_tonumber(L, 12);
	} else {
		new_angle->mPitch = 0.0f;
		new_angle->mYaw = 0.0f;
	}

	new_angle->mX1 = (float)lua_tonumber(L, 1);
	new_angle->mY1 = (float)lua_tonumber(L, 2);
	new_angle->mZ1 = (float)lua_tonumber(L, 3);
	new_angle->mAngle = (float)lua_tonumber(L, 4);
	new_angle->mLenght = (float)lua_tonumber(L, 5);
	new_angle->mRays = (float)lua_tonumber(L, 6);

	ObjectsToDraw.push_back(new_angle);

	return 0;
}

static int luaDraw3DStandingCone(lua_State* L) {
	if (lua_isnil(L, 1) || (lua_gettop(L) != 5 && lua_gettop(L) != 9)) {
		return 0;
	}

	StandingCone3D* new_stand_cone = new StandingCone3D;

	if (lua_gettop(L) > 5) {
		new_stand_cone->mR = (float)lua_tonumber(L, 6);
		new_stand_cone->mG = (float)lua_tonumber(L, 7);
		new_stand_cone->mB = (float)lua_tonumber(L, 8);
		new_stand_cone->mAlpha = (float)lua_tonumber(L, 9);
	} else {
		new_stand_cone->mR = 1.0f;
		new_stand_cone->mG = 1.0f;
		new_stand_cone->mB = 1.0f;
		new_stand_cone->mAlpha = 1.0f;
	}

	new_stand_cone->mX1 = (float)lua_tonumber(L, 1);
	new_stand_cone->mY1 = (float)lua_tonumber(L, 2);
	new_stand_cone->mZ1 = (float)lua_tonumber(L, 3);
	new_stand_cone->mRadius = (float)lua_tonumber(L, 4);
	new_stand_cone->mHeight = (float)lua_tonumber(L, 5);

	ObjectsToDraw.push_back(new_stand_cone);

	return 0;
}

void getAngles(float* outBank, float* outAttitude, float* outHeading) {
	float q[4];

	XPLMGetDatavf(quaternion, q, 0, 4);
	*outBank = atan2(2 * (q[0] * q[1] + q[2] * q[3]), 1 - 2 * (pow(q[1], 2) + pow(q[2], 2)));
	*outAttitude = asin(2 * (q[0] * q[2] - q[3] * q[1]));
	*outHeading = atan2(2 * (q[0] * q[3] + q[1] * q[2]), 1 - 2 * (pow(q[2], 2) + pow(q[3], 2)));
}

Vector rotateToModel(const Vector& inPoint) {
	float bank, attitude, heading;
	getAngles(&bank, &attitude, &heading);

	Matrix rotX, rotY, rotZ;
	rotX = rotateX(attitude);
	rotY = rotateY(-heading);
	rotZ = rotateZ(-bank);

	return rotZ * rotX * rotY * inPoint;
}

Vector rotateToLocal(const Vector& inPoint) {
	float bank, attitude, heading;
	getAngles(&bank, &attitude, &heading);

	Matrix rotX, rotY, rotZ;
	rotX = rotateX(-attitude);
	rotY = rotateY(heading);
	rotZ = rotateZ(bank);

	return rotY * rotX * rotZ * inPoint;
}

static void draw3dAdditions() {
	XPLMSetGraphicsState(0, 0, 0, 1, 0, 0, 0);
	if (ObjectsToDraw.size()) {
		for (Objects3D::const_iterator it = ObjectsToDraw.begin(); it != ObjectsToDraw.end(); ++it) {
			glPushMatrix();
			glColor4f((*it)->mR, (*it)->mG, (*it)->mB, (*it)->mAlpha);
			(*it)->draw_element();

			glPopMatrix();
		}
	} 
}

void Line3D::draw_element() {
	glBegin(GL_LINES);
		glVertex3f(mX1, mY1, mZ1);
		glVertex3f(mX2, mY2, mZ2);
	glEnd(); 
}

void Circle3D::setupVertices(std::vector<GLfloat>& verts, const GLsizei& num_segments) {
	float theta = -2 * M_PI / float(num_segments);
	float c = cosf(theta);
	float s = sinf(theta);
	float t;

	float x = mRadius;
	float y = 0;
	float z = 0;

	for (GLsizei i = 0; i < num_segments; i++) {
		verts[3 + 3 * i] = x + mX1;
		verts[3 + 3 * i + 1] = y + mY1;
		verts[3 + 3 * i + 2] = z + mZ1;

		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}

	if (mFilled) {
		verts[0] = mX1;
		verts[1] = mY1;
		verts[2] = mZ1;

		verts[num_segments * 3 + 3] = verts[3];
		verts[num_segments * 3 + 4] = verts[4];
		verts[num_segments * 3 + 5] = verts[5];
	}
}

void Circle3D::draw_element() {
	GLsizei n_segments = (int)(50 * sqrtf(mRadius));
	std::vector<GLfloat> vertices;
	vertices.resize(n_segments * 3 + 6);

	if (mHasFixedOrientation) {
		setupVertices(vertices, n_segments);
		glTranslatef(mX1, mY1, mZ1);
		glRotatef(mPitch, 1.0f, 0.0f, 0.0f);
		glRotatef(mYaw, 0.0f, 1.0f, 0.0f);
		glTranslatef(-mX1, -mY1, -mZ1);
	} else {
		Vector objToCamProj, lookAt, upAux, objToCam;
		double angleCosine;
		Vector Pos;
		Pos.x = mX1;
		Pos.y = mY1;
		Pos.z = mZ1;

		if (viewIsExternal) {
			Vector curPos;
			curPos.x = mX1;
			curPos.y = mY1;
			curPos.z = mZ1;
			Pos = rotateToLocal(curPos);

			float q[4];
			XPLMGetDatavf(quaternion, q, 0, 4);
			float bank = atan2(2 * (q[0] * q[1] + q[2] * q[3]), 1 - 2 * (pow(q[1], 2) + pow(q[2], 2)));
			float attitude = asin(2 * (q[0] * q[2] - q[3] * q[1]));
			float heading = atan2(2 * (q[0] * q[3] + q[1] * q[2]), 1 - 2 * (pow(q[2], 2) + pow(q[3], 2)));
			glRotatef(bank * 180.0 / M_PI, 0.0f, 0.0f, 1.0f);
			glRotatef(-attitude * 180.0 / M_PI, 1.0f, 0.0f, 0.0f);
			glRotatef(heading * 180.0 / M_PI, 0.0f, 1.0f, 0.0f);  					
		}

		mX1 = Pos.x;
		mY1 = Pos.y;
		mZ1 = Pos.z;
		setupVertices(vertices, n_segments);

		glTranslatef(Pos.x, Pos.y, Pos.z);

		objToCamProj.x = (lastViewX - lastLocalX) - Pos.x;
		objToCamProj.y = 0.0f;
		objToCamProj.z = (lastViewZ - lastLocalZ) - Pos.z;

		lookAt.x = 0.0f;
		lookAt.y = 0.0f;
		lookAt.z = 1.0f;

		objToCamProj.normalize();
		upAux = crossProduct(lookAt, objToCamProj);
		angleCosine = dotProduct(lookAt, objToCamProj);
		glRotatef(acos(angleCosine) * 180 / M_PI, upAux.x, upAux.y, upAux.z);
	
		objToCam.x = (lastViewX - lastLocalX) - Pos.x;
		objToCam.y = (lastViewY - lastLocalY) - Pos.y;
		objToCam.z = (lastViewZ - lastLocalZ) - Pos.z;
		objToCam.normalize();
		angleCosine = dotProduct(objToCamProj, objToCam);

		if (objToCam[1] < 0) {
			glRotatef(acos(angleCosine) * 180 / M_PI, 1, 0, 0);
		} else {
			glRotatef(acos(angleCosine) * 180 / M_PI, -1, 0, 0);
		}

		glTranslatef(-Pos.x, -Pos.y, -Pos.z); 
	}

	glVertexPointer(3, GL_FLOAT, 0, mFilled ? &vertices[0] : &vertices[3]);
	glDrawArrays(mFilled ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, mFilled ? n_segments + 2 : n_segments);
}

void Angle3D::draw_element() {
	std::vector<GLfloat> edge_vertices;
	edge_vertices.resize(mRays * 3 + 6);

	float distance_to_circle = mLenght * cosf(mAngle * M_PI / 360.0f);

	Circle3D circle;
	circle.mX1 = mX1;
	circle.mY1 = mY1;
	circle.mZ1 = mZ1 - distance_to_circle;
	circle.mFilled = false;
	circle.mRadius = mLenght * sinf(mAngle * M_PI / 360.0f);
	circle.setupVertices(edge_vertices, mRays);

	glTranslatef(mX1, mY1, mZ1);
	glRotatef(mPitch, 1.0f, 0.0f, 0.0f);
	glRotatef(mYaw, 0.0f, 1.0f, 0.0f);
	glTranslatef(-mX1, -mY1, -mZ1);

	glBegin(GL_LINES);
	for (std::size_t i = 3; i < mRays * 3 + 3; i += 3) {
		glVertex3f(mX1, mY1, mZ1);
		glVertex3f(edge_vertices[i], edge_vertices[i + 1], edge_vertices[i + 2]);
	}
	glEnd();
}

void StandingCone3D::draw_element() {
	GLsizei n_segments = (int)(50 * sqrtf(mRadius));
	std::vector<GLfloat> vertices;
	vertices.resize(n_segments * 3 + 6);

	Circle3D circle;
	circle.mX1 = mX1;
	circle.mY1 = mY1; 
	circle.mZ1 = mZ1 - mHeight;
	circle.mRadius = mRadius;
	circle.mFilled = true;
	circle.setupVertices(vertices, n_segments);

	glTranslatef(mX1, mY1, mZ1);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(-mX1, -mY1, -mZ1);

	glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, n_segments + 2);
	
	vertices[0] = mX1;
	vertices[1] = mY1;
	vertices[2] = mZ1;

	glDrawArrays(GL_TRIANGLE_FAN, 0, n_segments + 2);
}

static int luaLocalToModel(lua_State* L) {
	Vector inV;
	inV.x = (double)lua_tonumber(L, 1) - XPLMGetDatad(localx);
	inV.y = (double)lua_tonumber(L, 2) - XPLMGetDatad(localy);
	inV.z = (double)lua_tonumber(L, 3) - XPLMGetDatad(localz);

	Vector outV = rotateToModel(inV);

	lua_pushnumber(L, outV.x);
	lua_pushnumber(L, outV.y);
	lua_pushnumber(L, outV.z);

	return 3;
}

static int luaModelToLocal(lua_State* L) {
	Vector inV;
	inV.x = (double)lua_tonumber(L, 1);
	inV.y = (double)lua_tonumber(L, 2);
	inV.z = (double)lua_tonumber(L, 3);

	Vector outV = rotateToLocal(inV);

	lua_pushnumber(L, outV.x + XPLMGetDatad(localx));
	lua_pushnumber(L, outV.y + XPLMGetDatad(localy));
	lua_pushnumber(L, outV.z + XPLMGetDatad(localz));

	return 3;
}

void xap3d::draw3d(XPLMDrawingPhase phase)
{
    if (phase == xplm_Phase_Airplanes) {

        if ((hdr_pass_dr && XPLMGetDatai(hdr_pass_dr) == 1) || ++hdr_pass == 1)
        {
            //lastViewType = XPLMGetDatai(viewType)
            lastViewPitch = XPLMGetDataf(viewPitch);
            lastViewRoll = XPLMGetDataf(viewRoll);
            lastViewHeading = XPLMGetDataf(viewHeading);

            lastViewX = XPLMGetDataf(viewX);
            lastViewY = XPLMGetDataf(viewY);
            lastViewZ = XPLMGetDataf(viewZ);

            // extract current OpenGL matrixes
            extractMatrixes();

            drawBillboards();
        } 

	} else if (phase == xplm_Phase_LastScene) {
		lastLocalX = XPLMGetDatad(localx);
		lastLocalY = XPLMGetDatad(localy);
		lastLocalZ = XPLMGetDatad(localz);

		glDisable(GL_CULL_FACE);
		glPushMatrix();
		if (XPLMGetDatai(viewIsExternal)) {
			glTranslatef(lastLocalX, lastLocalY, lastLocalZ);
			XPLMGetDatavf(quaternion, lastQuat, 0, 4);
			float bank = atan2(2 * (lastQuat[0] * lastQuat[1] + lastQuat[2] * lastQuat[3]), 1 - 2 * (pow(lastQuat[1], 2) + pow(lastQuat[2], 2)));
			float attitude = asin(2 * (lastQuat[0] * lastQuat[2] - lastQuat[3] * lastQuat[1]));
			float heading = atan2(2 * (lastQuat[0] * lastQuat[3] + lastQuat[1] * lastQuat[2]), 1 - 2 * (pow(lastQuat[2], 2) + pow(lastQuat[3], 2)));
			glRotatef(heading * 180.0 / M_PI, 0.0f, -1.0f, 0.0f);
			glRotatef(attitude * 180.0 / M_PI, 1.0f, 0.0f, 0.0f);
			glRotatef(bank * 180.0 / M_PI, 0.0f, 0.0f, -1.0f);
		} 
		draw3dAdditions();
		glPopMatrix();
		glEnable(GL_CULL_FACE);
	} 
}

void xap3d::frameFinished()
{
    billboardsToDraw.clear();
	
	for (Objects3D::iterator it = ObjectsToDraw.begin(); it != ObjectsToDraw.end(); ++it) {
		delete *it;
	}	
	ObjectsToDraw.clear();
	
    hdr_pass = 0;
}

void xap3d::exportDraw3dFunctions(lua_State *L)
{
    LUA_REGISTER(L, "drawBillboard", luaDrawBillboard);
	LUA_REGISTER(L, "draw3DLine", luaDraw3DLine);
	LUA_REGISTER(L, "draw3DCircle", luaDraw3DCircle);
	LUA_REGISTER(L, "draw3DAngle", luaDraw3DAngle);
	LUA_REGISTER(L, "draw3DStandingCone", luaDraw3DStandingCone);
	LUA_REGISTER(L, "LocalToModel", luaLocalToModel);
	LUA_REGISTER(L, "ModelToLocal", luaModelToLocal);
}


