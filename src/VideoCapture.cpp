#include "VideoCapture.h"

//change this macro to fit your style of error handling
#define CHECK_HR(hr, msg) if (hrcheck(hr, msg)) return hr;

BOOL hrcheck(HRESULT hr, TCHAR* errtext)
{
	if (hr >= S_OK)
		return FALSE;
		
	//TCHAR szErr[MAX_ERROR_TEXT_LEN];
	//DWORD res = AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
	
	//if (res)
	//	printf("Error %x: %s\n%s\n",hr, errtext,szErr);
	//else
		printf("Error %x: %s\n", hr, errtext);
	return TRUE;
}

CComPtr<IPin> VideoCapture::GetPin(IBaseFilter *pFilter, LPCOLESTR pinname)
{
	IEnumPins *pEnum = NULL;
	CComPtr<IPin> pPin;
		
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (hrcheck(hr, L"Can't enumerate pins."))
		return NULL;
		
	while(pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_INFO pinfo;
		pPin->QueryPinInfo(&pinfo);
		BOOL found = !_wcsicmp(pinname, pinfo.achName);
		
		if (pinfo.pFilter) pinfo.pFilter->Release();
		
		if (found)
			return pPin;

		pPin.Release();
	}
	printf("Pin not found!\n");
	return NULL;
}

HRESULT VideoCapture::EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
    // Create the System Device Enumerator.
    ICreateDevEnum *pDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,  
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

    if (SUCCEEDED(hr))
    {
        // Create an enumerator for the category.
        hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
        if (hr == S_FALSE)
        {
            hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        }
        pDevEnum->Release();
    }
    return hr;
}


IBaseFilter* VideoCapture::FindTouchFilter(IEnumMoniker *pEnum)
{
    IMoniker *pMoniker = NULL;

    while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
    {
        IPropertyBag *pPropBag;
        HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        if (FAILED(hr))
        {
            pMoniker->Release();
            continue;  
        } 

        VARIANT var;
        VariantInit(&var);

        // Get description or friendly name.
        hr = pPropBag->Read(L"Description", &var, 0);
        if (FAILED(hr))
        {
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
        }
        if (SUCCEEDED(hr))
        {
			if(0 == wcscmp(var.bstrVal, L"Touch+ Camera"))
			{
				printf("Found Touch+ Camera\n");

				IBaseFilter *pCap = NULL;
				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCap);

				VariantClear(&var); 	
				pPropBag->Release();
				pMoniker->Release();

				return pCap;
			}

            printf("%S\n", var.bstrVal);
            VariantClear(&var); 
        }

        hr = pPropBag->Write(L"FriendlyName", &var);

        // WaveInID applies only to audio capture devices.
        hr = pPropBag->Read(L"WaveInID", &var, 0);
        if (SUCCEEDED(hr))
        {
            printf("WaveIn ID: %d\n", var.lVal);
            VariantClear(&var); 
        }

        hr = pPropBag->Read(L"DevicePath", &var, 0);
        if (SUCCEEDED(hr))
        {
            // The device path is not intended for display.
            printf("Device path: %S\n", var.bstrVal);
            VariantClear(&var); 
        }

        pPropBag->Release();
        pMoniker->Release();
    }

	return false;
}


