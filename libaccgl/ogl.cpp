#include "ogl.h"

#include <list>
#include <map>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <SOIL.h>
#include "glheaders.h"
#include "math2d.h"

#ifndef APL
#include <GL/gl.h>
#endif 

#ifdef LIN
#include <GL/glx.h>
#endif

#ifndef APL

// OpenGL functions
typedef GLenum(*CheckFramebufferStatus)(GLenum target);
static CheckFramebufferStatus glCheckFramebufferStatus;

typedef void(*GenRenderbuffers)(GLsizei n, GLuint *buffers);
static GenRenderbuffers glGenRenderbuffers = NULL;

typedef void(*DeleteRenderbuffers)(GLsizei n, GLuint* buffers);
static DeleteRenderbuffers glDeleteRenderbuffers = NULL;

typedef void(*BindRenderbuffer)(GLenum target, GLuint renderbuffer);
static BindRenderbuffer glBindRenderbuffer = NULL;

typedef void(*RenderbufferStorage)(GLenum target, GLenum internalformat,
	GLsizei width, GLsizei height);
static RenderbufferStorage glRenderbufferStorage = NULL;

typedef void(*FramebufferRenderbuffer)(GLenum target, GLenum attachment,
	GLenum renderbuffertarget, GLuint renderbuffer);
static FramebufferRenderbuffer glFramebufferRenderbuffer = NULL;

typedef void(*GenFramebuffers)(GLsizei n, GLuint *buffers);
static GenFramebuffers glGenFramebuffers = NULL;

typedef void(*BindFramebuffer)(GLenum target, GLuint framebuffer);
static BindFramebuffer glBindFramebuffer = NULL;

typedef void(*FramebufferTexture2D)(GLenum target, GLenum attachment,
	GLenum textarget, GLuint texture, GLint level);
static FramebufferTexture2D glFramebufferTexture2D = NULL;

typedef void(*DeleteFramebuffers)(GLsizei n, GLuint *buffers);
static DeleteFramebuffers glDeleteFramebuffers = NULL;

typedef void(*GenerateMipmap)(GLenum target);
static GenerateMipmap glGenerateMipmap = NULL;

typedef void(*BlendFuncSeparate)(GLenum srcRGB, GLenum destRGB, GLenum srcAlpha, GLenum destAlpha);
static BlendFuncSeparate glBlendFuncSeparate = NULL;

