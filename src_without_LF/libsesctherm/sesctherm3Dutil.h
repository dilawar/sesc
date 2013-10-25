#ifndef _SESCTHERM3D_UTIL_H
#define _SESCTHERM3D_UTIL_H

#include "sesctherm3Dinclude.h"

class sesctherm_utilities;
class RegressionLine;
class RegressionNonLinear;

template<typename T1, typename T2> class value_equals;


template<typename T1, typename T2>
class value_equals {
public:
    value_equals(T1 const & value) : value_(value) {}
	
    typedef std::pair<T1, T2> pair_type;
    bool operator() (pair_type p) const {
        return p.first == value_;
    }
private:
		T1 const & value_;
};



// ALL THE GENERAL UTILITY FUNCTIONS
class sesctherm_utilities {
public:
    inline static std::string stringify(double x)
	{
        std::ostringstream o;
        if (!(o << x)){
        	cerr << "BadConversion: stringify(double)" << endl;
        	exit(1);
        }       
        return(o.str());
	}
	inline static std::string stringify(int x)
	{
		std::ostringstream o;
		if (!(o << x)){
			cerr << "BadConversion: stringify(int)" << endl;
			exit(1);
		}
		
		return(o.str());
	}
	inline static std::string stringify(uint32_t x)
	{
		std::ostringstream o;
		if (!(o << x)){
			cerr << "BadConversion: stringify(uint32_t)" << endl;
			exit(1);
		}
		return(o.str());
	}
	inline static double convertToDouble(const std::string& s,
										 bool failIfLeftoverChars = true)
	{
		std::istringstream i(s);
		double x;
		char c;
		if (!(i >> x) || (failIfLeftoverChars && i.get(c))) {
			cerr << "BadConversion: convertToDouble(string)" << endl;
			exit(1);
		}
		return(x);
	}
	inline static int32_t convertToInt(const std::string& s,
									  bool failIfLeftoverChars = true)
	{
		std::istringstream i(s);
		int x;
		char c;
		if (!(i >> x) || (failIfLeftoverChars && i.get(c))){
			cerr << "BadConversion: convertToInt(string)" << endl;
			exit(1);
		}
		return(x);
	}    
	//----------------------------------------------------------------------------
	// Tokenize : Split up a string based upon delimiteres, defaults to space-deliminted (like strok())
	//
	
	static void Tokenize(const string& str,
						 std::vector<string>& tokens,
						 const string& delimiters = " ")
	{
		tokens.clear();
		//Skip delimiters at beginning.
		//Fine the location of the first character that is not one of the delimiters starting
		//from the beginning of the line of test
		string::size_type lastPos = str.find_first_not_of(delimiters,0);
		//Now find the next occurrance of a delimiter after the first one
		// (go to the end of the first character-deliminated item)
		string::size_type pos = str.find_first_of(delimiters, lastPos);
		//Now keep checking until we reach the end of the line
		while (string::npos != pos || string::npos != lastPos) {
			//Found a token, add it to the vector.
			//Take the most recent token and store it to "tokens"
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			//Skip delimiters to find the beginning of the next token
			lastPos = str.find_first_not_of(delimiters,pos);
			//Find the end of the next token
			pos = str.find_first_of(delimiters, lastPos);
		}
	}
	static void fatal(string s){
		cerr << s << endl;
		exit(1);
	}
	
	static int32_t eq(double x, double y)
	{
		return(fabs(x-y) <  DELTA);
	}
	
	static int32_t lt(double x, double y)
	{
		return( x<y && !eq(x,y) );
	}
	static int32_t le(double x, double y)
	{
		return( x<y || eq(x,y));
	}
	static int32_t gt(double x, double y){
		return( x>y && !eq(x,y) );
	}
	static int32_t ge(double x, double y)
	{
		return( x>y || eq(x,y));
	}
	static double min(double x, double y)
	{
		if(le(x,y))
			return(x);
		else
			return(y);	
	}
	static double max(double x, double y)
	{
		if(ge(x,y))
			return(x);
		else
			return(y);
	}
	
	static double abs(double x, double y)
	{
		if(ge(x,y))
			return(fabs(x-y));
		else
			return(fabs(y-x));	
		
	}
	struct myUnique {
		bool operator()(const double& a, const double& b)
	{
			return(EQ(a,b));
	}
	};
	
	
	
	
};

