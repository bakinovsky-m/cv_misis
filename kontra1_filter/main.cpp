#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;

static const uint BLACK = 0;
static const uint GREY = 127;
static const uint WHITE = 255;

const uint width = 100;
const uint height = 100;

void initImage(const Mat img)
{
    Mat kv {width, height, CV_8U, Scalar(BLACK)};
    circle(kv, Point(kv.rows/2, kv.cols/2), kv.cols/2, Scalar(WHITE), -1);
    Rect r (0, 0, kv.cols, kv.rows);
    copyTo(kv, img(r), Mat());

    kv = Mat{width, height, CV_8U, Scalar(GREY)};
    circle(kv, Point(kv.rows/2, kv.cols/2), kv.cols/2, Scalar(BLACK), -1);
    r.x += kv.cols;
    copyTo(kv, img(r), Mat());

    kv = Mat{width, height, CV_8U, Scalar(WHITE)};
    circle(kv, Point(kv.rows/2, kv.cols/2), kv.cols/2, Scalar(GREY), -1);
    r.x += kv.cols;
    copyTo(kv, img(r), Mat());

    kv = Mat{width, height, CV_8U, Scalar(WHITE)};
    circle(kv, Point(kv.rows/2, kv.cols/2), kv.cols/2, Scalar(BLACK), -1);
    r.y += kv.rows;
    r.x = 0;
    copyTo(kv, img(r), Mat());

    kv = Mat{width, height, CV_8U, Scalar(BLACK)};
    circle(kv, Point(kv.rows/2, kv.cols/2), kv.cols/2, Scalar(GREY), -1);
    r.x += kv.cols;
    copyTo(kv, img(r), Mat());

    kv = Mat{width, height, CV_8U, Scalar(GREY)};
    circle(kv, Point(kv.rows/2, kv.cols/2), kv.cols/2, Scalar(WHITE), -1);
    r.x += kv.cols;
    copyTo(kv, img(r), Mat());
}

int main()
{
  Mat img {width * 2, height * 3, CV_8U, Scalar(0)};

  initImage(img);

  string orig_winname = "OpenCV orig";

  imshow(orig_winname, img);
  moveWindow(orig_winname, 100, 100);

//  Mat kernel = Mat::zeros(3,3, CV_8U);
//  kernel.at<uint8_t>(1,1) = 1;
//  Mat kernel = Mat::ones(3,3,CV_8U);

  // box filter
  Mat_<double> kernel(3,3);
  kernel << 1,1,1,1,1,1,1,1,1;
  kernel /= 9;

  Mat dst;
  filter2D(img, dst, CV_8U, kernel);

  imshow("OpenCV filtered", dst);

  waitKey(0);
}