typedef void(*BlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
static BlendEquationSeparate glBlendEquationSeparate = NULL;

// opengl defines
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE 				0x8CD5
#endif

#ifndef GL_DRAW_FRAMEBUFFER_BINDING
#define GL_DRAW_FRAMEBUFFER_BINDING				GL_FRAMEBUFFER_BINDING
#endif

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER				0x8D40
#endif

#ifndef GL_FRAMEBUFFER_BINDING
#define GL_FRAMEBUFFER_BINDING				0x8CA6
#endif

#ifndef GL_RENDERBUFFER_BINDING
#define GL_RENDERBUFFER_BINDING				0x8CA7
#endif

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER				0x8CA9
#endif

#ifndef GL_RENDERBUFFER
#define GL_RENDERBUFFER				0x8D41
#endif

#ifndef GL_DEPTH_STENCIL
#define GL_DEPTH_STENCIL				0x84F9
#endif

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT				0x821A
#endif

#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0				0x8CE0
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE				0x812F
#endif

#ifndef GL_CONSTANT_COLOR
#define GL_CONSTANT_COLOR				0x8001
#endif

#ifndef GL_ONE_MINUS_CONSTANT_COLOR
#define GL_ONE_MINUS_CONSTANT_COLOR				0x8002
#endif

#ifndef GL_CONSTANT_ALPHA
#define GL_CONSTANT_ALPHA				0x8003
#endif

#ifndef GL_ONE_MINUS_CONSTANT_ALPHA
#define GL_ONE_MINUS_CONSTANT_ALPHA				0x8004
#endif

#ifndef GL_MIN
#define GL_MIN				0x8007
#endif

#ifndef GL_MAX
#define GL_MAX				0x8008
#endif

#ifndef GL_FUNC_ADD
#define GL_FUNC_ADD				0x8006
#endif

#ifndef GL_FUNC_SUBTRACT
#define GL_FUNC_SUBTRACT				0x800A
#endif

#ifndef GL_FUNC_REVERSE_SUBTRACT
#define GL_FUNC_REVERSE_SUBTRACT				0x800B
#endif

#endif // APL

#if !defined(LIN) && !defined(APL)
typedef void(*BlendEquation)(GLenum mode);
static BlendEquation glBlendEquation = NULL;

typedef void(*BlendColor)(GLfloat R, GLfloat G, GLfloat B, GLfloat A);
static BlendColor glBlendColor = NULL;
#endif

// rectangle, for now used to define clip areas only
struct Rect {
	double x, y, width, height;
};

// graphics context
struct OglCanvas
{
	/// pointers to this callbacks
	struct SaslGraphicsCallbacks callbacks;

	/// texture binder function (or NULL if not available)
	saslgl_bind_texture_2d_callback binderCallback;

	/// texture ID generator (or NULL if not available)
	saslgl_gen_tex_name_callback genTexNameCallback;

	// number of triangles drawn
	int triangles;

	/// number of lines drawn
	int lines;

	/// number of textures loaded
	int textures;

	/// size of loaded textures
	int texturesSize;

	// number of batches drawn
	int batches;

	// number of batches because of texture changed
	int batchTex;

	// number of batches because of translations
	int batchTrans;

	// number of batches because of untextured geometry
	int batchNoTex;

	// number of batches because of switch to lines
	int batchLines;

	/// current texture ID or 0 if texturing disabled
	int currentTexture;

	/// what to draw: GL_LINES or GL_TRIANGLES
	int currentMode;

	/// maximum size of vertex buffer
	GLsizei maxVertices;

	/// current number of vertices in buffer
	GLsizei numVertices;

	/// vertices buffer
	GLfloat *vertexBuffer;

	/// texture coords buffer
	GLfloat *texBuffer;

	/// vertex colors buffer
	GLfloat *colorBuffer;

	// transformation stack
	std::vector<Matrix> transform;

	// true if FBO functions allowed to use
	bool fboAvailable;
	
	// true if advanced blending available
	bool advancedBlendingAvailable;
	
	// map of FBOs by texture IDs
	std::map<int, GLuint> fboByTex;

	// RBOs, binded to FBOs
	std::vector<GLuint> RBOs;
	
	// default fbo
	GLuint defaultFbo;
	
	// stored components render targets
	std::vector<GLuint> renderTargetsIDs;	
	
	// render targets for specified components
	std::size_t componentsRenderTargetsCounter;

	// texture assigned to current fbo
	int currentFboTex;
};



/// initialize graphics before frame start
static void drawBegin(struct SaslGraphicsCallbacks *canvas)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_SCISSOR_TEST);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if (c->vertexBuffer) {
		glVertexPointer(2, GL_FLOAT, 0, c->vertexBuffer);
		glTexCoordPointer(2, GL_FLOAT, 0, c->texBuffer);
		glColorPointer(4, GL_FLOAT, 0, c->colorBuffer);
	}

	c->triangles = 0;
	c->lines = 0;
	c->batches = 0;

	c->batchTex = 0;
	c->batchTrans = 0;
	c->batchNoTex = 0;
	c->batchLines = 0;

	c->numVertices = 0;
	c->currentTexture = -1;
	c->currentMode = GL_TRIANGLES;

	c->transform.clear();
	c->transform.push_back(Matrix::identity());
}


