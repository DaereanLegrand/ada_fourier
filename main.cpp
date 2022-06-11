#include <iostream>
#include <complex>
#include <vector>
#include "AudioFile.h"

using std::cout;
using std::endl;
using std::complex;
using std::vector;
using cd = complex<double>;

const double PI = acos(-1);

unsigned findNextPowerOf2(unsigned n)
{
    n = n - 1;
    while (n & n - 1) {
        n = n & n - 1;        
    }
    return n << 1;
}

void fft(vector<cd>& a, bool invert) {
    int n = a.size();
    if (n == 1)
        return;

    vector<cd> a0(n / 2), a1(n / 2);
    for (int i = 0; i < n/2; i++) {

        a0[i] = a[2 * i];
        a1[i] = a[2 * i + 1];
    }
    fft(a0, invert);
    fft(a1, invert);

    double ang = 2 * PI / n * (invert ? -1 : 1);
    cd w(1), wn(cos(ang), sin(ang));
    for (int i = 0; 2 * i < n; i++) {
        a[i] = a0[i] + w * a1[i];
        a[i + n / 2] = a0[i] - w * a1[i];
        if (invert) {
            a[i] /= 2;
            a[i + n / 2] /= 2;
        }
        w *= wn;
    }
}

int main()
{
    AudioFile<double> audioFile;
    audioFile.load("C:/Users/sebpost/Documents/Universidad/hola.wav");

    int channel = 0;
    int numSamples = audioFile.getNumSamplesPerChannel();

    int numSamples2 = findNextPowerOf2(numSamples);

    vector<cd> x(numSamples2);

    for (int i = 0; i < numSamples; i++)
    {
        if (i < numSamples)
            x[i] = audioFile.samples[channel][i];
        else
            x[i] = 0;
    }

    cout << x.size();
    fft(x, false);
}

