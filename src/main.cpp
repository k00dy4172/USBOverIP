// main.cpp : 애플리케이션의 진입점을 정의합니다.
//

#include "main.h"
#include "USBManager.h"


using namespace std;

int main()
{
    USBManager manager;

    if (!manager.Initialize())
    {
        return -1;
    }
	manager.EnumerateDevices();

    return 0;
}
