#include <fstream>
#include <iostream>
#include <math.h>
#include <time.h>


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write_clean.h"

#include "surface.hpp"
#include "utils.hpp"


#define ACES_a 0.0245786f
#define ACES_b 0.0245786f
#define ACES_c 0.000090537f
#define ACES_d 0.983729f
#define ACES_e 0.4329510f


// Private macro of Surface class
#define _SET_SURF_AT(x, y, color) _surfData[(y)*surfWidth + (x)] = (color)
#define _SURF_AT(x, y) _surfData[(y)*surfWidth + (x)]



// --------- Constructors ---------
Surface::Surface() {}

Surface::Surface(Color* data, int w, int h): surfWidth(w), surfHeight(h) {
    surfSize  = surfWidth * surfHeight;
    surfAspectRatio = (float)surfWidth/surfHeight;
    _surfData = data;
}




// --------- Private Methods ---------
void Surface::_aces() {
    for (int i=0; i<surfSize; i++) {
        _surfData[i].r = std::max(0.f, (float)(_surfData[i].r*(_surfData[i].r+ACES_a) - ACES_b) / (_surfData[i].r * (_surfData[i].r*ACES_c + ACES_d) + ACES_e));
        _surfData[i].g = std::max(0.f, (float)(_surfData[i].g*(_surfData[i].g+ACES_a) - ACES_b) / (_surfData[i].g * (_surfData[i].g*ACES_c + ACES_d) + ACES_e));
        _surfData[i].b = std::max(0.f, (float)(_surfData[i].b*(_surfData[i].b+ACES_a) - ACES_b) / (_surfData[i].b * (_surfData[i].b*ACES_c + ACES_d) + ACES_e));
    }
}

void Surface::_reinhard() {
    for (int i=0; i<surfSize; i++) {
        _surfData[i].r = (float) _surfData[i].r/(1+_surfData[i].r);
        _surfData[i].g = (float) _surfData[i].g/(1+_surfData[i].g);
        _surfData[i].b = (float) _surfData[i].b/(1+_surfData[i].b);
    }
}

void Surface::_gamma() {
    for (int i=0; i<surfSize; i++) {
        _surfData[i].r = sqrtf(_surfData[i].r);
        _surfData[i].g = sqrtf(_surfData[i].g);
        _surfData[i].b = sqrtf(_surfData[i].b);
    }
}




// --------- Public Methods ---------
void Surface::tonemap() {
    // this->_aces();  // ACES TMO
    // this->_reinhard();  // Reinhard TMO
    this->_gamma();
}

void Surface::toU32Surface(uint32_t *buffer) {

    uint8_t r;
    uint8_t g;
    uint8_t b;

    for (int i=0; i<surfSize; i++) {
        r = std::max(0, std::min(255, int(_surfData[i].r * 0xFF)));
        g = std::max(0, std::min(255, int(_surfData[i].g * 0xFF)));
        b = std::max(0, std::min(255, int(_surfData[i].b * 0xFF)));

        buffer[i] = (uint32_t) (r<<24) | (g<<16) | (b<<8) | 0xFF;
    }
}


// Saving
int Surface::saveFloatBuffer(const char* file_path) {
    FILE *file = fopen(file_path, "wb");
    if (file == NULL) {
        return -1;
    }

    int size = surfSize * 3;
    int dim[2] = {surfWidth, surfHeight};
    fwrite(dim, sizeof(int)*2, 1, file);

    fwrite(_surfData, sizeof(float)*size, 1, file);
    fclose(file);

    return 0;
}

int Surface::savePPM(const char* file_path) {
    FILE *file = fopen(file_path, "wb");
    if (file == NULL) {
        return -1;
    }

    fprintf(file, "P6\n%d %d\n255\n", surfWidth, surfHeight);
    uint8_t *bytes = new uint8_t[3 * surfSize];

    int j=0;
    Color c;
    for (int i=0; i<surfSize; i++) {
        c = _surfData[i] * 255.f;
        bytes[j++] = std::max(0, std::min(0xff, int(c.r)));  // R
        bytes[j++] = std::max(0, std::min(0xff, int(c.g)));  // G
        bytes[j++] = std::max(0, std::min(0xff, int(c.b)));  // B
    }

    fwrite(bytes, 3*surfSize*sizeof(uint8_t), 1, file);
    fclose(file);

    delete[] bytes;
    return 0;
}

