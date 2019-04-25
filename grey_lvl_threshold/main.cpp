#include <opencv2/opencv.hpp>

#include <cmath>
#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <algorithm>

using namespace cv;
using namespace std;

using GL = uint8_t;

const GL MAX = 255;

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

int main(int argc, char ** argv){
  if(argc != 2)
    return -1;

  Mat img = imread(argv[1]);
  cout << img.type() << endl;

  p_g(img, 0);

  int best_T = 0;
  double best_rho = -1;
  for(GL i = 0; i < MAX; ++i)
  {
    static int counter = 0;
    double r = rho(img, i, MAX-1);
    cout << counter++ << " " << r << endl;
    if (r > best_rho)
    {
      best_rho = r;
      best_T = i;
    }
  }

  cout << "best rho: " << best_rho << endl;
  cout << "best T: " << best_T << endl;

  Mat new_img;
  threshold(img, new_img, best_T, 255, THRESH_BINARY);

  imshow("OpenCV orig", img);
  imshow("OpenCV thresh", new_img);
  waitKey(0);
}
