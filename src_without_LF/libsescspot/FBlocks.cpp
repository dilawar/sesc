#include <iostream>
#include <cmath>

#include "FBlocks.h"
#include "Grids.h"
#include "util.h"
#include "temperature.h"
 

using namespace std;

// Some of the routines are not called elsewhere.
// But if desired, they can be used to get more information
// for functional blocks and grid cells.

//Constructor
FBlocks::FBlocks (double xcord1, double ycord1, double xcord2, double ycord2, double pow, int32_t n) {

	index_ = n;

	x_1 = xcord1;
	y_1 = ycord1;
	x_2 = xcord2;
	y_2 = ycord2;

	power_ = pow;

	area_ = (x_2 - x_1) * (y_2 - y_1);

	garea_ = 0.0;

	temperature = 0.0;

	//The grid cells in this functional block haven't been updated
	GridsUpdated = false;
}


//Inspectors
double FBlocks::power() const {

	return power_;
}

double FBlocks::area() const {
	
	return area_;
}

double FBlocks::temp() const {

	return temperature;
}

double FBlocks::x1() const {

	return x_1;
}

double FBlocks::y1() const {

	return y_1;
}

double FBlocks::x2() const {

	return x_2;
}

double FBlocks::y2() const {

	return y_2;
}

double FBlocks::gx1() const {

	return gx_1;
}

double FBlocks::gy1() const {

	return gy_1;
}

double FBlocks::gx2() const {

	return gx_2;
}

double FBlocks::gy2() const {

	return gy_2;
}

int32_t FBlocks::index() const {

	return index_;
}

bool FBlocks::IsGridPresent(int32_t n) const {

	for (int32_t i = 0; i < (int) grids.size(); i++) {

		if (grids[i].number == n) {
			return true;
		}
	}
	return false;
}

void FBlocks::ListGrids() const {

	if ((int) grids.size() == 0) {
		cout << "No grids for this block!" << endl;
	}
	else {
		for (int32_t i = 0; i < (int) grids.size(); i++) {
			cout << grids[i].number << "," << grids[i].area << " ";
		}
		cout << endl;
	}
}

//Mutators
void FBlocks::power(double pow) {
	power_ = pow;
}

void FBlocks::temp(double temp_) {
	temperature = temp_;
}

void FBlocks::x1 (double xcord1) {
	x_1 = xcord1;
}

void FBlocks::y1 (double ycord1) {
	y_1 = ycord1;
}

void FBlocks::x2 (double xcord2) {
	x_2 = xcord2;
}

void FBlocks::y2 (double ycord2) {
	y_2 = ycord2;
}

void FBlocks::add_index (int32_t n) {
	index_ = n;
}

// Get the co-ordinates in terms of grid cells
void FBlocks::updateGridcoord (int32_t row, int32_t column, double chip_length, double chip_height) 
{
	gx_1 = x_1 / (chip_length / column);
	gy_1 = y_1 / (chip_height / row);
	gx_2 = x_2 / (chip_length / column);
	gy_2 = y_2 / (chip_height / row);
	garea_ = (gx_2 - gx_1) * (gy_2 - gy_1);
}

// Add a grid cell to the list of grid cells that a functional block (partially) occupies
void FBlocks::AddGrid(int32_t n, double gridarea) 
{
	if (GridsUpdated == false) {
		GridDetails temp;
		
		temp.number = n;
		temp.area = gridarea;
	
		grids.push_back(temp);
	}
}

