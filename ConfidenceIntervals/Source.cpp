#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

struct Limits
{
	double estimate;
	double UpL;
	double DownL;
};

double Gaussian(double x, double a) //using cumulative distribution
{
	return 0.5 * (1 + erf((x - a) / sqrt(2)));
}

double DownLimit(double estimate, double CL)
{
	double p1 = estimate - 20;
	double p2 = estimate;
	double p3;
	double alpha = (1.0 - CL) / 2;
	while (abs(p1 - p2) > 0.001)
	{
		p3 = (p1 + p2) / 2;
		if ((Gaussian(p3, estimate) - alpha) > 0) p2 = p3;
		else if ((Gaussian(p3, estimate) - alpha) < 0) p1 = p3;
		else return p3;
	}
	return p1;
}

double UpLimit(double estimate, double CL)
{
	double p1 = estimate;
	double p2 = estimate + 20;
	double p3;
	double alpha = 1.0 - (1.0 - CL) / 2;
	while (abs(p1 - p2) > 0.001)
	{
		p3 = (p1 + p2) / 2;
		if ((Gaussian(p3, estimate) - alpha) > 0) p2 = p3;
		else if ((Gaussian(p3, estimate) - alpha) < 0) p1 = p3;
		else return p3;
	}
	return p1;
}

int main()
{
	ofstream fout("fout.txt");
	double CL; //coinfidence level
	CL = 0.9;
	Limits L[101];
	double a = -2.0;
	for (int i = 0; i < 101; ++i, a += 0.04)
	{
		L[i].estimate = a;
		L[i].DownL = DownLimit(a, CL);
		L[i].UpL = UpLimit(a, CL);
		fout << L[i].estimate << ' ' << L[i].DownL << ' ' << L[i].UpL << endl;
	}
} 