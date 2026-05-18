#pragma once

#include "Eigen//Dense"
#include <vector>

typedef const Eigen::Ref<const Eigen::VectorXf>& VectorRef;

#define numDimensions 4
typedef Eigen::Matrix<float, numDimensions, 1> Vector;

class Item
{
public:
	Item(Eigen::VectorXf C);              // Be careful dangerous, change the variable
	~Item();

	Eigen::VectorXf v;             // Be careful dangerous, change the variable, it is originally float

	long cluster_i;               //The No of clusteri it belongs to
	double dis_to_i;               //Distance between it and clusteri_i
	double index_b_plus;           //Its index number in idistance system
	Eigen::VectorXd v_d;       //Be careful, add a new feature, a double type of the v feature Be careful, add a new feature, a double type of the v feature Be careful, add a new feature, a double type of the v feature

    std::vector<Eigen::VectorXf> i_low_d;        //Be careful, this is for store the low dimensional version of the items

	bool operator ==(const Item& I);

    Eigen::VectorXf v_low;             //The low dimensional form of the item v

private:
	float epsilon;



};

