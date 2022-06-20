#include <iostream>
#include <vector>
#include "complex"
#include "AudioFile.h"

using std::cout;
using std::endl;
using std::complex;
using std::string;
using std::vector;

/*Para encontrar la siguiente potencia de 2*/
unsigned findNextPowerOf2(unsigned n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;
	return n;
}

struct fourier {

	//nuestra libreria de audio
	AudioFile<double> audio;
	//el vector donde guardamos toda la data de nuestro audio
	vector<complex<double>> audio_data;
	int n = 0, total_data=0, bits_n=0;
	const float PI = 3.14159265359;

	double max = 0;

	fourier() {}

	fourier(string path) {
		audio.load(path);
		/* Despues de cargar el audio, nos aseguramos de que la cantidad de datos
		que devuelva sea potencia de 2. Si no lo es, llenamos con ceros hasta llegar
		a la potencia de 2 mas cercana.*/
		total_data = audio.getNumSamplesPerChannel();
		int resttopower = findNextPowerOf2(total_data) - total_data;
		audio_data.resize(total_data+resttopower);
		for (int i = 0; i < total_data; i++) 
			audio_data[i] = (complex<double>(audio.samples[0][i], 0));

		//llena de ceros
		for (int i = 0; i < resttopower; i++)
			audio_data[total_data+i]=(complex<double>(0, 0));
		
		n = audio_data.size();

		//saca el numero de bits de n
		while ((1 << bits_n) < n)
			bits_n++;
	}

	/*para usar el diagrama de la mariposa,
	Revertimos los bits, por ejemplo: 100 -> 001 , 011 -> 110*/
	int bit_reversal(int num) {
		int reversal = 0;
		for (int i = 0; i < bits_n; i++) {
			if (num & (1 << i))
				reversal |= 1 << (bits_n - 1 - i);
		}
		return reversal;
	}

	//Dividir y venceras iterativo (DIT FFT)
	void fft(bool invert) {

		/*Aplicamos bit_reversal en nuestra data*/
		int reversal = 0;
		for (int i = 0; i < n; i++) {
			reversal = bit_reversal(i);
			if (i < reversal)
				swap(audio_data[i], audio_data[reversal]);
		}

		for (int len = 2; len <= n; len <<= 1) {
			//Identidad de euler
			double ang = 2 * PI / len;
			if (!invert)
				ang = ang * -1;
			complex<double> euler_w(cos(ang), sin(ang));

			//Diagrama de la mariposa
			for (int i = 0; i < n; i += len) {
				complex<double> temp(1);
				for (int j = 0; j < len / 2; j++) {
					//y0=x0+x1*(raiz_de_unidad); y1=x0-x1*(raiz_de_unidad);
					complex<double> x0 = audio_data[i + j];
					complex<double> x1 = audio_data[i + j + len / 2] * temp;
					audio_data[i + j] = x0 + x1;
					audio_data[i + j + len / 2] = x0 - x1;
					temp *= euler_w;
				}
			}
		}

		if (invert) {
			for (complex<double>& x : audio_data)
				x /= n;
		}
	}



	void filter_reduction(float porcentage) {
		double magnitude;
		for (int i = 0; i < audio_data.size(); i++) {
			magnitude = abs(audio_data[i].real() + audio_data[i].imag());
			if (max < magnitude)
				max = magnitude;
		}

		double max_filter = max * porcentage;

		for (int i = 0; i < audio_data.size(); i++) {
			magnitude = abs(audio_data[i].real() + audio_data[i].imag());
			if (max_filter < magnitude) {
				audio_data[i] = complex<double>(0, 0);
			}
		}
	}

	//guarda el archivo modificado de data
	void save() {
		for (int i = 0; i < total_data; i++) {
			audio.samples[0][i] = audio_data[i].real();
			audio.samples[1][i] = audio_data[i].real();
		}
		audio.save("new_audio.wav");
	}
};

int main()
{
	fourier test("C:/Users/sebpost/Documents/Universidad/Buenas_nochesv4.wav");
	cout << test.audio_data[66000]<<endl;
	test.fft(0);
	cout << test.audio_data[66000] << endl;
	test.filter_reduction(0.3);
	test.fft(1);
	cout << test.audio_data[66000] << endl;
	test.save();
	cout << test.max << endl;
}
