#include "graph.h"

#include <math.h>
#include <assert.h>

#include "avionics.h"
#include "font.h"

using namespace xa;


static void setupMatrix(Avionics *avionics, double x, double y, 
        double width, double height,
        double originalWidth, double originalHeight)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

    graphics->translate_transform(graphics, x, y);
    graphics->scale_transform(graphics, 
            width / originalWidth, height / originalHeight);
}

/// Lua wrapper for setupMatrix
static int luaSetupMatrix(lua_State *L)
{
    setupMatrix(getAvionics(L), lua_tonumber(L, 1), lua_tonumber(L, 2), 
            lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), 
            lua_tonumber(L, 6));
    return 0;
}


static void saveContext(Avionics *avionics)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);
    graphics->push_transform(graphics);
}

/// Lua wrapper for saveContext
static int luaSaveContext(lua_State *L)
{
    saveContext(getAvionics(L));
    return 0;
}

static void restoreContext(Avionics *avionics)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);
    graphics->pop_transform(graphics);
}

/// Lua wrapper for saveContext
static int luaRestoreContext(lua_State *L)
{
    restoreContext(getAvionics(L));
    return 0;
}


/// Draw white frame (for debugging)
static void drawFrame(Avionics *avionics, double x, double y, 
        double width, double height)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

    graphics->draw_line(graphics, x, y, x + width, y, 1, 1, 1, 1);
    graphics->draw_line(graphics, x + width, y, x + width, y + height, 1, 1, 1, 1);
    graphics->draw_line(graphics, x + width, y + height, x, y + height, 1, 1, 1, 1);
    graphics->draw_line(graphics, x, y + height, x, y, 1, 1, 1, 1);
}

/// Lua wrapper for drawFrame
static int luaDrawFrame(lua_State *L)
{
    drawFrame(getAvionics(L), lua_tonumber(L, 1), lua_tonumber(L, 2), 
            lua_tonumber(L, 3), lua_tonumber(L, 4));
    return 0;
}

/// Draw non-filled rectangle
static void drawRectangle(Avionics *avionics, double x, double y, 
        double width, double height,
        double r, double g, double b, double a)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

    graphics->draw_triangle(graphics, 
            x, y + height, r, g, b, a,
            x + width, y + height, r, g, b, a,
            x + width, y, r, g, b, a);
    graphics->draw_triangle(graphics, 
            x, y + height, r, g, b, a,
            x + width, y, r, g, b, a,
            x, y, r, g, b, a);
}

/// Lua wrapper for drawRectangle
static int luaDrawRectangle(lua_State *L)
{
    drawRectangle(getAvionics(L), lua_tonumber(L, 1), lua_tonumber(L, 2), 
            lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), 
            lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));
    return 0;
}

/// Draw triangle of specified color
static void drawTriangle(Avionics *avionics, double x1, double y1, double x2, double y2,
        double x3, double y3,
        double r, double g, double b, double a)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

    graphics->draw_triangle(graphics, 
            x1, y1, r, g, b, a,
            x2, y2, r, g, b, a,
            x3, y3, r, g, b, a);
}

/// Lua wrapper for drawTriangle
static int luaDrawTriangle(lua_State *L)
{
    drawTriangle(getAvionics(L), lua_tonumber(L, 1), lua_tonumber(L, 2), 
            lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), 
            lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8), 
            lua_tonumber(L, 9), lua_tonumber(L, 10));
    return 0;
}

