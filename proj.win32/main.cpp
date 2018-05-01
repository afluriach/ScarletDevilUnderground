#include "Prefix.h"

#include "main.h"
#include "App.h"
#include "cocos2d.h"

USING_NS_CC;

int __stdcall _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // create the application instance
	App inst;
    return Application::getInstance()->run();
}