/// make sure vertex buffer is large enough to fit qty vertices
/// \param qty - additional vertices to add to vertex buffer
static void reserveSpace(OglCanvas *c, GLsizei qty)
{
	if (c->numVertices + qty > c->maxVertices) {
		c->maxVertices += (qty / 1024 + 1) * 1024;
		std::size_t s = sizeof(GLfloat) * (std::size_t)c->maxVertices;

		GLfloat* vertexRB = (GLfloat*)realloc(c->vertexBuffer, 2 * s);
		GLfloat* texRB = (GLfloat*)realloc(c->texBuffer, 2 * s);
		GLfloat* colorRB = (GLfloat*)realloc(c->colorBuffer, 4 * s);

		if (vertexRB) {
			c->vertexBuffer = vertexRB;
		}
		if (texRB) {
			c->texBuffer = texRB;
		}
		if (colorRB) {
			c->colorBuffer = colorRB;
		}

		glVertexPointer(2, GL_FLOAT, 0, c->vertexBuffer);
		glTexCoordPointer(2, GL_FLOAT, 0, c->texBuffer);
		glColorPointer(4, GL_FLOAT, 0, c->colorBuffer);
	}
}


/// Add vertex to buffers
static void addVertex(OglCanvas *c, GLfloat x, GLfloat y,
	GLfloat r, GLfloat g, GLfloat b, GLfloat a,
	GLfloat u, GLfloat v)
{
	reserveSpace(c, 1);

	Vector rv = c->transform.back() * Vector(x, y);
	x = rv.getX();
	y = rv.getY();

	std::size_t i = (std::size_t)c->numVertices * 2;
	c->vertexBuffer[i] = x;
	c->vertexBuffer[i + 1] = y;

	c->texBuffer[i] = u;
	c->texBuffer[i + 1] = v;

	i = (std::size_t)c->numVertices * 4;
	c->colorBuffer[i] = r;
	c->colorBuffer[i + 1] = g;
	c->colorBuffer[i + 2] = b;
	c->colorBuffer[i + 3] = a;

	c->numVertices++;
}

/// draw vertices accumulated in buffers
static void dumpBuffers(OglCanvas *c)
{
	if (c->numVertices) {
		glDrawArrays(c->currentMode, 0, c->numVertices);
		c->numVertices = 0;
		c->batches++;
	}
}



/// flush drawed graphics to screen
static void drawEnd(struct SaslGraphicsCallbacks *canvas)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);

	dumpBuffers(c);

	glPopAttrib();
	glPopClientAttrib();
	/*    printf("textures: %i (%i Kb) triangles: %i lines: %i  batches: %i\n",
	c->textures, c->texturesSize / 1024, c->triangles, c->lines,
	c->batches);
	printf("batches reasons: textures: %i  translation: %i  notex: %i  "
	"lines: %i\n", c->batchTex, c->batchTrans, c->batchNoTex,
	c->batchLines);*/
}


// stop texturing
static void disableTexture(OglCanvas *c)
{
	if (c->currentTexture) {
		if (c->numVertices)
			c->batchNoTex++;
		dumpBuffers(c);
		glDisable(GL_TEXTURE_2D);
		c->currentTexture = 0;
	}
}


// start texturing
static void setTexture(OglCanvas *c, int texId)
{
	if (c->currentTexture != texId) {
		if (c->numVertices)
			c->batchTex++;
		dumpBuffers(c);
		if (!c->currentTexture)
			glEnable(GL_TEXTURE_2D);
		if (c->binderCallback)
			c->binderCallback(texId);
		else
			glBindTexture(GL_TEXTURE_2D, texId);
		c->currentTexture = texId;
	}
}


// switch primitives to draw
static void setMode(OglCanvas *c, int mode)
{
	if (c->currentMode != mode) {
		if (c->numVertices)
			c->batchLines++;
		dumpBuffers(c);
		c->currentMode = mode;
	}
}


/// load texture to memory.
/// Returns texture ID or -1 on failure.  On success returns texture width
//  and height in pixels
static int loadTexture(struct SaslGraphicsCallbacks *canvas,
	const char *buffer, int length, int *width, int *height)
{
	OglCanvas *c = (OglCanvas*)canvas;
	if (!c)
		return -1;

	GLuint texId = 0;
	if (c->genTexNameCallback)
		texId = c->genTexNameCallback();

	unsigned id = SOIL_load_OGL_texture_from_memory(
		(const unsigned char*)buffer, length,
		0, texId, SOIL_FLAG_POWER_OF_TWO);
	if (!id)
		return -1;

	texId = id;

	// because of SOIL issue
	setTexture(c, id);

	if (width) {
		GLint w;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		*width = w;
	}
	if (height) {
		GLint h;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
		*height = h;
	}

	c->textures++;

	if (width && height)
		c->texturesSize += (*width) * (*height);

	return texId;
}


