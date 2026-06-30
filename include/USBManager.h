#pragma once

#include <libusb.h>

class USBManager
{
public:

    USBManager();
    ~USBManager();

    bool Initialize();
    void Shutdown();

    void EnumerateDevices();

private:
	void PrintDeviceInfo(libusb_device* Device, const libusb_device_descriptor &Descriptor, libusb_device_handle* DeviceHandle);

private:

	libusb_context* m_Context; //m_Context는 libusb의 컨텍스트의 포인터이다.
	//컨텍스트는 libusb의 상태를 나타내며, libusb_init() 함수 호출 시 생성된다.
	//libusb_context는 libusb의 초기화 및 종료, 장치 열기 및 닫기, 전송 수행 등과 같은 작업을 수행하는 데 첫 번째 매개변수로서 활용된다.
    //libusb의 작업장과 비슷한 역할을 한다.
};