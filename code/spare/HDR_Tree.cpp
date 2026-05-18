#include "HDR_Tree.h"
//#include "User.cpp"

HDR_Tree::HDR_Tree()
{
}

HDR_Tree::~HDR_Tree()
{
}

void HDR_Tree::setData(DataBase* dataBase) {
	data = dataBase;
}

void HDR_Tree::construct(long fanout, long threshold) {

	std::cout << "Constructing HDR Tree" << std::endl;
	std::cout << "Users: " << numUsers << std::endl;
	std::cout << "Features: " << numFeatures << std::endl;
	std::cout << "Window: " << windowSize << std::endl;
	std::cout << "Fanout: " << fanout << std::endl << std::endl;

	init();

	L = log(data->U.size()) / log(fanout);
	std::cout << "Estimated Height: " << L << " levels." << std::endl;

	data->initTransform(L);


// Be careful, here the low versions of the users and items are produced and stored in the specific containners of the users and items.
    for(auto u_ini: Users){
        projectUser(u_ini);
        for(int count_d=0; count_d<L; count_d=count_d+1){
            u_ini->u_low_d.emplace_back(uTrans[count_d]);
        }
    }


    for(auto i_ini: slidingWindow){
        projectItem(i_ini);
        for(int count_d_0=0; count_d_0<L_i; count_d_0=count_d_0+1){
            i_ini->i_low_d.emplace_back(iTrans[count_d_0]);
        }
    }

// Be careful, here the low versions of the users and items are produced and stored in the specific containners of the users and items.


	long numClusters = 0;

	auto C = new Clusters(Users, fanout, data->T[0]);





	for (auto Cj : *C) {

		if (Cj->Users.size() > 0) {
			Cj->maxdknn = std::get<1>(Cj->Users[0])->dknn;
			for (auto t : Cj->Users) {
				Cj->maxdknn = std::max(Cj->maxdknn, std::get<1>(t)->dknn);
			}
		}

		numClusters += 1;
		Cj->l = 1;

		for(auto c_item_0:*(dictionary_for_layer_itemcluster[Cj->l-1])){
            (Cj->distance_cross_u_t).emplace_back((Cj->center-c_item_0->center).norm());
		}
	}

	root = new NonLeafNode(C, 1, false);

	std::queue<Node *> q;

	q.push(root);

	long maxL = 1;              //Here the maxL should be 1; Here the maxL should be 1; Here the maxL should be 1; Here the maxL should be 1; Here the maxL should be 1;

	while (!q.empty()) {
		auto node = dynamic_cast<NonLeafNode *>(q.front());
		q.pop();

		auto Cp = node->clusters;

		for (auto Cj : Cp) {

			if (Cj->number < threshold) {
				auto LN = new LeafNode(Cj, false);

				node->children.emplace_back(LN);
				Cj->ptr = LN;

				//Be careful, here, the user points to its belonging leafnode, Be careful, here, the user points to its belonging leafnode, Be careful, here, the user points to its belonging leafnode

				for(auto user_leaf: LN->cluster->Users){
				    std::get<1>(user_leaf)->belong_leaf=LN;
				}

				//Be careful, here, the user points to its belonging leafnode, Be careful, here, the user points to its belonging leafnode, Be careful, here, the user points to its belonging leafnode

			}
			else {
				auto level = node->l + 1;
				maxL = std::max(maxL, level);

				if(level>L){
				    std::cout<<"Excceed the estimated level"<<std::endl;
				}

				auto Cpp = new Clusters(Cj, fanout, data->T[level-1]);
				for (auto Cj : *Cpp) {

					if (Cj->Users.size() > 0) {
						Cj->maxdknn = std::get<1>(Cj->Users[0])->dknn;
						for (auto t : Cj->Users) {
							Cj->maxdknn = std::max(Cj->maxdknn, std::get<1>(t)->dknn);
						}
					}

					numClusters += 1;
					Cj->l = level;

                    for(auto c_item_1:*(dictionary_for_layer_itemcluster[Cj->l-1])){
                        (Cj->distance_cross_u_t).emplace_back((Cj->center-c_item_1->center).norm());
                    }
				}

				auto NLN = new NonLeafNode(Cpp, level, false);

				node->children.emplace_back(NLN);
				Cj->ptr = NLN;
				q.push(NLN);
			}
		}
	}

	std::cout << "Actual Height: " << maxL << " levels." << std::endl;
	std::cout << "Clusters: " << numClusters << std::endl;

//	for (int i = 0; i < 100; i++) {
//		Users[0]->update(slidingWindow[i], Mode::Add);
//	}

//	for (int i = 0; i < 100; i++) {
//		if (Users[0]->dist(slidingWindow[i]) < Users[0]->dknn) {
//			Users[0]->update(slidingWindow[i], Mode::Remove);
//			break;
//		}
//	}


//	fillTree();

}

void HDR_Tree::updateUsers(Item* I, Mode m, int o, int n) {

	for (auto u : Rp) {

		if (verbose) {
			std::cout << "User at " << u->center.transpose() << " effected" << std::endl;
		}

		u->update(I, m);

		if (u->R.size() < k) {
			u->computeKNN(slidingWindow);
			numKnnComputions++;
		}

	}
}
void HDR_Tree::updateUsersParalell(Item* I, Mode m) {

//	std::chrono::steady_clock::time_point tA = std::chrono::steady_clock::now();

	int n = std::thread::hardware_concurrency();

	std::vector<std::thread> threads;

	for (long i = 0; i < n; i++) {
		threads.push_back(std::thread(
			&HDR_Tree::updateUsers, this, I, m, i, n
		));
	}

//	std::chrono::steady_clock::time_point tB = std::chrono::steady_clock::now();

	for (auto& th : threads) {
		th.join();
	}

//	std::chrono::steady_clock::time_point tC = std::chrono::steady_clock::now();

//	float cT = std::chrono::duration_cast<std::chrono::microseconds>(tB - tA).count();
//	float pT = std::chrono::duration_cast<std::chrono::microseconds>(tC - tB).count();

//	std::cout << "Create: " << cT << ", Process: " << pT << std::endl;

}
void HDR_Tree::updateUsersSerial(Item* I, Mode m) {

	std::chrono::steady_clock::time_point tA = std::chrono::steady_clock::now();

	for (auto u : Rp) {


		if (verbose) {
			std::cout << "User at " << u->center.transpose() << " effected" << std::endl;
		}

		u->update(I, m);

		if (u->R.size() < k) {
			u->computeKNN(slidingWindow);
			numKnnComputions++;
		}

	}

	std::chrono::steady_clock::time_point tB = std::chrono::steady_clock::now();
	updateTime = updateTime + (std::chrono::duration_cast<std::chrono::microseconds>(tB - tA).count()) / 1000000.0;

}

void HDR_Tree::projectItem(Item* I) {

	iTrans.clear();
	for (int i = 0; i < L; i++) {
		iTrans.emplace_back(data->T[i] * (I->v));
	}
}

void HDR_Tree::projectUser(User* U) {

	uTrans.clear();
	for (int i = 0; i < L; i++) {
		uTrans.emplace_back(data->T[i] * U->center);
	}
}

void HDR_Tree::userUpdate(User* U, Mode m) {

	/*UpdateNaiveRKNN(I, m);*/
	userUpdateHdrKNN(U, m);

}
void HDR_Tree::update(Item* I, Mode m) {

	/*UpdateNaiveRKNN(I, m);*/
	UpdateHdrKNN(I, m);

}
void HDR_Tree::UpdateNaiveRKNN(Item* I, Mode m) {

	Rp.clear();

	for (auto u : Users) {
		if (dist(I, u) < u->dknn) {
			/*std::chrono::steady_clock::time_point tA = std::chrono::steady_clock::now();
			u->computeKNN(slidingWindow);
			std::chrono::steady_clock::time_point tB = std::chrono::steady_clock::now();
			updateTime = updateTime +  (std::chrono::duration_cast<std::chrono::microseconds>(tB - tA).count())/1000000.0;*/
			Rp.emplace_back(u);
			/*numKnnComputions++;*/
		}
	}

	if (Rp.size() > 0) {
		//	updateUsersParalell(I, m);
		updateUsersSerial(I, m);

		//		std::chrono::steady_clock::time_point tC = std::chrono::steady_clock::now();

		AdjustMaxdknnPaper(root);
		//		std::chrono::steady_clock::time_point tD = std::chrono::steady_clock::now();

		//		AdjustMaxdknnPrev(root);
		//		std::chrono::steady_clock::time_point tE = std::chrono::steady_clock::now();

		//		std::cout << "Search Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tB - tA).count() << std::endl;
		//		std::cout << "Update Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tC - tB).count() << std::endl;
		//		std::cout << "Adjust Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tD - tC).count() << std::endl;
		//		std::cout << "Adjust Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tE - tD).count() << std::endl;
	}
}
void HDR_Tree::UpdateHdrKNN(Item* I, Mode m) {

	//	std::chrono::steady_clock::time_point tA = std::chrono::steady_clock::now();

	Rp.clear();

	projectItem(I);

	Search(root, I);

	//	std::chrono::steady_clock::time_point tB = std::chrono::steady_clock::now();
	//	std::cout << "Search Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tB - tA).count() << std::endl;

	if (verbose) {
		std::cout << "Searching item " << I->v.transpose() << std::endl;
	}

	if (Rp.size() > 0) {
		//	updateUsersParalell(I, m);
		updateUsersSerial(I, m);

		//		std::chrono::steady_clock::time_point tC = std::chrono::steady_clock::now();

		AdjustMaxdknnPaper(root);
		//		std::chrono::steady_clock::time_point tD = std::chrono::steady_clock::now();

		//		AdjustMaxdknnPrev(root);
		//		std::chrono::steady_clock::time_point tE = std::chrono::steady_clock::now();

		//		std::cout << "Search Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tB - tA).count() << std::endl;
		//		std::cout << "Update Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tC - tB).count() << std::endl;
		//		std::cout << "Adjust Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tD - tC).count() << std::endl;
		//		std::cout << "Adjust Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tE - tD).count() << std::endl;
	}
}

void HDR_Tree::userUpdateHdrKNN(User* U, Mode m) {

	projectUser(U);

	updateUser(root, U);


	//if (Rp.size() > 0) {
	//	//	updateUsersParalell(I, m);
	//	updateUsersSerial(I, m);

	//	AdjustMaxdknnPaper(root);
	//}
}


void HDR_Tree::Search(Node* node, Item* I) {

	if (node->type == NodeType::LeafNode) {
		auto LN = dynamic_cast<LeafNode*>(node);
		for (auto Ut : *LN) {

		    num_hdr_cal_users=num_hdr_cal_users+1;   //Add the calculation time of the users

			auto user = std::get<1>(Ut);
			if (dist(I, user) < user->dknn) {
				Rp.emplace_back(user);
			}
		}
	}
	else {
		auto NLN = dynamic_cast<NonLeafNode*>(node);
		auto& Cp = NLN->clusters;

		
		float d1,d2;

		for (long j = 0; j < Cp.size(); j++) {

		    num_hdr_cal_clusters=num_hdr_cal_clusters+1;   //Add the calculation time of the clusters

			d1 = dist(I, Cp[j]);
			d2 = Cp[j]->maxdknn;
			if (d1 < d2) {
				Search(NLN->children[j], I);
			}
		}
	}
}

void HDR_Tree::updateConstruct(Node* node) {

	std::queue<Node*> q;

	q.push(node);

	long maxL = 0;

	while (!q.empty()) {
		auto node = dynamic_cast<NonLeafNode*>(q.front());
		q.pop();

		auto Cp = node->clusters;

		for (auto Cj : Cp) {

			if (Cj->number < fix_threshold) {
				auto LN = new LeafNode(Cj, verbose);

				node->children.emplace_back(LN);
				Cj->ptr = LN;
			}
			else {
				auto level = node->l + 1;
				maxL = std::max(maxL, level);

				auto Cpp = new Clusters(Cj, 4, data->T[level - 1]);
				for (auto Cj : *Cpp) {

					if (Cj->Users.size() > 0) {
						Cj->maxdknn = std::get<1>(Cj->Users[0])->dknn;
						for (auto t : Cj->Users) {
							Cj->maxdknn = std::max(Cj->maxdknn, std::get<1>(t)->dknn);
						}
					}

					/*numClusters += 1;*/
					Cj->l = level;
				}

				auto NLN = new NonLeafNode(Cpp, level, verbose);

				node->children.emplace_back(NLN);
				Cj->ptr = NLN;
				q.push(NLN);
			}
		}
	}
}

void HDR_Tree::updateUser(Node* node, User* U) {



	if (node->type == NodeType::LeafNode) {
		auto LN = dynamic_cast<LeafNode*>(node);
		long maxL = 0;

		if (LN->cluster->number < fix_threshold) {
			U->computeKNN(slidingWindow);

			LN->cluster->Users.emplace_back(std::tuple<float, User*>(U->dknn, U));
			/*LN->cluster->number += 1;*/

			std::cout << "User added Successfully!!!" << std::endl;
			std::cout << "Number of users after insertion in LN: " << LN->cluster->number << std::endl;
			std::cout << "Number of users after insertion in LN: " << LN->cluster->Users.size() << std::endl;
		}
		else 
		{
			// We need to work on this part. If it gets overflow we have to split the node.

			/*long level = LN->cluster->l + 1;
			maxL = std::max(maxL, level);*/	

			/*updateConstruct(node);*/

			/*auto Cpp = new Clusters(LN->cluster, fix_threshold, data->T[level-1]);
			for (auto Cj : *Cpp) {
				if (Cj->Users.size() > 0) {
					Cj->maxdknn = std::get<1>(Cj->Users[0])->dknn;
					for (auto t : Cj->Users) {
						Cj->maxdknn = std::max(Cj->maxdknn, std::get<1>(t)->dknn);
					}
				}
				Cj->l = level;
			}

			auto NLN = new NonLeafNode(Cpp, level, verbose);*/

			//auto New_NLN = dynamic_cast<NonLeafNode*>(LN);

			//New_NLN->children.emplace_back(NLN);
			//
			//
			//
			///*parent->children.emplace_back(NLN);*/
			///*LN->cluster->ptr(NLN);*/

			//for (auto Cj : NLN->clusters) {
			//	/*Cj->ptr = NLN;*/
			//	std::cout << "Number of users after creation in in LN: " << Cj->number << std::endl;
			//	
			//	if (Cj->number < fix_threshold) {
			//		auto LN = new LeafNode(Cj, verbose);

			//		NLN->children.emplace_back(LN);
			//		Cj->ptr = LN;

			//	}
			//
			//}
		
			//node->children.emplace_back(NLN);
			//Cj->ptr = NLN;
			//q.push(NLN);

			/*if (LN->cluster->number < fix_threshold) {
				LN->cluster->Users.emplace_back(std::tuple(U->dknn, U));
			}*/
		}


	}

	else {
		auto NLN = dynamic_cast<NonLeafNode*>(node);
		auto& Cp = NLN->clusters;

		float d;
		int clust;
		auto minDist = std::numeric_limits<float>::max();

		for (long j = 0; j < Cp.size(); j++) {
			d = dist(U, Cp[j]);

			if (d < minDist) {
				minDist = d;
				clust = j;
			}
		}

		/*Cp[clust]->number += 1;*/

		if (Cp[clust]->radius < minDist) {
			Cp[clust]->radius = minDist;
		}

		/*parent = dynamic_cast<NonLeafNode*>(NLN);*/

		updateUser(NLN->children[clust], U);
	}
}

float HDR_Tree::dist(Item* I, Cluster* C) {

	numDistanceComputions++;
	float res = (iTrans[C->l - 1] - C->center).norm();
	return res - C->radius;
}
float HDR_Tree::dist(User* U, Cluster* C) {

	numDistanceComputions++;
	float res = (uTrans[C->l - 1] - C->center).norm();
	return res - C->radius;
}
float HDR_Tree::dist(Item* I, User* U) {
	numDistanceComputions++;
	float res = (U->center - (I->v)).norm();
	return res;

}
float HDR_Tree::dist(VectorRef A, VectorRef B) {

	if ((A.rows() != B.rows()) ||
		(A.cols() != B.cols())) {
		std::cout << "Error : float HDR_Tree::dist(VectorRef A, VectorRef B) : Dimension Mismatch" << std::endl;
		std::cout << " A : " << A.rows() << "x" << A.cols() << std::endl;
		std::cout << " B : " << B.rows() << "x" << B.cols() << std::endl;
	}


	numDistanceComputions++;
	float res = (A - B).norm();
	return res;
}




