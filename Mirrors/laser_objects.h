#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#define _USE_MATH_DEFINES
#include <cmath>

class LaserPointer {
private:
	float angle = atan(8.0f/27.0f);
	float radius = pow(pow(8.0f, 2.0f) + pow(27.0f, 2.0f), 0.5f);

public:
	float rotation = 0; //Represents the laser's angle with the x-axis, in radians
	D2D1_POINT_2F points[4] = { {-27.0f, -8.0f}, {27.0f, -8.0f}, {-27.0f, 8.0f}, {27.0f, 8.0f} }; //Top left, top right, bottom left, bottom right of the laser pointer's rectangle
	bool isOn = true;

	void move(float dx, float dy) {
		points[0].x += dx;
		points[1].x += dx;
		points[2].x += dx;
		points[3].x += dx;
		points[0].y += dy;
		points[1].y += dy;
		points[2].y += dy;
		points[3].y += dy;
	}
	void setRotation(float dr) {
		if (dr < 0.001) {
			rotation = 0.001;
		} else if (dr - M_PI / 2 < 0.001 && dr - M_PI / 2 > -0.001) {
			rotation = M_PI / 2 - 0.001;
		} else if (dr - M_PI < 0.001 && dr - M_PI > -0.001) {
			rotation = M_PI - 0.001;
		} else if (dr - (3 * M_PI) / 2 < 0.001 && dr - (3 * M_PI) / 2 > -0.001) {
			rotation = (3 * M_PI) / 2 - 0.001;
		} else {
			rotation = dr;
		}
		D2D1_POINT_2F center = D2D1::Point2F((points[3].x + points[0].x)/2, (points[2].y + points[1].y)/2);
		points[0].x = cos(rotation + (M_PI - angle)) * radius + center.x;
		points[0].y = -sin(rotation + (M_PI - angle)) * radius + center.y;
		points[1].x = cos(rotation + angle) * radius + center.x;
		points[1].y = -sin(rotation + angle) * radius + center.y;
		points[2].x = cos(rotation + M_PI + angle) * radius + center.x;
		points[2].y = -sin(rotation + M_PI + angle) * radius + center.y;
		points[3].x = cos(rotation - angle) * radius + center.x;
		points[3].y = -sin(rotation - angle) * radius + center.y;
	}
};

class LaserInteractable {
public:
	float rotation = 0; //Represents the laser object's angle with the x-axis, in radians
	D2D1_POINT_2F points[2] = { {-50.0f, -0.0f}, {50.0f, -0.0f} }; //Left endpoint, right endpoint
	BYTE type; //0 is a mirror, 1 is a window, 2 is a blocker

	LaserInteractable(BYTE type) : type(type) {}

	void move(float dx, float dy) {
		points[0].x += dx;
		points[1].x += dx;
		points[0].y += dy;
		points[1].y += dy;
	}
	void setRotation(float dr) {
		if (dr < 0.001) {
			rotation = 0.001;
		} else if (dr - M_PI/2 < 0.001 && dr - M_PI/2 > -0.001) {
			rotation = M_PI / 2 - 0.001;
		} else if (dr - M_PI < 0.001 && dr - M_PI > -0.001) {
			rotation = M_PI - 0.001;
		} else if (dr - (3 * M_PI) / 2 < 0.001 && dr - (3 * M_PI) / 2 > -0.001) {
			rotation = (3 * M_PI) / 2 - 0.001;
		} else {
			rotation = dr;
		}
		D2D1_POINT_2F center = D2D1::Point2F((points[0].x + points[1].x) / 2, (points[0].y + points[1].y) / 2);
		points[0].x = cos(rotation + M_PI) * 50 + center.x;
		points[0].y = -sin(rotation + M_PI) * 50 + center.y;
		points[1].x = cos(rotation) * 50 + center.x;
		points[1].y = -sin(rotation) * 50 + center.y;
	}
};

struct LaserBeam {
	int lastMirrorIntersectionIndex = -1;
	float rotation;
	D2D1_POINT_2F startingPoint;
	float opacity = 1.0f;
};