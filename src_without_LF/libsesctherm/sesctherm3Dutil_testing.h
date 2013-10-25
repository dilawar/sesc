#ifndef _SESCTHERM3D_UTIL_H
#define _SESCTHERM3D_UTIL_H


using namespace std;
//Note: indexing method works like cartesian coordinate system [x][y], where [x]=column, [y]=row, thus dyn_array[x] selects COLUMN x (not ROW x)


class sample_class {
public:
	sample_class(){
		value_=0;
	}
	
	double operator=(double value){
		value_=value;
		return(value_);
	}
	double operator=(int value){
		value_=(double)value;
		return(value_);
	}
	double value_;
};

template <class T>
class dynamic_array_row {
public:
    dynamic_array_row(){
        nrows_=1;
        ncols_=1;
        col_=0;
        max_row_=0;	//size is [0,0]
        max_col_=0;
		T* newalloc;
        for (int i=0;i<nrows_;i++) {
            data_.push_back(vector<T*>(ncols_));
			newalloc = new T[ncols_]; 
			newalloc_pointers_.push_back(newalloc);
			for(int j=0;j<ncols_;j++) 
				data_[i][j] = &newalloc[j];
        }
		
    }
	~dynamic_array_row(){
		for(uint32_t i=0;i<newalloc_pointers_.size();i++)
				delete [] newalloc_pointers_[i];
	}
	
    dynamic_array_row(int rows,int cols){
		T* newalloc;
        for (int i=0;i<rows;i++) {
            data_.push_back(vector<T*>(cols)); //store row
			newalloc = new T[cols];
			newalloc_pointers_.push_back(newalloc);
			for(int j=0;j<cols;j++){
				data_[i][j] = &newalloc[j];
			}
        }
        nrows_=rows;
        ncols_=cols;
        max_row_=0; //although we allocated the space, the actual size is [0,0]
        max_col_=0;
        col_=0;
    }
	
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
		//cout << "rows=" << rows << " cols=" << cols << endl;
		if(rows>nrows_){
			//cout <<  "nrows_=" << nrows_ << " rows=" << rows << endl;
			data_.resize(rows);
			nrows_=rows;
		}
		if(cols>ncols_){
			//cout <<  "ncols_=" << ncols_ << " cols=" << cols << endl;
			ncols_=cols;
		}
		
		for (int i=0;i<nrows_;i++) {
			if(data_[i].size()!=(uint32_t)ncols_){		//resize the given row as necessary
				//cout << "data_[" << i << "].size() [" << data_[i].size() << "] is not equal to ncols_ [" << ncols_ << "]" << endl;
				old_size=data_[i].size();
				//data_[i].resize(ncols_);
				vector<T*> newvector = vector<T*>(ncols_);
				for(uint32_t j=0;j<data_[i].size();j++)
					newvector[j]=data_[i][j];
				data_[i]=newvector;
				//cout << "data_[" << i << "].size() is now" << ncols_ << endl;
				newalloc = new T[ncols_];//T[ncols_-old_size]();//T[ncols_-old_size]; 
				newalloc_pointers_.push_back(newalloc);
				//cout << "old_size:" << old_size << " newsize: " << ncols_ << endl;
				for(int j=0;j<ncols_;j++){			//allocate new objects for the given row
					if (data_[i][j]==NULL){
						data_[i][j] = &(newalloc[j]);
						if(typeid(T)==typeid(int) ||
							typeid(T)==typeid(double) ||
							typeid(T)==typeid(char) ||
							typeid(T)==typeid(double) ||
							typeid(T)==typeid(short) ||
							typeid(T)==typeid(bool))
							*data_[i][j] = 0; 
					}
				}
			}
		}
	}
	
	
	void clear(){
		for(int i=0;i<nrows_;i++){
			for(int j=0;j<ncols_;j++){
				delete data_[i][j];
			}
			data_[i].clear();
		}
		data_.clear();
		nrows_=0;
		ncols_=0;
		max_row_=0;
		max_col_=0;
	}
	
	
	const T & operator[](int index) const{
		//cout << "Indexing Row:" << index << endl; //TESTING
		
		if (index<0 || col_<0) {
			cerr << "Dynamic Array bounds is negative! [" << col_ << "][" << index << "]" << endl;
			exit(1);
		}
		if (col_>max_col_)
			max_col_=col_;
		if (index>max_row_)
			max_row_=index;
		
		int newcolsize=ncols_;
		int newrowsize=nrows_;
		while (col_+1>=newcolsize) {
			newcolsize*=2;
			//the col index has approached the number of cols, double the number of cols
			//keep doubling the number of cols until the index is less than half of ncols_
		}
		//If the array index selected is greater than or equal to the the number of rows
		//then double the number of rows in all of the columns
		//and keep doubling until the index is less than half of nrows_
		while (index+1>=newrowsize) {
			newrowsize*=2;
		}
		increase_size(newrowsize,newcolsize);
		return(*data_[index][col_]);  
	}
	
	
	T & operator[](int index) {
		//cout << "Indexing Row:" << index << endl; //TESTING
		
		if (index<0 || col_<0) {
			cerr << "Array bounds is negative! [" << col_ << "][" << index << "]" << endl;
			exit(1);
		}
		if (col_>max_col_)
			max_col_=col_;
		if (index>max_row_)
			max_row_=index;
			
		int newcolsize=ncols_;
		int newrowsize=nrows_;
		while (col_+1>=newcolsize) {
			newcolsize*=2;
			//the col index has approached the number of cols, double the number of cols
			//keep doubling the number of cols until the index is less than half of ncols_
		}
		//If the array index selected is greater than or equal to the the number of rows
		//then double the number of rows in all of the columns
		//and keep doubling until the index is less than half of nrows_
		while (index+1>=newrowsize) {
			newrowsize*=2;
		}
		increase_size(newrowsize,newcolsize);
		return(*data_[index][col_]);
	}
	
	
	std::vector<T> & getrow(int row){
		return(data_[row]);
	}
	
	
	int col_; //row to select
	int nrows_; //number of rows
	int ncols_; //number of columns
	int max_row_;
	int max_col_;
	
private:
		std::vector< std::vector<T*> > data_; //this is the matrix (stored in data_[row][column])
		std::vector< T* > newalloc_pointers_;
};


template <class T>
class dynamic_array {
public:
	
    dynamic_array(){
		data_ = new dynamic_array_row<T>();
		
    }
	~dynamic_array(){
		delete data_;
	}
    dynamic_array(int rows, int32_t cols)
    {
        data_ = new dynamic_array_row<T>(rows,cols);
    }
    void clear(){
        data_->clear();
    }
    dynamic_array_row<T> & operator[](int i) {
		//cout << "Indexing row:" << i << endl; //TESTING
        data_->col_=i;    //store row value
        return(*data_);
    }
	
    const dynamic_array_row<T> & operator[](int i) const{
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
    dynamic_array_row<T>* data_;
};



#endif
