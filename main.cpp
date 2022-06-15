#include <iostream>
#include <vector>
#include "complex"
#include "AudioFile.h"

using std::cout;
using std::endl;
using std::complex;
using std::string;
using std::vector;

int reverse(int num, int lg_n) {
	int res = 0;
	for (int i = 0; i < lg_n; i++) {
		if (num & (1 << i))
			res |= 1 << (lg_n - 1 - i);
	}
	return res;
}

unsigned findNextPowerOf2(unsigned n)
{
	n = n - 1;

	while (n & n - 1) {
		n = n & n - 1;
	}
	return n << 1;
}

struct fourier {

	AudioFile<double> audio;
	vector<complex<double>> audio_data;
	int n = 0, total_data=0;
	float PI = 3.14159;

	fourier() {}

	fourier(string path) {
		audio.load(path);
		/* Despues de cargar el audio, nos aseguramos de que la cantidad de datos
		que devuelva sea potencia de 2, si no lo es llenamos con ceros hasta llegar
		a la potencia de 2 mas cercana.*/
		total_data = audio.getNumSamplesPerChannel();
		int resttopower = findNextPowerOf2(total_data) - total_data;
		audio_data.resize(total_data+resttopower);
		for (int i = 0; i < total_data; i++) 
			audio_data[i]=(complex<double>(audio.samples[0][i],0));

		for (int i = 0; i < resttopower; i++)
			audio_data[total_data+i]=(complex<double>(0, 0));
		
		n = audio_data.size();
	}

	//Dividir y venceras iterativo
	void fft(bool invert) {
		int lg_n = 0;
		while ((1 << lg_n) < n)
			lg_n++;

		for (int i = 0; i < n; i++) {
			if (i < reverse(i, lg_n))
				swap(audio_data[i], audio_data[reverse(i, lg_n)]);
		}

		for (int len = 2; len <= n; len <<= 1) {
			//Identidad de euler
			double ang = 2 * PI / len;
			if (!invert)
				ang = ang * -1;
			complex<double> euler(cos(ang), sin(ang));

			for (int i = 0; i < n; i += len) {
				complex<double> temp(1);
				for (int j = 0; j < len / 2; j++) {
					complex<double> u = audio_data[i + j];
					complex<double> v = audio_data[i + j + len / 2] * temp;
					audio_data[i + j] = u + v;
					audio_data[i + j + len / 2] = u - v;
					temp *= euler;
				}
			}
		}

		if (invert) {
			for (complex<double>& x : audio_data)
				x /= n;
		}
	}

	void save() {
		for (int i = 0; i < total_data; i++) {
			audio.samples[0][i] = audio_data[i].real();
		}
		audio.save("new_audio.wav");
	}
};

int main()
{
	fourier test("C:/Users/sebpost/Documents/Universidad/Buenas_nochesv4.wav");
	cout << test.audio_data[46000]<<endl;
	test.fft(0);
	cout << test.audio_data[46000] << endl;
	test.fft(1);
	cout << test.audio_data[46000] << endl;
	test.save();
}
