#include <string>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main()
{
  const string in = "in.jpg";
  const string out = "out.jpg";

  Mat img {imread(in)};
  circle(img, Point{0,0}, 10, Scalar(255, 0, 0));
  imwrite(out, img);
}