// Unload texture from video memory.
static void freeTexture(struct SaslGraphicsCallbacks *canvas, int textureId)
{
	GLuint id = (GLuint)textureId;
	glDeleteTextures(1, &id);
}


// draw line of specified color.
static void drawLine(struct SaslGraphicsCallbacks *canvas, double x1,
	double y1, double x2, double y2, double r, double g, double b, double a)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);

	c->lines++;

	disableTexture(c);
	setMode(c, GL_LINES);

	addVertex(c, x1, y1, r, g, b, a, 0, 0);
	addVertex(c, x2, y2, r, g, b, a, 0, 0);
}


// draw untextured triangle.
static void drawTriangle(struct SaslGraphicsCallbacks *canvas,
	double x1, double y1, double r1, double g1, double b1, double a1,
	double x2, double y2, double r2, double g2, double b2, double a2,
	double x3, double y3, double r3, double g3, double b3, double a3)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);

	c->triangles++;

	disableTexture(c);
	setMode(c, GL_TRIANGLES);

	addVertex(c, x1, y1, r1, g1, b1, a1, 0, 0);
	addVertex(c, x2, y2, r2, g2, b2, a2, 0, 0);
	addVertex(c, x3, y3, r3, g3, b3, a3, 0, 0);
}


// draw textured triangle.
static void drawTexturedTriangle(struct SaslGraphicsCallbacks *canvas,
	int textureId,
	double x1, double y1, double u1, double v1, double r1, double g1, double b1, double a1,
	double x2, double y2, double u2, double v2, double r2, double g2, double b2, double a2,
	double x3, double y3, double u3, double v3, double r3, double g3, double b3, double a3)
{
	OglCanvas *c = (OglCanvas*)canvas;
	if (!c)
		return;

	c->triangles++;

	setTexture(c, textureId);
	setMode(c, GL_TRIANGLES);

	addVertex(c, x1, y1, r1, g1, b1, a1, u1, v1);
	addVertex(c, x2, y2, r2, g2, b2, a2, u2, v2);
	addVertex(c, x3, y3, r3, g3, b3, a3, u3, v3);
}

// enable masking before drawing its form
static void drawMask(struct SaslGraphicsCallbacks* canvas) {
	OglCanvas* c = (OglCanvas*)canvas;
	assert(canvas);

	dumpBuffers(c);

	glPushAttrib(GL_STENCIL_BUFFER_BIT);
	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);

	glStencilMask(1);
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

// before drawing under mask
static void drawUnderMask(struct SaslGraphicsCallbacks* canvas) {
	OglCanvas* c = (OglCanvas*)canvas;
	assert(canvas);

	dumpBuffers(c);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

// disable masking
static void drawMaskEnd(struct SaslGraphicsCallbacks* canvas) {
	OglCanvas* c = (OglCanvas*)canvas;
	assert(canvas);

	dumpBuffers(c);

	glDisable(GL_STENCIL_TEST);
	glPopAttrib();

}

// enable clipping to rectangle
static void setClipArea(struct SaslGraphicsCallbacks *canvas,
	double x, double y, double width, double height)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);

	dumpBuffers(c);

	Rect r = { x, y, width, height };
	glEnable(GL_SCISSOR_TEST);

	Vector rv1 = c->transform.back() * Vector(r.x, r.y);
	float x1 = rv1.getX();
	float y1 = rv1.getY();
	Vector rv2 = c->transform.back() * Vector(r.width, r.height);
	float x2 = rv2.getX();
	float y2 = rv2.getY();

	glScissor(x1, y1, x2 - x1, y2 - y1);
}


// disable clipping
static void resetClipArea(struct SaslGraphicsCallbacks *canvas)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	dumpBuffers(c);
	glDisable(GL_SCISSOR_TEST);
}


