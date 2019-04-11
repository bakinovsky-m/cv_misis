#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;

int main()
{
  const uint width = 100;
  const uint height = 100;
  Mat img {width * 2, height * 3, CV_8U, Scalar(0)};

  Mat kv {width, height, CV_8U, Scalar(255)};
  circle(kv, Point(kv.rows/2, kv.cols/2), kv.cols/2, Scalar(0), -1);
  Rect r (0, 0, kv.cols, kv.rows);
  copyTo(kv, img(r), Mat());

  kv = Mat{width, height, CV_8U, Scalar(127)};
  circle(kv, Point(kv.rows/2, kv.cols/2), kv.cols/2, Scalar(255), -1);
  r.y += kv.rows;
  copyTo(kv, img(r), Mat());

  string orig_winname = "OpenCV orig";

  imshow(orig_winname, img);
  waitKey(0);
}