int Surface::savePNG(const char* file_name) {
    uint8_t *bytes = new uint8_t[3 * surfSize];
    Color c;

    int j = 0;
    for (int i = 0; i < surfSize; i++) {
        c = _surfData[i] * 255.f;
        bytes[j++] = std::max(0, std::min(0xff, (int)c.r));  // R
        bytes[j++] = std::max(0, std::min(0xff, (int)c.g));  // G
        bytes[j++] = std::max(0, std::min(0xff, (int)c.b));  // B
    }

    stbi_write_png(file_name, surfWidth, surfHeight, 3, bytes, 3*surfWidth*sizeof(uint8_t));
    delete[] bytes;
    return 0;
}


// Drawing Methods
void Surface::setAt(int x, int y, const Color &color) {
    _SET_SURF_AT(x, y, color);
}


void Surface::fill(const Color &color) {
	for (int i=0; i<surfSize; ++i) {
		_surfData[i] = color;
	}
}

void Surface::fillNoise() {
	srand(time(NULL));
	for (int i = 0; i < surfSize; ++i) {
		_surfData[i] = randColor();
	}
}


// Lines

// TODO: Excessive error checking required
// _DRAW_LINE(int x0, int y0, int x1, int y1, const Color &color, int lineWidth);
#define _DRAW_LINE(x0, y0, x1, y1, color, lineWidth) {      \
    if (x0 < 0 || x0 >= surfWidth  ||                       \
        x1 < 0 || x1 >= surfWidth  ||                       \
        y0 < 0 || y0 >= surfHeight ||                       \
        y1 < 0 || y1 >= surfHeight  ) {                     \
        return;                                             \
    }                                                       \
                                                            \
    int startX = x0;                                        \
    int startY = y0;                                        \
    int endX = x1;                                          \
    int endY = y1;                                          \
                                                            \
    int dx = abs(endX - startX);                            \
    int dy = abs(endY - startY);                            \
                                                            \
    if (dx == 0 && dy == 0) {                               \
        _SET_SURF_AT(startX, startY, color);                \
        return;                                             \
    }                                                       \
                                                            \
    int sx = (startX < endX) ? 1 : -1;                      \
    int sy = (startY < endY) ? 1 : -1;                      \
    int err = dx - dy;                                      \
                                                            \
    for (int i = 0; i < lineWidth; i++) {                   \
        int x = startX;                                     \
        int y = startY;                                     \
                                                            \
        while (x != endX || y != endY) {                    \
            _SET_SURF_AT(x, y, color);                      \
                                                            \
            int err2 = 2 * err;                             \
            if (err2 > -dy) {                               \
                err -= dy;                                  \
                x += sx;                                    \
            }                                               \
            if (err2 < dx) {                                \
                err += dx;                                  \
                y += sy;                                    \
            }                                               \
        }                                                   \
                                                            \
        _SET_SURF_AT(endX, endY, color);                    \
                                                            \
        startX += sy;                                       \
        endX += sy;                                         \
        startY -= sx;                                       \
        endY -= sx;                                         \
    }                                                       \
}                                                           \

void Surface::drawLine(int x0, int y0, int x1, int y1, const Color &color, int lineWidth) {
    _DRAW_LINE(x0,y0, x1,y1, color, lineWidth);
}

void Surface::drawLine(const Vec3 &v1, const Vec3 &v2, const Color &color, int lineWidth) {
    const int &x0 = v1.x;
    const int &y0 = v1.y;
    const int &x1 = v2.x;
    const int &y1 = v2.y;

    _DRAW_LINE(x0,y0, x1,y1, color, lineWidth);
}

void Surface::drawLine(const Line &line, const Color &color, int lineWidth) {
    const int &x0 = line.x0;
    const int &y0 = line.y0;
    const int &x1 = line.x1;
    const int &y1 = line.y1;

    _DRAW_LINE(x0,y0, x1,y1, color, lineWidth);
}


