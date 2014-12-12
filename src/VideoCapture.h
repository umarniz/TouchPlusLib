#pragma once
#pragma comment(lib, "strmiids")
//#pragma comment(linker, "/alternatename:_pWeakValue=_pDefaultWeakValue")

#include <atlbase.h>
#include <windows.h>
#include <dshow.h>
#include "qedit.h"
#include <initguid.h>

class __declspec(uuid("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")) SampleGrabber;  

//class __declspec(uuid("17CCA71B-ECD7-11D0-B908-00A0C9223196")) CLSID_TouchCamera;
//class __declspec(uuid("C1F400A0-3F08-11D3-9F0B-006008039E37")) CLSID_SampleGrabber;
//class __declspec(uuid("B87BEB7B-8D29-423F-AE4D-6582C10175AC")) CLSID_VideoRenderer;

//{17CCA71B-ECD7-11D0-B908-00A0C9223196}

DEFINE_GUID(CLSID_TouchCamera, 0x17CCA71B, 0xECD7, 0x11D0, 0xB9, 0x08, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96); //ksproxy.ax

//{C1F400A0-3F08-11D3-9F0B-006008039E37}
DEFINE_GUID(CLSID_SampleGrabber2, 0xC1F400A0, 0x3F08, 0x11D3, 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37); //qedit.dll

//{B87BEB7B-8D29-423F-AE4D-6582C10175AC}
DEFINE_GUID(CLSID_VideoRenderer, 0xB87BEB7B, 0x8D29, 0x423F, 0xAE, 0x4D, 0x65, 0x82, 0xC1, 0x01, 0x75, 0xAC); //quartz.dll


class VideoCapture
{
public:
	void Start();
	void Stop();

private:
	HRESULT BuildGraph(IGraphBuilder *pGraph);

	CComPtr<IPin> GetPin(IBaseFilter *pFilter, LPCOLESTR pinname);

	HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum);
	IBaseFilter* FindTouchFilter(IEnumMoniker *pEnum);
};