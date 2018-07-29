#include "spheres-app.h"

#pragma comment(lib, "d2d1")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	SpheresApp app;
	app.Initialize(hInstance, nShowCmd);
	return app.Run();
}