void HDR_Tree::init() {

	data->transformData(numFeatures, checkDuplicates);


//Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA;

    //data->generateUser(100000);

//Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA;

//Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA;

    data->generate_general_set();



    data->generate_user_set();
    data->generate_pca_matrix();

//Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA;


    data->computePCA();

	/*for (long i = 0; i < windowSize; i++) {
//		std::cout << i << "/" << data->D.size() << std::endl;
		auto dat = data->D[i];
		Item* I = new Item(dat);
		slidingWindow.emplace_back(I);
	}*/

//switch_slidingwindow switch_slidingwindow switch_slidingwindow switch_slidingwindow switch_slidingwindow switch_slidingwindow switch_slidingwindow switch_slidingwindow





    /*for (long i = 0; i < windowSize; i++) {                              //Be careful, change the mode of filling the sliding window
//		std::cout << i << "/" << data->D.size() << std::endl;
        auto dat = data_i->I.row(i);
        Item* I = new Item(dat);
        slidingWindow.emplace_back(I);
    }*/


    //Be careful, there is a loop to valid the slidingwindow here originally, the loop is moved to the init_i function now.




//switch_slidingwindow switch_slidingwindow switch_slidingwindow switch_slidingwindow switch_slidingwindow switch_slidingwindow switch_slidingwindow

	for (long i = 0; i < numUsers; i++) {
		auto usr = new User(data->U.row(i), k);
		Users.emplace_back(usr);
	}

	//Be careful, here we initialize the low dimensional version of the users and items, the codes are moved to construct and construct_i.







    //Be careful, here we initialize the low dimensional version of the users and items, the codes are moved to construct and construct_i.



	add_user_num=0;   //Be careful, this is for recording and calculating the number if users added

//	initUsersSerial();

    std::chrono::steady_clock::time_point begin_clock = std::chrono::steady_clock::now();

	computation_of_distance_init=0;
//---------------------------------------do not use thread--------------------------------do not use thread-----------------------------




//---------------------------------------do not use thread--------------------------------do not use thread------------------------------
    distance_time=0;
    //int trace_no_user=0;

    //int count_user=1;

    for(int i=0; i<Users.size(); i++){
        //Users[i]->computeKNN(slidingWindow);
        //
        //userComputeKNN(itemRoot_i, Users[i]);
        //std::cout<<trace_no_user<<std::endl;

        //std::cout<<"Start calculating the knn list of the "<<count_user<<"th user"<<std::endl;

        //std::cout<<" "<<std::endl;

        user_computeknn_id(Users[i], 0.02,0.02, false);

        //std::cout<<"Complete calculating the knn list of the "<<count_user<<"th user"<<std::endl;

        //std::cout<<" "<<std::endl;

        //count_user=count_user+1;
        //trace_no_user=trace_no_user+1;
        //Users[i]->computeKNN_sort(slidingWindow);
    }

    /*int count_u=1;

    for(auto u: Users){
        std::cout<<"The knn list of the "<<count_u<<"th user"<<std::endl;
        for(auto i:u->R){
            std::cout<<i->v<<std::endl;
            std::cout<<" "<<std::endl;
        }
        count_u=count_u+1;
    }*/





//---------------------------------------use thread---------------------------------------use thread--------------------------------------
	//initUsersParalell();
//---------------------------------------use thread---------------------------------------use thread--------------------------------------
    std::chrono::steady_clock::time_point end_clock = std::chrono::steady_clock::now();
    float diff = std::chrono::duration_cast<std::chrono::milliseconds>(end_clock - begin_clock).count();
    diff = diff / 1000.0f;
    printf("Initialization Time = %f \n", diff);

    init_time=diff;

	itemIndex = windowSize;

//	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
//	float diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
//	diff = diff / 1000.0f;

//	printf("Cluster Initialized in %i minuites, %.3f seconds \n", int(diff / 60), diff - 60.0f * int(diff / 60.0f));

}

void HDR_Tree::remove() {

	auto L = slidingWindow.back();
	slidingWindow.pop_back();
	update(L, Mode::Remove);
}

void HDR_Tree::insert(Item* I) {

//	std::chrono::steady_clock::time_point tA = std::chrono::steady_clock::now();

	if (slidingWindow.size() == windowSize) {

		remove();
	}

//	std::cout << "Computed Distance (Remove) " << numDistanceComputions << " times" << std::endl;
//	std::cout << "Computed KNN (Remove) " << numKnnComputions << " times" << std::endl;

//	std::chrono::steady_clock::time_point tB = std::chrono::steady_clock::now();

	slidingWindow.emplace(slidingWindow.begin(), I);

//	std::chrono::steady_clock::time_point tC = std::chrono::steady_clock::now();
	update(I, Mode::Add);

//	std::cout << "Computed Distance " << numDistanceComputions << " times" << std::endl;
//	std::cout << "Computed KNN " << numKnnComputions << " times" << std::endl;

//	std::chrono::steady_clock::time_point tD = std::chrono::steady_clock::now();

//	std::cout << "Remove Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tB - tA).count() << std::endl;
//	std::cout << "Emplace Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tC - tB).count() << std::endl;
//	std::cout << "Update Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tD - tC).count() << std::endl;

}
void HDR_Tree::userInsert(User* U) {

	userUpdate(U, Mode::Add);

}

void HDR_Tree::fillTree() {

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	for (long i = 0; i < windowSize; i++) {
		Item* I = new Item(data->D[i]);
		insert(I);

		if (i % 1000 == 0) {

			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			float diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
			diff = diff / 1000.0f;

			std::cout << "[" << diff << "] Processing Item " << i << std::endl;
		}
	}
}

float HDR_Tree::AdjustMaxdknnPrev(Node* n) {

	float Dist = 0;

	if (n->type == NodeType::LeafNode) {
		auto LN = dynamic_cast<LeafNode*>(n);
		for (auto Ut : *LN) {
			auto user = std::get<1>(Ut);
			auto D = user->dknn;
			Dist = std::max(Dist, D);
		}
	}
	else {
		auto NLN = dynamic_cast<NonLeafNode*>(n);
		auto Cp = NLN->clusters;

		for (auto Cj : Cp) {
			auto D = AdjustMaxdknnPrev(Cj->ptr);
			Dist = std::max(Dist, D);
		}
	}

	return Dist;
}
bool HDR_Tree::AdjustMaxdknnPaper(Node* n) {
	
	bool result = false;

	if (n->type == NodeType::LeafNode) {
		auto LN = dynamic_cast<LeafNode*>(n);

		long Len = Rp.size();
		for (auto Ut : *LN) {
			auto& user = std::get<1>(Ut);
			for (long i = 0; i < Len; i++) {
				if (Rp[i] == user) {
					return true;
				}
			}
		}
	}
	else {
		auto NLN = dynamic_cast<NonLeafNode*>(n);
		auto& Cp = NLN->clusters;

		for (auto Cj : Cp) {
			bool res = AdjustMaxdknnPaper(Cj->ptr);

			if (res) {
				result = true;

				float Dist = 0;

				if (Cj->ptr->type == NodeType::LeafNode) {

					auto LN = dynamic_cast<LeafNode*>(Cj->ptr);
					for (auto Ut : *LN) {
					
						auto user = std::get<1>(Ut);
						Dist = std::max(Dist, user->dknn);
					}
				}
				else {

					auto NLN = dynamic_cast<NonLeafNode*>(Cj->ptr);
					auto Cp = NLN->clusters;

					for (auto Cj : Cp) {
						Dist = std::max(Dist, Cj->maxdknn);
					}
				}

				Cj->maxdknn = Dist;
			}
		}
	}

	return result;
}

void HDR_Tree::Search(VectorRef V) {
	Item* I = new Item(V);

	Rp.clear();
	Search(root, I);
}

void HDR_Tree::initUsersSerial() {

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	for (long i = 0; i < Users.size(); i++) {
		Users[i]->computeKNN(slidingWindow);

		if (i % 1000 == 0) {

			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			float diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
			diff = diff / 1000.0f;

			std::cout << "[" << diff << "] Processing User " << i << std::endl;
		}
	}
}
void HDR_Tree::initUsersParalell() {

	int n = std::thread::hardware_concurrency();

	std::cout << "Using " << n << " paralell threads to initialize Users" << std::endl;

	std::vector<std::thread> threads;
	begin = std::chrono::steady_clock::now();

	for (long i = 0; i < n; i++) {
		threads.push_back(std::thread(
			&HDR_Tree::UsersParalell, this, i, n
		));
	}

	for (auto& th : threads) {
		th.join();
	}
}
void HDR_Tree::UsersParalell(int o, int n) {

	for (long i = o; i < Users.size(); i = i + n) {
        //-------------------------------------------------------------------------------//
        /*std::cout<<"check the root of the item hdr tree"<<std::endl;
        std::cout<<itemRoot_ptr<<std::endl;
        std::cout<<"check success"<<std::endl;*/
        //-------------------------------------------------------------------------------//
		userComputeKNN(itemRoot_i, Users[i]);   //test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_
        //Users[i]->computeKNN(slidingWindow);     //test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_
	}

	//for (long i = o; i < Users.size(); i=i+n) {
	//	/*Users[i]->computeKNN(slidingWindow);*/
	//	Users[i]->userComputeKNN(itemRoot_ptr);

	//	if (i % (n*250) == 0) {

	//		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	//		float diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	//		diff = diff / 1000.0f;

	//		std::cout << "[" << diff << "] Processing User " << i << std::endl;
	//	}
	//}
}

void HDR_Tree::userComputeKNN(itemNode* node, User* user) { // v1

	/*int clust;

	if (node->type == itemNodeType::itemLeafNode) {
		std::vector<std::tuple<float, Item*>> V;

		auto LN = dynamic_cast<itemLeafNode*>(node);
		long maxL = 0;

		float D;
		for (auto It : *LN) {
			auto item = std::get<1>(It);

			D = dist(user, item);

			std::tuple<float, Item*> T(D, item);
			V.emplace_back(T);

		}

		std::sort(V.begin(), V.end(),
			[&](const std::tuple<float, Item*>& a, const std::tuple<float, Item* >& b) -> bool {
				return std::get<0>(a) < std::get<0>(b);
			});

		for (int i = 0; i < k; i++) {
			user->R.emplace_back(std::get<1>(V[i]));
		}
	}
	else {
		auto NLN = dynamic_cast<itemNonLeafNode*>(node);
		auto& Cp = NLN->clusters;

		float D;
		auto minDist = std::numeric_limits<float>::max();

		for (long j = 0; j < Cp.size(); j++) {
			D = dist(user, Cp[j]);

			if (D < minDist) {
				minDist = D;
				clust = j;
			}
		}
		userComputeKNN(NLN->children[clust], user);
	}*/

	//test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_
    //projectUser_item(user);

    project_u_iclust_pca_init(user);     //generate the vector for user including every low dimension form for the user using item pca
                                         //Be careful//Be careful//Be careful//Be careful//Be careful//Be careful//Be careful//Be careful

    auto ini_iNLN = dynamic_cast<itemNonLeafNode*>(node);
    user->R.clear();
    std::vector<std::tuple<float, itemNonLeafNode*>> V;
    float upbound=std::numeric_limits<float>::max();
    std::vector<std::tuple<float, Item*>> d_i;
    float ini_dist=1;
    V.emplace_back(std::make_tuple(ini_dist, ini_iNLN));
    std::tuple<float, itemNonLeafNode*> pop_node;
    while(V.empty()==0){
        if(V.size()==1){
            pop_node=V.back();
            V.pop_back();
        }else{
            std::nth_element(V.begin(), V.begin() + V.size() - 1, V.end(),
                             [&](const std::tuple<float, itemNonLeafNode*>& a, const std::tuple<float, itemNonLeafNode* >& b) -> bool {
                                 return std::get<0>(a) >= std::get<0>(b);
                             });
            pop_node=V.back();
            V.pop_back();
        }
        if((std::get<0>(pop_node))>upbound){
            break;
        }else{
            for(auto C: std::get<1>(pop_node)->clusters){
                //float dist_r=dist_u_ic_str(user,C)-(C->radius);
                //computation_of_distance_init=computation_of_distance_init+1;   //Be careful here, maybe negative
// an optional from of distance calculation an optional from of distance calculation an optional from of distance calculation

                float dist_r=dist_u_iclust_init(C)-(C->radius);

// an optional from of distance calculation an optional from of distance calculation an optional from of distance calculation
                if(dist_r<=upbound){
                    if((C->ptr)->type==itemNodeType::itemLeafNode){
                        for(auto I: C->Items){
                            //float distance=dist(std::get<1>(I), user); Item* ite=std::get<1>(I);
                            //std::tuple<float, Item*>tuple_d_i =std::make_tuple(distance, ite);
                            d_i.emplace_back(std::make_tuple(dist(std::get<1>(I), user), std::get<1>(I)));
                            //computation_of_distance_init=computation_of_distance_init+1;    //Be careful here, maybe negative
                        }
                        if(d_i.size()>=k){
                            std::nth_element(d_i.begin(), d_i.begin() + k - 1, d_i.end(),
                                             [&](const std::tuple<float, Item*>& a, const std::tuple<float, Item*>& b) -> bool {
                                                 return std::get<0>(a) <= std::get<0>(b);
                                             });
                            d_i.assign(d_i.begin(), d_i.begin()+k);
                            upbound=std::get<0>(d_i[k-1]);
                        }
                    }else{
                        V.emplace_back(std::make_tuple(dist_r, dynamic_cast<itemNonLeafNode*>(C->ptr)));
                    }
                }
            }
        }
    }
    for(auto di: d_i){
        user->R.emplace_back(std::get<1>(di));
        user->dknn=std::get<0>(d_i[k-1]);
    }


    //test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_test_0_
}

float HDR_Tree::dist(User* U, itemCluster* C) { // v1 for User and Cluster

//	D = (I->v - center).squaredNorm();
	return (U->center - C->center).norm();
 
}

float HDR_Tree::dist(User* U, Item* I)
{
	return (U->center - (I->v)).norm();

}

//void User::dist(Item* I, float& D) { // v0 for any system
//
////	D = (I->v - center).squaredNorm();
//	D = (I->v - center).norm();
//
//	return;
//}

void HDR_Tree::streamIndex(long ind) {

//	std::chrono::steady_clock::time_point tA = std::chrono::steady_clock::now();

//	std::cout << "Streaming" << std::endl;
	Item* I = new Item(data->D[ind]);

//	std::chrono::steady_clock::time_point tB = std::chrono::steady_clock::now();

	insert(I);

//	std::chrono::steady_clock::time_point tC = std::chrono::steady_clock::now();

	//	std::cout << "Create Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tB - tA).count() << std::endl;
	//	std::cout << "Insert Time:" << std::chrono::duration_cast<std::chrono::microseconds>(tC - tB).count() << std::endl;
	//	std::cout << "------------------------------------------------------------" << std::endl;
}
void HDR_Tree::userStreamIndex(long ind) {

	User* U = new User(data->D[ind],k);
	userInsert(U);

}
void HDR_Tree::streamRandom() {

	/* Seed */
	std::random_device rd;

	/* Random number generator */
	std::default_random_engine generator(rd());

	/* Distribution on which to apply the generator */
	std::uniform_int_distribution<long unsigned> distribution(0, data->D.size() - 1);

	streamIndex(distribution(generator));
}
void HDR_Tree::userStreamRandom() {

	/* Seed */
	std::random_device rd;

	/* Random number generator */
	std::default_random_engine generator(rd());

	/* Distribution on which to apply the generator */
	std::uniform_int_distribution<long unsigned> distribution(0, data->D.size() - 1);

	userStreamIndex(distribution(generator));
}
void HDR_Tree::streamLinear() {

	streamIndex(itemIndex);
	itemIndex++;
}
void HDR_Tree::stream() {
	streamLinear();
	/*streamRandom();*/
}

void HDR_Tree::userStream() {
	/*streamLinear();*/
	userStreamRandom();
}


void HDR_Tree::syncUsers(int o, int n) {

	for (auto u : Rp) {
	
		if (u->R.size() < k) {
			u->computeKNN(slidingWindow);
		}
	}
}
void HDR_Tree::sync() {

//	std::cout << "Syncing " << Rp.size() << " Users. " << std::endl;

	for (auto u : Rp)
	{
		if (u->R.size() < k) {
			u->computeKNN(slidingWindow);
		}
	}

	Rp.clear();
}

//item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function
float HDR_Tree::dist_i(Item* I, itemCluster* C) {

    numDistanceComputions_i++;
    float res = (iTrans_i[C->l - 1] - C->center).norm();
    return res - C->radius;
}

float HDR_Tree::dist_i(User* U, itemCluster* C) {

    numDistanceComputions_i++;
    float res = (uTrans_i[C->l - 1] - C->center).norm();
    return res - C->radius;
}

float HDR_Tree::dist_i(Item* I, User* U) {
    numDistanceComputions_i++;
    float res = (U->center - I->v).norm();
    return res;
}

float HDR_Tree::dist_i(VectorRef A, VectorRef B) {

    if ((A.rows() != B.rows()) ||
        (A.cols() != B.cols())) {
        std::cout << "Error : float itemHDR_Tree::dist(VectorRef A, VectorRef B) : Dimension Mismatch" << std::endl;
        std::cout << " A : " << A.rows() << "x" << A.cols() << std::endl;
        std::cout << " B : " << B.rows() << "x" << B.cols() << std::endl;
    }


    numDistanceComputions_i++;
    float res = (A - B).norm();
    return res;
}

void HDR_Tree::projectItem_i(Item* I) {

    iTrans_i.clear();
    for (int i = 0; i < L_i; i++) {
        iTrans_i.emplace_back(data_i->T[i] * I->v);
    }
}

void HDR_Tree::setData_i(itemDataBase* dataBase) {
    data_i = dataBase;
}