HRESULT VideoCapture::BuildGraph(IGraphBuilder *pGraph)
{
	HRESULT hr = S_OK;
	
	CComPtr<IBaseFilter> pTouchCamera;
	CComPtr<ISampleGrabber> pSampleGrabber;
	CComPtr<IBaseFilter> pSampleGrabberF;

	IEnumMoniker *pEnum;

	//graph builder
	CComPtr<ICaptureGraphBuilder2> pBuilder;
	hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
	CHECK_HR(hr, L"Can't create Capture Graph Builder");
	hr = pBuilder->SetFiltergraph(pGraph);
	CHECK_HR(hr, L"Can't SetFiltergraph");

	//add Touch+ Camera
	hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
    if (SUCCEEDED(hr))
    {
		pTouchCamera = FindTouchFilter(pEnum);
        pEnum->Release();
    }

	hr = pGraph->AddFilter(pTouchCamera, L"Touch+ Camera");
	CHECK_HR(hr, L"Can't add Touch+ Camera to graph");
	
	//add SampleGrabber
	// Initialize core variables
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSampleGrabberF));
	CHECK_HR(hr, L"Can't create SampleGrabber");
	hr = pSampleGrabberF->QueryInterface(IID_ISampleGrabber, (void**)&pSampleGrabber);
	CHECK_HR(hr, L"Can't get SampleGrabber from filter");

	// Setup media format
	AM_MEDIA_TYPE pSampleGrabber_pmt;
	ZeroMemory(&pSampleGrabber_pmt, sizeof(AM_MEDIA_TYPE));
	pSampleGrabber_pmt.majortype = MEDIATYPE_Video;
	pSampleGrabber_pmt.subtype = MEDIASUBTYPE_MJPG;
	pSampleGrabber_pmt.formattype = FORMAT_VideoInfo;
	pSampleGrabber_pmt.bFixedSizeSamples = TRUE;
	pSampleGrabber_pmt.cbFormat = 88;
	pSampleGrabber_pmt.lSampleSize = 1843200;
	pSampleGrabber_pmt.bTemporalCompression = FALSE;
	VIDEOINFOHEADER pSampleGrabber_format;
	ZeroMemory(&pSampleGrabber_format, sizeof(VIDEOINFOHEADER));
	pSampleGrabber_format.dwBitRate = 884736000;
	pSampleGrabber_format.AvgTimePerFrame = 166666;
	pSampleGrabber_format.bmiHeader.biSize = 40;
	pSampleGrabber_format.bmiHeader.biWidth = 1280;
	pSampleGrabber_format.bmiHeader.biHeight = 480;
	pSampleGrabber_format.bmiHeader.biPlanes = 1;
	pSampleGrabber_format.bmiHeader.biBitCount = 24;
	pSampleGrabber_format.bmiHeader.biCompression = 1196444237;
	pSampleGrabber_format.bmiHeader.biSizeImage = 1843200;
	pSampleGrabber_pmt.pbFormat = (BYTE*)&pSampleGrabber_format;

	// Get pins for sample grabber and camera
	CComPtr<IPin> camPin = GetPin(pTouchCamera, L"Capture");
	CComPtr<IPin> sampleInput = GetPin(pSampleGrabberF, L"Input");
	CComPtr<IPin> sampleOutput = GetPin(pSampleGrabberF, L"Output");

	// Set media type for sample grabber and camera
	hr = pSampleGrabber->SetMediaType(&pSampleGrabber_pmt);
	CHECK_HR(hr, L"Can't set media type to sample grabber");

	CComQIPtr<IAMStreamConfig, &IID_IAMStreamConfig> camConfig(camPin);
	hr = camConfig->SetFormat(&pSampleGrabber_pmt);
	CHECK_HR(hr, L"Can't set camera pin format");


	// Add sample grabber filter
	hr = pGraph->AddFilter(pSampleGrabberF, L"SampleGrabber");
	CHECK_HR(hr, L"Can't add SampleGrabber to graph");


	//connect Touch+ Camera and SampleGrabber
	hr = pGraph->ConnectDirect(camPin, sampleInput, NULL);
	CHECK_HR(hr, L"Can't connect Touch+ Camera and SampleGrabber");

	printf("add MJPEG Decompressor\n");
	//add MJPEG Decompressor
	CComPtr<IBaseFilter> pMJPEGDecompressor;
	hr = pMJPEGDecompressor.CoCreateInstance(CLSID_MjpegDec);
	CHECK_HR(hr, L"Can't create MJPEG Decompressor");
	hr = pGraph->AddFilter(pMJPEGDecompressor, L"MJPEG Decompressor");

	CHECK_HR(hr, L"Can't add MJPEG Decompressor to graph");
	//connect SampleGrabber and MJPEG Decompressor
	hr = pGraph->ConnectDirect(sampleOutput, GetPin(pMJPEGDecompressor, L"XForm In"), NULL);
	CHECK_HR(hr, L"Can't connect SampleGrabber and MJPEG Decompressor");

	//add Color Space Converter
	CComPtr<IBaseFilter> pColorSpaceConverter;
	hr = pColorSpaceConverter.CoCreateInstance(CLSID_Colour);
	CHECK_HR(hr, L"Can't create Color Space Converter");
	hr = pGraph->AddFilter(pColorSpaceConverter, L"Color Space Converter");
	CHECK_HR(hr, L"Can't add Color Space Converter to graph");

	//connect MJPEG Decompressor and Color Space Converter
	hr = pGraph->ConnectDirect(GetPin(pMJPEGDecompressor, L"XForm Out"), GetPin(pColorSpaceConverter, L"Input"), NULL);
	CHECK_HR(hr, L"Can't connect MJPEG Decompressor and Color Space Converter");

	//add Video Renderer
	CComPtr<IBaseFilter> pVideoRenderer;
	hr = pVideoRenderer.CoCreateInstance(CLSID_VideoRenderer);
	CHECK_HR(hr, L"Can't create Video Renderer");
	hr = pGraph->AddFilter(pVideoRenderer, L"Video Renderer");
	CHECK_HR(hr, L"Can't add Video Renderer to graph");

	CComPtr<IVideoWindow> pVideoWindow;
	pVideoRenderer->QueryInterface(IID_PPV_ARGS(&pVideoWindow));
	pVideoWindow->put_Height(480);
	pVideoWindow->put_Width(1280);

	//connect Color Space Converter and Video Renderer
	hr = pGraph->ConnectDirect(GetPin(pColorSpaceConverter, L"XForm Out"), GetPin(pVideoRenderer, L"VMR Input0"), NULL);
	CHECK_HR(hr, L"Can't connect Color Space Converter and Video Renderer");

	return S_OK;
}

void VideoCapture::Start()
{
	HRESULT hr;

	CoInitialize(NULL);

	CComPtr<IGraphBuilder> graph;
	graph.CoCreateInstance(CLSID_FilterGraph);
	printf("Building graph...\n");

	hr = BuildGraph(graph);
	if (hr==S_OK)
	{
		printf("Running");
		CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl(graph);
		hr = mediaControl->Run();

		//CHECK_HR(hr, L"Can't run the graph");

		CComQIPtr<IMediaEvent, &IID_IMediaEvent> mediaEvent(graph);
		BOOL stop = FALSE;
		MSG msg;
		while(!stop)
		{
			long ev=0, p1=0, p2=0;
			Sleep(500);
			printf(".");
			while(PeekMessage(&msg, NULL, 0,0, PM_REMOVE))
			DispatchMessage(&msg);
			while (mediaEvent->GetEvent(&ev, &p1, &p2, 0)==S_OK)
			if (ev == EC_COMPLETE || ev == EC_USERABORT)
			{
				printf("Done!\n");
				stop = TRUE;
			}
			else if (ev == EC_ERRORABORT)
			{
				printf("An error occured: HRESULT=%x\n", p1);
				mediaControl->Stop();
				stop = TRUE;
			}
			mediaEvent->FreeEventParams(ev, p1, p2);
		}
	}
	CoUninitialize();
}