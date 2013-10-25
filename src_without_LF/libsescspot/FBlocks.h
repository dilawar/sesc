#ifndef FBLOCKS_H
#define FBLOCKS_H

#include <iostream>
#include <vector>

using namespace std;

class Grids;

class FBlocks {

friend class Grids;
public:

	//Constructor
	FBlocks (double xcord1, double ycord1, double xcord2, double ycord2, double pow, int32_t n);

	//Inspectors
	double power() const;
	double area() const;
	double temp() const;
	
	double x1 () const;
	double y1 () const;
	double x2 () const;
	double y2 () const;

	double gx1 () const;
	double gy1 () const;
	double gx2 () const;
	double gy2 () const;

	int32_t index() const;

	void ListGrids() const;

	bool IsGridPresent(int32_t n) const;

	//Mutators
	void power(double pow);
	void temp(double temp_);
	void x1 (double xcord1);
	void y1 (double ycord1);
	void x2 (double xcord2);
	void y2 (double ycord2);

	void add_index (int32_t n);

	void updateGridcoord (int32_t row, int32_t column, double chip_length, double chip_height);

	void AddGrid(int32_t n, double gridarea);

	void PartPower(int32_t col, vector<Grids> &G);

	double CalculateTemp (vector<double> &grid_t, int32_t offset, int32_t mode, int32_t g_col);

	void CalculateGridTemp(vector<Grids> &G);

	void ResetTemp();

	void SetGridUpdate(bool state);

//private:

	int32_t index_;

	double x_1;
	double y_1;
	double x_2;
	double y_2;

	double gx_1;
	double gy_1;
	double gx_2;
	double gy_2;

	double power_;

	double temperature;

	double area_;
	double garea_;

	struct GridDetails {
		
		int32_t number;
		double area;		
	};

	vector<GridDetails> grids;

	bool GridsUpdated;

};

#endif

