#ifndef _SESCTHERM3D_CLASSDECLARE_H
#define _SESCTHERM3D_CLASSDECLARE_H

//layer info
class sesctherm3Dlayerinfo;

//library
class sesctherm3Ddatalibrary;

//base
class model_unit;

//chip
class chip_flp_unit;
class chip_floorplan;

//config data
class config_data;


//dtm
//class DTM_data;
//class DTM_pid_control;

//graphics
class sesctherm3Dgraphics;
class sesctherm3Dgraphicsfiletype;
class sesctherm3Dgraphicscolor;

//material
class sesctherm3Dmaterial;
class sesctherm3Dmaterial_list;

//model
class sesctherm3Dmodel;


//ucool
class ucool_flp_unit;
class ucool_floorplan;


//utilities
class sesctherm_utilities;
class RegressionNonLinear;
class RegressionLine;
template<typename T1, typename T2> class value_equals;
template <class T> class dynamic_array;
template <class T> class dynamic_array_row;


//class sesctherm3Dinterconnect;
#endif