// Area weighted power partition
void FBlocks::PartPower(int32_t col, vector<Grids> &G) 
{	
	double x = gx_1;
	double y = gy_1;
	int32_t intx1, inty1, intx2, inty2;
	double dx1, dy1, dx2, dy2;
	double p = 0.0;
	int32_t grid, i, j;
	int32_t wholex, wholey;
	
	double length, height;

	bool subx2 = true;
	bool suby2 = true;

	intx1 = (int) x;
	dx1 = x - intx1;
	inty1 = (int) y;
	dy1 = y - inty1;
	intx2 = (int) gx_2;
	dx2 = gx_2 - intx2;
	inty2 = (int) gy_2;
	dy2 = gy_2 - inty2;
	
	length = 1 - dx1;
	height = 1 - dy1;

	if (intx1 == intx2) { //the length of the block is within a grid
		length = gx_2 - gx_1;
		subx2 = false;
	}
	
	if (inty1 == inty2) { //the height of the block is within a grid
		height = gy_2 - gy_1;
		suby2 = false;
	}

	if (eq((float)dx2,0.0)) {
		if ( (gx_2 - gx_1) <= 1.0 ) subx2 = false;
		intx2 = intx2 - 1;
		dx2 = 1;
	}

	if (eq((float)dy2,0.0)) {
		if ( (gy_2 - gy_1) <= 1.0 ) suby2 = false;
		inty2 = inty2 - 1;
		dy2 = 1;
	}

	wholex = intx2 - (intx1 + 1);
	wholey = inty2 - (inty1 + 1);
	
	//subx1
	p = (((height) * (length)) / garea_) * power_;
	x = x + (1 - dx1);
	grid = (int) (y) * col + (int)x;
	G[grid].power(p);
	AddGrid(grid, height * length);
	G[grid].AddGrid (index_, length * height);
	
	y = y + (1 - dy1);	
	
	for (i = 0; i < wholey; i++) {
		p = ((1 * (length)) / garea_) * power_;
		grid = (int) (y) * col + (int)x;
		G[grid].power(p);
		AddGrid(grid, 1 * length);
		G[grid].AddGrid (index_, length * 1);
		y = y + 1;
	}
	
	if (suby2 == true) {
		p = ((dy2 * (length)) / garea_) * power_;
		grid = (int) (y) * col + (int)x;
		G[grid].power(p);
		AddGrid(grid, dy2 * length);
		G[grid].AddGrid (index_, length * dy2);
	}

	//wholex
	for (j = 0; j < wholex; j++) {
		x = x + 1;
		y = gy_1;
		p = (((height) * 1) / garea_) * power_;
		grid = (int) (y) * col + (int)x;
		G[grid].power(p);
		AddGrid(grid, height * 1);
		G[grid].AddGrid (index_, 1 * height);

		y = y + (1 - dy1);	
		for (i = 0; i < wholey; i++) {
			p = ((1 * 1) / garea_) * power_;
			grid = (int) (y) * col + (int)x;
			G[grid].power(p);
			AddGrid(grid, 1 * 1);
			G[grid].AddGrid (index_, 1 * 1);
			y = y + 1;
		}
		
		if (suby2 == true) {
			p = ((dy2 * 1) / garea_) * power_;
			grid = (int) (y) * col + (int)x;
			G[grid].power(p);
			AddGrid(grid, dy2 * 1);
			G[grid].AddGrid (index_, dy2 * 1);
		}
	}
	
	if (subx2 == true) {
	//subx2
		y = gy_1;
		
		p = (((height) * dx2) / garea_) * power_;
		grid = (int) (y) * col + (int)x + 1;
		G[grid].power(p);
		AddGrid(grid, height * dx2);
		G[grid].AddGrid (index_, dx2 * height);
	
		y = y + (1 - dy1);	
		for (i = 0; i < wholey; i++) {
			p = ((1 * dx2) / garea_) * power_;
			grid = (int) (y) * col + (int)x + 1;
			G[grid].power(p);
			AddGrid(grid, 1 * dx2);
			G[grid].AddGrid (index_, 1 * dx2);
			y = y + 1;
		}
		
		if (suby2 == true) {
			p = ((dy2 * dx2) / garea_) * power_;
			grid = (int) (y) * col + (int)x + 1;
			G[grid].power(p);
			AddGrid(grid, dy2 * dx2);
			G[grid].AddGrid (index_, dy2 * dx2);
		}
	}
}

double FBlocks::CalculateTemp (vector<double> &grid_t, int32_t offset, int32_t mode, int32_t g_col) 
{
	double temp = 0.0;
	if ( (int) grids.size() <=1) {
		temp = grid_t[grids[0].number - 1 + offset];
		return temp;
	}
	else {
		double min, max; 
		min = max = grid_t[grids[0].number - 1 + offset];
		for (int32_t i = 0; i < (int) grids.size(); i++) {
			temp = temp + ((grids[i].area / garea_) * grid_t[grids[i].number - 1 + offset]);
			if (max < grid_t[grids[i].number - 1 + offset])
				max = grid_t[grids[i].number - 1 + offset];
			if (min > grid_t[grids[i].number - 1 + offset])
				min = grid_t[grids[i].number - 1 + offset];
		}
		int32_t g_cent_x = (int) (gx_1 + gx_2) / 2;
		int32_t g_cent_y =(int) (gy_1 + gy_2) / 2;
		int32_t cent_grid = g_cent_x + (g_col * g_cent_y);
		switch(mode) {
			case GRID_AVG:
					return temp;
			case GRID_MIN:
					return min;
			case GRID_MAX:
					return max;
			case GRID_CENTER:
					return grid_t[cent_grid + offset];
			default:
					fatal("unknown mapping mode\n");
					return 0.0;
		}
	}
}

void FBlocks::CalculateGridTemp(vector<Grids> &G) 
{
	int32_t i = 0;
	for (i = 0; i < (int) grids.size(); i++) {
		G[grids[i].number].temp(grids[i].area * temperature);
	}
}

void FBlocks::ResetTemp() 
{
	temperature = 0.0;
}

void FBlocks::SetGridUpdate(bool state) 
{
	GridsUpdated = state;
}

