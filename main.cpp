/**
 *  __  __             _          ____           _         ____  _ 
 * |  \/  | ___  _ __ | |_ ___   / ___|__ _ _ __| | ___   |  _ \(_)
 * | |\/| |/ _ \| '_ \| __/ _ \ | |   / _` | '__| |/ _ \  | |_) | |
 * | |  | | (_) | | | | ||  __/ | |__| (_| | |  | | (_) | |  __/| |
 * |_|  |_|\___/|_| |_|\__\___|  \____\__,_|_|  |_|\___/  |_|   |_|
 *
 * This program is a multithreaded C++ program that uses the Monte Carlo method
 * to make an estimate of the value of Pi.
 * 
 * Requirements:
 *   - C++ 11
 *
 * Compilation and running:
 *    g++ -std=c++11 main.cpp -o main
 *    ./main
 *
 * Usage:
 *    Just run the program and after pass any STDIN to terminate.
 *    (Press any key if input is from terminal)
 *
 * Outputing CSV data to file:
 *    # This parses the CSV data from the program output
 *    ./main | grep ".*,.*" > output.csv # Press enter to quit
 *
 *    # You might want to redirect the output of the program prior to saving
 *    # the CSV output to keep the programs's output
 *    ./main > output
 *    grep ".*,.*" output > output.csv
 *
 * I hope you find it helpful. I tried to document it as much as possible.
 * 
 * Distributed undet The MIT License (MIT)
 *
 * Copyright (c) 2014 Itay Grudev <itay@grudev.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdio>
#include <thread>
#include <list>
#include <chrono>
#include <random>

// After how many points (calculated per thread) should deviation be calculated 
#define analysis_samples_rate 1000000

bool * retire; // Flag to threads to quit
unsigned int * count; // Count of points incide the circle per thread
unsigned int * total; // Total number of points generated per thread

// An aproximation of Pi against which we use to make estimates of accuracy
const double test_pi = 3.14159265358979324;

// The number of threads the current system can sustain
unsigned supported_threads = std::thread::hardware_concurrency();

// Uniform random number distributions
// defined at global scope so all threads can use the same uniform distribution
// and thus achieve higher accuracy
std::random_device rdx;
std::random_device rdy;
std::mt19937_64 xgen(rdx());
std::mt19937_64 ygen(rdy());
std::uniform_real_distribution<> x(0, 1);
std::uniform_real_distribution<> y(0, 1);

// This struct stores deviation defined by to parameters - accuracy and number
// of points used to calculate that accuracy
struct deviation {
	long double accuracy;
	unsigned int points;
	deviation(long double accuracy, unsigned int points)
	 : accuracy(accuracy), points(points) { };
};

// In this list we store the deviations at a given time
std::list <deviation> deviation;

// This is the multi-threaded function that randomizes points and counts whether
// they are inside the circle
void pi_count(int);

// This function performs an analysis
// and accuracy estimate  at a given point in time
void analysis();

int main()
{
	// Supported threads is equal to zero if it cannot be found
	if(supported_threads == 0)
		// // Default to 2 since most systems today are dual core
		supported_threads = 2;

	// Dynamically allocate the counters and flags for threads
	count = new unsigned int[supported_threads];
	total = new unsigned int[supported_threads];
	retire = new bool[supported_threads];

	// Dynamically initialize thread handlers
	std::thread * threads = new std::thread[supported_threads];

	// Launch threads passing their sequential number and initializing their
	// kill flag to false
	printf("Launching %d Piculators threads.\n", supported_threads);
	for(int i = 0; i < supported_threads; ++i)
	{
		retire[i] = false;
		threads[i] = std::thread(std::bind(pi_count, i));
	}

	// Prompt the user to kill the program / wait as much as required
	printf("Press any key to terminate computation.\n");
	getchar(); // Wait until any kepress

	// Set interrupt flags to true and wait for threads to close
	for(int i = 0; i < supported_threads; ++i)
	{
		printf("Sending interrupt signal to thread: %d and waiting to close.\n", i);
		retire[i] = true;
		threads[i].join();
	}

	// Collect the data from all the threads (adding it to the first item)
	for(int i = 1; i < supported_threads; ++i)
	{
		count[0] += count[i];
		total[0] += count[i];
	}

	// Print calculated Pi values
	printf("Calculated Pi value of %Lf\nusing %d points.\n", (long double)count[0] / total[0] * 4, total[0]);

	// Print a CSV safe list of deviations from a Pi aproximation defined in the
	// top of the program at given points in time, based on number of points
	printf("Deviation at given points (CSV safe): \n");
	while( ! deviation.empty())
	{
		struct deviation diff = deviation.front();
		deviation.pop_front();
		printf("%d,%Lf\n", diff.points, diff.accuracy);
	}

	// Deallocate memory
	delete [] threads;

	return 0;
}

void pi_count(int n)
{
	// Initialize counters to 0
	count[n] = 0;
	total[n] = 0;

	// A forever loop with a counter
	for(unsigned long long i = 0;; ++i)
	{
		// Check of the interrupt flug is set
		if(retire[n])
			break;

		// Generate random coordinates form uniform real distributions
		long double x0 = x(xgen), y0 = y(ygen);

		// Check if the point defined bu the coordinates is has distance to the
		// center of the coordinate system less or equal to 1, thus belonging to
		// the circle
		//
		// Note that a long double is being used for maximum accuracy
		if(sqrtl(x0 * x0 + y0 * y0) <= 1)
		  ++count[n]; // Increment circle point counter for this specific thread
		++total[n]; // keep track of total generated points

		// The first thread also has the function to run the analysys function
		// from time to time
		if(n == 0 && i % analysis_samples_rate == 0)
			analysis();
	}
}

void analysis()
{
	// Initialize temp counters to zero for all points inside the circle and all
	// points generated
	int count_in = 0, count_total = 0;
	for(int i = 1; i < supported_threads; ++i)
	{
		// Sum the points
		count_in 	+= count[i];
		count_total += total[i];
	}
	// Make an estimate of the value of Pi, based on the current data
	long double calculated_pi = (long double)count_in / count_total * 4;

	// Populate the deviation list with the current deviation from the Pi value
	// approximation specified in the top of the program 
	deviation.push_back(deviation::deviation(test_pi - calculated_pi, count_total));
}
