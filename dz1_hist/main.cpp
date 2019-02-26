#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <exception>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;

constexpr int BASE = 256;
constexpr int HIST_WIDTH = 500;
constexpr int HIST_HEIGHT = 300;


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

void drawHistOnMat(const vector<Mat> & chans, Mat &dest)
{
  uint8_t ind = 0;
  for(auto ch : chans)
  {
    vector<int> bchan(BASE, 0);
    auto it = ch.begin<uint8_t>();
    auto end = ch.end<uint8_t>();
    for(; it != end; ++it)
    {
      bchan[*it] += 1;
    }

    int max_el = *max_element(bchan.begin(), bchan.end());

    array<Point, 256> pts;

    for(size_t i = 0; i < bchan.size(); ++ i)
    {
      int p = bchan[i];

      int x = static_cast<int>(((HIST_WIDTH/static_cast<double>(BASE)) * i) + HIST_WIDTH/static_cast<double>(bchan.size()) - 2);
      int y = static_cast<int>(HIST_HEIGHT - (static_cast<double>(p)/max_el)*HIST_HEIGHT);
      pts[i] = Point(x, y);
    }

    Vec3i color = {0,0,0};
    color[ind] = 255;
    Scalar sc = Scalar(color);

    polylines(dest, pts, false, sc, 2);
    ++ind;
  }
}

int main(int argc, char ** argv)
{
  if(argc != 2)
  {
    cout << "Usage: ./hist path_to_img" << endl;
    return -1;
  }

  cout << argv[1] << endl;

  Mat img = imread(string(argv[1]));

  cout << "type: " << type2str( img.type() ) << endl;
  cout << "channels: " << img.channels() << endl;

  vector<Mat> chans (static_cast<size_t>(img.channels()));
  split(img, chans);

  imshow("OpenCV orig", img);
  moveWindow("OpenCV orig", 10, 100);

  Mat hist (HIST_HEIGHT, HIST_WIDTH, img.type());
  drawHistOnMat(chans, hist);
  imshow("OpenCV hist", hist);
  waitKey();
}
