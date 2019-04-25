#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int argc, char ** argv)
{
  if(argc != 2)
  {
    cout << "usage: ./lab1 <path_to_image>" << endl;
    cout << "out.jpg will be create in current working directory" << endl;
  }
  const string in = argv[1];
  const string out = "out.jpg";

  Mat img {imread(in)};
  circle(img, Point{0,0}, 10, Scalar(255, 0, 0));
  imwrite(out, img);
}