void HDR_Tree::init_i() {

    data_i->transformData(numFeatures_i, checkDuplicates_i);


//Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA;

    //data_i->generateItem(200000);    //Be careful, this value N is set to be a very large number which is much larger than the window size/item number.

//Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA; Different PCA;

//Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA;

    data_i->generate_general_set();

    //std::cout<<"check_0"<<std::endl;

    data_i->generate_item_set();

    //std::cout<<"check_1"<<std::endl;

    data_i->generate_pca_matrix();

//Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA; Same PCA;

    data_i->computePCA();

    for (long i = 0; i < numItems_i; i++) {
        auto itm = new Item(data_i->I.row(i));
        Items_i.emplace_back(itm);
    }

    for (long i = 0; i < Items_i.size(); i++) {                //Be careful, change the mode of filling the sliding window, and this loop is moved from the init function
//		std::cout << i << "/" << data->D.size() << std::endl;
        slidingWindow.emplace_back(Items_i[i]);
    }

    add_item_num=0;        //Be careful, this is the number from recording and calculating the number of the items added





    /*itemIndex = windowSize;*/

    //	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    //	float diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    //	diff = diff / 1000.0f;

    //	printf("Cluster Initialized in %i minuites, %.3f seconds \n", int(diff / 60), diff - 60.0f * int(diff / 60.0f));

}


void HDR_Tree::construct_i(long fanout, long threshold) {

    std::cout << "Constructing HDR Tree for Items" << std::endl;
    std::cout << "Items: " << numItems_i << std::endl;
    std::cout << "Features: " << numFeatures_i << std::endl;
    std::cout << "Fanout: " << fanout << std::endl << std::endl;

    init_i();

    L_i = log(data_i->I.size()) / log(fanout);
    std::cout << "Estimated Height: " << L_i << " levels." << std::endl;

    data_i->initTransform(L_i);

//Be careful, here, the low dimensional versions of the items are produced and stored in the specific containers of the items, the codes are moved to the construction function for the user tree



//Be careful, here, the low dimensional versions of the items are produced and stored in the specific containers of the items, the codes are moved to the construction function for the user tree

    long numClusters_i = 0;

    auto first_layer_itemclusters=new std::vector<itemCluster*>;

    auto C = new itemClusters(Items_i, fanout, data_i->T[0]);

    long sequense_item_cluster_i_layer=0;

    for (auto Cj : *C) {

        numClusters_i += 1;
        Cj->l = 1;

        Cj->i_cluster_sequense=sequense_item_cluster_i_layer;
        sequense_item_cluster_i_layer=sequense_item_cluster_i_layer+1;
        (*first_layer_itemclusters).emplace_back(Cj);
    }

    //std::cout<<"check_0"<<std::endl;

    itemRoot_i = new itemNonLeafNode(C, 1, false);

    itemRoot_ptr_i = itemRoot_i;

    dictionary_for_layer_itemcluster.emplace_back(first_layer_itemclusters);

    auto a_sequense=new int;

    *a_sequense=(*first_layer_itemclusters).size();

    record_sequence.emplace_back(a_sequense);

    std::queue<itemNode*> q_i;

    q_i.push(itemRoot_i);

    long maxL_i = 1;

    while (!q_i.empty()) {
        auto node_i = dynamic_cast<itemNonLeafNode*>(q_i.front());
        q_i.pop();

        auto Cp_i = node_i->clusters;

        for (auto Cj_i : Cp_i) {

            if (Cj_i->number < threshold) {
                auto LN_i = new itemLeafNode(Cj_i, false);

                node_i->children.emplace_back(LN_i);
                Cj_i->ptr = LN_i;
            }
            else {
                auto level_i = node_i->l + 1;

                if(level_i>maxL_i){
                    auto b_sequense=new int;
                    *b_sequense=0;
                    record_sequence.emplace_back(b_sequense);
                    auto middle_layer_itemclusters=new std::vector<itemCluster*>;
                    dictionary_for_layer_itemcluster.emplace_back(middle_layer_itemclusters);
                }

                maxL_i = std::max(maxL_i, level_i);

                if(level_i>L_i){
                    std::cout<<"Excceed the estimated level"<<std::endl;
                }

                auto Cpp_i = new itemClusters(Cj_i, fanout, data_i->T[level_i - 1]);
                for (auto Cj_i : *Cpp_i) {

                    numClusters_i += 1;
                    Cj_i->l = level_i;
                    dictionary_for_layer_itemcluster[level_i-1]->emplace_back(Cj_i);
                    Cj_i->i_cluster_sequense=(*record_sequence[level_i-1]);
                    (*record_sequence[level_i-1])=(*record_sequence[level_i-1])+1;
                }

                auto NLN_i = new itemNonLeafNode(Cpp_i, level_i, false);

                node_i->children.emplace_back(NLN_i);
                Cj_i->ptr = NLN_i;
                q_i.push(NLN_i);
            }
        }
    }

    std::cout << "Actual Height: " << maxL_i << " levels." << std::endl;
    std::cout << "Clusters: " << numClusters_i << std::endl;

//Print the number of clusters in each level; Print the number of clusters in each level; Print the number of clusters in each level; Print the number of clusters in each level;

    /*for(auto v:dictionary_for_layer_itemcluster){
        std::cout<<v->size()<<std::endl;
        for(auto c:*v){
            std::cout<<c->i_cluster_sequense<<std::endl;
        }
    }*/

//Print the number of clusters in each level; Print the number of clusters in each level; Print the number of clusters in each level; Print the number of clusters in each level;
}

void HDR_Tree::projectUser_item(User* U){
    Trans_u_i.clear();
    for(long i=0; i<L_i; i++){
        Trans_u_i.emplace_back(data_i->T[i] * U->center);
    }
}

float HDR_Tree::dist_u_ic(User* U, itemCluster* C){
    float dis = (Trans_u_i[C->l - 1] - C->center).norm();
    return dis;
}

float HDR_Tree::dist_u_ic_str(User* U, itemCluster* C){
    float dis=(data_i->T[C->l-1]*U->center - C->center).norm();
    return dis;
}

void HDR_Tree::update_user_hdr(User* U, Mode m, Node* node){
    if(m==Mode::Add){

        if(node->type==NodeType::NonLeafNode){
            //projectUser(U); be careful, project it outside and before this function is used
            auto NLN=dynamic_cast<NonLeafNode*>(node);

            //std::cout<<"Calculating the "<<NLN->l<<" level node"<<std::endl;

            std::vector<std::tuple<float, Cluster*>>V;
            auto v_transformed=data->T[NLN->l-1]*U->center;     //Be careful, here we in advanced calculate the low dimension form of the user 1
            for(auto C_0: NLN->clusters){
                V.emplace_back(std::make_tuple((v_transformed-C_0->center).norm(), C_0));   //Be careful, here is associated with the note 1
            }

            //std::cout<<"Complete computing the distances between the newly inserted user and the centers of the clusters in the node"<<std::endl;

            std::nth_element(V.begin(), V.begin() + V.size() - 1, V.end(),
                             [&](const std::tuple<float, Cluster*>& a, const std::tuple<float, Cluster* >& b) -> bool {
                                 return std::get<0>(a) >= std::get<0>(b);
                             });

            //std::cout<<"Complete sorting the distances between the newly inserted user and the centers of the clusters in the node"<<std::endl;

            auto C_add=std::get<1>(V[V.size()-1]);

            //std::cout<<"Catch the cluster that the new user should be added to"<<std::endl;

            C_add->Users.emplace_back(std::make_tuple(std::get<0>(V[V.size()-1]), U));

            //std::cout<<"Complete adding the user into the proper node"<<std::endl;

            C_add->number=C_add->number+1;

            //std::cout<<"The current number of users "<<C_add->number<<" in the catched cluster after adding the new user"<<std::endl;

            std::sort(C_add->Users.begin(),  C_add->Users.end(),
                             [&](const std::tuple<float, User*>& a, const std::tuple<float, User*>& b) -> bool {
                                 return std::get<0>(a) <= std::get<0>(b);
                             });
            C_add->radius=std::get<0>(C_add->Users[(C_add->Users).size()-1]);
            if (U->dknn>C_add->maxdknn){
                C_add->maxdknn=U->dknn;
            }
            update_user_hdr( U, m, C_add->ptr);
        }
    }else{
        if(node->type==NodeType::NonLeafNode){
            auto NLN=dynamic_cast<NonLeafNode*>(node);
            //std::cout<<"1"<<std::endl;   //show show show show show show show show
            Cluster* C_catch;

            //std::cout<<"The original state of the C_catch "<<(C_catch->Users).size()<<std::endl;   //show show show show show show show show

            long i_catch;
            for(auto C:NLN->clusters){
                if(C->number!=0){
                    //std::cout<<"Not zero"<<std::endl;         //show show show show show show show
                    bool indicator= false;
                    for(long i=0; i<(C->Users).size(); i++){
                        if(std::get<1>(C->Users[i])==U){


                            C_catch=C;

                            //std::cout<<"catch success"<<std::endl;   //show show show show show show show

                            i_catch=i;
                            indicator= true;
                            break;
                        }
                    }
                    if(indicator==true){
                        break;
                    }
                }
            }

            //std::cout<<"2"<<std::endl;//show show show show show show show show
            //std::cout<<(C_catch->Users).size()<<" 3"<<std::endl;  //show show show show show show show show
            C_catch->number=C_catch->number-1;
            if(C_catch->number==0){
                C_catch->radius=0;
                C_catch->maxdknn=0;
                C_catch->Users.erase(C_catch->Users.begin()+i_catch);
            }else{
                //std::cout<<"4"<<std::endl;//show show show show show show show show
                //std::cout<<(C_catch->Users).size()<<" 5"<<std::endl;  //show show show show show show show show
                C_catch->Users.erase(C_catch->Users.begin()+i_catch);
                //std::cout<<"6"<<std::endl;//show show show show show show show show
                //std::cout<<(C_catch->Users).size()<<" 7"<<std::endl;   //show show show show show show show show
                C_catch->radius=std::get<0>(C_catch->Users[C_catch->Users.size()-1]);
                //std::cout<<"8"<<std::endl;//show show show show show show show show
                if(U->dknn==C_catch->maxdknn){
                    std::vector<float> V;
                    for(auto tp: C_catch->Users){
                        V.emplace_back(std::get<1>(tp)->dknn);
                    }
                    std::nth_element(V.begin(), V.begin() + V.size() - 1, V.end(),
                                     [&](const float& a, const float& b) -> bool {
                                         return a<=b;
                                     });
                    C_catch->maxdknn=V[V.size()-1];
                }
            }

            update_user_hdr( U, m, C_catch->ptr);
        }
    }
}

void HDR_Tree::project_u_iclust_pca_init(User* U){
    u_trans_i_clus_init.clear();
    for(int i=0;i<L_i;i++){
        u_trans_i_clus_init.emplace_back((data_i->T[i])*(U->center));
    }
}


float HDR_Tree::dist_u_iclust_init(itemCluster* C){
    float distance;
    distance=(u_trans_i_clus_init[C->l-1]-C->center).norm();
    return distance;
}


float HDR_Tree::dist_i_ic_update_itree(Item* I, itemCluster* IC){
    float distance_ic=(data_i->T[IC->l-1]*I->v-IC->center).norm();
    return distance_ic;
}

float HDR_Tree::dist_u_uc_update_utree(User* U, Cluster* UC){
    float distance_uc=(data->T[UC->l-1]*U->center-UC->center).norm();
    return distance_uc;
}


void HDR_Tree::update_item_hdr(itemNode* root_item_tree, Item* I, Mode mode){
    if(mode==Mode::Add){
        if(root_item_tree->type==itemNodeType::itemNonLeafNode){
            auto root_i=dynamic_cast<itemNonLeafNode*>(root_item_tree);
            itemCluster* catch_c;
            std::vector<std::tuple<float, itemCluster*>> V_d_c;
            auto v_i_transformed=data_i->T[root_i->l-1]*I->v;  //Be careful, here we calculate the low dimension form of the item in advanced
            for(auto C:root_i->clusters){
                float distance_i_c=(v_i_transformed-C->center).norm();    //Be careful. here is associated with the note 1.
                V_d_c.emplace_back(std::make_tuple(distance_i_c, C));
            }
            std::nth_element(V_d_c.begin(), V_d_c.begin() + V_d_c.size() - 1, V_d_c.end(),
                             [&](const std::tuple<float, itemCluster*>& a, const std::tuple<float, itemCluster*>& b) -> bool {
                                 return std::get<0>(a) >= std::get<0>(b);
                             });
            catch_c=std::get<1>(V_d_c[V_d_c.size()-1]);
            catch_c->number=catch_c->number+1;
            catch_c->Items.emplace_back(std::make_tuple(std::get<0>(V_d_c[V_d_c.size()-1]), I));
            std::nth_element(catch_c->Items.begin(), catch_c->Items.begin() + catch_c->Items.size() - 1, catch_c->Items.end(),
                             [&](const std::tuple<float, Item*>& a, const std::tuple<float, Item*>& b) -> bool {
                                 return std::get<0>(a) <= std::get<0>(b);
                             });
            catch_c->radius=std::get<0>(catch_c->Items[catch_c->Items.size()-1]);
            update_item_hdr(catch_c->ptr,  I, mode);
        }
    }else{
        if(root_item_tree->type==itemNodeType::itemNonLeafNode){
            auto root_i_reduce=dynamic_cast<itemNonLeafNode*>(root_item_tree);
            itemCluster* catch_c_reduce;
            int catch_no_i;
            for(auto C_c_r: root_i_reduce->clusters){
                if(C_c_r->Items.size()!=0){
                    bool indicator_c_if= false;
                    for(int i=0; i<C_c_r->Items.size();i++){
                        if(std::get<1>(C_c_r->Items[i])==I){
                            //std::cout<<"Catch successfully"<<std::endl;
                            catch_no_i=i;
                            catch_c_reduce=C_c_r;
                            indicator_c_if= true;
                            break;
                        }
                    }
                    if(indicator_c_if== true){
                        break;
                    }
                }
            }
            catch_c_reduce->Items.erase(catch_c_reduce->Items.begin()+catch_no_i);
            catch_c_reduce->number=catch_c_reduce->number-1;
            if(catch_c_reduce->number==0){
                catch_c_reduce->radius=0;
            }else{
                std::nth_element(catch_c_reduce->Items.begin(), catch_c_reduce->Items.begin() + catch_c_reduce->Items.size() - 1, catch_c_reduce->Items.end(),
                                 [&](const std::tuple<float, Item*>& a, const std::tuple<float, Item*>& b) -> bool {
                                     return std::get<0>(a) <= std::get<0>(b);
                                 });
                catch_c_reduce->radius=std::get<0>(catch_c_reduce->Items[catch_c_reduce->Items.size()-1]);
            }
            update_item_hdr(catch_c_reduce->ptr, I, mode);
        }
    }
}

void HDR_Tree::update_user(){
    auto u_out=Users[0];
    Users.erase(Users.begin());
    update_user_hdr(u_out,Mode::Remove, root);


    /*auto u_out=Users[0];
    Users.erase(Users.begin());*/
    //update_user_hdr(u_out,Mode::Remove, root);
}

void HDR_Tree::update_item(){
    auto i_out=slidingWindow[0];
    slidingWindow.erase(slidingWindow.begin());
    update_item_hdr(itemRoot_i,i_out,Mode::Remove);

    Rp.clear();

    projectItem(i_out);

    Search(root, i_out);

    if(Rp.size()>0){

        //std::cout<<Rp.size()<<std::endl;

        for(auto u:Rp){
            userComputeKNN(itemRoot_i, u);
        }

        AdjustMaxdknnPaper(root);
    }





    /*auto i_out=slidingWindow[0];
    slidingWindow.erase(slidingWindow.begin());*/
    /*update_item_hdr(itemRoot_i,i_out,Mode::Remove);

    Rp.clear();

    projectItem(i_out);

    Search(root, i_out);

    if(Rp.size()>0){
        for(auto u:Rp){
            userComputeKNN(itemRoot_i, u);
        }
    }

    AdjustMaxdknnPaper(root);*/

    /*for(auto u:Users){
        u->computeKNN(slidingWindow);
    }*/

}

void HDR_Tree::update_user_add(){

    auto new_user= new User(data->U.row(numUsers+add_user_num), k);

    //std::cout<<"Complete producing user"<<std::endl;

    userComputeKNN(itemRoot_i, new_user);



    Users.emplace_back(new_user);



    update_user_hdr(new_user, Mode::Add, root);

    add_user_num=add_user_num+1;

    /*auto new_user= new User(data->U.row(numUsers+add_user_num), k);
    new_user->computeKNN(slidingWindow);
    Users.emplace_back(new_user);
    //update_user_hdr(new_user, Mode::Add, root);

    add_user_num=add_user_num+1;*/


}

void HDR_Tree::update_item_add() {
    auto new_item = new Item(data_i->I.row(numItems_i + add_item_num));
    slidingWindow.emplace_back(new_item);
    update_item_hdr(itemRoot_i, new_item, Mode::Add);

    Rp.clear();

    projectItem(new_item);

    Search(root, new_item);

    if (Rp.size() > 0) {

        //std::cout<<Rp.size()<<std::endl;

        for (auto u:Rp) {
            u->update(new_item, Mode::Add);
        }

        AdjustMaxdknnPaper(root);

    }




    add_item_num = add_item_num + 1;

}
    /*auto new_item= new Item(data_i->I.row(numItems_i+add_item_num));
    slidingWindow.emplace_back(new_item);*/
    /*update_item_hdr(itemRoot_i,new_item, Mode::Add);

    Rp.clear();

    projectItem(new_item);

    Search(root, new_item);

    if(Rp.size()>0){
        for(auto u:Rp){
            u->update(new_item, Mode::Add);
        }
    }

    AdjustMaxdknnPaper(root);

    for(auto u:Users){
        u->computeKNN(slidingWindow);
    }


    add_item_num=add_item_num+1;*/
