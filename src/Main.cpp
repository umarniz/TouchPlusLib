#include <cstdio>
#include "VideoCapture.h"
 
#define TEST(x) printf("\nPress enter to test: " #x); getchar(); fflush(stdin); x;
 
void callback(unsigned char* data, int len, int bpp)
{
    //printf("DATA %d %d\n",len, bpp);
}


typedef int (CALLBACK* SPAEWB_EnumDevice)(int* param);
typedef int (CALLBACK* SPAEWB_GetDeviceName)(int id, char* name, char* data);
typedef int (CALLBACK* SPAEWB_SelectDevice)(int param);
typedef int (CALLBACK* SPAEWB_UnlockDevice)(int param);
typedef int (CALLBACK* SPAEWB_LockDevice)(int param);
typedef int (CALLBACK* SPAEWB_SetSensorType)(int type);
typedef int (CALLBACK* SPAEWB_DisableAE)();
typedef int (CALLBACK* SPAEWB_DisableAWB)();

SPAEWB_EnumDevice enumDevice;    // Function pointer
SPAEWB_GetDeviceName getDeviceName;    // Function pointer
SPAEWB_SelectDevice selectDevice;    // Function pointer
SPAEWB_UnlockDevice unlockDevice;    // Function pointer
SPAEWB_LockDevice lockDevice;    // Function pointer
SPAEWB_SetSensorType setSensorType;
SPAEWB_DisableAE disableAE;
SPAEWB_DisableAWB disableAWB;

bool InitSPAEAWB()
{
	HINSTANCE hDLL;               // Handle to DLL

	hDLL = LoadLibrary(L"eSPAEAWBCtrl");
	if (hDLL != NULL)
	{
	   enumDevice = (SPAEWB_EnumDevice)GetProcAddress(hDLL,
											   "_eSPAEAWB_EnumDevice@4");

	   getDeviceName = (SPAEWB_GetDeviceName)GetProcAddress(hDLL,
											   "_eSPAEAWB_GetDevicename@12");

	   selectDevice = (SPAEWB_SelectDevice)GetProcAddress(hDLL,
											   "_eSPAEAWB_SelectDevice@4");

	   unlockDevice = (SPAEWB_UnlockDevice)GetProcAddress(hDLL,
											   "_eSPAEAWB_SWUnlock@4");

	   lockDevice = (SPAEWB_LockDevice)GetProcAddress(hDLL,
											   "_eSPAEAWB_SWLock@4");

	   setSensorType = (SPAEWB_SetSensorType)GetProcAddress(hDLL,
											   "_eSPAEAWB_SetSensorType@4");

	   disableAE = (SPAEWB_DisableAE)GetProcAddress(hDLL,
											   "_eSPAEAWB_DisableAE@0");
	   
	   disableAWB = (SPAEWB_DisableAWB)GetProcAddress(hDLL,
											   "_eSPAEAWB_DisableAWB@0");

	   if (!enumDevice || !getDeviceName || !selectDevice || !unlockDevice ||!lockDevice ||!setSensorType || !disableAE || !disableAWB)
	   {
		  // handle the error
		  FreeLibrary(hDLL);
		  printf("Error opening eSPAEAWBCtrl\n");
	   }
	   else
		   return true;
	}
	else
		printf("Failed opening eSPDI.dll\n");

	return false;
}


typedef void* (WINAPIV* SPDI_Init)(void *ptr);
typedef int (WINAPIV* SPDI_GetDeviceNumber)(void* ptr);
typedef int (WINAPIV* SPDI_FindDevice)(void* ptr, int id);

SPDI_Init spdi_init;    // Function pointer
SPDI_GetDeviceNumber spdi_getDeviceNumber;    // Function pointer
SPDI_FindDevice spdi_findDevice;    // Function pointer

bool InitSPDI()
{
	HINSTANCE hDLL;               // Handle to DLL

	hDLL = LoadLibrary(L"eSPDI.dll");
	if (hDLL != NULL)
	{
	   spdi_init = (SPDI_Init)GetProcAddress(hDLL,
											   "EtronDI_Init");

	   spdi_getDeviceNumber = (SPDI_GetDeviceNumber)GetProcAddress(hDLL,
											   "EtronDI_GetDeviceNumber");

	   spdi_findDevice = (SPDI_FindDevice)GetProcAddress(hDLL,
											   "EtronDI_FindDevice");

	   if (!spdi_init || !spdi_getDeviceNumber || !spdi_findDevice)
	   {
		  // handle the error
		  FreeLibrary(hDLL);
		  printf("Error opening SPDI\n");
	   }
	   else
		   return true;
	}
	else
		printf("Failed opening eSPDI.dll\n");

	return false;
}

void* passPtr = 0;

bool UnlockSoftware()
{
	printf("Sizes: int %i char %i bool %i short %i pointer %i\n", sizeof(int), sizeof(char), sizeof(bool), sizeof(short), sizeof(void*));

	int result = 0;

	if (!InitSPAEAWB())
		return false;

	if (!InitSPDI())
		return false;

	printf("Loaded libraries\n");

	int deviceCount;
	
	void* temp;
	spdi_init(&passPtr);
	printf("spdi_init %i\n", result);
	

	deviceCount = spdi_getDeviceNumber(passPtr);
	printf("spdi_getDeviceNumber %i\n", deviceCount);
	result = spdi_findDevice(passPtr, deviceCount);
	printf("spdi_findDevice %i\n", result);
	
	int numDevices = 0;
	// call the function
	int ret = enumDevice(&numDevices);

		  
	printf("Found %i devices - Return %i\n", numDevices, ret);
	for (int i=0;i<numDevices;i++)
	{
		char name[500];
		char unknown[500];

		memset(name,0,sizeof(name));
		memset(unknown,0,sizeof(unknown));

		getDeviceName(i, name, unknown);
		printf("Device: %i - %s\n", i, name);
	}

	
	result = selectDevice(0);
	printf("Select Result: %i\n", result);

	result = setSensorType(1);
	printf("SetSenorTypeResult Result: %i\n", result);

	result = unlockDevice(263);
	printf("Unlock Result: %i\n", result);
	
	result = disableAE();
	printf("DisableAE Result: %i\n", result);

	result = disableAWB();
	printf("disableAWB Result: %i\n", result);


	printf("Done unlocking Touch+\n");
}

 
int main()
{
	if (!UnlockSoftware())
		printf("Failed to unlock\n");

    VideoCapture* vc        = new VideoCapture();
	vc->Start();

	lockDevice(0);

    return 0;
}