/// Draw circle with triangles
static void drawCircle(Avionics* avionics, double cx, double cy, double R, int segments,
	double r, double g, double b, double a) {

	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	std::size_t num_segments = segments;
	
	if (segments == -1) {
		num_segments = (int)(10 * sqrtf(R));
	} 

	float theta = -2 * 3.141592f / float(num_segments);
	float c = cosf(theta);
	float s = sinf(theta);
	float t;

	float x = R;
	float y = 0;

	float curVertX1, curVertY1;
	float curVertX2, curVertY2;

	curVertX1 = x + cx;
	curVertY1 = y + cy;

	t = x;
	x = c * x - s * y;
	y = s * t + c * y;

	for (std::size_t i = 0; i < num_segments; i++) {

		curVertX2 = x + cx;
		curVertY2 = y + cy;

		graphics->draw_triangle(graphics, cx, cy, r, g, b, a, 
										  curVertX1, curVertY1, r, g, b, a, 
										  curVertX2, curVertY2, r, g, b, a);
		
		curVertX1 = curVertX2;
		curVertY1 = curVertY2;

		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}
}

/// Lua wrapper for drawCircle
static int luaDrawCircle(lua_State *L)
{
	if (lua_gettop(L) != 3 && lua_gettop(L) != 4 && lua_gettop(L) != 8 && lua_gettop(L) != 7) {
		return 0;
	}

	int circle_segments;
	float r, g, b, a;

	if (lua_gettop(L) > 3) {
		circle_segments = (int)lua_tonumber(L, 4);
	} else {
		circle_segments = -1;
	}

	if (lua_gettop(L) > 6) {
		r = (float)lua_tonumber(L, 5);
		g = (float)lua_tonumber(L, 6);
		b = (float)lua_tonumber(L, 7);
	} else {
		r = 1.0f;
		g = 1.0f;
		b = 1.0f;
	}

	if (lua_gettop(L) > 7) {
		a = (float)lua_tonumber(L, 8);
	} else {
		a = 1.0f;
	}

	drawCircle(getAvionics(L), lua_tonumber(L, 1), lua_tonumber(L, 2),
		lua_tonumber(L, 3), circle_segments, r, g, b, a);

	return 0;
}

/// Draw line
static void drawLine(Avionics *avionics, double x1, double y1, 
        double x2, double y2, double r, double g, double b, double a)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

    graphics->draw_line(graphics, x1, y1, x2, y2, r, g, b, a);
}

/// Lua wrapper for drawLine
static int luaDrawLine(lua_State *L)
{
    drawLine(getAvionics(L), lua_tonumber(L, 1), lua_tonumber(L, 2), 
            lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), 
            lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));
    return 0;
}


static void drawTexture(Avionics *avionics, TexturePart *tex,
        double x, double y, double width, double height,
        float r, float g, float b, float a, bool is_upside_down)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

	if (is_upside_down) {
		graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
			x, y + height, tex->getX1(), tex->getY2(), r, g, b, a,
			x + width, y + height, tex->getX2(), tex->getY2(), r, g, b, a,
			x + width, y, tex->getX2(), tex->getY1(), r, g, b, a);
		graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
			x, y + height, tex->getX1(), tex->getY2(), r, g, b, a,
			x + width, y, tex->getX2(), tex->getY1(), r, g, b, a,
			x, y, tex->getX1(), tex->getY1(), r, g, b, a);
	} else {
		graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
			x, y + height, tex->getX1(), tex->getY1(), r, g, b, a,
			x + width, y + height, tex->getX2(), tex->getY1(), r, g, b, a,
			x + width, y, tex->getX2(), tex->getY2(), r, g, b, a);
		graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
			x, y + height, tex->getX1(), tex->getY1(), r, g, b, a,
			x + width, y, tex->getX2(), tex->getY2(), r, g, b, a,
			x, y, tex->getX1(), tex->getY2(), r, g, b, a);
	}
}


/// Read rgba from Lua call arguments
/// If rgb is not specified does nothinf
/// if alpha component is not specified it keep default
static void rgbaFromLua(lua_State *L, int base, float &r, float &g,
        float &b, float &a)
{
    if (base + 2 <= lua_gettop(L)) {
        r = lua_tonumber(L, base);
        g = lua_tonumber(L, base + 1);
        b = lua_tonumber(L, base + 2);
        if (base + 3 <= lua_gettop(L))
            a = lua_tonumber(L, base + 3);
    }
}


