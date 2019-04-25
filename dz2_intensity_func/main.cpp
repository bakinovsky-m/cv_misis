#include <iostream>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//using Vi = Vec<int8_t, 1>;

static string win_name = "OpenCV orig";
static string win_name_new = "OpenCV new";
//static Mat img_nnew;

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

void Foo(int tb_value, void* userdata)
{
  Mat img = *reinterpret_cast<Mat*>(userdata);
  Mat img_new;
  img.copyTo(img_new);

  for(auto el = img_new.begin<int8_t>(); el != img_new.end<int8_t>(); ++el)
  {
    int16_t temp = *el;
//    temp += tb_value;
    temp *= ((double)tb_value / 100);

    if(temp > 255)
    {
      temp = 255;
    } else if(temp < 0)
    {
      temp = 0;
    }


    *el += temp;
  }

  imshow(win_name_new, img_new);
  moveWindow(win_name_new, 100, 100);
}

int main(int argc, char ** argv)
{
  if(argc != 2)
  {
    cout << "Usage: ./intensity_func path_to_img" << endl;
    return -1;
  }

  cout << argv[1] << endl;

  Mat source_img = imread(string(argv[1]));
  cout << type2str(source_img.type()) << endl;
  vector<Mat> channels;
  split(source_img, channels);

  Mat img = channels.at(0);
  cout << type2str(img.type()) << endl;
  Mat img_new;
  img.copyTo(img_new);

  auto i = img.begin<uint8_t>();
  cout << *i << endl;
  i++;
  cout << *i << endl;
  i++;
  cout << *i << endl;
  i++;

  namedWindow(win_name);
  imshow(win_name, img);
  int tb_val = 0;
  createTrackbar("", win_name, &tb_val, 100, Foo, &img);
  imshow(win_name_new, img_new);
  moveWindow(win_name_new, 100, 100);
  waitKey(0);
}
