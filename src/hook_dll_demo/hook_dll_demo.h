// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the HOOKDLLDEMO_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// HOOKDLLDEMO_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef HOOKDLLDEMO_EXPORTS
#define HOOKDLLDEMO_API __declspec(dllexport)
#else
#define HOOKDLLDEMO_API __declspec(dllimport)
#endif

// This class is exported from the dll
class HOOKDLLDEMO_API Chookdlldemo {
public:
	Chookdlldemo(void);
	// TODO: add your methods here.
};

extern HOOKDLLDEMO_API int nhookdlldemo;

HOOKDLLDEMO_API int fnhookdlldemo(void);