// push affine translation state
static void pushTransform(struct SaslGraphicsCallbacks *canvas)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	Matrix m = c->transform.back();
	c->transform.push_back(m);
}

// pop affine transform state
static void popTransform(struct SaslGraphicsCallbacks *canvas)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	if (1 < c->transform.size())
		c->transform.pop_back();
	else
		printf("invalid pop!\n");
}


// apply move transform to current state
static void translateTransform(struct SaslGraphicsCallbacks *canvas,
	double x, double y)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	/*    if (c->numVertices)
	c->batchTrans++;
	dumpBuffers(c);
	glTranslated(x, y, 0);*/
	c->transform.back() = Matrix::translate(x, y) * c->transform.back();
}


// apply scale transform to current state
static void scaleTransform(struct SaslGraphicsCallbacks *canvas,
	double x, double y)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	/*if (c->numVertices)
	c->batchTrans++;
	dumpBuffers(c);
	glScaled(x, y, 1.0f);*/
	c->transform[c->transform.size() - 1] = Matrix::scale(x, y) * c->transform.back();
}

// apply rotate transform to current state
static void rotateTransform(struct SaslGraphicsCallbacks *canvas,
	double angle)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	/*if (c->numVertices)
	c->batchTrans++;
	dumpBuffers(c);
	glRotated(angle, 0, 0, -1.0);*/
	c->transform.back() = Matrix::rotate(-angle) * c->transform.back();
}


// find sasl texture in memory by size and marker color
// returns texture id or -1 if not found
static int findTexture(struct SaslGraphicsCallbacks *canvas,
	int width, int height, int *r, int *g, int *b, int *a)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);

	unsigned char *buf = new unsigned char[4 * std::size_t(width * height)];

	for (GLuint i = 0; i < 2048; i++) {
		if (glIsTexture(i)) {
			GLint w, h;
			glBindTexture(GL_TEXTURE_2D, i);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
			if ((w == width) && (h == height)) {
				if (a && r && g && b) {
					glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
					if ((10 > abs(*r - buf[0])) && (10 > abs(*g - buf[1])) &&
						(10 > abs(*b - buf[2])) && (10 > abs(*a - buf[3])))
					{
						if (c->currentTexture)
							glBindTexture(GL_TEXTURE_2D, c->currentTexture);
						delete[] buf;
						return i;
					}
				}
				else {
					if (c->currentTexture)
						glBindTexture(GL_TEXTURE_2D, c->currentTexture);
					delete[] buf;
					return i;
				}
			}
		}
	}

	if (c->currentTexture)
		glBindTexture(GL_TEXTURE_2D, c->currentTexture);
	delete[] buf;

	return -1;
}



// returns fbo currently binded
static GLuint getCurrentFbo()
{
	GLuint oldFbo;

	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&oldFbo);

	return oldFbo;
}



// find or allocate FBO object
static GLuint getFbo(OglCanvas *c, int textureId, int w, int h)
{
	std::map<int, GLuint>::const_iterator i = c->fboByTex.find(textureId);
	if (i == c->fboByTex.end()) {
		GLuint rbo;
		GLint previous;

		glGetIntegerv(GL_RENDERBUFFER_BINDING, &previous);

		// allocating new depth_stencil rbo
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, previous);

		GLuint fbo = -1;
		glGenFramebuffers(1, &fbo);
		GLuint oldFbo = getCurrentFbo();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, textureId, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		// job done, switch to old fbo
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldFbo);
		c->fboByTex[textureId] = fbo;
		c->RBOs.push_back(rbo);
		return fbo;
	}
	else
		return (*i).second;
}

// setup matrices and optional clear context
static void prepareFbo(OglCanvas *c, int textureId, int width, int height, bool clear)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	if (clear) {
		glClear(GL_COLOR_BUFFER_BIT);
	}
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	c->transform.push_back(Matrix::identity());
}