// Circles

// _DRAW_CIRCLE(int x, int y, int r, const Color &color, int thickness)
#define _DRAW_CIRCLE(x0, y0, r, color, thickness){          \
    if ( (r < 1) || (thickness < 1) ) return;               \
                                                            \
    int y_st = std::max(0, std::min( surfHeight, y0 - r));  \
    int y_en = std::max(0, std::min( surfHeight, y0+r+1 )); \
    int x_st = std::max(0, std::min( surfWidth,  x0 - r));  \
    int x_en = std::max(0, std::min( surfWidth,  x0+r+1 )); \
                                                            \
    int r_sq = r*r + 1;                                     \
    int rt_sq = (r-thickness)*(r-thickness);                \
                                                            \
    int d_sq;                                               \
                                                            \
    for (int y=y_st; y<y_en; y++) {                         \
        for (int x=x_st; x<x_en; x++) {                     \
                                                            \
            d_sq = (x-x0)*(x-x0) + (y-y0)*(y-y0);           \
            if ( (d_sq>rt_sq)  && (d_sq<r_sq) ) {           \
                _SET_SURF_AT(x, y, color);                  \
            }                                               \
        }                                                   \
    }                                                       \
}                                                           \

void Surface::drawCircle(int x0, int y0, int r, const Color &color, int thickness) {
    _DRAW_CIRCLE(x0, y0, r, color, thickness);
}

void Surface::drawCircle(const Vec3 &pos_vec, int r, const Color &color, int thickness) {
    const int &x0 = pos_vec.x;
    const int &y0 = pos_vec.y;

    _DRAW_CIRCLE(x0, y0, r, color, thickness);
}

void Surface::drawCircle(const Vec2 &pos_vec, int r, const Color &color, int thickness) {
    const int &x0 = pos_vec.x;
    const int &y0 = pos_vec.y;

    _DRAW_CIRCLE(x0, y0, r, color, thickness);
}

void Surface::drawCircle(const Circle &circle, const Color &color, int thickness) {
    const int &x0 = circle.x;
    const int &y0 = circle.y;
    const int &r = circle.r;

    _DRAW_CIRCLE(x0, y0, r, color, thickness);
}


// _FILL_CIRCLE(int x0, int y0, int r, const Color &color)
#define _FILL_CIRCLE(x0, y0, r, color){                       \
    if ( r < 1 )  return;                                     \
                                                              \
    int y_st = std::max(0, std::min( surfHeight, y0 - r));    \
    int y_en = std::max(0, std::min( surfHeight, y0+r+1 ));   \
    int x_st = std::max(0, std::min( surfWidth,  x0 - r));    \
    int x_en = std::max(0, std::min( surfWidth,  x0+r+1 ));   \
                                                              \
    int r_sq = r*r + 1;                                       \
    int d_sq;                                                 \
                                                              \
    for (int y=y_st; y<y_en; y++) {                           \
        for (int x=x_st; x<x_en; x++) {                       \
            d_sq = (x-x0)*(x-x0) + (y-y0)*(y-y0);             \
            if ( d_sq < r_sq ) {                              \
                _SET_SURF_AT( x, y, color);                   \
            }                                                 \
        }                                                     \
    }                                                         \
}                                                             \

void Surface::fillCircle(int x0, int y0, int r, const Color &color) {
    _FILL_CIRCLE(x0, y0, r, color);
}

void Surface::fillCircle(const Vec3 &pos_vec, int r, const Color &color) {
    const int &x0 = pos_vec.x;
    const int &y0 = pos_vec.y;

    _FILL_CIRCLE(x0, y0, r, color);
}

void Surface::fillCircle(const Vec2 &pos_vec, int r, const Color &color) {
    const int &x0 = pos_vec.x;
    const int &y0 = pos_vec.y;

    _FILL_CIRCLE(x0, y0, r, color);
}

void Surface::fillCircle(const Circle &circle, const Color &color) {
    const int &x0 = circle.x;
    const int &y0 = circle.y;
    const int &r  = circle.r;

    _FILL_CIRCLE(x0, y0, r, color);
}


// Rectangles

