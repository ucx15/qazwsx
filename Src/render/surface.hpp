#pragma once

#include "../math/vec.hpp"
#include "../math/color.hpp"
#include "../primitives/line.hpp"
#include "../primitives/tris.hpp"
#include "../primitives/rect.hpp"
#include "../primitives/circle.hpp"


class Surface {

	public:
		int surfWidth;
		int surfHeight;
		int surfSize;
		float surfAspectRatio;

	private:
		Color *_surfData;


	//Methods
	public:
		Surface();
		Surface(Color* data, int w, int h);

		// tonemapping
		void tonemap();

		// conversion
		void toU32Surface(uint32_t* buffer);


		// Saving
		int saveFloatBuffer(const char *file_path);
		int savePPM(const char *file_path);
		int savePNG(const char *file_path);


		// Drawing Methods
		void setAt(int x, int y, const Color &color);

		void fill(const Color &color);
		void fillNoise();


		void drawLine(int x0, int y0, int x1, int y1, const Color &color, int lineWidth);
		void drawLine(const Vec3 &v1, const Vec3 &v2, const Color &color, int lineWidth);
		void drawLine(const Line &line, const Color &color, int lineWidth);


		void drawCircle(int x0, int y0, int r, const Color &color, int thickness);
		void drawCircle(const Vec3 &pos_vec, int r, const Color &color, int thickness);
		void drawCircle(const Vec2 &pos_vec, int r, const Color &color, int thickness);
		void drawCircle(const Circle &cirlce, const Color &color, int thickness);

		void fillCircle(int x0, int y0, int r, const Color &color);
		void fillCircle(const Vec3 &pos_vec, int r, const Color &color);
		void fillCircle(const Vec2 &pos_vec, int r, const Color &color);
		void fillCircle(const Circle &cirlce, const Color &color);


		void drawRect(int x0, int y0, int w, int h, const Color &color, int thickness);
		void drawRect(const Vec3 &pos_vec, const Vec3 &size_vec, const Color &color, int thickness);
		void drawRect(const Rect &rect, const Color &color, int thickness);

		void fillRect(int x0, int y0, int w, int h, const Color &color);
		void fillRect(const Vec3 &pos_vec, const Vec3 &size_vec, const Color &color);
		void fillRect(const Rect &rect, const Color &color);


		void drawTris(int x0, int y0, int x1, int y1, int x2, int y2, const Color &color, int thickness);
		void drawTris(const Vec3 &v1,const Vec3 &v2, const Vec3 &v3, const Color &color, int thickness);
		void drawTris(const Vec2 &v1, const Vec2 &v2, const Vec2 &v3, const Color &color, int thickness);
		void drawTris(const Tris2D_i &tris, const Color &color, int thickness);
		void drawTris(const Tris2D &tris, const Color &color, int thickness);

		void fillTris(int x0, int y0, int x1, int y1, int x2, int y2, const Color &color);
		void fillTris(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Color &color);
		void fillTris(const Vec2 &v1, const Vec2 &v2, const Vec2 &v3, const Color &color);
		void fillTris(const Tris2D_i &tris, const Color &color);
		void fillTris(const Tris2D &tris, const Color &color);


	private:
		void _aces();
		void _reinhard();
		void _gamma();
};