// start rendering to texture
// pass -1 as texture ID to restore default render target
static int setRenderTarget(struct SaslGraphicsCallbacks *canvas,
	int textureId, bool clear)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);

	dumpBuffers(c);

	if (!c->fboAvailable) {
		printf("fbo not available\n");
		return -1;
	}

	if (-1 != textureId) {
		glEnable(GL_TEXTURE_2D);
		// save state
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		GLint w, h;
		glBindTexture(GL_TEXTURE_2D, textureId);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

		// enable fbo
		c->defaultFbo = getCurrentFbo();
		GLuint fbo = getFbo(c, textureId, w, h);
		if ((GLuint)-1 == fbo) {
			printf("can't create fbo\n");
			return -1;
		}
		c->currentFboTex = textureId;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glBindTexture(GL_TEXTURE_2D, 0);

		prepareFbo(c, textureId, w, h, clear);
	}
	else {
		// restore default fbo
		glBindFramebuffer(GL_FRAMEBUFFER, c->defaultFbo);
		glBindTexture(GL_TEXTURE_2D, c->currentFboTex);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		// restore x-plane state
		glPopAttrib();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		if (c->currentTexture)
			glBindTexture(GL_TEXTURE_2D, c->currentTexture);

		c->transform.pop_back();
	}

	return 0;
}

// creates new render target for components
static int getNewRenderTargetID(struct SaslGraphicsCallbacks *canvas, int context_width, int context_height) {

	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);

	GLuint newTexID = NULL;
	GLuint newFBOID;
	GLuint newRBOID;
	GLint XP_FBO, XP_RBO;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &XP_FBO);

	if (c->genTexNameCallback) {
		newTexID = c->genTexNameCallback();
	}


	std::vector<GLubyte> emptyData(context_width * context_height * 4, 0);
	glBindTexture(GL_TEXTURE_2D, newTexID);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, context_width, context_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &emptyData[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, NULL);

	glGetIntegerv(GL_RENDERBUFFER_BINDING, &XP_RBO);

	glGenRenderbuffers(1, &newRBOID);
	glBindRenderbuffer(GL_RENDERBUFFER, newRBOID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, context_width, context_height);
	glBindRenderbuffer(GL_RENDERBUFFER, XP_RBO);

	glGenFramebuffers(1, &newFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER, newFBOID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newTexID, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newRBOID);
	glBindFramebuffer(GL_FRAMEBUFFER, XP_FBO);

	c->fboByTex.insert(c->fboByTex.begin(), std::make_pair(newTexID, newFBOID));
	c->RBOs.insert(c->RBOs.begin(), newRBOID);
	
	c->renderTargetsIDs.push_back(newTexID);
	c->componentsRenderTargetsCounter++;

	return newTexID;
}

// Sets blending function for drawings
static void setBlendFunc(struct SaslGraphicsCallbacks *canvas, int srcBlend, int dstBlend) {
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	dumpBuffers(c);

	glBlendFunc(GLenum(srcBlend), GLenum(dstBlend));
}

// Sets separate blending functions(RGB, Alpha) for drawings
static void setBlendFuncSeparate(struct SaslGraphicsCallbacks *canvas, int srcBlendRGB, int dstBlendRGB,
	int srcBlendAlpha, int dstBlendAlpha) {


	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	if (!c->advancedBlendingAvailable) {
		return;
	}
	dumpBuffers(c);

	glBlendFuncSeparate(GLenum(srcBlendRGB), GLenum(dstBlendRGB), 
		GLenum(srcBlendAlpha), GLenum(dstBlendAlpha));
}

// Sets blending equation for drawings
static void setBlendEquation(struct SaslGraphicsCallbacks *canvas, int blendMode) {
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	if (!c->advancedBlendingAvailable) {
		return;
	}
	dumpBuffers(c);

	glBlendEquation(GLenum(blendMode));
}

// Sets separate blending equations(RGB, Alpha) for drawings
static void setBlendEquationSeparate(struct SaslGraphicsCallbacks *canvas, int blendModeRGB, int blendModeAlpha) {
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	if (!c->advancedBlendingAvailable) {
		return;
	}
	dumpBuffers(c);

	glBlendEquationSeparate(GLenum(blendModeRGB), GLenum(blendModeAlpha));
}

