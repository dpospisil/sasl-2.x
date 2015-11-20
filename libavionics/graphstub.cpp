#include "graphstub.h"

using namespace xa;


static void drawBegin(struct SaslGraphicsCallbacks *canvas)
{
}


/// flush drawed graphics to screen
static void drawEnd(struct SaslGraphicsCallbacks *canvas)
{
}


/// load texture to memory.
/// Returns texture ID or -1 on failure.  On success returns texture width
//  and height in pixels
static int loadTexture(struct SaslGraphicsCallbacks *canvas,
        const char *buf, int length, int *width, int *height)
{
    return -1;
}


// Unload texture from video memory.
static void freeTexture(struct SaslGraphicsCallbacks *canvas, int textureId)
{
}


// draw line of specified color.
static void drawLine(struct SaslGraphicsCallbacks *canvas, double x1,
        double y1, double x2, double y2, double r, double g, double b, double a)
{
}


// draw untextured triangle.
static void drawTriangle(struct SaslGraphicsCallbacks *canvas, 
        double x1, double y1, double r1, double g1, double b1, double a1,
        double x2, double y2, double r2, double g2, double b2, double a2,
        double x3, double y3, double r3, double g3, double b3, double a3)
{
}


// draw textured triangle.
static void drawTexturedTriangle(struct SaslGraphicsCallbacks *canvas, 
        int textureId,
        double x1, double y1, double u1, double v1, double r1, double g1, double b1, double a1,
        double x2, double y2, double u2, double v2, double r2, double g2, double b2, double a2,
        double x3, double y3, double u3, double v3, double r3, double g3, double b3, double a3)
{
}

// enable masking, prepare to draw it
static void drawMask(struct SaslGraphicsCallbacks *canvas) {

}

// prepare to draw under mask
static void drawUnderMask(struct SaslGraphicsCallbacks *canvas) {

}

// disable masking
static void drawMaskEnd(struct SaslGraphicsCallbacks *canvas) {

}

// enable clipping to rectangle
static void setClipArea(struct SaslGraphicsCallbacks *canvas, 
        double x1, double y1, double x2, double y2)
{
}


// disable clipping.
static void resetClipArea(struct SaslGraphicsCallbacks *canvas)
{
}


// push affine translation state
static void pushTransform(struct SaslGraphicsCallbacks *canvas)
{
}

// pop affine transform state
static void popTransform(struct SaslGraphicsCallbacks *canvas)
{
}


// apply move transform to current state
static void translateTransform(struct SaslGraphicsCallbacks *canvas, 
        double x, double y)
{
}


// apply scale transform to current state
static void scaleTransform(struct SaslGraphicsCallbacks *canvas, 
        double x, double y)
{
}

// apply rotate transform to current state
static void rotateTransform(struct SaslGraphicsCallbacks *canvas, 
        double angle)
{
}


// find sasl texture in memory by size and marker color
// returns texture id or -1 if not found
static int findTexture(struct SaslGraphicsCallbacks *canvas, 
        int width, int height, int *r, int *g, int *b, int *a)
{
    return -1;
}

// start rendering to texture
// pass -1 as texture ID to restore default render target
static int setRenderTarget(struct SaslGraphicsCallbacks *canvas, 
        int textureId, bool clear)
{
    return -1;
}

//generate framebuffers and renderbuffers for components on INIT stage
//return -1 on errors or render-target-texture ID on success
static int getNewRenderTargetID(struct SaslGraphicsCallbacks *canvas,
	int context_width, int context_height) {
	return -1;
}

// create new texture of specified size and store it under the same name 
// as old texture
// use it for textures used as render target
static void recreateTexture(struct SaslGraphicsCallbacks *canvas, 
        int textureId, int width, int height)
{
}

// Sets blending function for drawings
static void setBlendFunc(struct SaslGraphicsCallbacks *canvas, int srcBlend, int dstBlend) 
{
}

// Sets separate blending functions(RGB, Alpha) for drawings
static void setBlendFuncSeparate(struct SaslGraphicsCallbacks *canvas, int srcBlendRGB, int dstBlendRGB,
	int srcBlendAlpha, int dstBlendAlpha) 
{
}

// Sets blending equation for drawings
static void setBlendEquation(struct SaslGraphicsCallbacks *canvas, int blendMode) 
{
}

// Sets separate blending equations(RGB, Alpha) for drawings
static void setBlendEquationSeparate(struct SaslGraphicsCallbacks *canvas, int blendModeRGB, int blendModeAlpha) 
{
}

// Resets standard blending
static void resetBlending(struct SaslGraphicsCallbacks *canvas) 
{
}

// Sets specific blending color
static void setBlendColor(struct SaslGraphicsCallbacks *canvas, float R, float G, float B, float A)
{
}

static struct SaslGraphicsCallbacks callbacks = { drawBegin, drawEnd,
    loadTexture, freeTexture, drawLine, drawTriangle, drawTexturedTriangle,
	drawMask, drawUnderMask, drawMaskEnd,
    setClipArea, resetClipArea, pushTransform, popTransform, 
    translateTransform, scaleTransform, rotateTransform, findTexture,
    setRenderTarget, getNewRenderTargetID, recreateTexture, setBlendFunc,
	setBlendFuncSeparate, setBlendEquation, setBlendEquationSeparate, resetBlending,
	setBlendColor};


SaslGraphicsCallbacks* xa::getGraphicsStub()
{
    return &callbacks;
}