// _DRAW_RECT(int x0, int y0, int w, int h, const Color &color, int thickness)
#define _DRAW_RECT(x0, y0, w, h, color, thickness) {            \
    this->drawLine(x0, y0, x0+w, y0, color, thickness);         \
    this->drawLine(x0, (y0+h), x0+w, (y0+h), color, thickness); \
    this->drawLine(x0, y0, x0, (y0+h), color, thickness);       \
    this->drawLine(x0+w, y0, x0+w, y0+h, color, thickness);     \
}                                                               \

void Surface::drawRect(int x0, int y0, int w, int h, const Color &color, int thickness) {
    _DRAW_RECT(x0,y0, w,h, color, thickness);
}

void Surface::drawRect(const Vec3 &pos_vec, const Vec3 &size_vec, const Color &color, int thickness) {
    const int &x0 = pos_vec.x;
    const int &y0 = pos_vec.y;
    const int &w = size_vec.x;
    const int &h = size_vec.y;

    _DRAW_RECT(x0,y0, w,h, color, thickness);
}

void Surface::drawRect(const Rect &rect, const Color &color, int thickness) {
    const int &x0 = rect.x;
    const int &y0 = rect.y;
    const int &w = rect.w;
    const int &h = rect.h;

    _DRAW_RECT(x0,y0, w,h, color, thickness);
}


// _FILL_RECT(int x0, int y0, int w, int h, const Color &color)
#define _FILL_RECT(x0, y0, w, h, color) {     \
    int xSt = std::max(0, x0);                \
    int ySt = std::max(0, y0);                \
                                              \
    int xEn = std::min(surfWidth, xSt+w);     \
    int yEn = std::min(surfHeight, ySt+h);    \
                                              \
    for (int y=ySt; y<yEn; y++) {             \
        for (int x=xSt; x<xEn; x++) {         \
            _SET_SURF_AT(x,y,color);          \
        }                                     \
    }                                         \
}                                             \

void Surface::fillRect(int x0, int y0, int w, int h, const Color &color) {
    _FILL_RECT(x0,y0, w,h, color);
}

void Surface::fillRect(const Vec3 &pos_vec, const Vec3 &size_vec, const Color &color) {
    const int x0 = pos_vec.x;
    const int y0 = pos_vec.y;
    const int w = size_vec.x;
    const int h = size_vec.y;

    _FILL_RECT(x0,y0, w,h, color);
}

void Surface::fillRect(const Rect &rect, const Color &color) {
    const int x0 = rect.x;
    const int y0 = rect.y;
    const int w = rect.w;
    const int h = rect.h;

    _FILL_RECT(x0,y0, w,h, color);
}


// Triangles

// _DRAW_TRIS(int x0, int y0, int x1, int y1, int x2, int y2, const Color &color, int thickness)
#define _DRAW_TRIS(x0,y0, x1,y1, x2,y2, color, thickness) { \
    this->drawLine(x0, y0, x1, y1, color, thickness);       \
    this->drawLine(x1, y1, x2, y2, color, thickness);       \
    this->drawLine(x0, y0, x2, y2, color, thickness);       \
}                                                           \

void Surface::drawTris(int x0, int y0, int x1, int y1, int x2, int y2, const Color &color, int thickness) {
    _DRAW_TRIS(x0,y0, x1,y1, x2,y2, color, thickness);
}

void Surface::drawTris(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Color &color, int thickness) {
    const int &x0 = v1.x;
    const int &y0 = v1.y;
    const int &x1 = v2.x;
    const int &y1 = v2.y;
    const int &x2 = v3.x;
    const int &y2 = v3.y;

    _DRAW_TRIS(x0,y0, x1,y1, x2,y2, color, thickness);
}

void Surface::drawTris(const Vec2 &v1, const Vec2 &v2, const Vec2 &v3, const Color &color, int thickness) {
    const int &x0 = v1.x;
    const int &y0 = v1.y;
    const int &x1 = v2.x;
    const int &y1 = v2.y;
    const int &x2 = v3.x;
    const int &y2 = v3.y;

    _DRAW_TRIS(x0,y0, x1,y1, x2,y2, color, thickness);
}