// Sets specific blending color
static void setBlendColor(struct SaslGraphicsCallbacks *canvas, float R, float G, float B, float A) {
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	if (!c->advancedBlendingAvailable) {
		return;
	}
	dumpBuffers(c);

	glBlendColor(R, G, B, A);
}

// Resets standard blending
static void resetBlending(struct SaslGraphicsCallbacks *canvas) {
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);
	dumpBuffers(c);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (!c->advancedBlendingAvailable) {
		return;
	}
	glBlendEquation(GLenum(0x8006));
}


// create new texture of specified size and store it under the same name 
// as old texture
// use it for textures used as render target
static void recreateTexture(struct SaslGraphicsCallbacks *canvas,
	int textureId, int width, int height)
{
	OglCanvas *c = (OglCanvas*)canvas;
	assert(canvas);

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
		GL_BYTE, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	if (c->currentTexture)
		glBindTexture(GL_TEXTURE_2D, c->currentTexture);
}


#ifndef APL
// returns address of OpenGL functions.  check EXT variants if normal not found
typedef void(*Func)();
static Func getProcAddress(const char *name)
{
#ifndef WINDOWS
	Func res = glXGetProcAddressARB((GLubyte*)name);
#else
	Func res = (Func)wglGetProcAddress(name);
#endif
	if (!res) {
		char buf[250];
		strcpy(buf, name);
		strcat(buf, "EXT");
#ifndef WINDOWS
		res = glXGetProcAddressARB((GLubyte*)buf);
#else
		res = (Func)wglGetProcAddress(buf);
#endif
	}
	return res;
}
#endif // APL

// find pointers of OpenGL functions for advanced blending
static bool initAdvBlendingGLfunctions() {
#ifndef APL
	glBlendFuncSeparate = (BlendFuncSeparate)getProcAddress("glBlendFuncSeparate");
	glBlendEquationSeparate = (BlendEquationSeparate)getProcAddress("glBlendEquationSeparate");
#endif // APL

#if !defined(APL) && !defined(LIN)
	glBlendEquation = (BlendEquation)getProcAddress("glBlendEquation");
	glBlendColor = (BlendColor)getProcAddress("glBlendColor");
#endif

	return glBlendFuncSeparate && glBlendEquation && glBlendEquationSeparate &&
		glBlendColor;
}

// find pointers of OpenGL functions for FBO using
static bool initFBOGlfunctions()
{
#ifndef APL    
	glGenRenderbuffers = (GenRenderbuffers)getProcAddress("glGenRenderbuffers");
	glBindRenderbuffer = (BindRenderbuffer)getProcAddress("glBindRenderbuffer");
	glRenderbufferStorage = (RenderbufferStorage)getProcAddress("glRenderbufferStorage");
	glFramebufferRenderbuffer = (FramebufferRenderbuffer)getProcAddress("glFramebufferRenderbuffer");
	glCheckFramebufferStatus = (CheckFramebufferStatus)getProcAddress("glCheckFramebufferStatus");
	glDeleteRenderbuffers = (DeleteRenderbuffers)getProcAddress("glDeleteRenderbuffers");

	glGenFramebuffers = (GenFramebuffers)getProcAddress("glGenFramebuffers");
	glBindFramebuffer = (BindFramebuffer)getProcAddress("glBindFramebuffer");
	glFramebufferTexture2D = (FramebufferTexture2D)getProcAddress("glFramebufferTexture2D");
	glDeleteFramebuffers = (DeleteFramebuffers)getProcAddress("glDeleteFramebuffers");
	glGenerateMipmap = (GenerateMipmap)getProcAddress("glGenerateMipmap");
#endif // APL

	return glGenFramebuffers && glBindFramebuffer && glFramebufferTexture2D &&
		glDeleteFramebuffers && glGenerateMipmap && glGenRenderbuffers && glBindRenderbuffer &&
		glRenderbufferStorage && glFramebufferRenderbuffer && glCheckFramebufferStatus && glDeleteRenderbuffers;
}