/// Lua wrapper for drawTexture
static int luaDrawTexture(lua_State *L)
{
    if ((! lua_islightuserdata(L, 1) || lua_isnil(L, 1)))
        return 0;

    TexturePart *tex = (TexturePart*)lua_touserdata(L, 1);
    Avionics *avionics = getAvionics(L);

    float r, g, b, a;
    avionics->getBackgroundColor(r, g, b, a);
    rgbaFromLua(L, 6, r, g, b, a);

	int is_upside_down;

	if (lua_gettop(L) == 10) {
		is_upside_down = (int)lua_tonumber(L, 10);
	} else {
		is_upside_down = 0;
	}

    drawTexture(avionics, tex, lua_tonumber(L, 2), lua_tonumber(L, 3), 
            lua_tonumber(L, 4), lua_tonumber(L, 5), r, g, b, a, is_upside_down);

    return 0;
}

/// draw texture by ID, corresponds to component render target
static void drawRenderTarget(Avionics *avionics, int id,
	double x, double y, double width, double height,
	float r, float g, float b, float a) {

	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	graphics->draw_textured_triangle(graphics, id,
		x, y + height, 0, 1, r, g, b, a,
		x + width, y + height, 1, 1, r, g, b, a,
		x + width, y, 1, 0, r, g, b, a);
	graphics->draw_textured_triangle(graphics, id,
		x, y + height, 0, 1, r, g, b, a,
		x + width, y, 1, 0, r, g, b, a,
		x, y, 0, 0, r, g, b, a);
}

/// Lua wrapper for drawRenderTarget
static int luaDrawRenderTarget(lua_State* L) {
	if (lua_isnil(L, 1) || lua_gettop(L) != 9) {
		return 0;
	}
	
	Avionics *avionics = getAvionics(L);
	assert(avionics);

	float r, g, b, a;
	avionics->getBackgroundColor(r, g, b, a);
	rgbaFromLua(L, 6, r, g, b, a);

	drawRenderTarget(avionics, lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3),
		lua_tonumber(L, 4), lua_tonumber(L, 5), r, g, b, a);

	return 0;
}

static void drawTextureCoords(Avionics *avionics, TexturePart *tex,
	double x1, double y1, double x2, double y2, double x3, double y3,
	double x4, double y4,
	float r, float g, float b, float a)
{
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
		x1, y1, tex->getX1(), tex->getY1(), r, g, b, a,
		x2, y2, tex->getX2(), tex->getY1(), r, g, b, a,
		x3, y3, tex->getX2(), tex->getY2(), r, g, b, a);
	graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
		x1, y1, tex->getX1(), tex->getY1(), r, g, b, a,
		x3, y3, tex->getX2(), tex->getY2(), r, g, b, a,
		x4, y4, tex->getX1(), tex->getY2(), r, g, b, a);
}

/// Lua wrapper for drawTextureCoord
static int luaDrawTextureCoords(lua_State *L)
{
	if ((!lua_islightuserdata(L, 1) || lua_isnil(L, 1)))
		return 0;

	TexturePart *tex = (TexturePart*)lua_touserdata(L, 1);
	Avionics *avionics = getAvionics(L);

	float r, g, b, a;
	avionics->getBackgroundColor(r, g, b, a);
	rgbaFromLua(L, 10, r, g, b, a);

	drawTextureCoords(avionics, tex, lua_tonumber(L, 2), lua_tonumber(L, 3),
		lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6),
		lua_tonumber(L, 7), lua_tonumber(L, 8), lua_tonumber(L, 9),
		r, g, b, a);

	return 0;
}