//};

void HDR_Tree::update_user_user_tree(){
    auto u_out=Users[0];
    Users.erase(Users.begin());
    update_user_hdr(u_out,Mode::Remove, root);
};

void HDR_Tree::update_user_add_user_tree(){
    auto new_user= new User(data->U.row(numUsers+add_user_num), k);
    new_user->computeKNN(slidingWindow);
    Users.emplace_back(new_user);
    update_user_hdr(new_user,Mode::Add, root);
    add_user_num=add_user_num+1;
};

void HDR_Tree::update_item_user_tree(){
    auto item_out=slidingWindow[0];
    slidingWindow.erase(slidingWindow.begin());

    Rp.clear();

    //std::chrono::steady_clock::time_point begin_clock_item_add = std::chrono::steady_clock::now();  //For checking of the projection time, beginning

    //int test_0=2;

    projectItem(item_out);

    /*std::chrono::steady_clock::time_point end_clock_item_add = std::chrono::steady_clock::now();  //For checking of the projection time, ending

    int test_1=3;

    projection_time=projection_time+test_1-test_0;//std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_item_add - begin_clock_item_add).count();   //Add the projection time

    std::chrono::steady_clock::time_point begin_clock_item_add_hdr_0 = std::chrono::steady_clock::now();  //For checking of the search time on hdr tree, beginning*/

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    //num_hdr_cal_clusters=0;
    //num_hdr_cal_users=0;

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    Search(root, item_out);

    /*std::cout<<"The number of influenced users "<<Rp.size()<<std::endl;

    std::cout<<"The dknn values before recomputing the knn lists of deletion"<<std::endl;

    for(auto user_before: Rp){
        std::cout<<user_before->dknn<<" ";
    }
    std::cout<<" "<<std::endl;*/

    //To print the times of calculation of the distance

    //std::cout<<"The times of calculation of the distance between the item and the clusters is "<<num_hdr_cal_clusters<<std::endl;
    //std::cout<<"The times of calculation of the distance between the item and the users is "<<num_hdr_cal_users<<std::endl;
    //std::cout<<" "<<std::endl;

    //To print the times of calculation of the distance

    /*std::chrono::steady_clock::time_point end_clock_item_add_hdr_0 = std::chrono::steady_clock::now();  //For checking of the search time on hdr tree, ending

    search_time_HDR=search_time_HDR+std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_item_add_hdr_0 - begin_clock_item_add_hdr_0).count();*/


    if(Rp.size()>0){
        for(auto u:Rp){
            u->computeKNN(slidingWindow);
        }
        AdjustMaxdknnPaper(root);
    }



    /*std::cout<<"The dknn values after recomputing the knn lists of deletion"<<std::endl;

    for(auto user_after:Rp){
        std::cout<<user_after->dknn<<" ";
    }

    std::cout<<" "<<std::endl;

    std::cout<<" "<<std::endl;*/

};

void HDR_Tree::update_item_add_user_tree(){
    auto new_item = new Item(data_i->I.row(numItems_i + add_item_num));
    slidingWindow.emplace_back(new_item);
    //update_item_hdr(itemRoot_i, new_item, Mode::Add);

    Rp.clear();

    projectItem(new_item);

    Search(root, new_item);

    if (Rp.size() > 0) {
        for (auto u:Rp) {
            u->update(new_item, Mode::Add);
        }
    }

    AdjustMaxdknnPaper(root);


    add_item_num = add_item_num + 1;
};


//item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function




//Functions for the idistance and B_plus tree indexing structure
void HDR_Tree::form_item_clusters(long f){
    clusters_item=new Clustersi(slidingWindow, f);
    std::vector<double> radius_clusters;
    for(auto ic: *clusters_item){
        radius_clusters.emplace_back(ic->radius);
    }
    auto mp=std::max_element(radius_clusters.begin(), radius_clusters.end());
    max_r_item_cluster=radius_clusters[mp-radius_clusters.begin()];
}

void HDR_Tree::indexing_item(){
    for(int i=0; i<clusters_item->children.size();i++){
        //clusters_item->children[i]->sequece=i;
        for(auto I_T: clusters_item->children[i]->Items){
            std::get<1>(I_T)->cluster_i=i;
            std::get<1>(I_T)->dis_to_i=std::get<0>(I_T);
            std::get<1>(I_T)->index_b_plus=double(std::get<1>(I_T)->cluster_i)*(max_r_item_cluster+0.2)+std::get<1>(I_T)->dis_to_i;
            //Be careful, here we add 1 on the max radius Be careful, here we add 1 on the max radius Be careful, here we add 1 on the max radius
            //Be careful, here we add 1 on the max radius Be careful, here we add 1 on the max radius Be careful, here we add 1 on the max radius
        }
    }
}

void HDR_Tree::build_bp_tree(){
    bptree_item=new BPlusTree;
};


void HDR_Tree::insert_key_point(double key, Item* it){
    bptree_item->Insert(key, it);
}


void HDR_Tree::complete_tree(){
    indexing_item();
    build_bp_tree();
    for(auto I:slidingWindow){
        insert_key_point(I->index_b_plus, I);
    }
}

double HDR_Tree::distance_ci_u(User* u,  Clusteri* ci){
    double distance=((u->center).cast<double>()-(ci->center)).norm();        //Be careful, change the type of the return value from float to double Be careful, change the type of the return value from float to double Be careful, change the type of the return value from float to double
    return distance;                                     //Be careful, change the type of the return value from float to double Be careful, change the type of the return value from float to double Be careful, change the type of the return value from float to double
}


void HDR_Tree::user_computeknn_id(User* u, double r_rate, double delta_r_rate, bool print_if){
    if(print_if==true){
        std::vector<double> dis_u_ci_list;
        std::cout<<" "<<std::endl;                                                        //Count switch optional Count switch optional Count switch optional Count switch optional
        std::cout<<"Begin printing the information for every cluster"<<std::endl;         //Count switch optional Count switch optional Count switch optional Count switch optional
        std::cout<<" "<<std::endl;                                                        //Count switch optional Count switch optional Count switch optional Count switch optional
        int cluster_no=0;

        for(auto ci:clusters_item->children){
            std::cout<<"No."<<cluster_no<<"    ";                                       //Count switch optional Count switch optional Count switch optional Count switch optional
            ci->in_in=0;
            ci->out_in=0;
            ci->general_in=0;
            ci->in_visited_flag= false;
            ci->out_visited_flag=false;
            ci->visited_flag=false;
            double distance_u_ci=distance_ci_u(u, ci);

            std::cout<<"distance to user="<<distance_u_ci<<"    ";                   //Count switch optional Count switch optional Count switch optional Count switch optional
            std::cout<<"radius of "<<cluster_no<<"="<<ci->radius<<"    ";            //Count switch optional Count switch optional Count switch optional Count switch optional
            std::cout<<"the number of keys="<<ci->number<<"    ";                    //Count switch optional Count switch optional Count switch optional Count switch optional

            ci->distance_user=distance_u_ci;
            if(distance_u_ci>=ci->radius){
                ci->out= true;
                std::cout<<"out"<<" ";                                               //Count switch optional Count switch optional Count switch optional Count switch optional
            }else{
                ci->out=false;
                std::cout<<"in"<<" ";                                                //Count switch optional Count switch optional Count switch optional Count switch optional
            }
            dis_u_ci_list.emplace_back(distance_u_ci);
            std::cout<<" "<<std::endl;                                               //Count switch optional Count switch optional Count switch optional Count switch optional
            cluster_no=cluster_no+1;
        }
        auto max_r=std::max_element(dis_u_ci_list.begin(), dis_u_ci_list.end());
        auto max_r_real=dis_u_ci_list[max_r-dis_u_ci_list.begin()];
        double r=(r_rate)*(max_r_real+max_r_item_cluster+0.1);  //Be careful, here we add 0.1 on the max radius Be careful, here we add 0.1 on the max radius Be careful, here we add 0.1 on the max radius
        double delta_r=delta_r_rate*(max_r_real+max_r_item_cluster+0.1);  //Be careful, here we add 0.1 on the max radius Be careful, here we add 0.1 on the max radius Be careful, here we add 0.1 on the max radius
        std::vector<std::tuple<double, Item*>> item_distance_list;
        std::vector<Item*> candidate_list;
        std::vector<Item*>* pointer_candidate=&(candidate_list);
        double distance_k=std::numeric_limits<double>::max();
        int time=-1;
        int no_ci;
        //std::cout<<" "<<std::endl;
        while(time<int((1.0-r_rate)/delta_r_rate)+1){
            /*if((item_distance_list.size()==k)&&(distance_k<r)){
                candidate_list.clear();
                for(no_ci=0; no_ci<(clusters_item->children).size(); no_ci++){
                    if(clusters_item->children[no_ci]->visited_flag==false){
                        if(clusters_item->children[no_ci]->out==true){
                            if((r>clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius)&&((r<clusters_item->children[no_ci]->distance_user))){
                                bptree_item->get_range_item(pointer_candidate,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in,
                                                            false, true, false);
                                clusters_item->children[no_ci]->general_in=r-(clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius);
                            }
                            if(r>=clusters_item->children[no_ci]->distance_user){
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in, true, true, false);
                                clusters_item->children[no_ci]->general_in=clusters_item->children[no_ci]->radius;
                                clusters_item->children[no_ci]->visited_flag=true;
                            }
                        }
                        else{
                            if(clusters_item->children[no_ci]->in_visited_flag==false){
                                if(r<clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, false, true, false);
                                    clusters_item->children[no_ci]->in_in=r;
                                }
                                if(r>=clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, true, true, false);
                                    clusters_item->children[no_ci]->in_in=clusters_item->children[no_ci]->distance_user;
                                    clusters_item->children[no_ci]->in_visited_flag=true;
                                }
                            }
                            if(clusters_item->children[no_ci]->out_visited_flag==false){
                                if(r<clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+r, false, true, false );
                                    clusters_item->children[no_ci]->out_in=r;
                                }
                                if(r>=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius, false, true, false );
                                    clusters_item->children[no_ci]->out_in=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user;
                                    clusters_item->children[no_ci]->out_visited_flag=true;
                                }
                            }
                            if((clusters_item->children[no_ci]->in_visited_flag==true)&&(clusters_item->children[no_ci]->out_visited_flag==true)){
                                clusters_item->children[no_ci]->visited_flag=true;
                            }
                        }
                    }
                }
                if(candidate_list.size()==0){
                    break;
                }
                else{
                    for(auto i:candidate_list){
                        double distance_item_user=dist_i(i, u);
                        distance_time=distance_time+1;
                        if(distance_item_user<distance_k){
                            item_distance_list.emplace_back(std::make_tuple(distance_item_user, i));
                        }
                    }
                    std::nth_element(item_distance_list.begin(), item_distance_list.begin() + k - 1, item_distance_list.end(),
                                     [&](const std::tuple<double, Item*>& a, const std::tuple<double, Item*>& b) -> bool {
                                         return std::get<0>(a) <= std::get<0>(b);
                                     });
                    item_distance_list.assign(item_distance_list.begin(), item_distance_list.begin()+k);
                    distance_k=std::get<0>(item_distance_list[k-1]);
                    break;
                }
            }
            else{
                candidate_list.clear();
                for(no_ci=0; no_ci<(clusters_item->children).size(); no_ci++){
                    if(clusters_item->children[no_ci]->visited_flag==false){
                        if(clusters_item->children[no_ci]->out==true){
                            if((r>clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius)&&((r<clusters_item->children[no_ci]->distance_user))){
                                bptree_item->get_range_item(pointer_candidate,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in,
                                                            false, true, false);
                                clusters_item->children[no_ci]->general_in=r-(clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius);
                            }
                            if(r>=clusters_item->children[no_ci]->distance_user){
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in, true, true, false);
                                clusters_item->children[no_ci]->general_in=clusters_item->children[no_ci]->radius;
                                clusters_item->children[no_ci]->visited_flag=true;
                            }
                        }
                        else{
                            if(clusters_item->children[no_ci]->in_visited_flag==false){
                                if(r<clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, false, true, false);
                                    clusters_item->children[no_ci]->in_in=r;
                                }
                                if(r>=clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, true, true, false);
                                    clusters_item->children[no_ci]->in_in=clusters_item->children[no_ci]->distance_user;
                                    clusters_item->children[no_ci]->in_visited_flag=true;
                                }
                            }
                            if(clusters_item->children[no_ci]->out_visited_flag==false){
                                if(r<clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+r, false, true, false );
                                    clusters_item->children[no_ci]->out_in=r;
                                }
                                if(r>=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius, false, true, false );
                                    clusters_item->children[no_ci]->out_in=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user;
                                    clusters_item->children[no_ci]->out_visited_flag=true;
                                }
                            }
                            if((clusters_item->children[no_ci]->in_visited_flag==true)&&(clusters_item->children[no_ci]->out_visited_flag==true)){
                                clusters_item->children[no_ci]->visited_flag=true;
                            }
                        }
                    }
                }
                if(candidate_list.size()!=0){
                    for(auto i:candidate_list){
                        double distance_item_user=dist_i(i, u);
                        distance_time=distance_time+1;
                        if(distance_item_user<distance_k){
                            item_distance_list.emplace_back(std::make_tuple(distance_item_user, i));
                        }
                    }
                    if(item_distance_list.size()>=k){
                        std::nth_element(item_distance_list.begin(), item_distance_list.begin() + k - 1, item_distance_list.end(),
                                         [&](const std::tuple<double, Item*>& a, const std::tuple<double, Item*>& b) -> bool {
                                             return std::get<0>(a) <= std::get<0>(b);
                                         });
                        item_distance_list.assign(item_distance_list.begin(), item_distance_list.begin()+k);
                        distance_k=std::get<0>(item_distance_list[k-1]);
                        if(distance_k<r){
                            break;
                        }
                    }
                }
                r=r+delta_r;
                time=time+1;
            }*/

// replacement one  replacement one  replacement one  replacement one  replacement one  replacement one  replacement one  replacement one  replacement one

            candidate_list.clear();   //Be careful to switch this sentense
            for(no_ci=0; no_ci<(clusters_item->children).size(); no_ci++){

                std::cout<<"No."<<no_ci<<"   "<<"radius="<<clusters_item->children[no_ci]->radius<<"   "<<"distance to user="<<clusters_item->children[no_ci]->distance_user<<"   "<<"The "<<time+2<<" time"<<"   "<<"search_r="<<r<<std::endl;

                if(clusters_item->children[no_ci]->visited_flag==false){
                    std::cout<<"The beginning flag of "<<no_ci<<" cluster for the "<<time+2<<" time search is false"<<std::endl;
                    if(clusters_item->children[no_ci]->out==true){
                        std::cout<<"Out   "<<"The beginning general in is "<<clusters_item->children[no_ci]->general_in<<std::endl;
                        std::cout<<"The contribution of the "<<no_ci<<" cluster is to be showed, it has "<<clusters_item->children[no_ci]->number<<" items"<<std::endl;
                        if((r>clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius)&&((r<clusters_item->children[no_ci]->distance_user))){
                            std::cout<<"Out, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in<<std::endl;
                            bptree_item->get_range_item(pointer_candidate,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in,
                                                        false, true, true);

                            clusters_item->children[no_ci]->general_in=r-(clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius);
                        }
                        if(r>=clusters_item->children[no_ci]->distance_user){
                            std::cout<<"Out, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in<<std::endl;
                            bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in, true, true, true);
                            clusters_item->children[no_ci]->general_in=clusters_item->children[no_ci]->radius;
                            clusters_item->children[no_ci]->visited_flag=true;
                        }
                        std::cout<<"The contribution has been showed above"<<std::endl;
                        std::cout<<"After the "<<time+2<<"th loop, the general in of the "<<no_ci<<" cluster is "<<clusters_item->children[no_ci]->general_in<<"   "<<"The visited flag is "<<clusters_item->children[no_ci]->visited_flag<<std::endl;
                    }
                    else{
                        std::cout<<"The "<<no_ci<<" cluster is in"<<std::endl;
                        if(clusters_item->children[no_ci]->in_visited_flag==false){
                            std::cout<<"The beginning inward flag of "<<no_ci<<" cluster for the "<<time+2<<" time search is false"<<std::endl;
                            std::cout<<"The beginning in_in is "<<clusters_item->children[no_ci]->in_in<<std::endl;
                            std::cout<<"The contribution of the inward part of the "<<no_ci<<" cluster is to be showed, it has "<<clusters_item->children[no_ci]->number<<" items"<<std::endl;
                            if(r<clusters_item->children[no_ci]->distance_user){
                                std::cout<<"In, inward, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in<<std::endl;
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, false, true, true);
                                clusters_item->children[no_ci]->in_in=r;
                            }
                            if(r>=clusters_item->children[no_ci]->distance_user){
                                std::cout<<"In, inward, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in<<std::endl;
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, true, true, true);
                                clusters_item->children[no_ci]->in_in=clusters_item->children[no_ci]->distance_user;
                                clusters_item->children[no_ci]->in_visited_flag=true;
                            }
                            std::cout<<"The contribution has been showed above"<<std::endl;
                            std::cout<<"After the "<<time+2<<"th loop, the in in of the "<<no_ci<<" cluster is "<<clusters_item->children[no_ci]->in_in<<"   "<<"The inward visited flag is "<<clusters_item->children[no_ci]->in_visited_flag<<std::endl;
                        }
                        if(clusters_item->children[no_ci]->out_visited_flag==false){
                            std::cout<<"The beginning outward flag of "<<no_ci<<" cluster for the "<<time+2<<" time search is false"<<std::endl;
                            std::cout<<"The beginning out_in is "<<clusters_item->children[no_ci]->out_in<<std::endl;
                            std::cout<<"The contribution of the outward part of the "<<no_ci<<" cluster is to be showed, it has "<<clusters_item->children[no_ci]->number<<" items"<<std::endl;
                            if(r<clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                std::cout<<"In, outward, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+r<<std::endl;
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+r, false, true, true);
                                clusters_item->children[no_ci]->out_in=r;
                            }
                            if(r>=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                std::cout<<"In, outward, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius<<std::endl;
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius, false, true, true);
                                clusters_item->children[no_ci]->out_in=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user;
                                clusters_item->children[no_ci]->out_visited_flag=true;
                            }
                            std::cout<<"The contribution has been showed above"<<std::endl;
                            std::cout<<"After the "<<time+2<<"th loop, the out in of the "<<no_ci<<" cluster is "<<clusters_item->children[no_ci]->out_in<<"   "<<"The outward visited flag is "<<clusters_item->children[no_ci]->out_visited_flag<<std::endl;
                        }
                        if((clusters_item->children[no_ci]->in_visited_flag==true)&&(clusters_item->children[no_ci]->out_visited_flag==true)){
                            clusters_item->children[no_ci]->visited_flag=true;
                        }
                        std::cout<<"After the "<<time+2<<"th loop, the visited flag of the "<<no_ci<<" cluster is "<<clusters_item->children[no_ci]->visited_flag<<std::endl;
                    }
                }else{
                    std::cout<<"The beginning flag of "<<no_ci<<" cluster for the "<<time+2<<" time search is true, no need of search any more"<<std::endl;
                }
                std::cout<<" "<<std::endl;
            }
            if(candidate_list.size()!=0){
                for(auto i:candidate_list){
                    double distance_item_user=dist_i(i, u);
                    distance_time=distance_time+1;
                    if(distance_item_user<distance_k){
                        item_distance_list.emplace_back(std::make_tuple(distance_item_user, i));
                    }
                }
                if(item_distance_list.size()>=k){
                    std::nth_element(item_distance_list.begin(), item_distance_list.begin() + k - 1, item_distance_list.end(),
                                     [&](const std::tuple<double, Item*>& a, const std::tuple<double, Item*>& b) -> bool {
                                         return std::get<0>(a) <= std::get<0>(b);
                                     });
                    item_distance_list.assign(item_distance_list.begin(), item_distance_list.begin()+k);
                    distance_k=std::get<0>(item_distance_list[k-1]);
                    if(distance_k<r){
                        break;
                    }
                }
            }else{
                if(distance_k<r){
                    break;
                }
            }
            r=r+delta_r;
            time=time+1;