// initializa canvas structure
struct SaslGraphicsCallbacks* saslgl_init_graphics()
{
	OglCanvas *c = new OglCanvas;

	c->callbacks.draw_begin = drawBegin;
	c->callbacks.draw_end = drawEnd;
	c->callbacks.load_texture = loadTexture;
	c->callbacks.free_texture = freeTexture;
	c->callbacks.draw_line = drawLine;
	c->callbacks.draw_triangle = drawTriangle;
	c->callbacks.draw_textured_triangle = drawTexturedTriangle;
	c->callbacks.draw_mask = drawMask;
	c->callbacks.draw_under_mask = drawUnderMask;
	c->callbacks.draw_mask_end = drawMaskEnd;
	c->callbacks.set_clip_area = setClipArea;
	c->callbacks.reset_clip_area = resetClipArea;
	c->callbacks.push_transform = pushTransform;
	c->callbacks.pop_transform = popTransform;
	c->callbacks.translate_transform = translateTransform;
	c->callbacks.scale_transform = scaleTransform;
	c->callbacks.rotate_transform = rotateTransform;
	c->callbacks.find_texture = findTexture;
	c->callbacks.set_render_target = setRenderTarget;
	c->callbacks.get_new_render_target_id = getNewRenderTargetID;
	c->callbacks.recreate_texture = recreateTexture;
	c->callbacks.set_blend_func = setBlendFunc;
	c->callbacks.set_blend_func_separate = setBlendFuncSeparate;
	c->callbacks.set_blend_equation = setBlendEquation;
	c->callbacks.set_blend_equation_separate = setBlendEquationSeparate;
	c->callbacks.reset_blending = resetBlending;
	c->callbacks.set_blend_color = setBlendColor;

	c->binderCallback = NULL;
	c->genTexNameCallback = NULL;
	c->maxVertices = c->numVertices = 0;
	c->vertexBuffer = c->texBuffer = c->colorBuffer = NULL;
	c->fboAvailable = initFBOGlfunctions();
	c->advancedBlendingAvailable = initAdvBlendingGLfunctions();
	c->triangles = c->lines = c->textures = c->texturesSize = 0;
	c->batches = c->batchTrans = c->batchNoTex = c->batchLines = 0;
	c->currentTexture = 0;
	c->defaultFbo = 0;
	c->currentFboTex = 0;

	return (struct SaslGraphicsCallbacks*)c;
}


// free canvas structure
void saslgl_done_graphics(struct SaslGraphicsCallbacks *canvas)
{
	OglCanvas *c = (OglCanvas*)canvas;

	if (c) {
		if (c->fboByTex.size()) {
			for (std::map<int, GLuint>::iterator i = c->fboByTex.begin();
				i != c->fboByTex.end(); ++i) {

				glDeleteFramebuffers(1, &(*i).second);
			}
		}

		for (std::vector<GLuint>::iterator it = c->RBOs.begin(); it != c->RBOs.end(); ++it) {
			glDeleteRenderbuffers(1, &(*it));
		}

		for (std::vector<GLuint>::iterator it = c->renderTargetsIDs.begin(); 
			it != c->renderTargetsIDs.end(); ++it) {

			glDeleteTextures(1, (GLuint*)&(*it));
		}

		free(c->vertexBuffer);
		free(c->texBuffer);
		free(c->colorBuffer);
		delete c;
	}
}

void saslgl_set_texture2d_binder_callback(struct SaslGraphicsCallbacks *canvas,
	saslgl_bind_texture_2d_callback binder)
{
	OglCanvas *c = (OglCanvas*)canvas;
	if (!c)
		return;
	c->binderCallback = binder;
}

/// Setup texture name generator function.
/// \param canvas graphics canvas.
/// \param generator ID generator. if NULL default OpenGL function will be used
void saslgl_set_gen_tex_name_callback(struct SaslGraphicsCallbacks *canvas,
	saslgl_gen_tex_name_callback generator)
{
	OglCanvas *c = (OglCanvas*)canvas;
	if (!c)
		return;
	c->genTexNameCallback = generator;
}


