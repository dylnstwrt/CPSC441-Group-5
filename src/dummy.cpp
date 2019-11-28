#include <iostream>
#include <cstdio>
#include <ctime>


using namespace std;

int main()
{
	clock_t startTime = clock();
	clock_t testTime;
	clock_t timePassed;
	double secondsPassed;
	int woo;
	while (true)
	{
		// Code breaks if you type a letter
		cin >> woo;
		testTime = clock();
		timePassed = startTime - testTime;
		secondsPassed = timePassed / (double)CLOCKS_PER_SEC;
		cout << "Time" << secondsPassed << endl;

	}
	return 0;
}