using namespace std;
//Note: indexing method works like cartesian coordinate system [x][y], where [x]=column, [y]=row, thus dyn_array[x] selects COLUMN x (not ROW x)




template <class T>
class dynamic_array_rows {
public:
    dynamic_array_rows(){
        nrows_=0;
        ncols_=0;
        col_=0;
        max_row_=0;	//size is [0,0]
        max_col_=0;
 		datalibrary_=NULL;
		newalloc_pointers_.clear();
		data_.clear();
    }
	
	dynamic_array_rows(sesctherm3Ddatalibrary* datalibrary){
		datalibrary_=datalibrary;
        nrows_=0;
        ncols_=0;
        col_=0;
        max_row_=0;	//size is [0,0]
        max_col_=0;
		newalloc_pointers_.clear();
		data_.clear();
    }
	
	~dynamic_array_rows(){	
		for(uint32_t i=0;i<newalloc_pointers_.size();i++){
			cerr << "deleting newalloc pointers..." << endl;
			delete [] newalloc_pointers_[i];
		}
	}
	
    dynamic_array_rows(int rows,int cols, sesctherm3Ddatalibrary* datalibrary){
		datalibrary_=datalibrary;
        for (int i=0;i<rows;i++) {
            data_.push_back(vector<T*>(cols)); //store row
			T* newalloc = new T[cols];
			newalloc_pointers_.push_back(newalloc);
			for(int j=0;j<cols;j++){
				data_[i][j] = &newalloc[j];
				zero(data_[i][j]);					//zero the element
			}
        }
        nrows_=rows;
        ncols_=cols;
        max_row_=0; //although we allocated the space, the actual size is [0,0]
        max_col_=0;
        col_=0;
    }
	
	dynamic_array_rows(int rows,int cols){
		datalibrary_=NULL;
		T* newalloc;
        for (int i=0;i<rows;i++) {
            data_.push_back(vector<T*>(cols)); //store row
			newalloc = new T[cols];
			newalloc_pointers_.push_back(newalloc);
			for(int j=0;j<cols;j++){
				data_[i][j] = &newalloc[j];
				zero(data_[i][j]);					//zero the element
			}
        }
        nrows_=rows;
        ncols_=cols;
        max_row_=0; //although we allocated the space, the actual size is [0,0]
        max_col_=0;
        col_=0;
    }
	
	void zero(model_unit *value){ 
		model_unit::zero(*value,datalibrary_);
	}
	void zero(int *value) { *value= 0; }
	void zero(char *value) { *value= 0; }
	void zero(double *value) { *value= 0; }
	void zero(float *value) { *value= 0; }
	
	
	//the resize function should always INCREASE the size (not DECREASE)
    void increase_size(int rows, int32_t cols){
		T* newalloc;
		int old_size;
		if(rows<nrows_ || cols<ncols_){
			cerr << "sesctherm3Dutil dynamic array: increase_size => attempting to decrease size (not allowed)" << endl;
			exit(1);
		}
		
		if(rows==nrows_ && cols==ncols_)
			return;
		
		data_.resize(rows);
		nrows_=rows;
		ncols_=cols;
		
		
		for (int i=0;i<nrows_;i++) {
			if(data_[i].size()!=(uint32_t)ncols_){		//resize the given row as necessary
				old_size=data_[i].size();
				newalloc = new T[ncols_-old_size];	//allocate space for the other elements
				data_[i].resize(ncols_);					//resize the row
				
				newalloc_pointers_.push_back(newalloc);		//save a pointer to the newly allocated elements
				int itor=0;
				for(int j=old_size;j<ncols_;j++){					//store the newly allocated elements to the pointer locations			
					data_[i][j] = &(newalloc[itor]);	//store the element
					zero(data_[i][j]);					//zero the element
					itor++;
				}
			}
		}
    }
    
	int empty(){
		if(max_row_==0 && max_col_==0)
			return true;
		else
			return false;
	}
	
