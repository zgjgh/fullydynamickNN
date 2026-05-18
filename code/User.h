#pragma once

#include "Eigen//Dense"

#include <limits>
#include <vector>
#include <iostream>
//#include <execution>
#include <algorithm>
#include "Node.h"

typedef const Eigen::Ref<const Eigen::VectorXf>& VectorRef;

#include "Item.h"

#include "itemNode.h"

#include "itemCluster.h"

#include "Cluster_origin.h"



//#include "itemNonLeafNode.h"
//#include "itemLeafNode.h"

#include <set>

enum class Mode { Add, Remove };

class User
{
public:
	User(Eigen::VectorXf C, long KNN);    // Be careful dangerous, change the variable

	~User();

	void computeKNN(const std::vector<Item*>& L);
	/*void userComputeKNN(itemNode* node);*/
	/*void userComputeKNN(itemNode* node, User* U);*/

	void computeKNN_sort(const std::vector<Item*>& L);


	void update(Item* I, Mode m);

	float dknn;

	Eigen::VectorXf center;           // Be careful dangerous, change the variable

	std::vector <Item*> R;

	void dist(Item* I, float& D);
	float dist(float& D, struct itemCluster* C);
	float dist(float& D, Item* I);

	//float dist(User* user_d, Item* item_d);

    std::vector<Eigen::VectorXf> u_low_d;        //Be careful, this is for store the low dimensional version of the users
    std::vector<Eigen::VectorXf> u_low_d_i;      //This is for the low dimensional version of the users based on the delta tree.

    Node* belong_leaf;    //Be careful, this is used for get the belonging leafnode

    std::vector<Cluster_origin*> list_cluster_belong;

    std::vector<std::tuple<float,Item*>> knn_temporary;

    Eigen::VectorXf center_low;           // The low dimensional form of the user center

private:
	long k;

	;

	void append(Item* I);
	void remove(Item* I);
	void remove(long index);
};

