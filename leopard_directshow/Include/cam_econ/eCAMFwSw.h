// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ECAMFWSW_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ECAMFWSW_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef ECAMFWSW_EXPORTS
#define ECAMFWSW_API __declspec(dllexport)
#else
#define ECAMFWSW_API __declspec(dllimport)
#endif

#define VID					L"2560"
#define See3CAM_12CUNIR		L"C113"

enum e_GPIONumber
{
	Output80MP = 0,
	Input1_80MP = 1,
	Input2_80MP = 2,
	Output1CU50 = 3,
	Output2CU50 = 4,
	Input1CU50 = 5,
	Input2CU50 = 6,
	OutputAR0130 = 7
};

								                  
BOOL InitExtensionUnit(TCHAR *USBInstanceID);

BOOL DeinitExtensionUnit();

BOOL ReadFirmwareVersion (UINT8 *pMajorVersion, UINT8 *pMinorVersion1, UINT16 *pMinorVersion2, UINT16 *pMinorVersion3);

BOOL GetCameraUniqueID(TCHAR *szUniqueID);

BOOL EnableMasterMode();

BOOL EnableTriggerMode();

BOOL GetGpioLevel (UINT8 GpioPin,UINT8 *GpioValue);

BOOL SetGpioLevel (UINT8 GpioPin,UINT8 GpioValue);

UINT8 EnableCroppedVGAMode();

UINT8 EnableBinnedVGAMode();

BOOL GetFlashAR0130(UINT8 *Value);

BOOL SetFlashAR0130(UINT8 Value);


