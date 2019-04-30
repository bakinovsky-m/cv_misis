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

//const GL MAX = 255;
const GL MAX = numeric_limits<uint8_t>::max();

static vector<double> PG;
static bool PG_CACHED = false;

double p_g_compute(const Mat img, const GL g){
  double res = 0;
  for(auto it = img.begin<GL>(); it != img.end<GL>(); ++it)
  {
    if(*it == g)
      res++;
  }
  return res/(img.cols * img.rows);
}

double p_g(const Mat img, const GL g)
{
  if(PG_CACHED)
    return PG.at(g);
  else {
    cout << "full compute started" << endl;
    for(GL i = 0; i < MAX; ++i)
    {
      PG.push_back(p_g_compute(img, i));
    }
    PG_CACHED = true;
    cout << "full compute finished" << endl;
  }
  return PG.at(g);
}

double mu0_T(const Mat img, const GL T) {
  double up = 0;
  double bottom = 0;
  for (GL g = 0; g <= T; ++g) {
    up += g * p_g(img, g);
    bottom += p_g(img, g);
  }
  return up/bottom;
}

double mu1_T(const Mat img, const GL T, const GL n) {
  double up = 0;
  double bottom = 0;
  for (GL g = T + 1; g <= n; ++g) {
    up += g * p_g(img, g);
    bottom += p_g(img, g);
  }
  return up/bottom;
}

double E_x(const Mat img, const GL n) {
  double res = 0;
  for(GL g = 0; g <= n; ++g)
  {
    res += g * p_g(img, g);
  }
  return res;
}

double E_y_T(const Mat img, const GL T, const GL n)
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

double E_xy_T(const Mat img, const GL T, const GL n)
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

double E_xx(const Mat img, const GL n)
{
  double res = 0;
  for(GL g = 0; g <= n; ++g)
  {
    res += g*g * p_g(img, g);
  }
  return res;
}

double E_yy(const Mat img, const GL T, const GL n)
{
  vector<double> cache;
  double a = 0;
  for(GL g = 0; g <= T; ++g)
  {
    double mu = mu0_T(img, T);
    double pg = p_g(img, g);
    a += mu*mu * pg;
    cache.push_back(pg);
  }

  double b = 0;
  for(GL g = 0; g <= T; ++g)
  {
    double mu = mu1_T(img, T, n);
    double pg = cache.at(g);
    b += mu*mu * pg;
  }
  return a + b;
}

double V_x(const Mat img, const GL n)
{
  double e = E_x(img, n);
  return E_xx(img, n) - e*e;
}

double V_y_T(const Mat img, const GL T, const GL n)
{
  double e = E_y_T(img, T, n);
  return E_yy(img, T, n) - e*e;
}

double rho(const Mat img, const GL T, const GL n)
{
  double up = E_xy_T(img, T, n) - E_x(img, n) * E_y_T(img, T, n);
  double bottom = pow(V_x(img, n) * V_y_T(img, T, n), 0.5);
  return up/bottom;
}

vector<uint> calcHist(const Mat img)
{
  vector<uint> res (256, 0);
//  uint i = 0;
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

  Mat img = imread(argv[1]);

  if(img.channels() != 1)
  {
    cout << "image have more than one channels, will use the first one" << endl;
    vector<Mat> channels;
    split(img, channels);
    img = channels.at(0);
  }

  vector<uint> hist = calcHist(img);

  uint width = 3;
  uint height = 512;
  Mat r = Mat::zeros(height, 256*width, CV_8U);
  Mat b, g, hist_img;
  b = Mat::zeros(r.rows, r.cols, CV_8U);
  g = Mat::zeros(r.rows, r.cols, CV_8U);
  vector<Mat> channels = {b, g, r};
  merge(channels, hist_img);
  for(uint i = 0; i < MAX; ++i)
  {
    GL c = hist.at(i);
    rectangle(hist_img,
              Point(i*width, height),
              Point(i*width + width, height - c*(height/MAX)),
              Scalar(256, 256, 256),
              FILLED
              );
  }

  p_g(img, 0);

  uint best_T = 0;
  double best_rho = -1;
  // singlethreaded
  auto begin = chrono::steady_clock::now();
  for(GL i = 0; i < MAX; ++i)
  {
    double cur_rho = rho(img, i, MAX-1);
    circle(hist_img, Point(i*width, height - 100*(cur_rho)), 3, Scalar(0, 0, 256), -1);
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

  rectangle(hist_img, Point(best_T * width - width/4, 0), Point(best_T * width + width/4, height), Scalar(0, MAX, 0), -1);
  circle(hist_img, Point(best_T*width, height - 100*(best_rho)), 3, Scalar(0, 0, 256), -1);

  Mat new_img;
  threshold(img, new_img, best_T, 255, THRESH_BINARY);

  imshow("OpenCV orig", img);
  imshow("OpenCV thresh", new_img);
  imshow("OpenCV hist", hist_img);
  waitKey(0);
  imwrite("orig.png", img);
  imwrite("bin.png", new_img);
  imwrite("hist.png", hist_img);
}