/// draw texture with ability to set texture coords
static void drawTexturePart(Avionics *avionics, TexturePart *tex, 
        double x, double y, double width, double height, 
        double tx, double ty, double tw, double th,
        float r, float g, float b, float a, int is_upside_down)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

	double pw, ph, tx1, ty1, tx2, ty2;
	pw = tex->getX2() - tex->getX1();
	ph = tex->getY2() - tex->getY1();

	tx1 = tex->getX1() + pw * tx;
	ty1 = tex->getY1() + ph * ty;
	tx2 = tx1 + pw * tw;
	ty2 = ty1 + ph * th;

	if (is_upside_down) {	
		graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
			x, y + height, tx1, ty2, r, g, b, a,
			x + width, y + height, tx2, ty2, r, g, b, a,
			x + width, y, tx2, ty1, r, g, b, a);
		graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
			x, y + height, tx1, ty2, r, g, b, a,
			x + width, y, tx2, ty1, r, g, b, a,
			x, y, tx1, ty1, r, g, b, a);
	} else {
		graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
			x, y + height, tx1, ty1, r, g, b, a,
			x + width, y + height, tx2, ty1, r, g, b, a,
			x + width, y, tx2, ty2, r, g, b, a);
		graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
			x, y + height, tx1, ty1, r, g, b, a,
			x + width, y, tx2, ty2, r, g, b, a,
			x, y, tx1, ty2, r, g, b, a);
	}
}


/// Lua wrapper for drawTexturePart
static int luaDrawTexturePart(lua_State *L)
{
    if ((! lua_islightuserdata(L, 1) || lua_isnil(L, 1)))
        return 0;

    TexturePart *tex = (TexturePart*)lua_touserdata(L, 1);
    Avionics *avionics = getAvionics(L);
    
	int is_upside_down;

	if (lua_gettop(L) == 14) {
		is_upside_down = (int)lua_tonumber(L, 14);
	} else {
		is_upside_down = 0;
	}

    float r, g, b, a;
    avionics->getBackgroundColor(r, g, b, a);
    rgbaFromLua(L, 10, r, g, b, a);

    drawTexturePart(avionics, tex, lua_tonumber(L, 2), lua_tonumber(L, 3), 
            lua_tonumber(L, 4), lua_tonumber(L, 5),
            lua_tonumber(L, 6), lua_tonumber(L, 7), 
            lua_tonumber(L, 8), lua_tonumber(L, 9),
            r, g, b, a, is_upside_down);

    return 0;
}


static void drawRotatedTexture(Avionics *avionics, TexturePart *tex, 
        double angle, double x, double y, double width, double height,
        float r, float g, float b, float a, int is_upside_down)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

    graphics->push_transform(graphics);

    double centerX = x + width / 2.0;
    double centerY = y + height / 2.0;
    graphics->translate_transform(graphics, centerX, centerY);
    graphics->rotate_transform(graphics, angle);
    graphics->translate_transform(graphics, -centerX, -centerY);

    drawTexture(avionics, tex, x, y, width, height, r, g, b, a, is_upside_down);

    graphics->pop_transform(graphics);
}


/// Lua wrapper for drawRotatedTexture
static int luaDrawRotatedTexture(lua_State *L)
{
    if ((! lua_islightuserdata(L, 1) || lua_isnil(L, 1)))
        return 0;

    TexturePart *tex = (TexturePart*)lua_touserdata(L, 1);
    Avionics *avionics = getAvionics(L);
    
    float r, g, b, a;
    avionics->getBackgroundColor(r, g, b, a);
    rgbaFromLua(L, 7, r, g, b, a);

	int is_upside_down;

	if (lua_gettop(L) == 11) {
		is_upside_down = (int)lua_tonumber(L, 11);
	} else {
		is_upside_down = 0;
	}

    drawRotatedTexture(avionics, tex, lua_tonumber(L, 2), 
            lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), 
            lua_tonumber(L, 6), r, g, b, a, is_upside_down);

    return 0;
}

