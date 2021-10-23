#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <vector>
#include <sstream>

using namespace std;

typedef unsigned short HeatmapType;

class Complex {
    public:
        Complex(double r = 0.0, double i = 0.0)
            : _r(r), _i(i) {}

        Complex(const Complex&) = default;    // Use the data in Complex as default constructor

        double r() const { return _r;}
        double i() const {return _i;}

        Complex operator*(const Complex& other) {
            // (a + bi) (c + di)
            return Complex(_r * other._r - _i * other._i, _r * other._i + _i * other._r);    // This is how to multiply complex numbers Martin 2021
        }

        Complex operator+(const Complex& other) {
            return Complex(_r + other._r, _i + other._i);
        }

        double sqmagnitude() const {
            return _r * _r + _i * _i;
        }

    private:
        double _r, _i;

};

//
//    Utility 
//

void AllocHeatmap(HeatmapType**& o_heatmap, int width, int height){

    // Allocate a heatmap of the size of out image
    o_heatmap = new HeatmapType*[height];
    for (int i = 0; i < height; ++i) {
        o_heatmap[i] = new HeatmapType[width];
        for (int j = 0; j < width; ++j) {
            o_heatmap[i][j] = 0;
        }
    }
}

void FreeHeatmap(HeatmapType**& o_heatmap, int height) {    // This is freeing up memory and makes the program run more smooth
    for (int i = 0; i < height; ++i){
        delete[] o_heatmap[i];
        o_heatmap[i] = nullptr;
    }
    delete o_heatmap;
    o_heatmap = nullptr;
}

vector<Complex> buddhabrotPoints(const Complex& c, int nIterations) {
    int n = 0;
    Complex z;

    // This is how vector works
    // [_] // size: 1, used: 0
    // [1] // size: 1, used: 1
    // [1, 2]  // size: 2, used: 2      RESIZE HAPPENED
    // [1, 2, 3, _] // size: 3, used: 4      RESIZE HAPPENED
    // [1, 2, 3, 4] // size: 4, used: 4

    vector<Complex> toReturn;
    // nIterations: 10
    // [_]
    // [_, _, _,]
    // [_, _, _, _]
    // Resize happens at 10 positions in array since we iterate 10 times
    toReturn.reserve(nIterations);

    while (n < nIterations && z.sqmagnitude() <= 2.0) {
        z = z * z + c;
        ++n;

        toReturn.push_back(z);
    }

    // If point remains bounded through nIterations iterations, the point
    // is bounded, therefore in the Mandelbrot set, therefore of no interest to us
    if (n == nIterations) {
        return vector<Complex>();
    }
    else {
        return toReturn;
    }
}

int rowFromReal(double real, double minR, double maxR, int imageHeight) {
    // [minR, maxR]
    // [0, maxR - minR] // substract minR from n
    // [0, imageHeight] // multipy by (imageHeight / (maxR - minR))
    return static_cast<int>((real - minR) * (imageHeight / (maxR - minR)));

}

int colFromImaginary(double imag, double minI, double maxI, int imageWidth) {
    return static_cast<int>((imag - minI) * (imageWidth / (maxI - minI)));
}

void GenerateHeatmap(HeatmapType** o_heatmap, int imageWidth, int imageHeight, const Complex& minimum, const Complex& maximum, int nIterations, int nSamples, HeatmapType& o_maxHeatmapValue, string consoleMessagePrefix) {

    mt19937 rng;
    uniform_real_distribution<double> realDistribution(minimum.r(), maximum.r());
    uniform_real_distribution<double> imagDistribution(minimum.i(), maximum.i());

    rng.seed(chrono::high_resolution_clock::now().time_since_epoch().count());    // Start a timer

    auto next = chrono::high_resolution_clock::now() + chrono::seconds(5);


    //    Collect nSamples samples... (sample is just a random number c)
    for (int sampleIdx = 0; sampleIdx < nSamples; ++sampleIdx) {
    //    Each sample, get the list of points as the function escapes to infinity (if it does at all)

    	if (chrono::high_resolution_clock::now() > next) {
			next = chrono::high_resolution_clock::now() + chrono::seconds(30);
			cout << consoleMessagePrefix << "Samples Taken: " << sampleIdx << "/" << nSamples << endl;
		}
        
        Complex sample(realDistribution(rng), imagDistribution(rng));
       // vector<Complex> pointList = mandelbrot
       vector<Complex> pointsList = buddhabrotPoints(sample, nIterations);

       for (Complex& point : pointsList) {
           
            if (point.r() <= maximum.r() && point.r() >=minimum.r()
                && point.i() <= maximum.i() && point.i() >= minimum.i()) {
                    int row = rowFromReal(point.r(), minimum.r(), maximum.r(), imageHeight);
                    int col = colFromImaginary(point.i(), minimum.i(), maximum.i(), imageWidth);
                    ++o_heatmap[row][col];

                    if (o_heatmap[row][col] > o_maxHeatmapValue) {
                        o_maxHeatmapValue = o_heatmap[row][col];
                    }
            }

       }

    }
    // Map points to the heatmap
}

