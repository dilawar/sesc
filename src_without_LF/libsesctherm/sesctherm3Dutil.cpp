//----------------------------------------------------------------------------
// File: sesctherm3Dutil.cpp
//
// Description: 3-Dimensional Thermal Model with Micro-cooler implementation
// Authors    : Joseph Nayfach - Battilana
//
using namespace std;


#include <vector>
#include <iostream>
#include <sstream>


#include "sesctherm3Dutil_testing.h"





/*
Note: the handling of the nrows and ncols is a bit complicated.
The dynamic structure automatically expands in anticipation of new data. However, this new space should
not be considered used. Thus, there is no need for the user to be aware of the amount of space
currently being allocated. As such, nrows and ncols should represent the amount of USED space (not total space)

To accomplish this, max_x_ and max_y_ is maintained. It is assumed that if cell [x,y] is used, then cells
[1..x,1..y] are also used.

*/
/*

int main(){
	int i;
	
    dynamic_array<double> a(10,10);

    a[0][0]=0;
    a[0][1]=1;
    a[0][2]=2;
    a[0][3]=3;
    a[0][4]=4;
    a[0][5]=5;

    a[400][400]=10;
	//a.increase_size(10,100);

    // vector<int> temp=a[10].getrow();
    for (uint32_t i=0;i<a.nrows();i++) {
        for (uint32_t j=0;j<a.ncols();j++) {
            cout << a[j][i].value_ << " ";
        }
        std::cout << endl;
    }

	a.increase_size(4000,4000);
	cout << "next" << endl;
	for(uint32_t i=0;i<a.nrows();i++){
		for(uint32_t j=0;j<a.ncols();j++)
			cout << a[j][i].value_ << " ";
		std::cout << endl;
	}
    std::cout << "all good!" << endl;
	cin>>i;
}
*/
