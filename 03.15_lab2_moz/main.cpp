#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;

int main(int argc, char ** argv)
{
  if(argc != 2)
  {
    cout << "Usage: ./moz path_to_img" << endl;
    return -1;
  }

  Mat img = imread(string(argv[1]));
  if(img.empty())
    return -1;

  vector<Mat> chans;
  split(img, chans);

  array<Mat, 3> out;

  for(auto & m : out)
  {
    m = Mat(chans[0].rows * 2, chans[0].cols * 2, chans[0].type());
  }

  {
    Rect r (0,0, chans[0].cols, chans[0].rows);
    copyTo(chans[0], out[0](r), Mat());
    copyTo(chans[1], out[1](r), Mat());
    copyTo(chans[2], out[2](r), Mat());

    r.x = r.x + chans[0].cols;
    copyTo(chans[0], out[0](r), Mat());

    r.y = r.y + chans[1].rows;
    copyTo(chans[1], out[1](r), Mat());

    r.x -= chans[0].cols;
    copyTo(chans[2], out[2](r), Mat());
  }

  Mat outImg;
  merge(out, outImg);
  imshow("OpenCV", outImg);
  moveWindow("OpenCV", 10, 10);
  waitKey(0);
}