// replacement one  replacement one replacement one   replacement one   replacement one   replacement one   replacement one   replacement one  replacement one

        }
        u->R.clear();
        for(int count_item=0; count_item<k; count_item++){
            u->R.emplace_back(std::get<1>(item_distance_list[count_item]));
        }
        //std::cout<<candidate_list.size()<<std::endl;

        /*if(candidate_list.size()<6000){
            std::cout<<"--------------------------------------------------------------------------------------------------------"<<std::endl;
        }*/
        u->dknn=distance_k;
    }
    else{
        std::vector<double> dis_u_ci_list;
        //std::cout<<" "<<std::endl;                                                        //Count switch optional Count switch optional Count switch optional Count switch optional
        //std::cout<<"Begin printing the information for every cluster"<<std::endl;         //Count switch optional Count switch optional Count switch optional Count switch optional
        //std::cout<<" "<<std::endl;                                                        //Count switch optional Count switch optional Count switch optional Count switch optional
        int cluster_no=0;

        for(auto ci:clusters_item->children){
            //std::cout<<"No."<<cluster_no<<"    ";                                       //Count switch optional Count switch optional Count switch optional Count switch optional
            ci->in_in=0;
            ci->out_in=0;
            ci->general_in=0;
            ci->in_visited_flag= false;
            ci->out_visited_flag=false;
            ci->visited_flag=false;
            double distance_u_ci=distance_ci_u(u, ci);

            //std::cout<<"distance to user="<<distance_u_ci<<"    ";                   //Count switch optional Count switch optional Count switch optional Count switch optional
            //std::cout<<"radius of "<<cluster_no<<"="<<ci->radius<<"    ";            //Count switch optional Count switch optional Count switch optional Count switch optional
            //std::cout<<"the number of keys="<<ci->number<<"    ";                    //Count switch optional Count switch optional Count switch optional Count switch optional

            ci->distance_user=distance_u_ci;
            if(distance_u_ci>=ci->radius){
                ci->out= true;
                //std::cout<<"out"<<" ";                                               //Count switch optional Count switch optional Count switch optional Count switch optional
            }else{
                ci->out=false;
                //std::cout<<"in"<<" ";                                                //Count switch optional Count switch optional Count switch optional Count switch optional
            }
            dis_u_ci_list.emplace_back(distance_u_ci);
            //std::cout<<" "<<std::endl;                                               //Count switch optional Count switch optional Count switch optional Count switch optional
            cluster_no=cluster_no+1;
        }
        auto max_r=std::max_element(dis_u_ci_list.begin(), dis_u_ci_list.end());
        auto max_r_real=dis_u_ci_list[max_r-dis_u_ci_list.begin()];
        double r=(r_rate)*(max_r_real+max_r_item_cluster+0.1);  //Be careful, here we add 0.1 on the max radius Be careful, here we add 0.1 on the max radius Be careful, here we add 0.1 on the max radius
        double delta_r=delta_r_rate*(max_r_real+max_r_item_cluster+0.1);  //Be careful, here we add 0.1 on the max radius Be careful, here we add 0.1 on the max radius Be careful, here we add 0.1 on the max radius
        std::vector<std::tuple<double, Item*>> item_distance_list;
        std::vector<Item*> candidate_list;
        std::vector<Item*>* pointer_candidate=&(candidate_list);
        double distance_k=std::numeric_limits<double>::max();
        int time=-1;
        int no_ci;
        //std::cout<<" "<<std::endl;
        while(time<int((1.0-r_rate)/delta_r_rate)+1){
            /*if((item_distance_list.size()==k)&&(distance_k<r)){
                candidate_list.clear();
                for(no_ci=0; no_ci<(clusters_item->children).size(); no_ci++){
                    if(clusters_item->children[no_ci]->visited_flag==false){
                        if(clusters_item->children[no_ci]->out==true){
                            if((r>clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius)&&((r<clusters_item->children[no_ci]->distance_user))){
                                bptree_item->get_range_item(pointer_candidate,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in,
                                                            false, true, false);
                                clusters_item->children[no_ci]->general_in=r-(clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius);
                            }
                            if(r>=clusters_item->children[no_ci]->distance_user){
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in, true, true, false);
                                clusters_item->children[no_ci]->general_in=clusters_item->children[no_ci]->radius;
                                clusters_item->children[no_ci]->visited_flag=true;
                            }
                        }
                        else{
                            if(clusters_item->children[no_ci]->in_visited_flag==false){
                                if(r<clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, false, true, false);
                                    clusters_item->children[no_ci]->in_in=r;
                                }
                                if(r>=clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, true, true, false);
                                    clusters_item->children[no_ci]->in_in=clusters_item->children[no_ci]->distance_user;
                                    clusters_item->children[no_ci]->in_visited_flag=true;
                                }
                            }
                            if(clusters_item->children[no_ci]->out_visited_flag==false){
                                if(r<clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+r, false, true, false );
                                    clusters_item->children[no_ci]->out_in=r;
                                }
                                if(r>=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius, false, true, false );
                                    clusters_item->children[no_ci]->out_in=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user;
                                    clusters_item->children[no_ci]->out_visited_flag=true;
                                }
                            }
                            if((clusters_item->children[no_ci]->in_visited_flag==true)&&(clusters_item->children[no_ci]->out_visited_flag==true)){
                                clusters_item->children[no_ci]->visited_flag=true;
                            }
                        }
                    }
                }
                if(candidate_list.size()==0){
                    break;
                }
                else{
                    for(auto i:candidate_list){
                        double distance_item_user=dist_i(i, u);
                        distance_time=distance_time+1;
                        if(distance_item_user<distance_k){
                            item_distance_list.emplace_back(std::make_tuple(distance_item_user, i));
                        }
                    }
                    std::nth_element(item_distance_list.begin(), item_distance_list.begin() + k - 1, item_distance_list.end(),
                                     [&](const std::tuple<double, Item*>& a, const std::tuple<double, Item*>& b) -> bool {
                                         return std::get<0>(a) <= std::get<0>(b);
                                     });
                    item_distance_list.assign(item_distance_list.begin(), item_distance_list.begin()+k);
                    distance_k=std::get<0>(item_distance_list[k-1]);
                    break;
                }
            }
            else{
                candidate_list.clear();
                for(no_ci=0; no_ci<(clusters_item->children).size(); no_ci++){
                    if(clusters_item->children[no_ci]->visited_flag==false){
                        if(clusters_item->children[no_ci]->out==true){
                            if((r>clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius)&&((r<clusters_item->children[no_ci]->distance_user))){
                                bptree_item->get_range_item(pointer_candidate,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in,
                                                            false, true, false);
                                clusters_item->children[no_ci]->general_in=r-(clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius);
                            }
                            if(r>=clusters_item->children[no_ci]->distance_user){
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in, true, true, false);
                                clusters_item->children[no_ci]->general_in=clusters_item->children[no_ci]->radius;
                                clusters_item->children[no_ci]->visited_flag=true;
                            }
                        }
                        else{
                            if(clusters_item->children[no_ci]->in_visited_flag==false){
                                if(r<clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, false, true, false);
                                    clusters_item->children[no_ci]->in_in=r;
                                }
                                if(r>=clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, true, true, false);
                                    clusters_item->children[no_ci]->in_in=clusters_item->children[no_ci]->distance_user;
                                    clusters_item->children[no_ci]->in_visited_flag=true;
                                }
                            }
                            if(clusters_item->children[no_ci]->out_visited_flag==false){
                                if(r<clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+r, false, true, false );
                                    clusters_item->children[no_ci]->out_in=r;
                                }
                                if(r>=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                    bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius, false, true, false );
                                    clusters_item->children[no_ci]->out_in=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user;
                                    clusters_item->children[no_ci]->out_visited_flag=true;
                                }
                            }
                            if((clusters_item->children[no_ci]->in_visited_flag==true)&&(clusters_item->children[no_ci]->out_visited_flag==true)){
                                clusters_item->children[no_ci]->visited_flag=true;
                            }
                        }
                    }
                }
                if(candidate_list.size()!=0){
                    for(auto i:candidate_list){
                        double distance_item_user=dist_i(i, u);
                        distance_time=distance_time+1;
                        if(distance_item_user<distance_k){
                            item_distance_list.emplace_back(std::make_tuple(distance_item_user, i));
                        }
                    }
                    if(item_distance_list.size()>=k){
                        std::nth_element(item_distance_list.begin(), item_distance_list.begin() + k - 1, item_distance_list.end(),
                                         [&](const std::tuple<double, Item*>& a, const std::tuple<double, Item*>& b) -> bool {
                                             return std::get<0>(a) <= std::get<0>(b);
                                         });
                        item_distance_list.assign(item_distance_list.begin(), item_distance_list.begin()+k);
                        distance_k=std::get<0>(item_distance_list[k-1]);
                        if(distance_k<r){
                            break;
                        }
                    }
                }
                r=r+delta_r;
                time=time+1;
            }*/

// replacement one  replacement one  replacement one  replacement one  replacement one  replacement one  replacement one  replacement one  replacement one

            candidate_list.clear();   //Be careful to switch this sentense
            for(no_ci=0; no_ci<(clusters_item->children).size(); no_ci++){

                //std::cout<<"No."<<no_ci<<"   "<<"radius="<<clusters_item->children[no_ci]->radius<<"   "<<"distance to user="<<clusters_item->children[no_ci]->distance_user<<"   "<<"The "<<time+2<<" time"<<"   "<<"search_r="<<r<<std::endl;

                if(clusters_item->children[no_ci]->visited_flag==false){
                    //std::cout<<"The beginning flag of "<<no_ci<<" cluster for the "<<time+2<<" time search is false"<<std::endl;
                    if(clusters_item->children[no_ci]->out==true){
                        //std::cout<<"Out   "<<"The beginning general in is "<<clusters_item->children[no_ci]->general_in<<std::endl;
                        //std::cout<<"The contribution of the "<<no_ci<<" cluster is to be showed, it has "<<clusters_item->children[no_ci]->number<<" items"<<std::endl;
                        if((r>clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius)&&((r<clusters_item->children[no_ci]->distance_user))){
                            //std::cout<<"Out, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in<<std::endl;
                            bptree_item->get_range_item(pointer_candidate,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in,
                                                        false, true, false);

                            clusters_item->children[no_ci]->general_in=r-(clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->radius);
                        }
                        if(r>=clusters_item->children[no_ci]->distance_user){
                            //std::cout<<"Out, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in<<std::endl;
                            bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->general_in, true, true, false);
                            clusters_item->children[no_ci]->general_in=clusters_item->children[no_ci]->radius;
                            clusters_item->children[no_ci]->visited_flag=true;
                        }
                        //std::cout<<"The contribution has been showed above"<<std::endl;
                        //std::cout<<"After the "<<time+2<<"th loop, the general in of the "<<no_ci<<" cluster is "<<clusters_item->children[no_ci]->general_in<<"   "<<"The visited flag is "<<clusters_item->children[no_ci]->visited_flag<<std::endl;
                    }
                    else{
                        //std::cout<<"The "<<no_ci<<" cluster is in"<<std::endl;
                        if(clusters_item->children[no_ci]->in_visited_flag==false){
                            //std::cout<<"The beginning inward flag of "<<no_ci<<" cluster for the "<<time+2<<" time search is false"<<std::endl;
                            //std::cout<<"The beginning in_in is "<<clusters_item->children[no_ci]->in_in<<std::endl;
                            //std::cout<<"The contribution of the inward part of the "<<no_ci<<" cluster is to be showed, it has "<<clusters_item->children[no_ci]->number<<" items"<<std::endl;
                            if(r<clusters_item->children[no_ci]->distance_user){
                                //std::cout<<"In, inward, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in<<std::endl;
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-r, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, false, true, false);
                                clusters_item->children[no_ci]->in_in=r;
                            }
                            if(r>=clusters_item->children[no_ci]->distance_user){
                                //std::cout<<"In, inward, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in<<std::endl;
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2), double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user-clusters_item->children[no_ci]->in_in, true, true, false);
                                clusters_item->children[no_ci]->in_in=clusters_item->children[no_ci]->distance_user;
                                clusters_item->children[no_ci]->in_visited_flag=true;
                            }
                            //std::cout<<"The contribution has been showed above"<<std::endl;
                            //std::cout<<"After the "<<time+2<<"th loop, the in in of the "<<no_ci<<" cluster is "<<clusters_item->children[no_ci]->in_in<<"   "<<"The inward visited flag is "<<clusters_item->children[no_ci]->in_visited_flag<<std::endl;
                        }
                        if(clusters_item->children[no_ci]->out_visited_flag==false){
                            //std::cout<<"The beginning outward flag of "<<no_ci<<" cluster for the "<<time+2<<" time search is false"<<std::endl;
                            //std::cout<<"The beginning out_in is "<<clusters_item->children[no_ci]->out_in<<std::endl;
                            //std::cout<<"The contribution of the outward part of the "<<no_ci<<" cluster is to be showed, it has "<<clusters_item->children[no_ci]->number<<" items"<<std::endl;
                            if(r<clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                //std::cout<<"In, outward, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+r<<std::endl;
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+r, false, true, false);
                                clusters_item->children[no_ci]->out_in=r;
                            }
                            if(r>=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user){
                                //std::cout<<"In, outward, the base space is "<<max_r_item_cluster+0.2<<" The distance is "<<clusters_item->children[no_ci]->distance_user<<" The radius is "<<clusters_item->children[no_ci]->radius<<" the search floor is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in<<" the cell is "<<double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius<<std::endl;
                                bptree_item->get_range_item(pointer_candidate, double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->distance_user+clusters_item->children[no_ci]->out_in,double(no_ci)*(max_r_item_cluster+0.2)+clusters_item->children[no_ci]->radius, false, true, false);
                                clusters_item->children[no_ci]->out_in=clusters_item->children[no_ci]->radius-clusters_item->children[no_ci]->distance_user;
                                clusters_item->children[no_ci]->out_visited_flag=true;
                            }
                            //std::cout<<"The contribution has been showed above"<<std::endl;
                            //std::cout<<"After the "<<time+2<<"th loop, the out in of the "<<no_ci<<" cluster is "<<clusters_item->children[no_ci]->out_in<<"   "<<"The outward visited flag is "<<clusters_item->children[no_ci]->out_visited_flag<<std::endl;
                        }
                        if((clusters_item->children[no_ci]->in_visited_flag==true)&&(clusters_item->children[no_ci]->out_visited_flag==true)){
                            clusters_item->children[no_ci]->visited_flag=true;
                        }
                        //std::cout<<"After the "<<time+2<<"th loop, the visited flag of the "<<no_ci<<" cluster is "<<clusters_item->children[no_ci]->visited_flag<<std::endl;
                    }
                }else{
                    //std::cout<<"The beginning flag of "<<no_ci<<" cluster for the "<<time+2<<" time search is true, no need of search any more"<<std::endl;
                }
                //std::cout<<" "<<std::endl;
            }
            if(candidate_list.size()!=0){
                for(auto i:candidate_list){
                    double distance_item_user=dist_i(i, u);
                    distance_time=distance_time+1;
                    if(distance_item_user<distance_k){
                        item_distance_list.emplace_back(std::make_tuple(distance_item_user, i));
                    }
                }
                if(item_distance_list.size()>=k){
                    std::nth_element(item_distance_list.begin(), item_distance_list.begin() + k - 1, item_distance_list.end(),
                                     [&](const std::tuple<double, Item*>& a, const std::tuple<double, Item*>& b) -> bool {
                                         return std::get<0>(a) <= std::get<0>(b);
                                     });
                    item_distance_list.assign(item_distance_list.begin(), item_distance_list.begin()+k);
                    distance_k=std::get<0>(item_distance_list[k-1]);
                    if(distance_k<r){
                        break;
                    }
                }
            }else{
                if(distance_k<r){
                    break;
                }
            }
            r=r+delta_r;
            time=time+1;





