#include "Item.h"



Item::Item(Eigen::VectorXf C)    // Be careful dangerous, change the variable
{
	v = C;
	epsilon = 1e-4;
	v_d=C.cast<double>();   //Be careful, add a new feature, a double type of the v feature Be careful, add a new feature, a double type of the v feature Be careful, add a new feature, a double type of the v feature
}                           //Be careful, add a new feature, a double type of the v feature Be careful, add a new feature, a double type of the v feature Be careful, add a new feature, a double type of the v feature


Item::~Item()
{
}

bool Item::operator ==(const Item& I) {

	if ((v - I.v).norm() < epsilon) {
		return true;
	}
	else {
		return false;
	}
}
