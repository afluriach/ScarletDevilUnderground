#include "Prefix.h"

#include "main.h"

USING_NS_CC;

int __stdcall _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // create the application instance
	unique_ptr<App> inst = make_unique<App>();
    return Application::getInstance()->run();
}
