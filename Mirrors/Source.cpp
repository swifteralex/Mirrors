#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include "basewin.h"
#include "laser_objects.h"
#include <vector>
#define M_PI 3.14159265358979323846
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <wincodec.h>

class MainWindow : public BaseWindow<MainWindow> {
private:
	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	ID2D1SolidColorBrush* pBrush;
	IWICImagingFactory* pIWICFactory;
	ID2D1Bitmap* rotateImage;
	ID2D1Bitmap* trashImage;
	ID2D1Bitmap* powerImage;
	HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);

	BYTE selectedButton = 0;
	float previousX;
	float previousY;
	float previousXDrag;
	float previousYDrag;

	int highlightedLaserPointer[2] = { -1, 0 }; //First number is the index of the highlighted laser pointer (-1 if none highlighted). Second number is what is being highlighted (0 = body, 1 = rotate icon, 2 = trash icon, 3 = power icon)
	bool draggingLaserPointer = false;
	bool rotatingLaserPointer = false;

	int highlightedLaserInteractable[2] = { -1, 0 };
	bool draggingLaserInteractable = false;
	bool rotatingLaserInteractable = false;

	bool pressingPowerButton = false;
	bool redraw = false; //When the mouse is done selecting an object, the image needs to be redrawn to get rid of the icons.

	std::vector<LaserPointer> laserPointers; //A dynamic array of all current laser pointers
	std::vector<LaserInteractable> laserInteractables; //A dynamic array of all current mirrors, windows, and blockers
	std::vector<LaserBeam> laserBeams; //Each laser in this array has a rotation value and a start value. The program will calculate the next spot where each laser hits.
	std::vector<LaserBeam> newLaserBeams;

