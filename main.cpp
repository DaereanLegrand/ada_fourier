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

bool print = false;

void fft(vector<cd>& a, bool invert) {
    int n = a.size();
    if (n == 1)
        return;

    vector<cd> a0(n / 2), a1(n / 2);
    for (int i = 0; i < n/2; i++) {
        if (a1.size() == 202 or print==false) {
            print = true;
            cout << "a1.size: " << a1.size() << endl;
            cout << 2 * i + 1 << endl;
            cout << i << endl;
        }
        a0[i] = a[2 * i];
        a1[i] = a[2 * i + 1];
    }
    fft(a0, invert);
    fft(a1, invert);

    double ang = 2 * PI / n * (invert ? -1 : 1);
    cd w(1), wn(cos(ang), sin(ang));
    for (int i = 0; 2 * i < n; i++) {
        cout << i << endl;
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

    vector<cd> x(numSamples);

    for (int i = 0; i < numSamples; i++)
    {
        x[i] = audioFile.samples[channel][i];
    }

    cout << x.size();
    fft(x, false);
}
