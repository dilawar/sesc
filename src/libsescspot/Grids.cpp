#include <iostream>
#include <cmath>

#include "Grids.h"	

using namespace std;

// Some of the routines are not called elsewhere.
// But if desired, they can be used to get more information
// for functional blocks and grid cells.

//Constructor
Grids::Grids (int32_t r, int32_t c, int32_t n, double chip_length, double chip_height) 
{
	power_ = 0.0;
	temperature = 0.0;
	
	double x = chip_length / c;
	double y = chip_height / r;

	x_1 = ((n - 1) % c) * x;
	x_2 = (((n - 1) % c) + 1) * x;
	y_1 = ((n - 1) / c) * y;
	y_2 = (((n - 1) / c) + 1) * y;

	gx_1 = ((n - 1) % c); 
	gx_2 = (((n - 1) % c) + 1);
	gy_1 = ((n - 1) / c);
	gy_2 = (((n - 1) / c) + 1);
}

//Inspectors
double Grids::power() const 
{
	return power_;
}
	
double Grids::temp() const 
{
	return temperature;
}

double Grids::x1 () const 
{
	return x_1;
}

double Grids::y1 () const 
{
	return y_1;
}

double Grids::x2 () const 
{
	return x_2;
}

double Grids::y2 () const 
{
	return y_2;
}


double Grids::gx1 () const 
{
	return gx_1;
}

double Grids::gy1 () const 
{
	return gy_1;
}

double Grids::gx2 () const 
{
	return gx_2;
}

double Grids::gy2 () const 
{
	return gy_2;
}

//Mutators

void Grids::power(double pow) 
{
	power_ = power_ + pow;
}

void Grids::temp(double temp_) 
{
	temperature = temperature + temp_;
}

void Grids::ResetTemp() 
{
	temperature = 0.0;
}

void Grids::AddGrid(int32_t n, double gridarea) 
{
		BlkDetails temp;
       
  	temp.number = n;
        temp.area = gridarea;
        
        blocks.push_back(temp);
}


double Grids::CalculateGridTemp(vector<double> &block_t, int32_t offset) 
{
        double temp = 0.0;
        for (int32_t i = 0; i < (int) blocks.size(); i++) { 
                temp = temp + (blocks[i].area * block_t[blocks[i].number + offset]);
        }
                
        return temp;
}

double Grids::CalculateGridPower(vector<double> &block_pow, int32_t offset, vector<FBlocks> &F) 
{
	double pow = 0.0;
        for (int32_t i = 0; i < (int) blocks.size(); i++) {
           pow = pow + ( (blocks[i].area / F[blocks[i].number].garea_) * 
				   block_pow[blocks[i].number + offset] );
        }
        
        return pow;
}
