#include "opencv2/opencv.hpp"
#include <iostream>

#include "keyboard_control.h"

using namespace std;
using namespace cv;

//ESC is exit key
const int EXIT_KEY = 27;
//
const int EROSION_SIZE = 5;
//minimal and maximal
//threshold value of red color
const int MIN_COLOR_VALUE = 0;
const int MAX_COLOR_VALUE = 15;
//minimal area of controller
const int MIN_AREA = 1000;
//minimal angle
const int ANGLE_DELTA = 10;


//Draw rotated rectangle - copy&pasted from the WEB:
//http://code.google.com/p/opencv-feature-tracker/source/browse/src/draw.cpp
//(OpenCV doesn't have default function of drawing it)
void drawRotatedRect(Mat & im, const RotatedRect & rot_rect, CvScalar color)
{
    CvPoint2D32f box_vtx[4];
    cvBoxPoints(rot_rect, box_vtx);

    // copied shamelessly from minarea.c
    // it initialize to the last point, then connect to point 0, point 1, point 2 pair-wise
    CvPoint pt0, pt;
    pt0.x = cvRound(box_vtx[3].x);
    pt0.y = cvRound(box_vtx[3].y);
    for(int i = 0; i < 4; i++ )
    {
        pt.x = cvRound(box_vtx[i].x);
        pt.y = cvRound(box_vtx[i].y);
        line(im, pt0, pt, color, 1, CV_AA, 0);
        pt0 = pt;
    }
}


//Function finds in the frame
//red rectangle (controller)
//and returns its rotated angle
int getControllerAngle(Mat &frame)
{
    //some values initialization
    Mat hsv,
        result,
        element = getStructuringElement(MORPH_ELLIPSE,
                                        Size(2*EROSION_SIZE+1, 2*EROSION_SIZE+1),
                                        Point(EROSION_SIZE, EROSION_SIZE));
    vector<vector<Point2i> > contours;

    int maxSquare,
        maxIdx;

    //convert image to HSV
    cvtColor(frame, hsv, CV_BGR2HSV);

    //binarize image (thresholding)
    //get red pixels
    inRange(hsv, Scalar(MIN_COLOR_VALUE, 150, 150), Scalar(MAX_COLOR_VALUE, 255, 255), result);

    //remove noise
    erode(result, result, element);
    dilate(result, result, element);

    //find all contours
    findContours(result, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    //in this block find contour with the biggest area
    if (contours.size() > 0)
    {
        maxSquare = 0;
        //cycle through all contours and find the biggest contour
        for (size_t i=0; i<contours.size(); i++)
        {
            //TODO
            size_t area = contourArea(contours[i]);
            if (area > maxSquare)
            {
                maxSquare = area;
                maxIdx = i;
            }
        }

        //if contour's area is bigger than some value
        if (maxSquare > MIN_AREA)
        {
            //find bounding rectangle
            RotatedRect rect = minAreaRect(contours[maxIdx]);

            //draw it on the frame
            drawRotatedRect(frame, rect, cvScalar(0,255,0));

            //return angle
            return abs(rect.angle);
        }
    }

    return -1;
}


//check if controller angle is left or right
inline bool checkAngleLeft(int angle)
{
    return angle > 45 && angle < 90 - ANGLE_DELTA;
}

inline bool checkAngleRight(int angle)
{
    return angle < 45 && angle > ANGLE_DELTA;
}


int main(int, char**)
{
    if (initDisplay())
    {
        //Open the default camera
        VideoCapture cap(0);

        //Check if we succeeded
        if(!cap.isOpened())
        {
            return -1;
        }

        int angle;
        Mat frame;

        for(;;)
        {
            //get a new frame from camera
            cap >> frame;

            //get curAngle of controller
            angle = getControllerAngle(frame);

            if (angle >= 0)
            {
                //this means that there's controller in the frame

                //so let's go!
                pressUpKey();

                if (checkAngleLeft(angle))
                {
                    pressLeftKey();
                }
                else
                if (checkAngleRight(angle))
                {
                    pressRightKey();
                }
                else
                {
                    //controller is not rotated
                    releaseLeftKey();
                    releaseRightKey();
                }
            }
            else
            {
                releaseAllKeysKey();
            }

            imshow("Window", frame);

            if (waitKey(1) == EXIT_KEY)
            {
                break;
            }
        }

        closeDisplay();

        //The camera will be deinitialized automatically in VideoCapture destructor
        return 0;
    }
    else
    {
        return -1;
    }

}
