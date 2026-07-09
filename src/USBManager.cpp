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
        if (Descriptor.idVendor != 5426) continue; // 레이저 마우스만 감지

        libusb_device_handle* DeviceHandle = nullptr;

        Result = //0을 반환하면 성공, 음수 값을 반환하면 실패이다.
            libusb_open( //장치를 열고, DeviceHandle에 장치 핸들을 저장한다.
                Device, //조회할 장치의 주소를 첫 번째 매개변수로 전달한다.
                &DeviceHandle //DeviceHandle의 주소를 매개변수로 전달한다.
                //libusb_open() 함수는 장치를 열고, DeviceHandle에 장치 핸들을 저장한다.
            );

        if (Result == LIBUSB_ERROR_NOT_SUPPORTED) continue;

        libusb_set_auto_detach_kernel_driver(
            DeviceHandle,
            1
        );
        std::cout << "========================================" << std::endl;
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


        if (Result != LIBUSB_SUCCESS) //장치를 여는 데 실패하면, libusb_open() 함수는 음수 값을 반환한다.
        {
            std::cout << "Failed to open device "
                << i
                << " : "
                << libusb_error_name(Result)
                << std::endl;
            continue;
        }
		PrintDeviceInfo(Descriptor, DeviceHandle); //장치의 제조사 문자열을 출력한다.

        //PrintConfiguration(Device);

        ReadReportDescriptor(
            Device,
            DeviceHandle
        );
        
        libusb_close(DeviceHandle); //핸들을 닫는다. 
       
    }
    libusb_free_device_list(DeviceList, 1); //DeviceList에 할당된 메모리를 해제한다. 또한 참조자 수를 1 감소 시킨다.
}
void USBManager::PrintDeviceInfo(const libusb_device_descriptor &Descriptor, libusb_device_handle* DeviceHandle) {
    const std::string DataString[3] = { "Manufacturer", "Product", "SerialNumber"};
    const uint8_t DataIndex[3] = {Descriptor.iManufacturer, Descriptor.iProduct, Descriptor.iSerialNumber};
	
    for (int i = 0; i < 3; i++)
    {
        unsigned char Data[256]{};
        int Result = //성공하면 문자열의 길이를 반환하고, 실패하면 음수 값을 반환한다.
            libusb_get_string_descriptor_ascii(//장치의 제조사 문자열을 가져온다.
                DeviceHandle, //장치 핸들을 첫 번째 매개변수로 전달한다.
                DataIndex[i], //장치 설명자에서 Data의 문자열의 인덱스를 두 번째 매개변수로 전달한다.
                &Data[0], ///Data 배열의 첫 번째 요소의 주소를 세 번째 매개변수로 전달한다.
                256 //Data 배열의 크기를 네 번째 매개변수로 전달한다.
            );
        if (Result > 0) std::cout <<DataString[i]<< ": " << Data << std::endl;
        else
        {
            std::cout 
                <<"Failed to get"
                <<DataString[i]<<" string for device "
                << " : "
                << libusb_error_name(Result)
                << std::endl;
        }
    }
}

void USBManager::PrintConfiguration(libusb_device* Device)
{
    libusb_config_descriptor* Config = nullptr;

    int Result = //성공하면 0을 반환하고 실패하면 음수를 반환한다.
        libusb_get_config_descriptor(
            Device,
            0,
            &Config
        );

    if (Result != LIBUSB_SUCCESS)
    {
        std::cout << "Failed to get configuration. : "
            << libusb_error_name(Result)
            << std::endl;
        return;
    }

    std::cout
        << "Found Configuration Interfaces : "
        << static_cast<int> (Config->bNumInterfaces)
        << std::endl;
    PrintInterfaceInfo(Config);

    libusb_free_config_descriptor(Config);

}

void USBManager::PrintInterfaceInfo(const libusb_config_descriptor* Config) 
{
    for(uint8_t i = 0;i < Config->bNumInterfaces;i++) 
    {
        const libusb_interface& Interface = Config->interface[i];
        const libusb_interface_descriptor& Descriptor = Interface.altsetting[0];
        if (Descriptor.bInterfaceClass != LIBUSB_CLASS_HID) continue;
        std::cout << "--------- HID DEVICE FOUND ----------" << std::endl;
        std::cout
            << "Interface: "
            << static_cast<int>(i)
            << std::endl;
        std::cout
            << "Class : "
            << GetInterfaceClassName(Descriptor.bInterfaceClass)
            << "\n"
            << std::endl;

        PrintEndpointInfo(Descriptor);

    }

}

void USBManager::PrintEndpointInfo(const libusb_interface_descriptor& Descriptor)
{
    for (uint8_t i = 0; i < Descriptor.bNumEndpoints; i++)
    {
        const libusb_endpoint_descriptor& Endpoint = Descriptor.endpoint[i];
            
        std::cout
            << "Endpoint : "
            << static_cast<int>(i)
            << std::endl;
        std::cout
            << "Address : 0x"
            << std::hex
            << static_cast<int>(Endpoint.bEndpointAddress)
            << std::dec
            << std::endl;
        if (Endpoint.bEndpointAddress & LIBUSB_ENDPOINT_IN)
        {
            std::cout
                << "Direction : IN"
                << std::endl;
        }
        else
        {
            std::cout
                << "Direction : OUT"
                << std::endl;
        }
        std::cout
            << "Max Packet Size : "
            << Endpoint.wMaxPacketSize
            <<"\n"
            << std::endl;

        std::cout
            << "Transfer Type : "
            << GetTransferTypeName(Endpoint.bmAttributes)
            << std::endl;
    }
}

