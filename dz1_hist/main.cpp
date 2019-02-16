#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <exception>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;

constexpr int BASE = 8;
constexpr int HIST_WIDTH = 500;
constexpr int HIST_HEIGHT = 250;


string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

Mat getHistMat(const Mat & chan, Scalar sc = Scalar(255, 255, 255))
{
  vector<int> bchan(BASE, 0);
  auto it = chan.begin<int>();
  auto end = chan.end<int>();
  for(; it != end; ++it)
  {
    int ind = (*it) % BASE;
    if (ind != -1 && ind >= 0)
    {
      bchan[ind] += 1;
    }
  }


  Mat hist (HIST_HEIGHT, HIST_WIDTH, CV_8UC3, Scalar(255, 255, 255));


  int size = bchan.size();
  int max_el = *max_element(bchan.begin(), bchan.end());

  for(int i = 0; i < size; ++ i)
  {
    int p = bchan[i];

    double x1 = (HIST_WIDTH/(double)BASE) * i + 1;
    double y1 = HIST_HEIGHT;
    Point p1 = Point(x1, y1);

    double x2 = ((HIST_WIDTH/(double)BASE) * i) + HIST_WIDTH/(double)size - 2;
    double y2 = HIST_HEIGHT - ((double)p/max_el)*HIST_WIDTH;
    Point p2 = Point(x2, y2);
    rectangle(hist, p1, p2, sc, FILLED);
  }

  return hist;
}

int main(int argc, char ** argv)
{
  if(argc != 2)
  {
    cout << "Usage: ./hist path_to_img" << endl;
    return -1;
  }

  Mat img = imread(string(argv[1]));

  cout << "type: " << type2str( img.type() ) << endl;
  cout << "channels: " << img.channels() << endl;

  vector<Mat> chans;
  split(img, chans);

  imshow("OpenCV1", img);
  moveWindow("OpenCV1", 0, 100);
  imshow("OpenCV hist red", getHistMat(chans[0], Scalar(0, 0, 255)));
  imshow("OpenCV hist green", getHistMat(chans[1], Scalar(0, 255, 0)));
  moveWindow("OpenCV hist green", 1100, 100);
  imshow("OpenCV hist blue", getHistMat(chans[2], Scalar(255, 0, 0)));
  moveWindow("OpenCV hist blue", 500, 100);
  waitKey();
}