static void drawRotatedTextureCenter(Avionics *avionics, TexturePart *tex,
	double angle, double c_x, double c_y, double x, double y, double width, double height,
	float r, float g, float b, float a, int is_upside_down)
{
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	graphics->push_transform(graphics);

	double centerX = c_x;
	double centerY = c_y;
	graphics->translate_transform(graphics, centerX, centerY);
	graphics->rotate_transform(graphics, angle);
	graphics->translate_transform(graphics, -centerX, -centerY);

	drawTexture(avionics, tex, x, y, width, height, r, g, b, a, is_upside_down);

	graphics->pop_transform(graphics);
}


/// Lua wrapper for drawRotatedTextureCenter
static int luaDrawRotatedTextureCenter(lua_State *L)
{
	if ((!lua_islightuserdata(L, 1) || lua_isnil(L, 1)))
		return 0;

	TexturePart *tex = (TexturePart*)lua_touserdata(L, 1);
	Avionics *avionics = getAvionics(L);

	float r, g, b, a;
	avionics->getBackgroundColor(r, g, b, a);
	rgbaFromLua(L, 9, r, g, b, a);

	int is_upside_down;

	if (lua_gettop(L) == 13) {
		is_upside_down = (int)lua_tonumber(L, 13);
	} else {
		is_upside_down = 0;
	}

	drawRotatedTextureCenter(avionics, tex, lua_tonumber(L, 2),
		lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5),
		lua_tonumber(L, 6), lua_tonumber(L, 7),
		lua_tonumber(L, 8), r, g, b, a, is_upside_down);

	return 0;
}

static void drawRotatedTexturePart(Avionics *avionics, TexturePart *tex, 
        double angle, double x, double y, double width, double height,
        double tx, double ty, double tw, double th,
        float r, float g, float b, float a, int is_upside_down)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

    graphics->push_transform(graphics);
    double centerX = x + width / 2.0;
    double centerY = y + height / 2.0;
    graphics->translate_transform(graphics, centerX, centerY);
    graphics->rotate_transform(graphics, angle);
    graphics->translate_transform(graphics, -centerX, -centerY);

    drawTexturePart(avionics, tex, x, y, width, height, tx, ty, tw, th,
            r, g, b, a, is_upside_down);

    graphics->pop_transform(graphics);
}


/// Lua wrapper for drawRotatedTexturePart
static int luaDrawRotatedTexturePart(lua_State *L)
{
    if ((! lua_islightuserdata(L, 1) || lua_isnil(L, 1)))
        return 0;

    TexturePart *tex = (TexturePart*)lua_touserdata(L, 1);
    Avionics *avionics = getAvionics(L);
    
    float r, g, b, a;
    avionics->getBackgroundColor(r, g, b, a);
    rgbaFromLua(L, 11, r, g, b, a);
	 
	int is_upside_down;
	
	if (lua_gettop(L) == 15) {
		is_upside_down = (int)lua_tonumber(L, 15);
	} else {
		is_upside_down = 0;
	}

    drawRotatedTexturePart(avionics, tex, lua_tonumber(L, 2), 
            lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), 
            lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8), 
            lua_tonumber(L, 9), lua_tonumber(L, 10),
            r, g, b, a, is_upside_down);

    return 0;
}


/// Lua wrapper for drawFont
static int luaDrawFont(lua_State *L)
{
    if ((! lua_islightuserdata(L, 1) || lua_isnil(L, 1)))
        return 0;

    Font *font = (Font*)lua_touserdata(L, 1);
    if (! font)
        return 0;
    Avionics *avionics = getAvionics(L);

    float r, g, b, a;
    avionics->getBackgroundColor(r, g, b, a);
    rgbaFromLua(L, 5, r, g, b, a);
    
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

    drawFont(font, graphics, lua_tonumber(L, 2), lua_tonumber(L, 3), 
            lua_tostring(L, 4), r, g, b, a);

    return 0;
}