    void clear(){
		if(!empty()){
			for(uint32_t i=0;i<newalloc_pointers_.size();i++)
				delete [] newalloc_pointers_[i];				//delete the data at [i][j]
		}
		for(int i=0;i<nrows_;i++){							//delete the row vector
			data_[i].clear();
		}
		data_.clear();										//delete the column vector
		nrows_=0;
		ncols_=0;
		max_row_=0;
		max_col_=0;
	}
	
	
    const T & operator[](int index) const{
		
#ifdef DEBUG
		if (index<0 || col_<0) {
			cerr << "Dynamic Array bounds is negative! [" << col_ << "][" << index << "]" << endl;
			exit(1);
		}
#endif
		
		max_col_=MAX(col_, max_col_);
		max_row_=MAX(index,max_row_);
		
		if (index < nrows_ && col_ < ncols_)
			return(*data_[index][col_]); // common case
		
		
		
		int newcolsize=ncols_;
		int newrowsize=nrows_;
		while (newcolsize<=col_+1) {
			newcolsize=(newcolsize+1)*2;
			//the col index has approached the number of cols, double the number of cols
			//keep doubling the number of cols until the index is less than half of ncols_
		}
		//If the array index selected is greater than or equal to the the number of rows
		//then double the number of rows in all of the columns
		//and keep doubling until the index is less than half of nrows_
		while (newrowsize<=index+1) {
			newrowsize=(newrowsize+1)*2;
		}
		increase_size(newrowsize,newcolsize);
		return(*data_[index][col_]);  
    }
	
	
    T & operator[](int index) {
#ifdef DEBUG
		if (index<0 || col_<0) {
			cerr << "Dynamic Array bounds is negative! [" << col_ << "][" << index << "]" << endl;
			exit(1);
		}
#endif
		
		max_col_=MAX(col_, max_col_);
		max_row_=MAX(index,max_row_);
		
		if (index < nrows_ && col_ < ncols_)
			return(*data_[index][col_]); // common case
		
		
		
		int newcolsize=ncols_;
		int newrowsize=nrows_;
		while (newcolsize<=col_+1) {
			newcolsize=(newcolsize+1)*2;
			//the col index has approached the number of cols, double the number of cols
			//keep doubling the number of cols until the index is less than half of ncols_
		}
		//If the array index selected is greater than or equal to the the number of rows
		//then double the number of rows in all of the columns
		//and keep doubling until the index is less than half of nrows_
		while (newrowsize<=index+1) {
			newrowsize=(newrowsize+1)*2;
		}
		increase_size(newrowsize,newcolsize);
		return(*data_[index][col_]);  
    }
    
	
    std::vector<T> & getrow(int row){
		return(data_[row]);
    }
    
    
    int32_t col_; //row to select
    int32_t nrows_; //number of rows
    int32_t ncols_; //number of columns
    int32_t max_row_;
    int32_t max_col_;
    
private:
		std::vector< std::vector<T*> > data_; //this is the matrix (stored in data_[row][column])
    std::vector< T* > newalloc_pointers_;
    sesctherm3Ddatalibrary* datalibrary_;
};


template <class T>
class dynamic_array {
public:
	
    dynamic_array(){
		data_ = new dynamic_array_rows<T>();		
		datalibrary_=NULL;
    }
	
	dynamic_array(sesctherm3Ddatalibrary* datalibrary){
		data_ = new dynamic_array_rows<T>(datalibrary);		
		datalibrary_=datalibrary;	
	}
	
	~dynamic_array(){
		//data_->clear();
		delete data_;
	}
    dynamic_array(int rows, int32_t cols, sesctherm3Ddatalibrary* datalibrary)
    {
        data_ = new dynamic_array_rows<T>(rows,cols, datalibrary);
    }
	
	
    dynamic_array(int rows, int32_t cols)
    {
        data_ = new dynamic_array_rows<T>(rows,cols);
    }
	
	
    void clear(){
        data_->clear();
    }
    dynamic_array_rows<T> & operator[](int i) {
		//cout << "Indexing row:" << i << endl; //TESTING
        data_->col_=i;    //store row value
        return(*data_);
    }
	
    const dynamic_array_rows<T> & operator[](int i) const{
		//cout << "Indexing row:" << i << endl; //TESTING
        data_->col_=i;    //store row value
        return(*data_);
    }
	
    void increase_size(int rows, int32_t cols){
        data_->increase_size(rows,cols);
    }
	//Note: nrows=max_row_+1
	
    uint32_t nrows(){
        return(data_->max_row_+1);
    }
	//Note: ncols=max_col_+1
	
    uint32_t ncols(){
        return(data_->max_col_+1);
    }
private:
    dynamic_array_rows<T>* data_;
	sesctherm3Ddatalibrary* datalibrary_;
};




#endif