void Surface::drawTris(const Tris2D &tris, const Color &color, int thickness) {
    const int &x0 = tris.v1.x;
    const int &y0 = tris.v1.y;
    const int &x1 = tris.v2.x;
    const int &y1 = tris.v2.y;
    const int &x2 = tris.v3.x;
    const int &y2 = tris.v3.y;

    _DRAW_TRIS(x0,y0, x1,y1, x2,y2, color, thickness);
}

void Surface::drawTris(const Tris2D_i &tris, const Color &color, int thickness) {
    const int &x0 = tris.x0;
    const int &y0 = tris.y0;
    const int &x1 = tris.x1;
    const int &y1 = tris.y1;
    const int &x2 = tris.x2;
    const int &y2 = tris.y2;

    _DRAW_TRIS(x0,y0, x1,y1, x2,y2, color, thickness);
}


// _FILL_TRIS( int x0, int y0, int x1, int y1, int x2, int y2, const Color &color )
#define _FILL_TRIS(x0,y0, x1,y1, x2,y2, color) {                                \
    if ((x0==x1 && y0==y1) || (x0==x2 && y0==y2) ||(x1==x2 && y1==y2)) return;  \
                                                                                \
    /* ScanLine Approach */                                                     \
    if (y0>y1) { std::swap(x0, x1); std::swap(y0, y1); }                        \
    if (y0>y2) { std::swap(x0, x2); std::swap(y0, y2); }                        \
    if (y1>y2) { std::swap(x1, x2); std::swap(y1, y2); }                        \
                                                                                \
    float invslope1 = (x1-x0) / float(y1-y0);                                   \
    float invslope2 = (x2-x0) / float(y2-y0);                                   \
                                                                                \
    float curx1 = x0;                                                           \
    float curx2 = x0;                                                           \
                                                                                \
    for (int y=y0; y<y1; y++) {                                                 \
        this->drawLine(int(curx1), y, int(curx2), y, color, 1);                 \
        curx1 += invslope1;                                                     \
        curx2 += invslope2;                                                     \
    }                                                                           \
                                                                                \
    invslope1 = (x2-x1) / float(y2-y1);                                         \
    curx1 = x1;                                                                 \
                                                                                \
    for (int y=y1; y<=y2; y++) {                                                \
        this->drawLine(int(curx1), y, int(curx2), y, color, 1);                 \
        curx1 += invslope1;                                                     \
        curx2 += invslope2;                                                     \
    }                                                                           \
}                                                                               \

void Surface::fillTris(int x0, int y0, int x1, int y1, int x2, int y2, const Color &color) {
    _FILL_TRIS(x0,y0, x1,y1, x2,y2, color);
}

void Surface::fillTris(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Color &color) {
    int x0 = v1.x;
    int y0 = v1.y;
    int x1 = v2.x;
    int y1 = v2.y;
    int x2 = v3.x;
    int y2 = v3.y;

    _FILL_TRIS(x0,y0, x1,y1, x2,y2, color);
}

void Surface::fillTris(const Vec2 &v1, const Vec2 &v2, const Vec2 &v3, const Color &color) {
    int x0 = v1.x;
    int y0 = v1.y;
    int x1 = v2.x;
    int y1 = v2.y;
    int x2 = v3.x;
    int y2 = v3.y;

    _FILL_TRIS(x0,y0, x1,y1, x2,y2, color);
}

void Surface::fillTris(const Tris2D &tris, const Color &color) {
    int x0 = tris.v1.x;
    int y0 = tris.v1.y;
    int x1 = tris.v2.x;
    int y1 = tris.v2.y;
    int x2 = tris.v3.x;
    int y2 = tris.v3.y;

    _FILL_TRIS(x0,y0, x1,y1, x2,y2, color);
}

void Surface::fillTris(const Tris2D_i &tris, const Color &color) {
    int x0 = tris.x0;
    int y0 = tris.y0;
    int x1 = tris.x1;
    int y1 = tris.y1;
    int x2 = tris.x2;
    int y2 = tris.y2;

    _FILL_TRIS(x0,y0, x1,y1, x2,y2, color);
}
