#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <iostream>
#include "FFT.h"

using namespace std;

class Spectrogram
{
public:
	Spectrogram(CooleyTukeyFFT fft);

	void generateSignalWindow();
	void spectrum(float const& maxSpectrumBars);
	void scrollingSpectrum(float const& maxSpectrumDistance);
	void scrollingHorizontalSpectrum(float const& maxSpectrumDistance);
	void ToggleDisp();
	void update();
	void draw(sf::RenderWindow& window);

private:
	CooleyTukeyFFT fft;
	sf::SoundBuffer buf;
	sf::Sound sounds;
	
	int sampleRate;
	int sampleCount;
	int FFTSize;
	int currSample;

	char doubleSpectrum = 1;
	SampleArray frequencyBins;

	int VertexArray_x = -200;
	int VertexArray_y = 901;

	//affects how the spectrum and scrolling_spectrum are packed into the sfml window
	float x_max = 1200;
	float y_max = 1900;

	vector<ComplexVal> sample;
	vector<float> window;

	sf::VertexArray SpectrumView;
	sf::VertexArray ScrollingView;
	sf::VertexArray horiScrollingView;

	std::vector<sf::Vertex> scroll;
	std::vector<sf::Vertex> scrollHorizontal;

};
//
// external function

unsigned findNextPowerOf2(unsigned n)
{
	n = n - 1;

	while (n & n - 1) {
		n = n & n - 1;
	}
	return n << 1;
}

int powerof2(int n)
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
// by .Cpp 

Spectrogram::Spectrogram(CooleyTukeyFFT sound)
{
	this->fft = sound;
	std::string* path = fft.getPathOfSource();
	if (!buf.loadFromFile(*path))
		cout << "no se cargo el archivo con este path: " << path << std::endl;

	sounds.setBuffer(buf);
	sounds.setLoop(false);
	sounds.play();

	sampleRate = buf.getSampleRate() * buf.getChannelCount();
	sampleCount = buf.getSampleCount();
	//sampleCount = powerof2(sampleCount);
	//sampleCount = findNextPowerOf2(sampleCount);
	if (fft.getFFTSize() < sampleCount) FFTSize = fft.getFFTSize();
	else FFTSize = sampleCount;

	//FFTSize = powerof2(sampleCount);
	cout << "NORMAL: " << sampleCount << "," << FFTSize << "P2:" << findNextPowerOf2(sampleCount) << "," << findNextPowerOf2(FFTSize) << endl;

	currSample = 0;

	for(int i = 0; i < FFTSize; i++)
	{
		window.push_back(0.25 + - 0.05*cos(2 * PI*i / (float)FFTSize)); //define the signal windowing function
	}

	//sample.resize(findNextPowerOf2(FFTSize));
	sample.resize(FFTSize);

}

void Spectrogram::generateSignalWindow()
{
	currSample = sounds.getPlayingOffset().asSeconds() * sampleRate;
	if (currSample + FFTSize < sampleCount)
	{
		for (int i = currSample; i < FFTSize + currSample; i++)
		{
			//get the ith sample from the sound buffer and place it in the array to draw
			sample[i - currSample] = ComplexVal(buf.getSamples()[i] * window[i - currSample], 0);
		}
	}
}

void Spectrogram::update()
{
	generateSignalWindow();

	SpectrumView.clear();
	ScrollingView.clear();
	horiScrollingView.clear();

	frequencyBins = SampleArray(sample.data(), FFTSize);
	this->fft.FFT(frequencyBins);
	//float max = 80000000; //visualization sensitivity (height) larger number is shorter spectrum
	float max = 5000000; //visualization sensitivity (height) larger number is shorter spectrum
	
	spectrum(max);
	
						  
	scrollingSpectrum(max);
	if (doubleSpectrum)	scrollingHorizontalSpectrum(max);
	

}