void USBManager::ReadReportDescriptor(
    libusb_device* Device,
    libusb_device_handle* DeviceHandle)
{
    libusb_config_descriptor* Config = nullptr;

    int Result =
        libusb_get_config_descriptor(
            Device,
            0,
            &Config
        );
    if (Result != LIBUSB_SUCCESS)
    {
        std::cout
            << "Failed to get configuration."
            << std::endl;
        return;
    }

    for (uint8_t i = 0; i < Config->bNumInterfaces; i++)
    {
        std::cout << "\n----------------------------------------------" << std::endl;
        const libusb_interface& Interface = Config->interface[i];

        const libusb_interface_descriptor& Descriptor =
            Interface.altsetting[0];

        if (Descriptor.bInterfaceClass != LIBUSB_CLASS_HID)
        {
            continue;
        }

        std::cout
            << "Preparing Report Descriptor for Interface "
            << static_cast<int>(Descriptor.bInterfaceNumber)
            << std::endl;

        const unsigned char* Extra = Descriptor.extra;

        if (Descriptor.extra_length < 9)
        {
            std::cout << "No HID Descriptor" << std::endl;
            continue;
        }

        uint16_t ReportDescriptorLength =
            (Extra[8] << 8) | Extra[7];

        std::cout
            << "Extra Descriptor Size : "
            << static_cast<int>(Extra[0])
            << std::endl;

        std::cout
            << "HID Version (BCD) : 0x"
            << std::hex
            << ((Extra[3] << 8) | Extra[2])
            << std::dec
            << std::endl;

        std::cout
            << "Report Descriptor Length : "
            << ReportDescriptorLength
            << std::endl;
        unsigned char* ReportDescriptor =
            new unsigned char[ReportDescriptorLength];

        Result =
            libusb_control_transfer(
                DeviceHandle,
                0x81,
                LIBUSB_REQUEST_GET_DESCRIPTOR,
                (0x22 << 8),
                Descriptor.bInterfaceNumber,
                ReportDescriptor,
                ReportDescriptorLength,
                1000
            );
        std::cout
            << "Control Transfer Returned : "
            << Result
            << std::endl;
        if (Result < 0)
        {
            std::cout
                << "Failed to read Report Descriptor : "
                << libusb_error_name(Result)
                << std::endl;
        }
        else
        {
            std::cout
                << "Report Descriptor Size : "
                << Result
                << std::endl;

  
        }
        ClaimInterface(Descriptor, DeviceHandle);

        
        delete[] ReportDescriptor; 
    }
   
    libusb_free_config_descriptor(Config);
    

}

void USBManager::ClaimInterface(
    const libusb_interface_descriptor& Descriptor,
    libusb_device_handle* DeviceHandle
)
{
    int Result = libusb_claim_interface(
        DeviceHandle,
        Descriptor.bInterfaceNumber
    );

    if (Result != LIBUSB_SUCCESS)
    {
        std::cout
            << "Claim failed : "
            << libusb_error_name(Result)
            << std::endl;
    }
    else
    {
        std::cout
            << "Interface claimed!"
            << std::endl;
    }
    ReadInterruptData(Descriptor, DeviceHandle);

    libusb_release_interface(
        DeviceHandle,
        Descriptor.bInterfaceNumber
    );
}

void USBManager::ReadInterruptData(
    const libusb_interface_descriptor& Descriptor,
    libusb_device_handle* DeviceHandle
) 
{
    for (uint8_t i = 0; i < Descriptor.bNumEndpoints; i++)
    {
        const libusb_endpoint_descriptor& Endpoint =
            Descriptor.endpoint[i];

        if ((Endpoint.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
            != LIBUSB_ENDPOINT_IN)
        {
            continue;
        }

        if ((Endpoint.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK)
            != LIBUSB_TRANSFER_TYPE_INTERRUPT)
        {
            continue;
        }
        unsigned char Buffer[64]{};

        int ActualLength = 0;

        int Result =
            libusb_interrupt_transfer(
                DeviceHandle,
                Endpoint.bEndpointAddress,
                Buffer,
                sizeof(Buffer),
                &ActualLength,
                1000
            );

        if (Result == LIBUSB_SUCCESS)
        {
            std::cout
                << "Received "
                << ActualLength
                << " bytes"
                << std::endl;
        }
        else
        {
            std::cout
                << "Read failed : "
                << libusb_error_name(Result)
                << std::endl;
            std::cout
                << "Endpoint Address : 0x"
                << std::hex
                << static_cast<int>(Endpoint.bEndpointAddress)
                << std::dec
                << std::endl;
            std::cout
                << "Interface Number : "
                << static_cast<int>(Descriptor.bInterfaceNumber)
                << std::endl;
        }


    }
}

const char* USBManager::GetTransferTypeName(uint8_t Attributes)
{
    switch(Attributes & LIBUSB_TRANSFER_TYPE_MASK)
    {
    case LIBUSB_TRANSFER_TYPE_BULK:
         return "Bulk";
    case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
        return "Isochronous";
    case LIBUSB_TRANSFER_TYPE_CONTROL:
        return "Control";
    case LIBUSB_TRANSFER_TYPE_INTERRUPT:
        return "Interrupt";
    default:
        return "Unknown";

    }
}
const char* USBManager::GetInterfaceClassName(uint8_t InterfaceClass)
{
    switch (InterfaceClass)
    {
    case LIBUSB_CLASS_HID:
        return "HID";

    case LIBUSB_CLASS_AUDIO:
        return "Audio";

    case LIBUSB_CLASS_HUB:
        return "Hub";

    case LIBUSB_CLASS_MASS_STORAGE:
        return "Mass Storage";

    case LIBUSB_CLASS_COMM:
        return "Communication";

    case LIBUSB_CLASS_VENDOR_SPEC:
        return "Vendor Specific";

    default:
        return "Unknown";
    }
}