static void rotatePoint(double &x, double &y, double ox, double oy, 
        double centerX, double centerY, double angle, TexturePart *tex)
{
    double pw = tex->getX2() - tex->getX1();
    double ph = tex->getY2() - tex->getY1();

    double tx = ox - centerX;
    double ty = oy - centerY;
    x = tx * cos(angle) - ty * sin(angle) + centerX;
    y = ty * cos(angle) + tx * sin(angle) + centerY;

    x = tex->getX1() + x * pw;
    y = tex->getY1() + y * ph;
}


/// can't find good name for this
/// draw rectangle textured in strange way: texture coordinates are rotated
/// by specified angle
static void drawIntricatelyTexturedRectangle(Avionics *avionics, 
        TexturePart *tex, double angle, double x, double y, double width, 
        double height, double tx, double ty, double tw, double th,
        float r, float g, float b, float a)
{
    SaslGraphicsCallbacks *graphics = avionics->getGraphics();
    assert(graphics);

    double tx1 = tx;
    double ty1 = ty;
    double tx2 = tx1 + tw;
    double ty2 = ty1 + th;

    double tcx = (tx2 + tx1) / 2;
    double tcy = (ty2 + ty1) / 2;

    double c1x, c1y;
    rotatePoint(c1x, c1y, tx1, ty1, tcx, tcy, angle, tex);
    double c2x, c2y;
    rotatePoint(c2x, c2y, tx2, ty1, tcx, tcy, angle, tex);
    double c3x, c3y;
    rotatePoint(c3x, c3y, tx2, ty2, tcx, tcy, angle, tex);
    double c4x, c4y;
    rotatePoint(c4x, c4y, tx1, ty2, tcx, tcy, angle, tex);
    
    graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
            x, y + height, c1x, c1y, r, g, b, a,
            x + width, y + height, c2x, c2y, r, g, b, a,
            x + width, y, c3x, c3y, r, g, b, a);
    graphics->draw_textured_triangle(graphics, tex->getTexture()->getId(),
            x, y + height, c1x, c1y, r, g, b, a,
            x + width, y, c3x, c3y, r, g, b, a,
            x, y, c4x, c4y, r, g, b, a);
}


/// Lua wrapper for drawIntricatelyTexturedRectangle
static int luaDrawIntricatelyTexturedRectangle(lua_State *L)
{
    if ((! lua_islightuserdata(L, 1) || lua_isnil(L, 1)))
        return 0;

    TexturePart *tex = (TexturePart*)lua_touserdata(L, 1);
    Avionics *avionics = getAvionics(L);
    
    float r, g, b, a;
    avionics->getBackgroundColor(r, g, b, a);
    rgbaFromLua(L, 11, r, g, b, a);

    drawIntricatelyTexturedRectangle(avionics, tex, lua_tonumber(L, 2), 
            lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), 
            lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8), 
            lua_tonumber(L, 9), lua_tonumber(L, 10),
            r, g, b, a);

    return 0;
}

/// Lua wrapper for drawMask
static int luaDrawMask(lua_State* L) {
	Avionics *avionics = getAvionics(L);
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	graphics->draw_mask(graphics);

	return 0;
}

/// Lua wrapper for drawUnderMask
static int luaDrawUnderMask(lua_State* L) {
	Avionics *avionics = getAvionics(L);
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	graphics->draw_under_mask(graphics);

	return 0;
}

/// Lua wrapper for drawMaskEnd
static int luaDrawMaskEnd(lua_State* L) {
	Avionics *avionics = getAvionics(L);
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	graphics->draw_mask_end(graphics);

	return 0;
}

static int luaSetClipArea(lua_State *L)
{
	Avionics *avionics = getAvionics(L);
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	graphics->set_clip_area(graphics, lua_tonumber(L, 1), lua_tonumber(L, 2),
		lua_tonumber(L, 3), lua_tonumber(L, 4));

	return 0;
}


static int luaResetClipArea(lua_State *L)
{
	Avionics *avionics = getAvionics(L);
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	graphics->reset_clip_area(graphics);

	return 0;
}