int colorFromHeatmap(HeatmapType inputValue, HeatmapType maxHeatmapValue, int maxColor) {
    double scale = static_cast<double>(maxColor) / maxHeatmapValue;

    /*
    if (inputValue * scale > 0.8) {    // Use this if the colors get washed out
        return maxColor;
    }
    */

    return inputValue * scale;
}

string elapsedTime(chrono::nanoseconds elapsedTime)
{
	chrono::hours hrs = chrono::duration_cast<chrono::hours>(elapsedTime);
	chrono::minutes mins = chrono::duration_cast<chrono::minutes>(elapsedTime - hrs);
	chrono::seconds secs = chrono::duration_cast<chrono::seconds>(elapsedTime - hrs - mins);
	chrono::milliseconds mils = chrono::duration_cast<chrono::milliseconds>(elapsedTime - hrs - mins - secs);

	stringstream ss("");

	if (hrs.count() > 24)
	{
		ss << hrs.count() / 24 << " Days, " << hrs.count() % 24 << " Hours, ";
	}
	else if (hrs.count() > 0)
	{
		ss << hrs.count() << " Hours, ";
	}

	if (mins.count() > 0)
	{
		ss << mins.count() << " Minutes, ";
	}

	if (secs.count() > 0)
	{
		ss << secs.count() << " Seconds, ";
	}

	if (mils.count() > 0)
	{
		ss << mils.count() << " Milliseconds";
	}

	return ss.str();
}

int main() {

    const Complex MINIMUM(-2.0, -1.5);    // Change min and max values to change the appearance of the buddha
    const Complex MAXIMUM(1.0, 1.5);
    const int IMAGE_HEIGHT = 400;
    const int IMAGE_WIDTH = 400;
    const int RED_ITERS = 50;
    const int GREEN_ITERS = 500;
    const int BLUE_ITERS = 50;

    const int SAMPLE_COUNT = IMAGE_WIDTH * IMAGE_HEIGHT * 100;    // Taking * on another constant value to increase the amount of samples. Starting with 1 though.

    auto startTime = chrono::high_resolution_clock::now();

    ofstream imgOut("out.ppm");
    if (!imgOut) {
        cout << "Could not open image file for writing!" << endl;
        cout << "Press ENTER to continue..." << endl;
        cin.ignore();
        return EXIT_FAILURE;
    }

    HeatmapType maxHeatmapValue = 0;    // Change this value if you want to have it so the colors blend differently, well, the maximum value of a color changes in the ppm
    HeatmapType** red;
    HeatmapType** green;
    HeatmapType** blue;    // Originally this goes on the stack so can't take too great values on height and width without crashing so we set a pointer reference to pointer to work on heap instead of stack

    AllocHeatmap(red, IMAGE_WIDTH, IMAGE_HEIGHT);
    AllocHeatmap(green, IMAGE_WIDTH, IMAGE_HEIGHT);
    AllocHeatmap(blue, IMAGE_WIDTH, IMAGE_HEIGHT);

    // Generate heatmap
    GenerateHeatmap(red, IMAGE_WIDTH, IMAGE_HEIGHT, MINIMUM, MAXIMUM, RED_ITERS,
		SAMPLE_COUNT, maxHeatmapValue, "Red Channel: ");
	GenerateHeatmap(green, IMAGE_WIDTH, IMAGE_HEIGHT, MINIMUM, MAXIMUM, GREEN_ITERS,
		SAMPLE_COUNT, maxHeatmapValue, "Green Channel: ");
	GenerateHeatmap(blue, IMAGE_WIDTH, IMAGE_HEIGHT, MINIMUM, MAXIMUM, BLUE_ITERS,
		SAMPLE_COUNT, maxHeatmapValue, "Blue Channel: ");
    // Scale the heatmap down
    for (int row = 0; row < IMAGE_HEIGHT; ++row) {
        for (int col = 0; col < IMAGE_WIDTH; ++col) {
            red[row][col] = colorFromHeatmap(red[row][col], maxHeatmapValue, 255);
            green[row][col] = colorFromHeatmap(green[row][col], maxHeatmapValue, 255);
            blue[row][col] = colorFromHeatmap(blue[row][col], maxHeatmapValue, 255);
        }

    }

    // Write PPM header
    imgOut << "P3" << endl;
    imgOut << IMAGE_WIDTH << " " << IMAGE_HEIGHT << endl;
    imgOut << 255 << endl;



    // Write PPM image from color maps
    for (int row = 0; row < IMAGE_HEIGHT; ++row) {
        for (int col = 0; col < IMAGE_WIDTH; ++col) {
            imgOut << red[row][col] << " "; 
            imgOut << green[row][col] << " ";
            imgOut << blue[row][col] << "   ";
        }
        imgOut << endl;
    } 

    FreeHeatmap(red, IMAGE_HEIGHT);
    FreeHeatmap(green, IMAGE_HEIGHT);
    FreeHeatmap(blue, IMAGE_HEIGHT);

    auto endPoint = chrono::high_resolution_clock::now();

    cout << "Time elapsed: " << elapsedTime(endPoint - startTime) << endl;
    cout << "Finished generating image. Open in GIMP to view. Press ENTER to exit." << endl;
    cin.ignore();

    return EXIT_SUCCESS;
}