// replacement one  replacement one replacement one   replacement one   replacement one   replacement one   replacement one   replacement one  replacement one

        }
        u->R.clear();
        for(int count_item=0; count_item<k; count_item++){
            u->R.emplace_back(std::get<1>(item_distance_list[count_item]));
        }
        //std::cout<<candidate_list.size()<<std::endl;

        /*if(candidate_list.size()<6000){
            std::cout<<"--------------------------------------------------------------------------------------------------------"<<std::endl;
        }*/
        u->dknn=distance_k;
    }
}



//Functions for the idistance and B_plus tree indexing structure


//Functions for the sphere tree

void HDR_Tree::construct_sphere(long fanout, long threshold) {

    std::cout << "Constructing The sphere tree" << std::endl;
    std::cout << "Users: " << numUsers << std::endl;
    std::cout << "Features: " << numFeatures << std::endl;
    std::cout << "Window: " << windowSize << std::endl;
    std::cout << "Fanout: " << fanout << std::endl << std::endl;

    //init();

    //L = log(data->U.size()) / log(fanout);
    //std::cout << "Estimated Height: " << L << " levels." << std::endl;

    //data->initTransform(L);
    long numClusters = 0;

    auto C = new Clustersu(Users, fanout);
    for (auto Cj : *C) {

        if (Cj->Users.size() > 0) {
            Cj->maxdknn = std::get<1>(Cj->Users[0])->dknn;
            for (auto t : Cj->Users) {
                Cj->maxdknn = std::max(Cj->maxdknn, double(std::get<1>(t)->dknn));
            }
        }

        numClusters += 1;
        Cj->l = 1;
    }

    root_sphere = new userNonLeafNode(C, 1, false);

    std::queue<userNode *> q;

    q.push(root_sphere);

    long maxL = 0;

    while (!q.empty()) {
        auto node = dynamic_cast<userNonLeafNode *>(q.front());
        q.pop();

        auto Cp = node->clusters;

        for (auto Cj : Cp) {

            if (Cj->number < threshold) {
                auto LN = new userLeafNode(Cj, false);

                node->children.emplace_back(LN);
                Cj->ptr = LN;
            }
            else {
                auto level = node->l + 1;
                maxL = std::max(maxL, level);

                /*if(level>L){
                    std::cout<<"Excceed the estimated level"<<std::endl;
                }*/

                auto Cpp = new Clustersu(Cj, fanout);
                for (auto Cj : *Cpp) {

                    if (Cj->Users.size() > 0) {
                        Cj->maxdknn = std::get<1>(Cj->Users[0])->dknn;
                        for (auto t : Cj->Users) {
                            Cj->maxdknn = std::max(Cj->maxdknn, double(std::get<1>(t)->dknn));
                        }
                    }

                    numClusters += 1;
                    Cj->l = level;
                }

                auto NLN = new userNonLeafNode(Cpp, level, false);

                node->children.emplace_back(NLN);
                Cj->ptr = NLN;
                q.push(NLN);
            }
        }
    }

    std::cout << "Actual Height: " << maxL << " levels." << std::endl;
    std::cout << "Clusters: " << numClusters << std::endl;

//	for (int i = 0; i < 100; i++) {
//		Users[0]->update(slidingWindow[i], Mode::Add);
//	}

//	for (int i = 0; i < 100; i++) {
//		if (Users[0]->dist(slidingWindow[i]) < Users[0]->dknn) {
//			Users[0]->update(slidingWindow[i], Mode::Remove);
//			break;
//		}
//	}


//	fillTree();

}

double HDR_Tree::dist_cu_i(Clusteru* cu, Item* i){
    double distance=(cu->center-i->v_d).norm();
    return distance;
}


double HDR_Tree::dist_ud_id(User* ud, Item* id){
    double distance=(ud->center.cast<double>(), id->v_d).norm();
    return distance;
}


float HDR_Tree::dist_cu_u(Clusteru* cu, User* u){
    float distance=((cu->center).cast<float>()-u->center).norm();
    return distance;
}

void HDR_Tree::search_sphere(userNode *node_sphere, Item *I) {






    if (node_sphere->type == userNodeType::userLeafNode) {
        auto LN = dynamic_cast<userLeafNode*>(node_sphere);
        for (auto Ut : *LN) {

            num_sphere_cal_users=num_sphere_cal_users+1;    //Add the calculation time for users

            auto user = std::get<1>(Ut);
            if (dist(I, user) < user->dknn) {
                Rp_sphere.emplace_back(user);
            }
        }
    }
    else {
        auto NLN = dynamic_cast<userNonLeafNode*>(node_sphere);
        auto& Cp = NLN->clusters;


        double d1,d2;

        for (long j = 0; j < Cp.size(); j++) {

            num_sphere_cal_clusters=num_sphere_cal_clusters+1;   //Add the calculation time for clusters

            d1 = dist_cu_i(Cp[j],I)-Cp[j]->radius;
            d2 = Cp[j]->maxdknn;
            if (d1 < d2) {
                search_sphere(NLN->children[j], I);
            }
        }
    }
}


bool HDR_Tree::AdjustMaxdknnsphere(userNode* n) {

    bool result = false;

    if (n->type == userNodeType::userLeafNode) {
        auto LN = dynamic_cast<userLeafNode*>(n);

        long Len = Rp_sphere.size();
        for (auto Ut : *LN) {
            auto& user = std::get<1>(Ut);
            for (long i = 0; i < Len; i++) {
                if (Rp_sphere[i] == user) {
                    return true;
                }
            }
        }
    }
    else {
        auto NLN = dynamic_cast<userNonLeafNode*>(n);
        auto& Cp = NLN->clusters;

        for (auto Cj : Cp) {
            bool res = AdjustMaxdknnsphere(Cj->ptr);

            if (res) {
                result = true;

                float Dist = 0;

                if (Cj->ptr->type == userNodeType::userLeafNode) {

                    auto LN = dynamic_cast<userLeafNode*>(Cj->ptr);
                    for (auto Ut : *LN) {

                        auto user = std::get<1>(Ut);
                        Dist = std::max(Dist, user->dknn);
                    }
                }
                else {

                    auto NLN = dynamic_cast<userNonLeafNode*>(Cj->ptr);
                    auto Cp = NLN->clusters;

                    for (auto Cj : Cp) {
                        Dist = std::max(Dist, float(Cj->maxdknn));
                    }
                }

                Cj->maxdknn = Dist;
            }
        }
    }

    return result;
}


void HDR_Tree::update_sphere(User* U, Mode m, userNode* node){
    if(m==Mode::Add){

        if(node->type==userNodeType::userNonLeafNode){
            //projectUser(U); be careful, project it outside and before this function is used
            auto NLN=dynamic_cast<userNonLeafNode*>(node);
            std::vector<std::tuple<float, Clusteru*>>V;
            auto v_transformed=U->center;     //Be careful, here we in advanced calculate the low dimension form of the user 1
            for(auto C_0: NLN->clusters){
                V.emplace_back(std::make_tuple((v_transformed-(C_0->center).cast<float>()).norm(), C_0));   //Be careful, here is associated with the note 1
            }
            std::nth_element(V.begin(), V.begin() + V.size() - 1, V.end(),
                             [&](const std::tuple<float, Clusteru*>& a, const std::tuple<float, Clusteru* >& b) -> bool {
                                 return std::get<0>(a) >= std::get<0>(b);
                             });
            auto C_add=std::get<1>(V[V.size()-1]);
            C_add->Users.emplace_back(std::make_tuple(std::get<0>(V[V.size()-1]), U));
            C_add->number=C_add->number+1;
            std::sort(C_add->Users.begin(),  C_add->Users.end(),
                      [&](const std::tuple<float, User*>& a, const std::tuple<float, User*>& b) -> bool {
                          return std::get<0>(a) <= std::get<0>(b);
                      });
            C_add->radius=std::get<0>(C_add->Users[(C_add->Users).size()-1]);
            if (double(U->dknn)>C_add->maxdknn){
                C_add->maxdknn=double(U->dknn);
            }
            update_sphere( U, m, C_add->ptr);
        }
    }else{
        if(node->type==userNodeType::userNonLeafNode){
            auto NLN=dynamic_cast<userNonLeafNode*>(node);
            //std::cout<<"1"<<std::endl;   //show show show show show show show show
            Clusteru* C_catch;

            //std::cout<<"The original state of the C_catch "<<(C_catch->Users).size()<<std::endl;   //show show show show show show show show

            long i_catch;
            for(auto C:NLN->clusters){
                if(C->number!=0){
                    //std::cout<<"Not zero"<<std::endl;         //show show show show show show show
                    bool indicator= false;
                    for(long i=0; i<(C->Users).size(); i++){
                        if(std::get<1>(C->Users[i])==U){


                            C_catch=C;

                            //std::cout<<"catch success"<<std::endl;   //show show show show show show show

                            i_catch=i;
                            indicator= true;
                            break;
                        }
                    }
                    if(indicator==true){
                        break;
                    }
                }
            }

            //std::cout<<"2"<<std::endl;//show show show show show show show show
            //std::cout<<(C_catch->Users).size()<<" 3"<<std::endl;  //show show show show show show show show
            C_catch->number=C_catch->number-1;
            if(C_catch->number==0){
                C_catch->radius=0;
                C_catch->maxdknn=0;
                C_catch->Users.erase(C_catch->Users.begin()+i_catch);
            }else{
                //std::cout<<"4"<<std::endl;//show show show show show show show show
                //std::cout<<(C_catch->Users).size()<<" 5"<<std::endl;  //show show show show show show show show
                C_catch->Users.erase(C_catch->Users.begin()+i_catch);
                //std::cout<<"6"<<std::endl;//show show show show show show show show
                //std::cout<<(C_catch->Users).size()<<" 7"<<std::endl;   //show show show show show show show show
                C_catch->radius=std::get<0>(C_catch->Users[C_catch->Users.size()-1]);
                //std::cout<<"8"<<std::endl;//show show show show show show show show
                if(U->dknn==C_catch->maxdknn){
                    std::vector<float> V;
                    for(auto tp: C_catch->Users){
                        V.emplace_back(std::get<1>(tp)->dknn);
                    }
                    std::nth_element(V.begin(), V.begin() + V.size() - 1, V.end(),
                                     [&](const float& a, const float& b) -> bool {
                                         return a<=b;
                                     });
                    C_catch->maxdknn=V[V.size()-1];
                }
            }

            update_sphere( U, m, C_catch->ptr);
        }
    }
}

void HDR_Tree::update_user_spheretree(){
    auto u_out=Users[0];
    Users.erase(Users.begin());
    update_sphere(u_out,Mode::Remove, root_sphere);
};

void HDR_Tree::update_user_add_spheretree(){
    auto new_user= new User(data->U.row(numUsers+add_user_num), k);
    new_user->computeKNN(slidingWindow);
    Users.emplace_back(new_user);
    update_sphere(new_user,Mode::Add, root_sphere);
    add_user_num=add_user_num+1;
};

void HDR_Tree::update_item_spheretree(){
    auto item_out=slidingWindow[0];
    slidingWindow.erase(slidingWindow.begin());

    Rp_sphere.clear();



    //projectItem(item_out);

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    //num_sphere_cal_clusters=0;
    //num_sphere_cal_users=0;

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    search_sphere(root_sphere, item_out);

    //To print the times of calculation of the distance

    //std::cout<<"The times of calculation of the distance between the item and the clusters is "<<num_sphere_cal_clusters<<std::endl;
    //std::cout<<"The times of calculation of the distance between the item and the users is "<<num_sphere_cal_users<<std::endl;
    //std::cout<<" "<<std::endl;

    //To print the times of calculation of the distance

    if(Rp_sphere.size()>0){

        //std::cout<<Rp_sphere.size()<<std::endl;

        for(auto u:Rp_sphere){
            u->computeKNN(slidingWindow);
        }
        AdjustMaxdknnsphere(root_sphere);
    }
};

void HDR_Tree::update_item_add_spheretree(){
    auto new_item = new Item(data_i->I.row(numItems_i + add_item_num));
    slidingWindow.emplace_back(new_item);

    //std::cout<<"The center of the new item is "<<std::endl;

    //std::cout<<new_item->v<<std::endl;



    Rp_sphere.clear();

    search_sphere(root_sphere, new_item);
    //std::cout<<" "<<std::endl;
    //std::cout<<"The number of influenced users "<<Rp_sphere.size()<<std::endl;
    //std::cout<<" "<<std::endl;
    //std::cout<<"The centers of the affected users and the dknn values of the affected users before recomputing the knn lists of deletion"<<std::endl;

    /*for(auto user_before: Rp_sphere){
        //std::cout<<"The center of the affected user: "<<std::endl;
        //std::cout<<user_before->center<<std::endl;
        std::cout<<" The dknn of the affected user: "<<user_before->dknn<<" "<<std::endl;
        //std::cout<<" "<<std::endl;
    }*/
    //std::cout<<" "<<std::endl;
    //std::cout<<" "<<std::endl;

    if (Rp_sphere.size() > 0) {

        for (auto u:Rp_sphere) {

            /*std::cout<<"The center of the affected user: "<<std::endl;
            std::cout<<u->center<<std::endl;
            std::cout<<" The dknn of the affected user: "<<u->dknn<<" "<<std::endl;
            std::cout<<" "<<std::endl;*/
            u->update(new_item, Mode::Add);
        }
        AdjustMaxdknnsphere(root_sphere);
    }

    //std::cout<<" "<<std::endl;
    //std::cout<<" "<<std::endl;

    /*std::cout<<"The dknn values after recomputing the knn lists of deletion"<<std::endl;

    for(auto user_after:Rp_sphere){
        std::cout<<user_after->dknn<<" ";
    }

    std::cout<<" "<<std::endl;*/

    //std::cout<<" "<<std::endl;



    add_item_num = add_item_num + 1;
};

void HDR_Tree::update_user_knnjoinplus(){
    auto u_out=Users[0];
    Users.erase(Users.begin());
    update_sphere(u_out,Mode::Remove, root_sphere);
};
void HDR_Tree::update_user_add_knnjoinplus(){
    auto new_user= new User(data->U.row(numUsers+add_user_num), k);
    user_computeknn_id(new_user, 0.02, 0.02, false);
    Users.emplace_back(new_user);
    update_sphere(new_user,Mode::Add, root_sphere);
    add_user_num=add_user_num+1;
}