static int luaSetBlendFunc(lua_State *L)
{
	if (lua_gettop(L) != 2 && lua_gettop(L) != 4) {
		return 0;
	}
	Avionics *avionics = getAvionics(L);
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	if (lua_gettop(L) == 2) {
		graphics->set_blend_func(graphics, lua_tonumber(L, 1), lua_tonumber(L, 2));
	} else {
		graphics->set_blend_func_separate(graphics, lua_tonumber(L, 1), lua_tonumber(L, 2),
			lua_tonumber(L, 3), lua_tonumber(L, 3));
	}

	return 0;
}

static int luaSetBlendEquation(lua_State *L) {
	if (lua_gettop(L) > 2) {
		return 0;
	}

	Avionics *avionics = getAvionics(L);
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	if (lua_gettop(L) == 1) {
		graphics->set_blend_equation(graphics, lua_tonumber(L, 1));
	} else {
		graphics->set_blend_equation_separate(graphics, lua_tonumber(L, 1), lua_tonumber(L, 2));
	}

	return 0;
}

static int luaResetBlending(lua_State *L) {
	Avionics *avionics = getAvionics(L);
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	graphics->reset_blending(graphics);
	return 0;
}

static int luaSetBlendColor(lua_State *L) {
	if (lua_gettop(L) != 4) {
		return 0;
	}

	Avionics *avionics = getAvionics(L);
	SaslGraphicsCallbacks *graphics = avionics->getGraphics();
	assert(graphics);

	graphics->set_blend_color(graphics, lua_tonumber(L, 1), lua_tonumber(L, 2),
		lua_tonumber(L, 3), lua_tonumber(L, 3));
	return 0;
}

void xa::exportGraphToLua(Luna &lua)
{
    lua_State *L = lua.getLua();

    LUA_REGISTER(L, "setTranslation", luaSetupMatrix);
    LUA_REGISTER(L, "saveGraphicsContext", luaSaveContext);
    LUA_REGISTER(L, "restoreGraphicsContext", luaRestoreContext);
    LUA_REGISTER(L, "drawFrame", luaDrawFrame);
    LUA_REGISTER(L, "drawTexture", luaDrawTexture);
	LUA_REGISTER(L, "drawRenderTarget", luaDrawRenderTarget);
	LUA_REGISTER(L, "drawTextureCoords", luaDrawTextureCoords);
    LUA_REGISTER(L, "drawRotatedTexture", luaDrawRotatedTexture);
	LUA_REGISTER(L, "drawRotatedTextureCenter", luaDrawRotatedTextureCenter);
    LUA_REGISTER(L, "drawTexturePart", luaDrawTexturePart);
    LUA_REGISTER(L, "drawRotatedTexturePart", luaDrawRotatedTexturePart);
    LUA_REGISTER(L, "drawRectangle", luaDrawRectangle);
    LUA_REGISTER(L, "drawTriangle", luaDrawTriangle);
	LUA_REGISTER(L, "drawCircle", luaDrawCircle);
    LUA_REGISTER(L, "drawLine", luaDrawLine);
    LUA_REGISTER(L, "drawText", luaDrawFont);
    LUA_REGISTER(L, "drawTexturedRect", luaDrawIntricatelyTexturedRectangle);
	LUA_REGISTER(L, "drawMask", luaDrawMask);
	LUA_REGISTER(L, "drawUnderMask", luaDrawUnderMask);
	LUA_REGISTER(L, "drawMaskEnd", luaDrawMaskEnd);
	LUA_REGISTER(L, "setClipArea", luaSetClipArea);
	LUA_REGISTER(L, "resetClipArea", luaResetClipArea);
	LUA_REGISTER(L, "setBlendFunc", luaSetBlendFunc);
	LUA_REGISTER(L, "setBlendEquation", luaSetBlendEquation);
	LUA_REGISTER(L, "resetBlending", luaResetBlending);
	LUA_REGISTER(L, "setBlendColor", luaSetBlendColor);
}