void Spectrogram::spectrum(float const& maxSpectrumBars)
{
	SpectrumView.setPrimitiveType(sf::Lines); //reset draw type from clear
	sf::Vector2f position(VertexArray_x, VertexArray_y); //location to draw so it looks like its coming off the bottom left corner
	sf::Vector2f positionVert(0, -50);

	for (float i = 1; i < FFTSize; i *= 1.001) //increment by small distance to draw detailed frequency bands - multiplying looks better than adding
	{
		sf::Vector2f binnedSample(log2f(i) / log2f(FFTSize), abs(frequencyBins[(int)i]));
		sf::Vector2f outline(binnedSample.x * x_max, -binnedSample.y / maxSpectrumBars * y_max); //-y because it will be added to the position vector
		SpectrumView.append(sf::Vertex(position + outline, sf::Color(71, 222, 44, 20)));

		sf::Vector2f fill(binnedSample.x * x_max, 0);
		SpectrumView.append(sf::Vertex(position + fill, sf::Color(71, 222, 44, 20)));
		if (doubleSpectrum)
		{
			float temp = binnedSample.x;
			binnedSample.x = binnedSample.y;
			binnedSample.y = temp;

			sf::Vector2f outlineVertz(binnedSample.x / maxSpectrumBars * (x_max / 8), binnedSample.y * (y_max / 8));
			SpectrumView.append(sf::Vertex(positionVert + outlineVertz, sf::Color(71, 222, 44, 10)));

			sf::Vector2f fillVert(0, binnedSample.y * (y_max / 8));
			SpectrumView.append(sf::Vertex(positionVert + fillVert, sf::Color(71, 222, 44, 10)));
		}
	}
	
}

void Spectrogram::scrollingSpectrum(float const& maxSpectrumDistance)
{
	ScrollingView.setPrimitiveType(sf::LineStrip);
	sf::Vector2f position(VertexArray_x, VertexArray_y);

	sf::Vector2f binnedSample;
	float alphaDecay = .0001;
	double fade_distance = 5e6;
	long long size = scroll.size() - fade_distance;

	for (float i = (size > 0 ? size : 0); i < scroll.size(); i++)
	{
		if (doubleSpectrum) scroll[i].position += sf::Vector2f(.95, -1); //push the next frame of the spectrum back at the given vector
		else scroll[i].position += sf::Vector2f(0, -1);
		bool test = !doubleSpectrum;
		if (scroll[i].color.a != 0 && scroll[i].position.y <= 250 && doubleSpectrum) //if the alpha != 0 and the position doesn't interfere with the horizontal spectrum
			scroll[i].color = sf::Color(255, 255, 255, scroll[i].color.a - alphaDecay);

	}

	for (float i = 1; i < FFTSize; i *= 1.03) //same purpose as in spectrum, this loop consumes much more resources so larger increment value
	{
		binnedSample = sf::Vector2f(log2(i) / log2(FFTSize), abs(frequencyBins[(int)i]));
		scroll.push_back(sf::Vertex(position + sf::Vector2f(binnedSample.x * x_max, -binnedSample.y / maxSpectrumDistance * y_max), sf::Color::Color(255, 255, 255, 10)));
	}
	ScrollingView.clear();
	for (int i = (size > 0 ? size : 0); i < scroll.size(); i++)
	{
		ScrollingView.append(scroll[i]);
	}
}

void Spectrogram::scrollingHorizontalSpectrum(float const& maxSpectrumDistance)
{
	horiScrollingView.setPrimitiveType(sf::LineStrip);
	sf::Vector2f positionHori(0, -73);
	sf::Vector2f binnedSampleHori;

	float alphaDecay = .0001;
	double fade_distance = 5e6;
	long long size = scrollHorizontal.size() - fade_distance;

	for (float i = (size > 0 ? size : 0); i < scrollHorizontal.size(); i++)
	{
		scrollHorizontal[i].position += sf::Vector2f(1, 0);

		if (scrollHorizontal[i].color.a != 0 && scrollHorizontal[i].position.x >= x_max && doubleSpectrum)
			scrollHorizontal[i].color = sf::Color(255, 255, 255, scrollHorizontal[i].color.a - alphaDecay);
	}

	for (float i = 1; i < FFTSize; i *= 1.03) //same purpose as in spectrum, this loop consumes much more resources so larger increment value
	{
		binnedSampleHori = sf::Vector2f(abs(frequencyBins[(int)i]), log2(i) / log2(FFTSize));
		scrollHorizontal.push_back(sf::Vertex(positionHori + sf::Vector2f(binnedSampleHori.x / (maxSpectrumDistance) * (x_max / 6), binnedSampleHori.y * (y_max / 6)), sf::Color::Color(255, 255, 255, 10)));
		//scrollHorizontal.push_back(sf::Vertex(positionVert + sf::Vector2f(binnedSample.x / (maxSpectrumDistance/8) * x_max, (binnedSample.y * y_max)), sf::Color::Color(255, 255, 255, 10)));
	}
	horiScrollingView.clear();
	for (int i = (size > 0 ? size : 0); i < scrollHorizontal.size(); i++)
	{
		horiScrollingView.append(scrollHorizontal[i]);
	}
}

void Spectrogram::ToggleDisp()
{
	doubleSpectrum ^= 1;
}



void Spectrogram::draw(sf::RenderWindow& window)
{
	window.draw(SpectrumView);
	window.draw(ScrollingView);
	window.draw(horiScrollingView);
}
