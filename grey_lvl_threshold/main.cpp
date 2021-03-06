#include <opencv2/opencv.hpp>

#include <cmath>
#include <iostream>
#include <vector>
#include <thread>
#include <numeric>
#include <map>
#include <chrono>

using namespace cv;
using namespace std;

using GL = uint8_t;

const GL MAX = numeric_limits<uint8_t>::max();

static vector<double> PG;
static bool PG_CACHED = false;

double p_g_compute(const Mat &img, const GL g){
  double res = 0;
  auto qwe = img.end<GL>();
  for(auto it = img.begin<GL>(); it != qwe; ++it)
  {
    if(*it == g)
      res++;
  }
  return res/img.total();
}

double p_g(const Mat &img, const GL g)
{
  if(PG_CACHED)
    return PG.at(g);
  else {
    cout << "full compute started" << endl;
    for(GL i = 0; i < MAX; ++i)
    {
      cout << int(i) << "/" << int(MAX) << endl;
      PG.push_back(p_g_compute(img, i));
    }
    PG_CACHED = true;
    cout << "full compute finished" << endl;
  }
  return PG.at(g);
}

double mu0_T(const Mat &img, const GL T) {
  double up = 0;
  double bottom = 0;
  for (GL g = 0; g <= T; ++g) {
    auto pg = p_g(img, g);
    up += g * pg;
    bottom += pg;
  }
  return up/bottom;
}

double mu1_T(const Mat &img, const GL T, const GL n) {
  double up = 0;
  double bottom = 0;
  for (GL g = T + 1; g <= n; ++g) {
    auto pg = p_g(img, g);
    up += g * pg;
    bottom += pg;
  }
  return up/bottom;
}

double E_x(const Mat &img, const GL n) {
  double res = 0;
  for(GL g = 0; g <= n; ++g)
  {
    res += g * p_g(img, g);
  }
  return res;
}

double E_y_T(const Mat &img, const GL T, const GL n)
{
  double a = 0;
  for(GL g = 0; g <= T; ++g)
  {
    a += mu0_T(img, T) * p_g(img, g);
  }

  double b = 0;
  for(GL g = T+1; g <= n; ++g)
  {
    b += mu1_T(img, T, n) * p_g(img, g);
  }
  return a + b;
}

double E_xy_T(const Mat &img, const GL T, const GL n)
{
  double a = 0;
  for(GL g = 0; g <= T; ++g)
  {
    a += g * mu0_T(img, T) * p_g(img, g);
  }

  double b = 0;
  for(GL g = T+1; g <= n; ++g)
  {
    b += g * mu1_T(img, T, n) * p_g(img, g);
  }
  return a + b;
}

double E_xx(const Mat &img, const GL n)
{
  double res = 0;
  for(GL g = 0; g <= n; ++g)
  {
    res += g*g * p_g(img, g);
  }
  return res;
}

double E_yy(const Mat &img, const GL T, const GL n)
{
  double a = 0;
  for(GL g = 0; g <= T; ++g)
  {
    double mu = mu0_T(img, T);
    double pg = p_g(img, g);
    a += mu*mu * pg;
  }

  double b = 0;
  for(GL g = T+1; g < n; ++g)
  {
    double mu = mu1_T(img, T, n);
    double pg = p_g(img, g);
    b += mu*mu * pg;
  }
  return a + b;
}

double V_x(const Mat &img, const GL n)
{
  double e = E_x(img, n);
  return E_xx(img, n) - e*e;
}

double V_y_T(const Mat &img, const GL T, const GL n)
{
  double e = E_y_T(img, T, n);
  return E_yy(img, T, n) - e*e;
}

double rho(const Mat &img, const GL T, const GL n)
{
  double up = E_xy_T(img, T, n) - E_x(img, n) * E_y_T(img, T, n);
  double pow_arg = V_x(img, n) * V_y_T(img, T, n);
  double bottom = pow(pow_arg, 0.5);
  return up/bottom;
}

vector<uint> myCalcHist(const Mat &img)
{
  vector<uint> res (256, 0);
  for(auto it = img.begin<GL>(); it != img.end<GL>(); ++it)
  {
    res[*it]++;
  }
  return res;
}

int main(int argc, char ** argv){
  if(argc != 2)
  {
    cout << "usage: ./grey_lvl_threshold <path_to_image>" << endl;
    return -1;
  }
  string filename = argv[1];

  Mat img = imread(filename);

  if(img.channels() != 1)
  {
    cout << "image have more than one channels, will use the first one" << endl;
    vector<Mat> channels;
    split(img, channels);
    img = channels.at(0);
  }

//  vector<uint> hist = myCalcHist(img);

  uint width = 3;
  uint height = 512;
  Mat r = Mat::zeros(height, 256*width, CV_8U);
  Mat b, g, hist_img;
  b = Mat::zeros(r.rows, r.cols, CV_8U);
  g = Mat::zeros(r.rows, r.cols, CV_8U);
  vector<Mat> channels = {b, g, r};
  merge(channels, hist_img);


  Mat histt;
  const int chans = img.channels();
  int histSize = 256;
  float range[] = { 0, 256 }; //the upper boundary is exclusive
  const float* histRange = { range };
  calcHist(&img, 1, nullptr, Mat(), histt, 1, &histSize, &histRange);

  uint hist_h = 512;
  uint hist_w = 1024;
  uint bin_w = 4;
  Mat histImage(hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
  normalize(histt, histt, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  for( int i = 1; i < histSize; i++ )
  {
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(histt.at<float>(i-1)) ),
            Point( bin_w*(i), hist_h - cvRound(histt.at<float>(i)) ),
            Scalar( 255, 255, 255), 2, 8, 0  );
  }

  p_g(img, 0);

  uint best_T = 0;
  double best_rho = -1;
  // singlethreaded
  auto begin = chrono::steady_clock::now();
  for(GL i = 0; i < MAX; ++i)
  {
    double cur_rho = rho(img, i, MAX-1);
//    cout << cur_rho << endl;
//    circle(hist_img, Point(i*width, height - 100*(cur_rho)), 3, Scalar(0, 0, 256), -1);
    circle(histImage, Point(i*bin_w, hist_h - 100*(cur_rho)), 3, Scalar(0, 0, 256), -1);
    if (cur_rho > best_rho)
    {
      best_rho = cur_rho;
      best_T = i;
    }
  }
  auto end = chrono::steady_clock::now();

  cout << "done in time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;

  cout << "best rho: " << best_rho << endl;
  cout << "best T: " << best_T << endl;

  rectangle(histImage, Point(best_T * bin_w - bin_w/4, 0), Point(best_T * bin_w + bin_w/4, hist_h), Scalar(0, MAX, 0), -1);
  circle(histImage, Point(best_T*bin_w, hist_h - 100*(best_rho)), 3, Scalar(0, 0, 256), -1);

  Mat new_img;
  threshold(img, new_img, best_T, 255, THRESH_BINARY);

  imshow("OpenCV orig", img);
  imshow("OpenCV thresh", new_img);
//  imshow("OpenCV hist1", hist_img);
  imshow("OpenCV hist", histImage);
  waitKey(0);
  imwrite(filename + "_orig.png", img);
  imwrite(filename + "_bin.png", new_img);
  imwrite(filename + "_hist.png", histImage);
}