public:
	void CalculateLasers(); //Draws the program's lasers.

	void RenderScene() { //Draws lasers, then laser objects, then the interactive menu
		pBrush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f)); // Draw the white canvas
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		D2D1_RECT_F canvas = D2D1::RectF(0, 0, rc.right, rc.bottom);
		pRenderTarget->FillRectangle(&canvas, pBrush);

		CalculateLasers();

		for (int i = 0; i < laserInteractables.size(); i++) { // Draw mirrors, windows, and blockers
			switch (laserInteractables[i].type) {
			case 0:
				pBrush->SetColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
				break;
			case 1:
				pBrush->SetColor(D2D1::ColorF(0.529f, 0.808f, 0.922f, 0.5f));
				break;
			case 2:
				pBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
				break;
			}
			pRenderTarget->DrawLine(
				D2D1::Point2F(laserInteractables[i].points[0].x, laserInteractables[i].points[0].y), 
				D2D1::Point2F(laserInteractables[i].points[1].x, laserInteractables[i].points[1].y), pBrush, 3, NULL
			);
		}

		pBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
		for (int i = 0; i < laserPointers.size(); i++) { // Draw laser pointers
			D2D1_POINT_2F center = D2D1::Point2F((laserPointers[i].points[3].x + laserPointers[i].points[0].x) / 2, (laserPointers[i].points[2].y + laserPointers[i].points[1].y) / 2);
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(laserPointers[i].rotation * (-180 / M_PI), center));
			D2D1_RECT_F rectangle1 = D2D1::RectF(center.x - 27, center.y - 8, center.x + 27, center.y + 8);
			pRenderTarget->FillRectangle(&rectangle1, pBrush);
		}
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		DrawMenu();
	}

	void DrawMenu() {
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		pBrush->SetColor(D2D1::ColorF(0.95f, 0.95f, 0.95f));
		D2D1_RECT_F menu = D2D1::RectF(0, rc.bottom - 60, rc.right, rc.bottom);
		pRenderTarget->FillRectangle(&menu, pBrush);
		pBrush->SetColor(D2D1::ColorF(0.9f, 0.9f, 0.9f));

		D2D1_RECT_F menuButton1 = D2D1::RectF(rc.right / 2 - 115, rc.bottom - 55, rc.right / 2 - 65, rc.bottom - 5);
		pRenderTarget->FillRectangle(&menuButton1, pBrush);
		pBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
		D2D1_RECT_F menuButton1A = D2D1::RectF(rc.right / 2 - 96, rc.bottom - 15, rc.right / 2 - 82, rc.bottom - 5);
		pRenderTarget->FillRectangle(&menuButton1A, pBrush);
		pBrush->SetColor(D2D1::ColorF(1.0f, 0.0f, 0.0f));
		D2D1_RECT_F menuButton1B = D2D1::RectF(rc.right / 2 - 90, rc.bottom - 55, rc.right / 2 - 88, rc.bottom - 15);
		pRenderTarget->FillRectangle(&menuButton1B, pBrush);

		pBrush->SetColor(D2D1::ColorF(0.9f, 0.9f, 0.9f));
		D2D1_RECT_F menuButton2 = D2D1::RectF(rc.right / 2 - 55, rc.bottom - 55, rc.right / 2 - 5, rc.bottom - 5);
		pRenderTarget->FillRectangle(&menuButton2, pBrush);
		pBrush->SetColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
		pRenderTarget->DrawLine(D2D1::Point2F(rc.right / 2 - 40, rc.bottom - 53), D2D1::Point2F(rc.right/2 - 15, rc.bottom - 7), pBrush, 3, NULL);

		pBrush->SetColor(D2D1::ColorF(0.9f, 0.9f, 0.9f));
		D2D1_RECT_F menuButton3 = D2D1::RectF(rc.right / 2 + 5, rc.bottom - 55, rc.right / 2 + 55, rc.bottom - 5);
		pRenderTarget->FillRectangle(&menuButton3, pBrush);
		pBrush->SetColor(D2D1::ColorF(0.529f, 0.808f, 0.922f, 0.5f));
		pRenderTarget->DrawLine(D2D1::Point2F(rc.right / 2 + 20, rc.bottom - 53), D2D1::Point2F(rc.right / 2 + 45, rc.bottom - 7), pBrush, 3, NULL);

		pBrush->SetColor(D2D1::ColorF(0.9f, 0.9f, 0.9f));
		D2D1_RECT_F menuButton4 = D2D1::RectF(rc.right / 2 + 65, rc.bottom - 55, rc.right / 2 + 115, rc.bottom - 5);
		pRenderTarget->FillRectangle(&menuButton4, pBrush);
		pBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
		pRenderTarget->DrawLine(D2D1::Point2F(rc.right / 2 + 80, rc.bottom - 53), D2D1::Point2F(rc.right / 2 + 105, rc.bottom - 7), pBrush, 3, NULL);

		pBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
		for (int i = 0; i < 4; i++) {
			D2D1_RECT_F plus1 = D2D1::RectF(rc.right / 2 - 70 + 60*i, rc.bottom - 47, rc.right / 2 - 80 + 60 * i, rc.bottom - 45);
			pRenderTarget->FillRectangle(&plus1, pBrush);
			D2D1_RECT_F plus2 = D2D1::RectF(rc.right / 2 - 74 + 60 * i, rc.bottom - 41, rc.right / 2 - 76 + 60 * i, rc.bottom - 51);
			pRenderTarget->FillRectangle(&plus2, pBrush);
		}
	}

	void LoadResourceBitmap(PCWSTR resourceName, PCWSTR resourceType, ID2D1Bitmap** ppBitmap) {
		IWICBitmapDecoder* pDecoder = NULL;
		IWICBitmapFrameDecode* pSource = NULL;
		IWICStream* pStream = NULL;
		IWICFormatConverter* pConverter = NULL;

		HRSRC imageResHandle = FindResourceW(GetModuleHandleW(NULL), resourceName, resourceType);
		HGLOBAL imageResDataHandle = LoadResource(GetModuleHandleW(NULL), imageResHandle);
		void* pImageFile = LockResource(imageResDataHandle);
		DWORD imageFileSize = SizeofResource(GetModuleHandleW(NULL), imageResHandle);
		pIWICFactory->CreateStream(&pStream);
		pStream->InitializeFromMemory((BYTE*)(pImageFile), imageFileSize);
		pIWICFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnLoad, &pDecoder);
		pDecoder->GetFrame(0, &pSource);
		pIWICFactory->CreateFormatConverter(&pConverter);
		pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);
		pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
	}

	int CalculateInequalityCountLaserPointer(int i, float x, float y, D2D1_POINT_2F boundaryPoint1, D2D1_POINT_2F boundaryPoint2, D2D1_POINT_2F boundaryPoint3, D2D1_POINT_2F boundaryPoint4) {
		float line1Slope = (boundaryPoint1.y - boundaryPoint2.y) / (boundaryPoint1.x - boundaryPoint2.x);
		float line1B = -boundaryPoint1.x * line1Slope + boundaryPoint1.y;
		float line2Slope = (boundaryPoint1.y - boundaryPoint3.y) / (boundaryPoint1.x - boundaryPoint3.x);
		float line2B = -boundaryPoint1.x * line2Slope + boundaryPoint1.y;
		float line3Slope = (boundaryPoint2.y - boundaryPoint4.y) / (boundaryPoint2.x - boundaryPoint4.x);
		float line3B = -boundaryPoint2.x * line3Slope + boundaryPoint2.y;
		float line4Slope = (boundaryPoint3.y - boundaryPoint4.y) / (boundaryPoint3.x - boundaryPoint4.x);
		float line4B = -boundaryPoint3.x * line4Slope + boundaryPoint3.y;
		int inequalityCount = 0;
		if (y >= line1Slope * x + line1B && laserPointers[i].rotation < M_PI / 2 ||
			y >= line1Slope * x + line1B && laserPointers[i].rotation >(3 * M_PI) / 2 ||
			y <= line1Slope * x + line1B && (laserPointers[i].rotation > M_PI / 2 && laserPointers[i].rotation < (3 * M_PI) / 2)) {
			if (x >= boundaryPoint1.x && x <= boundaryPoint2.x && laserPointers[i].rotation < M_PI / 2 ||
				x >= boundaryPoint1.x && x <= boundaryPoint2.x && laserPointers[i].rotation >(3 * M_PI) / 2 ||
				x >= boundaryPoint2.x && x <= boundaryPoint1.x && (laserPointers[i].rotation > M_PI / 2 && laserPointers[i].rotation < (3 * M_PI) / 2)) {
				inequalityCount++;
			}
		}
		if (y <= line2Slope * x + line2B && (laserPointers[i].rotation > 0 && laserPointers[i].rotation < M_PI) ||
			y >= line2Slope * x + line2B && laserPointers[i].rotation > M_PI) {
			if (x >= boundaryPoint1.x && x <= boundaryPoint3.x && (laserPointers[i].rotation > 0 && laserPointers[i].rotation < M_PI) ||
				x >= boundaryPoint3.x && x <= boundaryPoint1.x && laserPointers[i].rotation > M_PI) {
				inequalityCount++;
			}
		}
		if (y >= line3Slope * x + line3B && (laserPointers[i].rotation > 0 && laserPointers[i].rotation < M_PI) ||
			y <= line3Slope * x + line3B && laserPointers[i].rotation > M_PI) {
			if (x >= boundaryPoint2.x && x <= boundaryPoint4.x && (laserPointers[i].rotation > 0 && laserPointers[i].rotation < M_PI) ||
				x >= boundaryPoint4.x && x <= boundaryPoint2.x && laserPointers[i].rotation > M_PI) {
				inequalityCount++;
			}
		}
		if (y <= line4Slope * x + line4B && laserPointers[i].rotation < M_PI / 2 ||
			y <= line4Slope * x + line4B && laserPointers[i].rotation >(3 * M_PI) / 2 ||
			y >= line4Slope * x + line4B && (laserPointers[i].rotation > M_PI / 2 && laserPointers[i].rotation < (3 * M_PI) / 2)) {
			if (x >= boundaryPoint3.x && x <= boundaryPoint4.x && laserPointers[i].rotation < M_PI / 2 ||
				x >= boundaryPoint3.x && x <= boundaryPoint4.x && laserPointers[i].rotation >(3 * M_PI) / 2 ||
				x >= boundaryPoint4.x && x <= boundaryPoint3.x && (laserPointers[i].rotation > M_PI / 2 && laserPointers[i].rotation < (3 * M_PI) / 2)) {
				inequalityCount++;
			}
		}
		return inequalityCount;
	}

	int CalculateInequalityCountMirror(int i, float x, float y, D2D1_POINT_2F boundaryPoint1, D2D1_POINT_2F boundaryPoint2, D2D1_POINT_2F boundaryPoint3, D2D1_POINT_2F boundaryPoint4) {
		float line1Slope = (boundaryPoint1.y - boundaryPoint2.y) / (boundaryPoint1.x - boundaryPoint2.x);
		float line1B = -boundaryPoint1.x * line1Slope + boundaryPoint1.y;
		float line2Slope = (boundaryPoint1.y - boundaryPoint3.y) / (boundaryPoint1.x - boundaryPoint3.x);
		float line2B = -boundaryPoint1.x * line2Slope + boundaryPoint1.y;
		float line3Slope = (boundaryPoint2.y - boundaryPoint4.y) / (boundaryPoint2.x - boundaryPoint4.x);
		float line3B = -boundaryPoint2.x * line3Slope + boundaryPoint2.y;
		float line4Slope = (boundaryPoint3.y - boundaryPoint4.y) / (boundaryPoint3.x - boundaryPoint4.x);
		float line4B = -boundaryPoint3.x * line4Slope + boundaryPoint3.y;
		int inequalityCount = 0;
		if (y >= line1Slope * x + line1B && laserInteractables[i].rotation < M_PI / 2 ||
			y >= line1Slope * x + line1B && laserInteractables[i].rotation >(3 * M_PI) / 2 ||
			y <= line1Slope * x + line1B && (laserInteractables[i].rotation > M_PI / 2 && laserInteractables[i].rotation < (3 * M_PI) / 2)) {
			if (x >= boundaryPoint1.x && x <= boundaryPoint2.x && laserInteractables[i].rotation < M_PI / 2 ||
				x >= boundaryPoint1.x && x <= boundaryPoint2.x && laserInteractables[i].rotation >(3 * M_PI) / 2 ||
				x >= boundaryPoint2.x && x <= boundaryPoint1.x && (laserInteractables[i].rotation > M_PI / 2 && laserInteractables[i].rotation < (3 * M_PI) / 2)) {
				inequalityCount++;
			}
		}
		if (y <= line2Slope * x + line2B && (laserInteractables[i].rotation > 0 && laserInteractables[i].rotation < M_PI) ||
			y >= line2Slope * x + line2B && laserInteractables[i].rotation > M_PI) {
			if (x >= boundaryPoint1.x && x <= boundaryPoint3.x && (laserInteractables[i].rotation > 0 && laserInteractables[i].rotation < M_PI) ||
				x >= boundaryPoint3.x && x <= boundaryPoint1.x && laserInteractables[i].rotation > M_PI) {
				inequalityCount++;
			}
		}
		if (y >= line3Slope * x + line3B && (laserInteractables[i].rotation > 0 && laserInteractables[i].rotation < M_PI) ||
			y <= line3Slope * x + line3B && laserInteractables[i].rotation > M_PI) {
			if (x >= boundaryPoint2.x && x <= boundaryPoint4.x && (laserInteractables[i].rotation > 0 && laserInteractables[i].rotation < M_PI) ||
				x >= boundaryPoint4.x && x <= boundaryPoint2.x && laserInteractables[i].rotation > M_PI) {
				inequalityCount++;
			}
		}
		if (y <= line4Slope * x + line4B && laserInteractables[i].rotation < M_PI / 2 ||
			y <= line4Slope * x + line4B && laserInteractables[i].rotation >(3 * M_PI) / 2 ||
			y >= line4Slope * x + line4B && (laserInteractables[i].rotation > M_PI / 2 && laserInteractables[i].rotation < (3 * M_PI) / 2)) {
			if (x >= boundaryPoint3.x && x <= boundaryPoint4.x && laserInteractables[i].rotation < M_PI / 2 ||
				x >= boundaryPoint3.x && x <= boundaryPoint4.x && laserInteractables[i].rotation >(3 * M_PI) / 2 ||
				x >= boundaryPoint4.x && x <= boundaryPoint3.x && (laserInteractables[i].rotation > M_PI / 2 && laserInteractables[i].rotation < (3 * M_PI) / 2)) {
				inequalityCount++;
			}
		}
		return inequalityCount;
	}
	
	PCWSTR  ClassName() const { return L"Sample Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};




void MainWindow::CalculateLasers() {
	laserBeams.clear();

	for (int i = 0; i < laserPointers.size(); i++) { //Initilize the array with all of the active laser pointers
		if (laserPointers[i].isOn == true) {
			LaserBeam lb;
			lb.rotation = laserPointers[i].rotation;
			lb.startingPoint = { (laserPointers[i].points[1].x + laserPointers[i].points[3].x) / 2, (laserPointers[i].points[1].y + laserPointers[i].points[3].y) / 2 };
			laserBeams.push_back(lb);
		}
	}

	int overflowProtection = 0;
	while (laserBeams.size() != 0) {

		overflowProtection++;
		if (overflowProtection > 1000 || laserBeams.size() > 1000) {
			laserBeams.clear();
			newLaserBeams.clear();
			break;
		}

		for (int i = 0; i < laserBeams.size(); i++) {

			float laserBeamSlope = -tan(laserBeams[i].rotation);
			float laserBeamB = laserBeams[i].startingPoint.y - (laserBeams[i].startingPoint.x * laserBeamSlope);
			int closestIntersectedMirrorIndex = -1;
			D2D1_POINT_2F closestIntersectionPoint = { 99999999.0f, 99999999.0f };

			for (int j = 0; j < laserInteractables.size(); j++) { //Go through the list of all mirrors and find which one the laser hits

				if (laserBeams[i].lastMirrorIntersectionIndex == j) {
					continue;
				}

				float mirrorSlope = -tan(laserInteractables[j].rotation);
				float mirrorB = laserInteractables[j].points[0].y - (laserInteractables[j].points[0].x * mirrorSlope);
				D2D1_POINT_2F intersection = { (laserBeamB - mirrorB) / (mirrorSlope - laserBeamSlope), ((laserBeamB - mirrorB) / (mirrorSlope - laserBeamSlope)) * laserBeamSlope + laserBeamB };

				if (intersection.x < laserInteractables[j].points[1].x && intersection.x > laserInteractables[j].points[0].x || //Is the beam intersecting a mirror?
					intersection.x > laserInteractables[j].points[1].x && intersection.x < laserInteractables[j].points[0].x) {
					if (laserBeams[i].startingPoint.x <= intersection.x && laserBeams[i].rotation <= M_PI / 2 ||
						laserBeams[i].startingPoint.x <= intersection.x && laserBeams[i].rotation >= (3 * M_PI / 2) ||
						laserBeams[i].startingPoint.x >= intersection.x && laserBeams[i].rotation >= M_PI / 2 && laserBeams[i].rotation <= 3 * (M_PI / 2)) {

						if (abs(intersection.x - laserBeams[i].startingPoint.x) < abs(closestIntersectionPoint.x - laserBeams[i].startingPoint.x) || //Is this intersection closer to the beam's start?
							abs(intersection.y - laserBeams[i].startingPoint.y) < abs(closestIntersectionPoint.y - laserBeams[i].startingPoint.y)) {

							closestIntersectionPoint = intersection;
							closestIntersectedMirrorIndex = j;
						}
					}
				}
			}

			if (closestIntersectedMirrorIndex != -1) { //Gives the laser beam's intersected mirror
				pBrush->SetColor(D2D1::ColorF(1.0f, 0.0f, 0.0f, laserBeams[i].opacity));
				pRenderTarget->DrawLine(D2D1::Point2F(laserBeams[i].startingPoint.x, laserBeams[i].startingPoint.y),
					D2D1::Point2F(closestIntersectionPoint.x, closestIntersectionPoint.y), pBrush, 2, NULL);

				if (laserInteractables[closestIntersectedMirrorIndex].type == 0) {
					float mirrorAngle = laserInteractables[closestIntersectedMirrorIndex].rotation;
					if (mirrorAngle > M_PI) {
						mirrorAngle -= M_PI;
					}
					LaserBeam lp;
					lp.startingPoint = closestIntersectionPoint;
					lp.rotation = 2 * M_PI - (laserBeams[i].rotation - 2 * mirrorAngle);
					lp.lastMirrorIntersectionIndex = closestIntersectedMirrorIndex;
					lp.opacity = laserBeams[i].opacity;
					if (lp.rotation > 2 * M_PI) {
						lp.rotation -= 2 * M_PI;
					} else if (lp.rotation < 0) {
						lp.rotation += 2 * M_PI;
					}
					newLaserBeams.push_back(lp);
				} else if (laserInteractables[closestIntersectedMirrorIndex].type == 1) {
					float mirrorAngle = laserInteractables[closestIntersectedMirrorIndex].rotation;
					if (mirrorAngle > M_PI) {
						mirrorAngle -= M_PI;
					}
					LaserBeam lp;
					lp.startingPoint = closestIntersectionPoint;
					lp.rotation = 2 * M_PI - (laserBeams[i].rotation - 2 * mirrorAngle);
					lp.lastMirrorIntersectionIndex = closestIntersectedMirrorIndex;
					lp.opacity = laserBeams[i].opacity / 2;
					if (lp.rotation > 2 * M_PI) {
						lp.rotation -= 2 * M_PI;
					} else if (lp.rotation < 0) {
						lp.rotation += 2 * M_PI;
					}
					LaserBeam lp2;
					lp2.startingPoint = closestIntersectionPoint;
					lp2.rotation = laserBeams[i].rotation;
					lp2.lastMirrorIntersectionIndex = closestIntersectedMirrorIndex;
					lp2.opacity = laserBeams[i].opacity / 2;
					if (lp2.opacity > 0.00001) {
						newLaserBeams.push_back(lp);
						newLaserBeams.push_back(lp2);
					}
				}

				//TODO: Calculate reflection angle and create a new LaserBeam object

			} else {
				pBrush->SetColor(D2D1::ColorF(1.0f, 0.0f, 0.0f, laserBeams[i].opacity));
				pRenderTarget->DrawLine(D2D1::Point2F(laserBeams[i].startingPoint.x, laserBeams[i].startingPoint.y),
					D2D1::Point2F(laserBeams[i].startingPoint.x + 5000 * cos(laserBeams[i].rotation), laserBeams[i].startingPoint.y - 5000 * sin(laserBeams[i].rotation)), pBrush, 2, NULL);
			}

		}

		laserBeams.clear();
		for (int i = 0; i < newLaserBeams.size(); i++) {
			laserBeams.push_back(newLaserBeams[i]);
		}
		newLaserBeams.clear();

	}

}




LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {

	case WM_KEYDOWN: {
		if (wParam == 0x43) {
			int placeholderSize = laserPointers.size();
			for (int i = 0; i < placeholderSize; i++) {
				laserPointers.erase(laserPointers.begin());
			}
			placeholderSize = laserInteractables.size();
			for (int i = 0; i < placeholderSize; i++) {
				laserInteractables.erase(laserInteractables.begin());
			}
			pRenderTarget->BeginDraw();
			RenderScene();
			pRenderTarget->EndDraw();
		}
		return 0;
	}

	case WM_LBUTTONDOWN: {
		if (selectedButton != 0) {
			RECT rc;
			GetClientRect(m_hwnd, &rc);
			pRenderTarget->BeginDraw();
			int rando = rand() % 628;
			if (selectedButton == 1) {
				LaserPointer lp;
				lp.move(rc.right / 2, rc.bottom / 2);
				lp.setRotation(rando / 100.0f);
				laserPointers.push_back(lp);
			} else if (selectedButton == 2) {
				LaserInteractable li(0);
				li.move(rc.right / 2, rc.bottom / 2);
				li.setRotation(rando / 100.0f);
				laserInteractables.push_back(li);
			} else if (selectedButton == 3) {
				LaserInteractable li(1);
				li.move(rc.right / 2, rc.bottom / 2);
				li.setRotation(rando / 100.0f);
				laserInteractables.push_back(li);
			} else if (selectedButton == 4) {
				LaserInteractable li(2);
				li.move(rc.right / 2, rc.bottom / 2);
				li.setRotation(rando / 100.0f);
				laserInteractables.push_back(li);
			}
			RenderScene();
			pBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.2f));
			D2D1_RECT_F selection = D2D1::RectF(rc.right / 2 - 175 + selectedButton * 60, rc.bottom - 55, rc.right / 2 - 125 + selectedButton * 60, rc.bottom - 5);
			pRenderTarget->FillRectangle(&selection, pBrush);
			pRenderTarget->EndDraw();
			return 0;
		}

		if (highlightedLaserPointer[0] != -1 && highlightedLaserPointer[1] == 0) {
			draggingLaserPointer = true;
			return 0;
		}

		if (highlightedLaserPointer[0] != -1 && highlightedLaserPointer[1] == 1) {
			rotatingLaserPointer = true;
			return 0;
		}

		if (highlightedLaserPointer[0] != -1 && highlightedLaserPointer[1] == 2) {
			laserPointers.erase(laserPointers.begin() + highlightedLaserPointer[0]);
			SetCursor(hCursor);
			pRenderTarget->BeginDraw();
			RenderScene();
			pRenderTarget->EndDraw();
			highlightedLaserPointer[0] = -1;
			return 0;
		}

		if (highlightedLaserPointer[0] != -1 && highlightedLaserPointer[1] == 3) {
			if (laserPointers[highlightedLaserPointer[0]].isOn == true) {
				laserPointers[highlightedLaserPointer[0]].isOn = false;
			} else {
				laserPointers[highlightedLaserPointer[0]].isOn = true;
			}

			pressingPowerButton = true;
			pRenderTarget->BeginDraw();
			int i = highlightedLaserPointer[0];
			RenderScene();
			float imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 45);
			float imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 45);
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation((laserPointers[i].rotation + M_PI / 2) * (-180 / M_PI), D2D1::Point2F(imageX, imageY)));
			pRenderTarget->DrawBitmap(rotateImage, D2D1::RectF(imageX - 9, imageY - 3, imageX + 9, imageY + 3));
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 71);
			imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 71);
			pRenderTarget->DrawBitmap(trashImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
			imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 97);
			imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 97);
			pRenderTarget->DrawBitmap(powerImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
			DrawMenu();
			pRenderTarget->EndDraw();

			return 0;
		}

		if (highlightedLaserInteractable[0] != -1 && highlightedLaserInteractable[1] == 0) {
			draggingLaserInteractable = true;
			return 0;
		}

		if (highlightedLaserInteractable[0] != -1 && highlightedLaserInteractable[1] == 1) {
			rotatingLaserInteractable = true;
			return 0;
		}

		if (highlightedLaserInteractable[0] != -1 && highlightedLaserInteractable[1] == 2) {
			laserInteractables.erase(laserInteractables.begin() + highlightedLaserInteractable[0]);
			SetCursor(hCursor);
			pRenderTarget->BeginDraw();
			RenderScene();
			pRenderTarget->EndDraw();
			highlightedLaserInteractable[0] = -1;
			return 0;
		}

		return 0;
	}

	case WM_LBUTTONUP: {
		if (selectedButton != 0) {
			RECT rc;
			GetClientRect(m_hwnd, &rc);
			pRenderTarget->BeginDraw();
			DrawMenu();
			pBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.1f));
			D2D1_RECT_F selection = D2D1::RectF(rc.right / 2 - 175 + selectedButton * 60, rc.bottom - 55, rc.right / 2 - 125 + selectedButton * 60, rc.bottom - 5);
			pRenderTarget->FillRectangle(&selection, pBrush);
			pRenderTarget->EndDraw();
			return 0;
		}

		if (draggingLaserPointer == true) {
			draggingLaserPointer = false;
			return 0;
		}

		if (rotatingLaserPointer == true) {
			rotatingLaserPointer = false;
			return 0;
		}

		if (pressingPowerButton == true) {
			pressingPowerButton = false;

			pRenderTarget->BeginDraw();
			int i = highlightedLaserPointer[0];
			RenderScene();
			float imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 45);
			float imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 45);
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation((laserPointers[i].rotation + M_PI / 2)* (-180 / M_PI), D2D1::Point2F(imageX, imageY)));
			pRenderTarget->DrawBitmap(rotateImage, D2D1::RectF(imageX - 9, imageY - 3, imageX + 9, imageY + 3));
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 71);
			imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 71);
			pRenderTarget->DrawBitmap(trashImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
			imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 97);
			imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 97);
			pRenderTarget->DrawBitmap(powerImage, D2D1::RectF(imageX - 10, imageY - 10, imageX + 10, imageY + 10));
			DrawMenu();
			pRenderTarget->EndDraw();
			return 0;
		}

		if (draggingLaserInteractable == true) {
			draggingLaserInteractable = false;
			return 0;
		}

		if (rotatingLaserInteractable == true) {
			rotatingLaserInteractable = false;
			return 0;
		}

		return 0;
	}

	case WM_MOUSEMOVE: {
		float x = LOWORD(lParam);
		float y = HIWORD(lParam);
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		if (y >= rc.bottom - 55 || previousY >= rc.bottom - 55) { //Menu stuff
			pRenderTarget->BeginDraw();
			DrawMenu();
			selectedButton = 0;
			D2D1_RECT_F selection;
			pBrush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.1f));
			if (x >= rc.right / 2 - 115 && x <= rc.right / 2 - 65 && y >= rc.bottom - 55 && y <= rc.bottom - 5) {
				selectedButton = 1;
				selection = D2D1::RectF(rc.right / 2 - 115, rc.bottom - 55, rc.right / 2 - 65, rc.bottom - 5);
			} else if (x >= rc.right / 2 - 55 && x <= rc.right / 2 - 5 && y >= rc.bottom - 55 && y <= rc.bottom - 5) {
				selectedButton = 2;
				selection = D2D1::RectF(rc.right / 2 - 55, rc.bottom - 55, rc.right / 2 - 5, rc.bottom - 5);
			} else if (x >= rc.right / 2 + 5 && x <= rc.right / 2 + 55 && y >= rc.bottom - 55 && y <= rc.bottom - 5) {
				selectedButton = 3;
				selection = D2D1::RectF(rc.right / 2 + 5, rc.bottom - 55, rc.right / 2 + 55, rc.bottom - 5);
			} else if (x >= rc.right / 2 + 65 && x <= rc.right / 2 + 115 && y >= rc.bottom - 55 && y <= rc.bottom - 5) {
				selectedButton = 4;
				selection = D2D1::RectF(rc.right / 2 + 65, rc.bottom - 55, rc.right / 2 + 115, rc.bottom - 5);
			}
			if (selectedButton != 0) {
				pRenderTarget->FillRectangle(&selection, pBrush);
				SetCursor(LoadCursor(NULL, IDC_HAND));
			}
			pRenderTarget->EndDraw();
			previousX = x;
			previousY = y;
			return 0;
		} 

		if (draggingLaserPointer == true) {
			laserPointers[highlightedLaserPointer[0]].move(x - previousXDrag, y - previousYDrag);
			pRenderTarget->BeginDraw();
			RenderScene();
			pRenderTarget->EndDraw();
			previousXDrag = x;
			previousYDrag = y;
			return 0;
		}

		if (rotatingLaserPointer == true) {
			float centerX = (laserPointers[highlightedLaserPointer[0]].points[0].x + laserPointers[highlightedLaserPointer[0]].points[3].x)/2;
			float centerY = (laserPointers[highlightedLaserPointer[0]].points[0].y + laserPointers[highlightedLaserPointer[0]].points[3].y) / 2;
			float aangle;
			if (x >= centerX && y <= centerY) {
				aangle = atan(abs(y - centerY) / abs(x - centerX)) + M_PI;
			} else if (x <= centerX && y <= centerY) {
				aangle = (M_PI / 2) - abs(atan((y - centerY) / (x - centerX))) + (3 * M_PI) / 2;
			} else if (x <= centerX && y >= centerY) {
				aangle = abs(atan((y - centerY) / (x - centerX)));
			} else {
				aangle = (M_PI / 2) - abs(atan((y - centerY) / (x - centerX))) + M_PI / 2;
			}
			laserPointers[highlightedLaserPointer[0]].setRotation(aangle);
			pRenderTarget->BeginDraw();
			RenderScene();
			pRenderTarget->EndDraw();
			return 0;
		}

		if (draggingLaserInteractable == true) {
			laserInteractables[highlightedLaserInteractable[0]].move(x - previousXDrag, y - previousYDrag);
			pRenderTarget->BeginDraw();
			RenderScene();
			pRenderTarget->EndDraw();
			previousXDrag = x;
			previousYDrag = y;
			return 0;
		}

		if (rotatingLaserInteractable == true) {
			float centerX = (laserInteractables[highlightedLaserInteractable[0]].points[0].x + laserInteractables[highlightedLaserInteractable[0]].points[1].x) / 2;
			float centerY = (laserInteractables[highlightedLaserInteractable[0]].points[0].y + laserInteractables[highlightedLaserInteractable[0]].points[1].y) / 2;
			float aangle;
			if (x >= centerX && y <= centerY) {
				aangle = atan(abs(y - centerY) / abs(x - centerX)) + M_PI;
			} else if (x <= centerX && y <= centerY) {
				aangle = (M_PI / 2) - abs(atan((y - centerY) / (x - centerX))) + (3 * M_PI) / 2;
			} else if (x <= centerX && y >= centerY) {
				aangle = abs(atan((y - centerY) / (x - centerX)));
			} else {
				aangle = (M_PI / 2) - abs(atan((y - centerY) / (x - centerX))) + M_PI / 2;
			}
			laserInteractables[highlightedLaserInteractable[0]].setRotation(aangle);
			pRenderTarget->BeginDraw();
			RenderScene();
			pRenderTarget->EndDraw();
			return 0;
		}

		for (int i = 0; i < laserPointers.size(); i++) { //Highlighting laser pointers

			{ //Check if the mouse is over the laser pointer's body
				D2D1_POINT_2F boundaryPoint1 = { laserPointers[i].points[0].x - (cos(M_PI / 4 - laserPointers[i].rotation) * pow(50, 0.5f)),
					laserPointers[i].points[0].y - (sin(M_PI / 4 - laserPointers[i].rotation) * pow(50, 0.5f)) };
				D2D1_POINT_2F boundaryPoint2 = { laserPointers[i].points[1].x + (cos(M_PI / 4 + laserPointers[i].rotation) * pow(50, 0.5f)),
					laserPointers[i].points[1].y - (sin(M_PI / 4 + laserPointers[i].rotation) * pow(50, 0.5f)) };
				D2D1_POINT_2F boundaryPoint3 = { laserPointers[i].points[2].x - (cos(M_PI / 4 + laserPointers[i].rotation) * pow(50, 0.5f)),
					laserPointers[i].points[2].y + (sin(M_PI / 4 + laserPointers[i].rotation) * pow(50, 0.5f)) };
				D2D1_POINT_2F boundaryPoint4 = { laserPointers[i].points[3].x + (cos(M_PI / 4 - laserPointers[i].rotation) * pow(50, 0.5f)),
					laserPointers[i].points[3].y + (sin(M_PI / 4 - laserPointers[i].rotation) * pow(50, 0.5f)) };

				if (CalculateInequalityCountLaserPointer(i, x, y, boundaryPoint1, boundaryPoint2, boundaryPoint3, boundaryPoint4) >= 2) {
					SetCursor(LoadCursor(NULL, IDC_HAND));

					pRenderTarget->BeginDraw();
					RenderScene();
					float imageX = ((boundaryPoint1.x + boundaryPoint4.x) / 2) - (cos(-laserPointers[i].rotation) * 45);
					float imageY = ((boundaryPoint1.y + boundaryPoint4.y) / 2) - (sin(-laserPointers[i].rotation) * 45);
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation((laserPointers[i].rotation + M_PI / 2) * (-180 / M_PI), D2D1::Point2F(imageX, imageY)));
					pRenderTarget->DrawBitmap(rotateImage, D2D1::RectF(imageX - 9, imageY - 3, imageX + 9, imageY + 3));
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
					imageX = ((boundaryPoint1.x + boundaryPoint4.x) / 2) - (cos(-laserPointers[i].rotation) * 71);
					imageY = ((boundaryPoint1.y + boundaryPoint4.y) / 2) - (sin(-laserPointers[i].rotation) * 71);
					pRenderTarget->DrawBitmap(trashImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
					imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 97);
					imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 97);
					pRenderTarget->DrawBitmap(powerImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
					DrawMenu();
					pRenderTarget->EndDraw();

					redraw = true;
					highlightedLaserPointer[0] = i;
					highlightedLaserPointer[1] = 0;
					previousXDrag = x;
					previousYDrag = y;
					return 0;
				} else {
					highlightedLaserPointer[0] = -1;
				}
			}

			{ //Check if the mouse is over the laser pointer's ROTATE icon
				D2D1_POINT_2F boundaryPoint1 = { laserPointers[i].points[0].x - cos(laserPointers[i].rotation - atan(5.0f/31.0f)) * pow(986, 0.5f),
					laserPointers[i].points[0].y + sin(laserPointers[i].rotation - atan(5.0f / 31.0f)) * pow(986, 0.5f) };
				D2D1_POINT_2F boundaryPoint2 = { laserPointers[i].points[0].x - (cos(M_PI / 4 - laserPointers[i].rotation) * pow(50, 0.5f)),
					laserPointers[i].points[0].y - (sin(M_PI / 4 - laserPointers[i].rotation) * pow(50, 0.5f)) };
				D2D1_POINT_2F boundaryPoint3 = { laserPointers[i].points[2].x - cos(laserPointers[i].rotation + atan(5.0f / 31.0f)) * pow(986, 0.5f),
					laserPointers[i].points[2].y + sin(laserPointers[i].rotation + atan(5.0f / 31.0f)) * pow(986, 0.5f) };
				D2D1_POINT_2F boundaryPoint4 = { laserPointers[i].points[2].x - (cos(M_PI / 4 + laserPointers[i].rotation) * pow(50, 0.5f)),
					laserPointers[i].points[2].y + (sin(M_PI / 4 + laserPointers[i].rotation) * pow(50, 0.5f)) };

				if (CalculateInequalityCountLaserPointer(i, x, y, boundaryPoint1, boundaryPoint2, boundaryPoint3, boundaryPoint4) >= 2) {
					SetCursor(LoadCursor(NULL, IDC_HAND));

					pRenderTarget->BeginDraw();
					RenderScene();
					float imageX = ((boundaryPoint1.x + boundaryPoint4.x) / 2);
					float imageY = ((boundaryPoint1.y + boundaryPoint4.y) / 2);
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation((laserPointers[i].rotation + M_PI / 2) * (-180 / M_PI), D2D1::Point2F(imageX, imageY)));
					pRenderTarget->DrawBitmap(rotateImage, D2D1::RectF(imageX - 10.5, imageY - 3.5, imageX + 10.5, imageY + 3.5));
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
					imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 71);
					imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 71);
					pRenderTarget->DrawBitmap(trashImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
					imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 97);
					imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 97);
					pRenderTarget->DrawBitmap(powerImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
					DrawMenu();
					pRenderTarget->EndDraw();

					redraw = true;
					highlightedLaserPointer[0] = i;
					highlightedLaserPointer[1] = 1;
					return 0;
				} else {
					highlightedLaserPointer[0] = -1;
				}
			}

			{ //Check if the mouse is over the laser pointer's TRASH icon
				D2D1_POINT_2F boundaryPoint1 = { laserPointers[i].points[0].x - cos(laserPointers[i].rotation - atan(5.0f / 57.0f)) * pow(3274, 0.5f),
					laserPointers[i].points[0].y + sin(laserPointers[i].rotation - atan(5.0f / 57.0f)) * pow(3274, 0.5f) };
				D2D1_POINT_2F boundaryPoint2 = { laserPointers[i].points[0].x - cos(laserPointers[i].rotation - atan(5.0f / 31.0f)) * pow(986, 0.5f),
					laserPointers[i].points[0].y + sin(laserPointers[i].rotation - atan(5.0f / 31.0f)) * pow(986, 0.5f) };
				D2D1_POINT_2F boundaryPoint3 = { laserPointers[i].points[2].x - cos(laserPointers[i].rotation + atan(5.0f / 57.0f)) * pow(3274, 0.5f),
					laserPointers[i].points[2].y + sin(laserPointers[i].rotation + atan(5.0f / 57.0f)) * pow(3274, 0.5f) };
				D2D1_POINT_2F boundaryPoint4 = { laserPointers[i].points[2].x - cos(laserPointers[i].rotation + atan(5.0f / 31.0f)) * pow(986, 0.5f),
					laserPointers[i].points[2].y + sin(laserPointers[i].rotation + atan(5.0f / 31.0f)) * pow(986, 0.5f) };

				if (CalculateInequalityCountLaserPointer(i, x, y, boundaryPoint1, boundaryPoint2, boundaryPoint3, boundaryPoint4) >= 2) {
					SetCursor(LoadCursor(NULL, IDC_HAND));

					pRenderTarget->BeginDraw();
					RenderScene();
					float imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 45);
					float imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 45);
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation((laserPointers[i].rotation + M_PI / 2) * (-180 / M_PI), D2D1::Point2F(imageX, imageY)));
					pRenderTarget->DrawBitmap(rotateImage, D2D1::RectF(imageX - 9, imageY - 3, imageX + 9, imageY + 3));
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
					imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 71);
					imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 71);
					pRenderTarget->DrawBitmap(trashImage, D2D1::RectF(imageX - 10, imageY - 10, imageX + 10, imageY + 10));
					imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 97);
					imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 97);
					pRenderTarget->DrawBitmap(powerImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
					DrawMenu();
					pRenderTarget->EndDraw();

					redraw = true;
					highlightedLaserPointer[0] = i;
					highlightedLaserPointer[1] = 2;
					return 0;
				} else {
					highlightedLaserPointer[0] = -1;
				}
			}

			{ //Check if the mouse is over the laser pointer's POWER icon
				D2D1_POINT_2F boundaryPoint1 = { laserPointers[i].points[0].x - cos(laserPointers[i].rotation - atan(5.0f / 83.0f)) * pow(6914, 0.5f),
					laserPointers[i].points[0].y + sin(laserPointers[i].rotation - atan(5.0f / 83.0f)) * pow(6914, 0.5f) };
				D2D1_POINT_2F boundaryPoint2 = { laserPointers[i].points[0].x - cos(laserPointers[i].rotation - atan(5.0f / 57.0f)) * pow(3274, 0.5f),
					laserPointers[i].points[0].y + sin(laserPointers[i].rotation - atan(5.0f / 57.0f)) * pow(3274, 0.5f) };
				D2D1_POINT_2F boundaryPoint3 = { laserPointers[i].points[2].x - cos(laserPointers[i].rotation + atan(5.0f / 83.0f)) * pow(6914, 0.5f),
					laserPointers[i].points[2].y + sin(laserPointers[i].rotation + atan(5.0f / 83.0f)) * pow(6914, 0.5f) };
				D2D1_POINT_2F boundaryPoint4 = { laserPointers[i].points[2].x - cos(laserPointers[i].rotation + atan(5.0f / 57.0f)) * pow(3274, 0.5f),
					laserPointers[i].points[2].y + sin(laserPointers[i].rotation + atan(5.0f / 57.0f)) * pow(3274, 0.5f) };

				if (CalculateInequalityCountLaserPointer(i, x, y, boundaryPoint1, boundaryPoint2, boundaryPoint3, boundaryPoint4) >= 2) {
					SetCursor(LoadCursor(NULL, IDC_HAND));

					pRenderTarget->BeginDraw();
					RenderScene();
					float imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 45);
					float imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 45);
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation((laserPointers[i].rotation + M_PI / 2) * (-180 / M_PI), D2D1::Point2F(imageX, imageY)));
					pRenderTarget->DrawBitmap(rotateImage, D2D1::RectF(imageX - 9, imageY - 3, imageX + 9, imageY + 3));
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
					imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 71);
					imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 71);
					pRenderTarget->DrawBitmap(trashImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
					imageX = ((laserPointers[i].points[0].x + laserPointers[i].points[3].x) / 2) - (cos(-laserPointers[i].rotation) * 97);
					imageY = ((laserPointers[i].points[0].y + laserPointers[i].points[3].y) / 2) - (sin(-laserPointers[i].rotation) * 97);
					pRenderTarget->DrawBitmap(powerImage, D2D1::RectF(imageX - 10, imageY - 10, imageX + 10, imageY + 10));
					DrawMenu();
					pRenderTarget->EndDraw();

					redraw = true;
					highlightedLaserPointer[0] = i;
					highlightedLaserPointer[1] = 3;
					return 0;
				} else {
					highlightedLaserPointer[0] = -1;
				}
			}
		}

		for (int i = 0; i < laserInteractables.size(); i++) { //Highlighting laser interactables (mirrors, windows, and blockers)

			{ //Check if the mouse is over the object's body
				D2D1_POINT_2F boundaryPoint1 = { laserInteractables[i].points[0].x - cos(M_PI / 4 - laserInteractables[i].rotation) * pow(338, 0.5f),
					laserInteractables[i].points[0].y - sin(M_PI / 4 - laserInteractables[i].rotation)* pow(338, 0.5f) };
				D2D1_POINT_2F boundaryPoint2 = { laserInteractables[i].points[1].x + cos(M_PI / 4 + laserInteractables[i].rotation) * pow(338, 0.5f),
					laserInteractables[i].points[1].y - sin(M_PI / 4 + laserInteractables[i].rotation) * pow(338, 0.5f) };
				D2D1_POINT_2F boundaryPoint3 = { laserInteractables[i].points[0].x - cos(M_PI / 4 + laserInteractables[i].rotation) * pow(338, 0.5f),
					laserInteractables[i].points[0].y + sin(M_PI / 4 + laserInteractables[i].rotation) * pow(338, 0.5f) };
				D2D1_POINT_2F boundaryPoint4 = { laserInteractables[i].points[1].x + (cos(M_PI / 4 - laserInteractables[i].rotation) * pow(338, 0.5f)),
					laserInteractables[i].points[1].y + (sin(M_PI / 4 - laserInteractables[i].rotation) * pow(338, 0.5f)) };

				if (CalculateInequalityCountMirror(i, x, y, boundaryPoint1, boundaryPoint2, boundaryPoint3, boundaryPoint4) >= 2) {
					SetCursor(LoadCursor(NULL, IDC_HAND));

					pRenderTarget->BeginDraw();
					RenderScene();
					float imageX = ((boundaryPoint1.x + boundaryPoint4.x) / 2) - (cos(-laserInteractables[i].rotation) * 76);
					float imageY = ((boundaryPoint1.y + boundaryPoint4.y) / 2) - (sin(-laserInteractables[i].rotation) * 76);
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation((laserInteractables[i].rotation + M_PI / 2) * (-180 / M_PI), D2D1::Point2F(imageX, imageY)));
					pRenderTarget->DrawBitmap(rotateImage, D2D1::RectF(imageX - 9, imageY - 3, imageX + 9, imageY + 3));
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
					imageX = ((boundaryPoint1.x + boundaryPoint4.x) / 2) - (cos(-laserInteractables[i].rotation) * 102);
					imageY = ((boundaryPoint1.y + boundaryPoint4.y) / 2) - (sin(-laserInteractables[i].rotation) * 102);
					pRenderTarget->DrawBitmap(trashImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
					DrawMenu();
					pRenderTarget->EndDraw();

					redraw = true;
					highlightedLaserInteractable[0] = i;
					highlightedLaserInteractable[1] = 0;
					previousXDrag = x;
					previousYDrag = y;
					return 0;
				} else {
					highlightedLaserInteractable[0] = -1;
				}
			}

			{ //Check if the mouse is over the object's ROTATE icon
				D2D1_POINT_2F boundaryPoint1 = { laserInteractables[i].points[0].x - cos(laserInteractables[i].rotation - atan(13.0f / 39.0f)) * pow(1690, 0.5f),
					laserInteractables[i].points[0].y + sin(laserInteractables[i].rotation - atan(13.0f / 39.0f)) * pow(1690, 0.5f) };
				D2D1_POINT_2F boundaryPoint2 = { laserInteractables[i].points[0].x - cos(M_PI / 4 - laserInteractables[i].rotation) * pow(338, 0.5f),
					laserInteractables[i].points[0].y - sin(M_PI / 4 - laserInteractables[i].rotation) * pow(338, 0.5f) };
				D2D1_POINT_2F boundaryPoint3 = { laserInteractables[i].points[0].x - cos(laserInteractables[i].rotation + atan(13.0f / 39.0f)) * pow(1690, 0.5f),
					laserInteractables[i].points[0].y + sin(laserInteractables[i].rotation + atan(13.0f / 39.0f)) * pow(1690, 0.5f) };
				D2D1_POINT_2F boundaryPoint4 = { laserInteractables[i].points[0].x - cos(M_PI / 4 + laserInteractables[i].rotation) * pow(338, 0.5f),
					laserInteractables[i].points[0].y + sin(M_PI / 4 + laserInteractables[i].rotation) * pow(338, 0.5f) };

				if (CalculateInequalityCountMirror(i, x, y, boundaryPoint1, boundaryPoint2, boundaryPoint3, boundaryPoint4) >= 2) {
					SetCursor(LoadCursor(NULL, IDC_HAND));

					pRenderTarget->BeginDraw();
					RenderScene();
					float imageX = (-cos(laserInteractables[i].rotation) * 76) + (laserInteractables[i].points[0].x + laserInteractables[i].points[1].x) / 2;
					float imageY = (sin(laserInteractables[i].rotation) * 76) + (laserInteractables[i].points[0].y + laserInteractables[i].points[1].y) / 2;
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation((laserInteractables[i].rotation + M_PI / 2) * (-180 / M_PI), D2D1::Point2F(imageX, imageY)));
					pRenderTarget->DrawBitmap(rotateImage, D2D1::RectF(imageX - 10.5, imageY - 3.5, imageX + 10.5, imageY + 3.5));
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
					imageX = (-cos(laserInteractables[i].rotation) * 102) + (laserInteractables[i].points[0].x + laserInteractables[i].points[1].x) / 2;
					imageY = (sin(laserInteractables[i].rotation) * 102) + (laserInteractables[i].points[0].y + laserInteractables[i].points[1].y) / 2;
					pRenderTarget->DrawBitmap(trashImage, D2D1::RectF(imageX - 9, imageY - 9, imageX + 9, imageY + 9));
					DrawMenu();
					pRenderTarget->EndDraw();

					redraw = true;
					highlightedLaserInteractable[0] = i;
					highlightedLaserInteractable[1] = 1;
					return 0;
				} else {
					highlightedLaserInteractable[0] = -1;
				}
			}

			{ //Check if the mouse is over the object's TRASH icon
				D2D1_POINT_2F boundaryPoint1 = { laserInteractables[i].points[0].x - cos(laserInteractables[i].rotation - atan(13.0f / 65.0f)) * pow(4394, 0.5f), // CHANGE
					laserInteractables[i].points[0].y + sin(laserInteractables[i].rotation - atan(13.0f / 65.0f)) * pow(4394, 0.5f) };
				D2D1_POINT_2F boundaryPoint2 = { laserInteractables[i].points[0].x - cos(laserInteractables[i].rotation - atan(13.0f / 39.0f)) * pow(1690, 0.5f),
					laserInteractables[i].points[0].y + sin(laserInteractables[i].rotation - atan(13.0f / 39.0f)) * pow(1690, 0.5f) };
				D2D1_POINT_2F boundaryPoint3 = { laserInteractables[i].points[0].x - cos(laserInteractables[i].rotation + atan(13.0f / 65.0f)) * pow(4394, 0.5f), // CHANGE
					laserInteractables[i].points[0].y + sin(laserInteractables[i].rotation + atan(13.0f / 65.0f)) * pow(4394, 0.5f) };
				D2D1_POINT_2F boundaryPoint4 = { laserInteractables[i].points[0].x - cos(laserInteractables[i].rotation + atan(13.0f / 39.0f)) * pow(1690, 0.5f),
					laserInteractables[i].points[0].y + sin(laserInteractables[i].rotation + atan(13.0f / 39.0f)) * pow(1690, 0.5f) };

				if (CalculateInequalityCountMirror(i, x, y, boundaryPoint1, boundaryPoint2, boundaryPoint3, boundaryPoint4) >= 2) {
					SetCursor(LoadCursor(NULL, IDC_HAND));

					pRenderTarget->BeginDraw();
					RenderScene();
					float imageX = (-cos(laserInteractables[i].rotation) * 76) + (laserInteractables[i].points[0].x + laserInteractables[i].points[1].x) / 2;
					float imageY = (sin(laserInteractables[i].rotation) * 76) + (laserInteractables[i].points[0].y + laserInteractables[i].points[1].y) / 2;
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation((laserInteractables[i].rotation + M_PI / 2) * (-180 / M_PI), D2D1::Point2F(imageX, imageY)));
					pRenderTarget->DrawBitmap(rotateImage, D2D1::RectF(imageX - 9, imageY - 3, imageX + 9, imageY + 3));
					pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
					imageX = (-cos(laserInteractables[i].rotation) * 102) + (laserInteractables[i].points[0].x + laserInteractables[i].points[1].x) / 2;
					imageY = (sin(laserInteractables[i].rotation) * 102) + (laserInteractables[i].points[0].y + laserInteractables[i].points[1].y) / 2;
					pRenderTarget->DrawBitmap(trashImage, D2D1::RectF(imageX - 10, imageY - 10, imageX + 10, imageY + 10));
					DrawMenu();
					pRenderTarget->EndDraw();

					redraw = true;
					highlightedLaserInteractable[0] = i;
					highlightedLaserInteractable[1] = 2;
					return 0;
				} else {
					highlightedLaserInteractable[0] = -1;
				}
			}

		}

		SetCursor(hCursor);
		previousX = x;
		previousY = y;
		if (redraw == true) {
			redraw = false;
			pRenderTarget->BeginDraw();
			RenderScene();
			pRenderTarget->EndDraw();
		}

		return 0;
	}

	case WM_SIZE: {
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
		pRenderTarget->Resize(size);
		pRenderTarget->BeginDraw();
		RenderScene();
		pRenderTarget->EndDraw();
		return 0;
	}

	case WM_CREATE: {
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
		pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hwnd, size), &pRenderTarget);
		pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.0f, 0.0f), &pBrush);

		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)(&pIWICFactory));
		LoadResourceBitmap(L"rotateImage", L"Image", &rotateImage);
		LoadResourceBitmap(L"trashImage", L"Image", &trashImage);
		LoadResourceBitmap(L"powerImage", L"Image", &powerImage);

		srand(time(0));
		return 0;
	}

	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}

	default: {
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}

	}
	return TRUE;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
	MainWindow win;

	if (!win.Create(hInstance, L"MIRRORS", WS_TILEDWINDOW)) {
		return 0;
	}
	ShowWindow(win.Window(), nCmdShow);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}