void HDR_Tree::update_item_knnjoinplus(){
    auto item_out=slidingWindow[0];
    slidingWindow.erase(slidingWindow.begin());

    bptree_item->Remove(item_out->index_b_plus);

    /*if((item_out->index_b_plus-130.068493)*(item_out->index_b_plus-130.068493)<0.000001){
        std::cout<<"Catch Catch Catch Catch Catch Catch Catch Catch Catch Catch Catch Catch Catch Catch Catch Catch"<<std::endl;
    }*/

    Rp_sphere.clear();

    //projectItem(item_out);

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    //num_sphere_cal_clusters=0;
    //num_sphere_cal_users=0;

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    search_sphere(root_sphere, item_out);

    //To print the times of calculation of the distance

    //std::cout<<"The times of calculation of the distance between the item and the clusters is "<<num_sphere_cal_clusters<<std::endl;
    //std::cout<<"The times of calculation of the distance between the item and the users is "<<num_sphere_cal_users<<std::endl;
    //std::cout<<" "<<std::endl;

    //To print the times of calculation of the distance

    if(Rp_sphere.size()>0){

        //std::cout<<Rp_sphere.size()<<std::endl;
        //int count_user=1;
        for(auto u:Rp_sphere){

            //std::cout<<"Start calculate the knn list of the "<<count_user<<"th user"<<std::endl;

            //std::cout<<" "<<std::endl;

            user_computeknn_id(u,0.02,0.02, false);

            //std::cout<<"Complete calculate the knn list of the "<<count_user<<"th user"<<std::endl;

            //std::cout<<" "<<std::endl;

            //count_user=count_user+1;
        }
        AdjustMaxdknnsphere(root_sphere);
    }
};




void HDR_Tree::update_item_add_knnjoinplus(){

    auto new_item = new Item(data_i->I.row(numItems_i + add_item_num));
    slidingWindow.emplace_back(new_item);

    //std::cout<<"The center of the new item is "<<std::endl;
    //std::cout<<new_item->v<<std::endl;

    Rp_sphere.clear();

    search_sphere(root_sphere, new_item);

    //std::cout<<" "<<std::endl;

    //std::cout<<"The number of influenced users: "<<Rp_sphere.size()<<std::endl;

    //std::cout<<" "<<std::endl;

    //std::cout<<"The centers of the affected users and the dknn values of the affected users before recomputing the knn lists of deletion"<<std::endl;

    //std::cout<<" "<<std::endl;

    /*for(auto user_before: Rp_sphere){
        //std::cout<<"The center of the affected user: "<<std::endl;
        //std::cout<<user_before->center<<std::endl;
        std::cout<<" The dknn of the affected user: "<<user_before->dknn<<" "<<std::endl;
    }
    //std::cout<<" "<<std::endl;
    std::cout<<" "<<std::endl;*/


    if (Rp_sphere.size() > 0) {

        for (auto u:Rp_sphere) {
            /*std::cout<<"The center of the affected user: "<<std::endl;
            std::cout<<u->center<<std::endl;
            std::cout<<" The dknn of the affected user: "<<u->dknn<<" "<<std::endl;*/
            u->update(new_item, Mode::Add);
            //std::cout<<" "<<std::endl;
        }
        AdjustMaxdknnsphere(root_sphere);
    }

    //std::cout<<" "<<std::endl;
    //std::cout<<" "<<std::endl;

    /*std::cout<<"The dknn values after recomputing the knn lists of deletion"<<std::endl;

    for(auto user_after:Rp_sphere){
        std::cout<<user_after->dknn<<" ";
    }

    std::cout<<" "<<std::endl;*/

    //std::cout<<" "<<std::endl;


    add_item_num = add_item_num + 1;


    //The original version of the add function introduced from the sphere mode
};


double HDR_Tree::distance_item_clusteri(Item* i_item, Clusteri* c_clusteri){
    double distance=(i_item->v_d-c_clusteri->center).norm();
    return distance;
}




//Functions for the sphere tree




//Funtion for optimization of deletion

void HDR_Tree::update_item_op_delet_usertree(){
    auto item_out=slidingWindow[0];
    slidingWindow.erase(slidingWindow.begin());

    Rp.clear();

    //std::chrono::steady_clock::time_point begin_clock_item_add = std::chrono::steady_clock::now();  //For checking of the projection time, beginning

    //int test_0=2;

    projectItem(item_out);

    /*std::chrono::steady_clock::time_point end_clock_item_add = std::chrono::steady_clock::now();  //For checking of the projection time, ending

    int test_1=3;

    projection_time=projection_time+test_1-test_0;//std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_item_add - begin_clock_item_add).count();   //Add the projection time

    std::chrono::steady_clock::time_point begin_clock_item_add_hdr_0 = std::chrono::steady_clock::now();  //For checking of the search time on hdr tree, beginning*/

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    //num_hdr_cal_clusters=0;
    //num_hdr_cal_users=0;

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    Search(root, item_out);

    /*std::cout<<"The number of influenced users "<<Rp.size()<<std::endl;

    std::cout<<"The dknn values before recomputing the knn lists of deletion"<<std::endl;

    for(auto user_before: Rp){
        std::cout<<user_before->dknn<<" ";
    }
    std::cout<<" "<<std::endl;*/


    //std::cout<<"Check 1"<<std::endl;

    //To print the times of calculation of the distance

    //std::cout<<"The times of calculation of the distance between the item and the clusters is "<<num_hdr_cal_clusters<<std::endl;
    //std::cout<<"The times of calculation of the distance between the item and the users is "<<num_hdr_cal_users<<std::endl;
    //std::cout<<" "<<std::endl;

    //To print the times of calculation of the distance

    /*std::chrono::steady_clock::time_point end_clock_item_add_hdr_0 = std::chrono::steady_clock::now();  //For checking of the search time on hdr tree, ending

    search_time_HDR=search_time_HDR+std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_item_add_hdr_0 - begin_clock_item_add_hdr_0).count();*/


    if(Rp.size()>0){
        for(auto u:Rp){
            std::vector<float> stored_bound;
            auto LN=dynamic_cast<LeafNode*>(u->belong_leaf);
            for(auto u_friend:LN->cluster->Users){
                for(auto i_knn: std::get<1>(u_friend)->R){
                    float dist_bound=dist(i_knn,u);
                    if (dist_bound>u->dknn){
                        stored_bound.emplace_back(dist_bound);
                    }
                }
            }
            if(stored_bound.size()!=0){
                float bound_deletion=*min_element(stored_bound.begin(),stored_bound.end());
                u->update(item_out, Mode::Remove);
                /*float floor_bound=dist(u->R[k-2], u);*/
                std::vector<std::tuple<float, Item*>> stored_knn;
                for(auto item_0: slidingWindow){
                    float dist_low=(u->u_low_d[2]-item_0->i_low_d[2]).norm();
                    if(dist_low<=bound_deletion){
                        float real_dist=dist(item_0, u);
                        if(real_dist>u->dknn){
                            stored_knn.emplace_back(std::make_tuple(real_dist, item_0));
                        }
                    }
                }
                if(stored_knn.size()!=0){
                    std::nth_element(stored_knn.begin(), stored_knn.begin() + stored_knn.size() - 1, stored_knn.end(),
                                     [&](const std::tuple<float, Item*>& a, const std::tuple<float, Item* >& b) -> bool {
                                         return std::get<0>(a) > std::get<0>(b);
                                     });
                    u->R.emplace_back(std::get<1>(stored_knn[stored_knn.size()-1]));
                    u->dknn=std::get<0>(stored_knn[stored_knn.size()-1]);
                }else{
                    u->computeKNN_sort(slidingWindow);
                }

            }else{
                u->computeKNN_sort(slidingWindow);
            }
        }

        AdjustMaxdknnPaper(root);


    }

    /*std::cout<<"The dknn values after recomputing the knn lists of deletion"<<std::endl;

    for(auto user_after:Rp){
        std::cout<<user_after->dknn<<" ";
    }

    std::cout<<" "<<std::endl;

    std::cout<<" "<<std::endl;*/


};

void HDR_Tree::search_deletion_opt(Node* node, Item* I){
    if (node->type == NodeType::LeafNode) {
        auto LN = dynamic_cast<LeafNode*>(node);
        for (auto Ut : *LN) {

            num_hdr_cal_users=num_hdr_cal_users+1;   //Add the calculation time of the users

            auto user = std::get<1>(Ut);
            if (dist(I, user) < user->dknn) {
                Rp_0.emplace_back(user);
            }
        }
    }
    else {
        auto NLN = dynamic_cast<NonLeafNode*>(node);
        auto& Cp = NLN->clusters;


        float d1,d2;

        for (long j = 0; j < Cp.size(); j++) {

            num_hdr_cal_clusters=num_hdr_cal_clusters+1;   //Add the calculation time of the clusters
            std::cout<<"Check 2"<<std::endl;
            d1 = (I->i_low_d[Cp[j]->l - 1] - Cp[j]->center).norm();
            std::cout<<"Check 3"<<std::endl;
            d2 = Cp[j]->maxdknn+Cp[j]->radius;
            if (d1 <= d2) {
                search_deletion_opt(NLN->children[j], I);
            }
        }
    }
}





//Funtion for optimization of deletion

//Funtion for the deletion update when using only one HDR tree for the users for comparison with the optimization deletion method, using computeKNN_sort.

void HDR_Tree::update_item_user_tree_compute_sort(){
    auto item_out=slidingWindow[0];
    slidingWindow.erase(slidingWindow.begin());

    Rp.clear();

    //std::chrono::steady_clock::time_point begin_clock_item_add = std::chrono::steady_clock::now();  //For checking of the projection time, beginning

    //int test_0=2;

    projectItem(item_out);

    /*std::chrono::steady_clock::time_point end_clock_item_add = std::chrono::steady_clock::now();  //For checking of the projection time, ending

    int test_1=3;

    projection_time=projection_time+test_1-test_0;//std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_item_add - begin_clock_item_add).count();   //Add the projection time

    std::chrono::steady_clock::time_point begin_clock_item_add_hdr_0 = std::chrono::steady_clock::now();  //For checking of the search time on hdr tree, beginning*/

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    //num_hdr_cal_clusters=0;
    //num_hdr_cal_users=0;

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    Search(root, item_out);

    //To print the times of calculation of the distance

    //std::cout<<"The times of calculation of the distance between the item and the clusters is "<<num_hdr_cal_clusters<<std::endl;
    //std::cout<<"The times of calculation of the distance between the item and the users is "<<num_hdr_cal_users<<std::endl;
    //std::cout<<" "<<std::endl;

    //To print the times of calculation of the distance

    /*std::chrono::steady_clock::time_point end_clock_item_add_hdr_0 = std::chrono::steady_clock::now();  //For checking of the search time on hdr tree, ending

    search_time_HDR=search_time_HDR+std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_item_add_hdr_0 - begin_clock_item_add_hdr_0).count();*/


    if(Rp.size()>0){
        for(auto u:Rp){
            u->computeKNN_sort(slidingWindow);
        }
    }

    AdjustMaxdknnPaper(root);    //Put the function for the tree adjustment within the if structure or not???
};


//Function for the deletion update when using only one HDR tree for the users for comparison with the optimization deletion method, using computeKNN_sort.

//The function for the distance between item and user cluster, combination of dimension reduction and distance calculation


float HDR_Tree::distance_u_ucluster(Item *i, Cluster *c) {
    numDistanceComputions++;
    float res = (data->T[c->l-1] * i->v - c->center).norm();
    return res - c->radius;
}

//The function for the distance between item and user cluster, combination of dimension reduction and distance calculation

//The function for building the little tree The function for building the little tree The function for building the little tree

int HDR_Tree::construct_littletree(std::vector<User *> *tree_user, LeafNode *tree_node, NonLeafNode*tree_root, DataBase *tree_data,
                                    long fan, long thresh) {
    std::cout << "Constructing little HDR Tree" << std::endl;
    std::cout << "Users: " << tree_user->size() << std::endl;
    std::cout << "Features: " << numFeatures << std::endl;
    //std::cout << "Window: " << windowSize << std::endl;
    std::cout << "Fanout: " << fan << std::endl << std::endl;
    std::cout << "Threshold: "<<thresh<<std::endl<<std::endl;

    //init();

    tree_data->U.resize(tree_user->size(), numFeatures);

    int k = 0;
    for (long index = 0; index < tree_user->size(); index++)
    {

        tree_data->U.row(k) = (*tree_user)[index]->center;
        k = k + 1;
    }

    tree_data->computePCA();

    if(tree_user->size()<thresh){
        free(tree_root);
        auto C_single=new Cluster;
        for(auto user_less:*tree_user){
            C_single->Users.emplace_back(std::make_tuple(0,user_less));
        }
        tree_node->cluster=C_single;
        return 0;
    }else{
        free(tree_node);
        int L_tem;
        L_tem = log(tree_user->size()) / log(fan);
        std::cout << "Estimated Height: " << L_tem << " levels." << std::endl;

        tree_data->initTransform(L_tem);


// Be careful, here the low versions of the users and items are produced and stored in the specific containners of the users and items.
        /*for(auto u_ini: Users){
            projectUser(u_ini);
            for(int count_d=0; count_d<L; count_d=count_d+1){
                u_ini->u_low_d.emplace_back(uTrans[count_d]);
            }
        }


        for(auto i_ini: slidingWindow){
            projectItem(i_ini);
            for(int count_d_0=0; count_d_0<L_i; count_d_0=count_d_0+1){
                i_ini->i_low_d.emplace_back(iTrans[count_d_0]);
            }
        }*/

// Be careful, here the low versions of the users and items are produced and stored in the specific containners of the users and items.


        long numClusters = 0;

        auto C = new Clusters(*tree_user, fan, tree_data->T[0]);
        for (auto Cj : *C) {

            if (Cj->Users.size() > 0) {
                Cj->maxdknn = std::get<1>(Cj->Users[0])->dknn;
                for (auto t : Cj->Users) {
                    Cj->maxdknn = std::max(Cj->maxdknn, std::get<1>(t)->dknn);
                }
            }

            numClusters += 1;
            Cj->l = 1;
        }




        tree_root->l=1;
        //root = new NonLeafNode(C, 1, false);

        //tree_node->clusters=C->children;
        for(auto cluster_begin: C->children){
            tree_root->clusters.emplace_back(cluster_begin);
        }

        std::cout<<"Successfull goinng out"<<std::endl;

        std::queue<Node *> q;

        q.push(tree_root);

        long maxL = 0;

        while (!q.empty()) {
            auto node = dynamic_cast<NonLeafNode *>(q.front());
            q.pop();

            auto Cp = node->clusters;

            for (auto Cj : Cp) {

                if (Cj->number < thresh) {
                    auto LN = new LeafNode(Cj, false);

                    node->children.emplace_back(LN);
                    Cj->ptr = LN;

                    //Be careful, here, the user points to its belonging leafnode, Be careful, here, the user points to its belonging leafnode, Be careful, here, the user points to its belonging leafnode

                    for(auto user_leaf: LN->cluster->Users){
                        std::get<1>(user_leaf)->belong_leaf=LN;
                    }

                    //Be careful, here, the user points to its belonging leafnode, Be careful, here, the user points to its belonging leafnode, Be careful, here, the user points to its belonging leafnode

                }
                else {
                    auto level = node->l + 1;
                    maxL = std::max(maxL, level);

                    if(level>L_tem){
                        std::cout<<"Excceed the estimated level"<<std::endl;
                    }

                    auto Cpp = new Clusters(Cj, fan, tree_data->T[level-1]);
                    for (auto Cj : *Cpp) {

                        if (Cj->Users.size() > 0) {
                            Cj->maxdknn = std::get<1>(Cj->Users[0])->dknn;
                            for (auto t : Cj->Users) {
                                Cj->maxdknn = std::max(Cj->maxdknn, std::get<1>(t)->dknn);
                            }
                        }

                        numClusters += 1;
                        Cj->l = level;
                    }

                    auto NLN = new NonLeafNode(Cpp, level, false);

                    node->children.emplace_back(NLN);
                    Cj->ptr = NLN;
                    q.push(NLN);
                }
            }
        }

        std::cout << "Actual Height: " << maxL << " levels." << std::endl;
        std::cout << "Clusters: " << numClusters << std::endl;
        return L_tem;
    }







}


//The function for building the little tree The function for building the little tree The function for building the little tree

//The function for building the clusters of the users The function for building the clusters of the users The function for building the clusters of the users

void HDR_Tree::build_clustersu(long n_c) {
    clusters_user_tree=new Clustersu(Users, n_c);
}

//The function for building the clusters of the users The function for building the clusters of the users The function for building the clusters of the users

//The function for constructing the clusters for building the trees The function for constructing the clusters for building the trees The function for constructing the clusters for building the trees

