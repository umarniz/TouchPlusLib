#include <cstdio>
#include "DSVideoCapture.h"
#include "TouchPlus.h"
 
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\opencv.hpp"

using namespace cv;

int main()
{
	if (!UnlockSoftware())
		printf("Failed to unlock\n");

    DSVideoCapture* vc        = new DSVideoCapture();
	vc->Start();

	int height = vc->GetSampleHeight();
	int width = vc->GetSampleWidth();
	int channels = vc->GetSampleChannels();
	Mat frame = Mat(height, width, CV_8UC3);
	
	assert(frame.isContinuous());

	Mat frameL, frameR;

	while (1)
	{
		cvWaitKey(10);
		vc->GrabFrame((long*)frame.data);

		frameL = Mat(frame, Rect(0,0,width/2,height));
		frameR = Mat(frame, Rect(width/2,0,width/2,height));

		imshow( "FrameL", frameL );
		imshow( "FrameR", frameR );
	}

	vc->Finish();
	lockDevice(0);

    return 0;
}