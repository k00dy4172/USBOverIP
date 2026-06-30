#include "USBManager.h"
//#include <libusb.h> 헤더에서 참조를 했기 때문에 필요 없음
#include <iostream>


USBManager::USBManager()
{
    m_Context = nullptr;
}

USBManager::~USBManager()
{
    Shutdown();
}

bool USBManager::Initialize()
{
	int Result = libusb_init(&m_Context); //m_Context는 libsub의 컨텍스트를 나타내는 포인터이다. 
	//libusb_init() 함수는 libusb를 초기화하고, m_Context에 libusb_context 구조체의 포인터를 할당한다.
    //즉, libusb_init() 함수는 m_Context의 주소를 받는다. 

    if (Result != LIBUSB_SUCCESS)
    {
        std::cout << "Failed to initialize libusb." << std::endl;
        return false;
    }

    std::cout << "libusb initialized successfully." << std::endl;

    return true;
}

void USBManager::Shutdown()
{
    if (m_Context != nullptr)
    {
        libusb_exit(m_Context);

        m_Context = nullptr;

        std::cout << "libusb shutdown completed." << std::endl;
    }
}

void USBManager::EnumerateDevices()
{
    libusb_device** DeviceList = nullptr;

    ssize_t DeviceCount = libusb_get_device_list(
		m_Context, //컨텍스트의 포인터를 첫 번째 매개변수로 받는다.
		&DeviceList //DeviceList 변수의 주소를 전달한다.
                    //libusb는 이 주소를 통해 DeviceList가 가리킬 장치 목록의 시작 주소를 저장한다.
    ); //libusb_get_device_list() 함수는 연결된 장치의 총 개수, DeviceList에 장치 목록을 할당한다.
    // 이 함수가 실행되면 libusb는 m_Context 내부가 아니라, '완전히 새로운 독립적인 메모리 공간(힙)'에 
    // 장치 목록 배열(주소록)을 새로 만들고, DeviceList 변수에 그 주소록의 시작 위치를 할당한다.
	if (DeviceCount < 0)
	{
		std::cout << "Failed to get USB device list." << std::endl;
		return;
	}
	std::cout << "Found " << DeviceCount << " USB devices." << std::endl; //연결된 장치의 총 개수를 출력한다.
    
    for (ssize_t i = 0;i < DeviceCount;i++) 
    {
		libusb_device* Device = DeviceList[i]; //주소록(DeviceList)에서 i번째 장치의 주소를 가져온다.
        libusb_device_descriptor Descriptor;
		int Result = //성공 시 0을 반환하고, 실패 시 음수 값을 반환한다. 
            libusb_get_device_descriptor(
				Device, //조회할 장치의 주소를 전달한다.
				&Descriptor //Descriptor 변수의 주소를 전달한다. libusb는 이 주소를 통해 Descriptor에 장치의 정보를 저장한다.
            );
		if (Result != LIBUSB_SUCCESS) //조회에 실패하면, libusb_get_device_descriptor() 함수는 음수 값을 반환한다.
		{
			std::cout << "Failed to get device descriptor for device " << i << std::endl;
			continue;
		}
		std::cout //Vendor ID와 Product ID를 16진수로 출력한다.
            << "VID : 0x"
            << std::hex
            << Descriptor.idVendor
            << std::dec
            << std::endl;

        std::cout
            << "PID : 0x"
            << std::hex
            << Descriptor.idProduct
            << std::dec
            << std::endl;

    }

    libusb_free_device_list(DeviceList, 1);
}