void HDR_Tree::complete_little_trees(long n_t, long l_f, long l_thresh){
    int divide_r;
    double r1;
    double r2;

    //This is for calculate the sequence of the cluster computed

    int k_cluster=1;

    //This is for calculate the sequence of the cluster computed

    for (auto cluster_u: *clusters_user_tree){

        //This is for calculate the sequence of the cluster computed

        std::cout<<"The "<<k_cluster<<"th cluster is being computed"<<std::endl;

        //This is for calculate the sequence of the cluster computed

        //This is for showing the number of users that the being computed cluster contains

        std::cout<<"The "<<k_cluster<<"th cluster has "<<cluster_u->Users.size()<<" users"<<std::endl;

        //This is for showing the number of users that the being computed cluster contains

        //This is for showing the radius of the being computed cluster

        std::cout<<"The "<<k_cluster<<"th cluster has the radius of "<<cluster_u->radius<<std::endl;

        //This is for showing the radius of the being computed cluster

        //Seperate the description of the cluster and the blocks

        std::cout<<" "<<std::endl;

        //Seperate the description of the cluster and the blocks

        for(divide_r=1; divide_r<=n_t; divide_r++){

            //This is for show the sequence of the block that is being computed

            std::cout<<"The "<<divide_r<<"th block is being computed"<<std::endl;

            //This is for show the sequence of the block that is being computed


            auto v_section=new std::vector<User*>;
            std::vector<std::tuple<double, User*>> distance_user;
            r1=cluster_u->radius*(divide_r-1)*1.0/(n_t*1.0);
            r2=cluster_u->radius*(divide_r)*1.0/(n_t*1.0);

            //This is for showing the lower bound and the upper bound of a block

            std::cout<<"The "<<divide_r<<"th block in the "<<k_cluster<<"th cluster has the lower bound of "<<r1<<" in the radius of "<<cluster_u->radius<<std::endl;

            std::cout<<"The "<<divide_r<<"th block in the "<<k_cluster<<"th cluster has the upper bound of "<<r2<<" in the radius of "<<cluster_u->radius<<std::endl;


            //This is for showing the lower bound and the upper bound of a block


            for(auto user_l:cluster_u->Users){
                if((std::get<0>(user_l)>r1)&&(std::get<0>(user_l)<=r2)){
                    distance_user.emplace_back(user_l);
                    v_section->emplace_back(std::get<1>(user_l));
                }
            }
            if(v_section->size()==0){
                free(v_section);

                //For informing that the block is empty
                std::cout<<"The "<<divide_r<<"th block is empty"<<std::endl;
                //For informing that the block is empty
            }else{

                //For informing the number of users in a block

                std::cout<<"The "<<divide_r<<"th block has "<<v_section->size()<<" users"<<std::endl;

                //For informing the number of users in a block


                cluster_u->section_user.emplace_back(v_section);
                std::vector<float> store_dknn;
                for(auto u_little:*v_section){
                    store_dknn.emplace_back(u_little->dknn);
                }
                std::sort(store_dknn.begin(),  store_dknn.end(),
                                 [&](const float& a, const float& b) -> bool {
                                     return a <=b;
                                 });
                std::sort(distance_user.begin(), distance_user.end(),
                                 [&](const std::tuple<double, User*>& a, const std::tuple<double, User* >& b) -> bool {
                                     return std::get<0>(a) <= std::get<0>(b);
                                 });
                //Clusters* substitude;

                //The information for the block

                std::cout<<"The maxdknn of the "<<divide_r<<"th block in the "<<k_cluster<<"th cluster is "<<store_dknn[store_dknn.size()-1]<<std::endl;
                std::cout<<"The real lower bound of the "<<divide_r<<"th block in the "<<k_cluster<<"th cluster is "<<std::get<0>(distance_user[0])<<std::endl;
                std::cout<<"The real upper bound of the "<<divide_r<<"th block in the "<<k_cluster<<"th cluster is "<<std::get<0>(distance_user[distance_user.size()-1])<<std::endl;
                //The information for the block

                //Start building tree

                std::cout<<"Start building the tree for the "<<divide_r<<"th block in the "<<k_cluster<<"th cluster"<<std::endl;

                //Start building tree
                std::cout<<"Temporary check, start, if the nullptr can be used to initialize a nonleafnode"<<std::endl;

                auto little_root=new NonLeafNode();
                little_root->type=NodeType::NonLeafNode;

                auto little_node=new LeafNode();
                little_node->type=NodeType::LeafNode;

                std::cout<<"Temporary check, complete, if the nullptr can be used to initialize a nonleafnode"<<std::endl;
                auto little_data=new DataBase;
                little_data->dim=numFeatures;
                int height_return=construct_littletree(v_section,little_node, little_root, little_data, l_f, l_thresh);
                auto little_tree=new Littletree;
                little_tree->height=height_return;
                little_tree->littledatabase=little_data;
                little_tree->whole_maxdknn=store_dknn[store_dknn.size()-1];
                little_tree->whole_radius=std::get<0>(distance_user[distance_user.size()-1]);
                little_tree->whole_low_radius=std::get<0>(distance_user[0]);
                if(height_return==0){
                    little_tree->littleroot=little_node;
                }else{
                    little_tree->littleroot=little_root;
                }
                //little_tree->height=height_return;
                little_tree->little_section=v_section;
                cluster_u->little_tree.emplace_back(little_tree);

                //Complete building tree

                std::cout<<"Complete building the tree for the "<<divide_r<<"th block in the "<<k_cluster<<"th cluster"<<std::endl;

                //Complete building tree


            }
            //For dividing the description between different blocks

            std::cout<<" "<<std::endl;

            //For dividing the description between different blocks
        }

        //This is for calculate the sequence of the cluster computed

        k_cluster=k_cluster+1;

        //This is for calculate the sequence of the cluster computed

        //This is for divide the description of different blocks in a cluster

        std::cout<<" "<<std::endl;
        std::cout<<" "<<std::endl;
        std::cout<<" "<<std::endl;

        //This is for divide the description of different blocks in a cluster
    }
}

//The function for constructing the clusters for building the trees The function for constructing the clusters for building the trees The function for constructing the clusters for building the trees


//A kind of projection for projecting the item by the little data A kind of projection for projecting the item by the little data A kind of projection for projecting the item by the little data

void HDR_Tree::project_item_little(Item* i_p, DataBase* db_l, int height_in){
    iTrans.clear();
    for (int i = 0; i < height_in; i++) {
        iTrans.emplace_back(db_l->T[i] * (i_p->v));
    }
}

//A kind of projection for projecting the item by the little data A kind of projection for projecting the item by the little data A kind of projection for projecting the item by the little data


//Delete item using multiple hdr trees. Delete item using multiple hdr trees. Delete item using multiple hdr trees. Delete item using multiple hdr trees.

void HDR_Tree::update_item_multipletree(){
    auto item_out=slidingWindow[0];
    slidingWindow.erase(slidingWindow.begin());

    //Rp.clear();

    //root_influenced.clear();

    //std::chrono::steady_clock::time_point begin_clock_item_add = std::chrono::steady_clock::now();  //For checking of the projection time, beginning

    //int test_0=2;

    //projectItem(item_out);

    /*std::chrono::steady_clock::time_point end_clock_item_add = std::chrono::steady_clock::now();  //For checking of the projection time, ending

    int test_1=3;

    projection_time=projection_time+test_1-test_0;//std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_item_add - begin_clock_item_add).count();   //Add the projection time

    std::chrono::steady_clock::time_point begin_clock_item_add_hdr_0 = std::chrono::steady_clock::now();  //For checking of the search time on hdr tree, beginning*/

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    //num_hdr_cal_clusters=0;
    //num_hdr_cal_users=0;

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    //Search(root, item_out);

    /*std::cout<<"The number of influenced users "<<Rp.size()<<std::endl;

    std::cout<<"The dknn values before recomputing the knn lists of deletion"<<std::endl;

    for(auto user_before: Rp){
        std::cout<<user_before->dknn<<" ";
    }
    std::cout<<" "<<std::endl;*/

    //To print the times of calculation of the distance

    //std::cout<<"The times of calculation of the distance between the item and the clusters is "<<num_hdr_cal_clusters<<std::endl;
    //std::cout<<"The times of calculation of the distance between the item and the users is "<<num_hdr_cal_users<<std::endl;
    //std::cout<<" "<<std::endl;

    //To print the times of calculation of the distance

    /*std::chrono::steady_clock::time_point end_clock_item_add_hdr_0 = std::chrono::steady_clock::now();  //For checking of the search time on hdr tree, ending

    search_time_HDR=search_time_HDR+std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_item_add_hdr_0 - begin_clock_item_add_hdr_0).count();*/

    //int size_rp=Rp.size();
    double distance_icu;
    for(auto cluster_full:*clusters_user_tree){
        distance_icu=dist_cu_i(cluster_full, item_out);
        for(auto tree_structure:cluster_full->little_tree){
            if((distance_icu<=tree_structure->whole_radius+tree_structure->whole_maxdknn)&&(distance_icu>=tree_structure->whole_low_radius-tree_structure->whole_maxdknn)){
                if(tree_structure->littleroot->type==NodeType::NonLeafNode){
                    project_item_little(item_out, tree_structure->littledatabase, tree_structure->height);
                    tree_structure->littlerp.clear();
                    Search_block(tree_structure->littleroot,item_out,tree_structure);
                    if(tree_structure->littlerp.size()!=0){
                        for(auto user_partial:tree_structure->littlerp){

                        }
                        AdjustMaxdknnPaper_littletree(tree_structure->littleroot, tree_structure);
                        std::vector<float> dknnlist;
                        for(auto user_partial:tree_structure->littlerp){
                            dknnlist.emplace_back(user_partial->dknn);
                        }
                        std::sort(dknnlist.begin(),  dknnlist.end(),
                                  [&](const float& a, const float& b) -> bool {
                                      return a <=b;
                                  });
                        if(dknnlist[dknnlist.size()-1]>tree_structure->whole_maxdknn){
                            tree_structure->whole_maxdknn=dknnlist[dknnlist.size()-1];
                        }
                    }
                }else{
                    tree_structure->littlerp.clear();
                    Search_block(tree_structure->littleroot,item_out,tree_structure);
                    if(tree_structure->littlerp.size()!=0){
                        for(auto user_partial:tree_structure->littlerp){
                            user_partial->computeKNN(slidingWindow);
                        }
                        std::vector<float> dknnlist;
                        for(auto user_partial:tree_structure->littlerp){
                            dknnlist.emplace_back(user_partial->dknn);
                        }
                        std::sort(dknnlist.begin(),  dknnlist.end(),
                                  [&](const float& a, const float& b) -> bool {
                                      return a <=b;
                                  });
                        if(dknnlist[dknnlist.size()-1]>tree_structure->whole_maxdknn){
                            tree_structure->whole_maxdknn=dknnlist[dknnlist.size()-1];
                        }
                    }
                }
            }
        }
    }

    /*std::cout<<"The dknn values after recomputing the knn lists of deletion"<<std::endl;

    for(auto user_after:Rp){
        std::cout<<user_after->dknn<<" ";
    }

    std::cout<<" "<<std::endl;

    std::cout<<" "<<std::endl;*/
};

//Delete item using multiple hdr trees. Delete item using multiple hdr trees. Delete item using multiple hdr trees. Delete item using multiple hdr trees.

//Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees.

void HDR_Tree::update_item_add_multipletree(){
    auto new_item = new Item(data_i->I.row(numItems_i + add_item_num));
    slidingWindow.emplace_back(new_item);
    //update_item_hdr(itemRoot_i, new_item, Mode::Add);

    //Rp.clear();

    //root_influenced.clear();

    //std::chrono::steady_clock::time_point begin_clock_item_add = std::chrono::steady_clock::now();  //For checking of the projection time, beginning

    //int test_0=2;

    //projectItem(item_out);

    /*std::chrono::steady_clock::time_point end_clock_item_add = std::chrono::steady_clock::now();  //For checking of the projection time, ending

    int test_1=3;

    projection_time=projection_time+test_1-test_0;//std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_item_add - begin_clock_item_add).count();   //Add the projection time

    std::chrono::steady_clock::time_point begin_clock_item_add_hdr_0 = std::chrono::steady_clock::now();  //For checking of the search time on hdr tree, beginning*/

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    //num_hdr_cal_clusters=0;
    //num_hdr_cal_users=0;

    //To initialize the count values for the times of calculation of the distances between the clusters and the item and of the distances between the users and the item

    //Search(root, item_out);

    /*std::cout<<"The number of influenced users "<<Rp.size()<<std::endl;

    std::cout<<"The dknn values before recomputing the knn lists of deletion"<<std::endl;

    for(auto user_before: Rp){
        std::cout<<user_before->dknn<<" ";
    }
    std::cout<<" "<<std::endl;*/

    //To print the times of calculation of the distance

    //std::cout<<"The times of calculation of the distance between the item and the clusters is "<<num_hdr_cal_clusters<<std::endl;
    //std::cout<<"The times of calculation of the distance between the item and the users is "<<num_hdr_cal_users<<std::endl;
    //std::cout<<" "<<std::endl;

    //To print the times of calculation of the distance

    /*std::chrono::steady_clock::time_point end_clock_item_add_hdr_0 = std::chrono::steady_clock::now();  //For checking of the search time on hdr tree, ending

    search_time_HDR=search_time_HDR+std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_item_add_hdr_0 - begin_clock_item_add_hdr_0).count();*/

    double distance_icu;
    for(auto cluster_full:*clusters_user_tree){
        distance_icu=dist_cu_i(cluster_full, new_item);
        for(auto tree_structure:cluster_full->little_tree){
            if((distance_icu<=tree_structure->whole_radius+tree_structure->whole_maxdknn)&&(distance_icu>=tree_structure->whole_low_radius-tree_structure->whole_maxdknn)){
                if(tree_structure->littleroot->type==NodeType::NonLeafNode){
                    project_item_little(new_item, tree_structure->littledatabase, tree_structure->height);
                    tree_structure->littlerp.clear();
                    Search_block(tree_structure->littleroot,new_item,tree_structure);
                    if(tree_structure->littlerp.size()!=0){
                        for(auto user_partial:tree_structure->littlerp){
                            user_partial->update(new_item,Mode::Add);
                        }
                        AdjustMaxdknnPaper_littletree(tree_structure->littleroot, tree_structure);
                        std::vector<float> dknnlist;
                        for(auto user_partial:*(tree_structure->little_section)){
                            dknnlist.emplace_back(user_partial->dknn);
                        }
                        std::sort(dknnlist.begin(),  dknnlist.end(),
                                  [&](const float& a, const float& b) -> bool {
                                      return a <=b;
                                  });
                        tree_structure->whole_maxdknn=dknnlist[dknnlist.size()-1];

                    }
                }else{
                    tree_structure->littlerp.clear();
                    Search_block(tree_structure->littleroot,new_item,tree_structure);
                    if(tree_structure->littlerp.size()!=0){
                        for(auto user_partial:tree_structure->littlerp){
                            user_partial->update(new_item, Mode::Add);
                        }
                        std::vector<float> dknnlist;
                        for(auto user_partial:*(tree_structure->little_section)){
                            dknnlist.emplace_back(user_partial->dknn);
                        }
                        std::sort(dknnlist.begin(),  dknnlist.end(),
                                  [&](const float& a, const float& b) -> bool {
                                      return a <=b;
                                  });
                        tree_structure->whole_maxdknn=dknnlist[dknnlist.size()-1];

                    }
                }
            }
        }
    }
    add_item_num = add_item_num + 1;
};

//Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees.

//For searching on the blocks, for searching on the blocks, for searching on the blocks, for searching on the blocks

void HDR_Tree::Search_block(Node* node, Item* I, Littletree* lp_pointer){
    if (node->type == NodeType::LeafNode) {
        auto LN = dynamic_cast<LeafNode*>(node);
        for (auto Ut : *LN) {

            num_hdr_cal_users=num_hdr_cal_users+1;   //Add the calculation time of the users

            auto user = std::get<1>(Ut);
            if (dist(I, user) < user->dknn) {
                lp_pointer->littlerp.emplace_back(user);
            }
        }
    }
    else {
        auto NLN = dynamic_cast<NonLeafNode*>(node);
        auto& Cp = NLN->clusters;


        float d1,d2;

        for (long j = 0; j < Cp.size(); j++) {

            num_hdr_cal_clusters=num_hdr_cal_clusters+1;   //Add the calculation time of the clusters

            d1 = dist(I, Cp[j]);
            d2 = Cp[j]->maxdknn;
            if (d1 < d2) {
                Search_block(NLN->children[j], I, lp_pointer);
            }
        }
    }
}

//For searching on the blocks, for searching on the blocks, for searching on the blocks, for searching on the blocks



//For adjusting the maxdknn of the little trees For adjusting the maxdknn of the little trees For adjusting the maxdknn of the little trees

bool HDR_Tree::AdjustMaxdknnPaper_littletree(Node* n, Littletree* lpt_pointer){
    bool result = false;

    if (n->type == NodeType::LeafNode) {
        auto LN = dynamic_cast<LeafNode*>(n);

        long Len = lpt_pointer->littlerp.size();
        for (auto Ut : *LN) {
            auto& user = std::get<1>(Ut);
            for (long i = 0; i < Len; i++) {
                if (lpt_pointer->littlerp[i] == user) {
                    return true;
                }
            }
        }
    }
    else {
        auto NLN = dynamic_cast<NonLeafNode*>(n);
        auto& Cp = NLN->clusters;

        for (auto Cj : Cp) {
            bool res = AdjustMaxdknnPaper_littletree(Cj->ptr, lpt_pointer);

            if (res) {
                result = true;

                float Dist = 0;

                if (Cj->ptr->type == NodeType::LeafNode) {

                    auto LN = dynamic_cast<LeafNode*>(Cj->ptr);
                    for (auto Ut : *LN) {

                        auto user = std::get<1>(Ut);
                        Dist = std::max(Dist, user->dknn);
                    }
                }
                else {

                    auto NLN = dynamic_cast<NonLeafNode*>(Cj->ptr);
                    auto Cp = NLN->clusters;

                    for (auto Cj : Cp) {
                        Dist = std::max(Dist, Cj->maxdknn);
                    }
                }

                Cj->maxdknn = Dist;
            }
        }
    }

    return result;
};


//For adjusting the maxdknn of the little trees For adjusting the maxdknn of the little trees For adjusting the maxdknn of the little trees