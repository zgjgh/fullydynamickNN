#pragma once

#include <queue>

#include "DataBase.h"

#include "Clusters.h"

#include "NonLeafNode.h"

#include "LeafNode.h"

#include "Item.h"

#include "itemHDR_Tree.h"  //carefull carefull carefull carefull carefull carefull carefull carefull carefull carefull carefull

#include <thread>

#include "Clustersi.h"

#include "BPlusTree.h"

#include "Clustersu.h"

#include "userLeafNode.h"

#include "userNonLeafNode.h"

//#include <chrono>

#include <windows.h>

typedef const Eigen::Ref<const Eigen::VectorXf>& VectorRef;

class HDR_Tree : itemHDR_Tree
{
public:
	HDR_Tree();
	~HDR_Tree();

	void setData(DataBase* dataBase);

	void construct(long fanout, long threshold);
	void updateConstruct(Node* node);


	void stream();
	void userStream();

	void remove();

	void sync();

	long k;
	long windowSize;
	long numFeatures;
	bool verbose;
	long numUsers;
	bool checkDuplicates;
	float updateTime;

	std::vector<User*> Rp;
	void Search(VectorRef V);

	long numDistanceComputions;
	long numKnnComputions;
    DataBase* data;
    std::vector<User*> Users;

//item_hdr_tree_feature item_hdr_tree_feature item_hdr_tree_feature item_hdr_tree_feature item_hdr_tree_feature item_hdr_tree_feature item_hdr_tree_feature item_hdr_tree_feature

    long k_i;
    long windowSize_i;
    long numFeatures_i;
    bool verbose_i;
    long numItems_i;
    bool checkDuplicates_i;
    float updateTime_i;
    std::vector<User*> Rp_i;
    long numDistanceComputions_i;
    long numKnnComputions_i;
    itemNode* itemRoot_ptr_i;
    itemDataBase* data_i;
    std::vector<Eigen::VectorXf> iTrans_i;
    std::vector<Eigen::VectorXf> uTrans_i;
    long itemIndex_i;
    std::vector <Item*> slidingWindow_i;
    long  threshold_i;
    long  fix_threshold_i = 5;
    std::chrono::steady_clock::time_point begin_i;
    std::vector<Item*> Items_i;
    long L_i;
    itemNode* itemRoot_i;
    std::vector<Eigen::VectorXf> Trans_u_i;
    std::vector <Item*> slidingWindow;
    std::vector<Eigen::VectorXf> u_trans_i_clus_init;
    void project_u_iclust_pca_init(User* U);
    float dist_u_iclust_init(itemCluster* C);



//item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function item_hdr_tree_function
    float dist_i(Item* I, itemCluster* C);
    float dist_i(User* U, itemCluster* C);
    float dist_i(Item* I, User* U);
    float dist_i(VectorRef A, VectorRef B);
    void projectItem_i(Item* I);
    void setData_i(itemDataBase* dataBase);
    void init_i();
    void construct_i(long fanout, long threshold);
    void projectUser_item(User* U);
    float dist_u_ic(User* U, itemCluster* C);
    float dist_u_ic_str(User* U, itemCluster* C);
    void update_user_hdr(User* U, Mode m, Node* node);
    float init_time;
    float computation_of_distance_init;

    void update_item_hdr(itemNode* root_item_tree, Item* I, Mode m);

    float dist_i_ic_update_itree(Item* I, itemCluster* IC);
    float dist_u_uc_update_utree(User* U, Cluster* UC);
    void update_user();
    void update_item();

    long add_user_num;
    long add_item_num;

    void update_user_add();

    void update_item_add();

    void update_user_user_tree();

    void update_user_add_user_tree();

    void update_item_user_tree();

    void update_item_add_user_tree();







//item_hdr_tree item_hdr_tree item_hdr_tree item_hdr_tree item_hdr_tree item_hdr_tree item_hdr_tree item_hdr_tree item_hdr_tree item_hdr_tree

//The features for the idistance and b+ tree indexing structure

    Clustersi* clusters_item;     //For storing the clusters of items on full dimension for confirmming the reference points and the space between the real value corresponding to the reference points on the axis

    BPlusTree* bptree_item;     //For storing the real value corresponding to the points on the axis.

    double max_r_item_cluster;   //For storing the max radius of the item clusters on full dimension to confirm the space between the real value corresponding to the reference points

    void form_item_clusters(long f);   //For formming the clusters of the items on full dimension

    void indexing_item();    //For indexing the items from high dimensional data poiints to real value

    void build_bp_tree();    //For building a bplus tree to store the keys and the item*

    void insert_key_point(double key, Item* it);   //For inserting the key-pointer pairs to the bplus tree

    void complete_tree();   //Complete the initial state of the bptree_item

    double distance_ci_u(User* u, Clusteri* ci);        //Compute the distance between the center of an clusteri and an user
    //Be careful, change the type of the return value from float to double Be careful, change the type of the return value from float to double Be careful, change the type of the return value from float to double
    //Be careful, change the type of the return value from float to double Be careful, change the type of the return value from float to double Be careful, change the type of the return value from float to double


    void user_computeknn_id(User* u, double r_rate, double delta_r_rate, bool print_if);   //Use idistance and B_plus tree to compute the knn list of an user
    int distance_time;       //The times of calculation of the distances



//The features for the idistance and b+ tree indexing structure

//The features for the sphere tree

    userNode* root_sphere;
    void construct_sphere(long fanout, long threshold);
    double dist_cu_i(Clusteru* cu, Item* i);
    void search_sphere(userNode* node_sphere, Item* i);
    double dist_ud_id(User* ud, Item* id);
    std::vector<User*> Rp_sphere;
    bool AdjustMaxdknnsphere(userNode* n);
    void update_sphere(User* U, Mode m, userNode* node);
    float dist_cu_u(Clusteru* cu, User* u);
    void update_user_spheretree();
    void update_user_add_spheretree();
    void update_item_spheretree();
    void update_item_add_spheretree();


//The features for the sphere tree


//The features for checking the power of the sphere tree

    int num_sphere_cal_clusters;
    int num_sphere_cal_users;


//The features for checking the power of the sphere tree

//The features for checking the power of the HDR tree

    int num_hdr_cal_clusters;
    int num_hdr_cal_users;


//The features for checking the power of the HDR tree

//The features for checking the projection time and the search time for the HDR tree

    float projection_time;
    float search_time_HDR;

//The features for checking the projection time and the search time for the HDR tree

//The features for checking the search time for the sphere tree

    float search_time_spheretree;

//The features for checking the search time for the sphere tree

//The features for update the knn lists and the indexing structure of the KNN+ method

    void update_user_knnjoinplus();
    void update_user_add_knnjoinplus();
    void update_item_knnjoinplus();
    void update_item_add_knnjoinplus();

    double distance_item_clusteri(Item* i_item, Clusteri* c_clusteri);



//The features for update the knn lists and the indexing structure of the KNN+ method


//The function for optimization of deletion


    void update_item_op_delet_usertree();
    std::vector<User*> Rp_0;
    void search_deletion_opt(Node* node, Item*I);


//The function for optimization of deletion


//The function for the deletion update when using only one HDR tree for the users for comparison with the optimization method, using computeKNN_sort.

    void update_item_user_tree_compute_sort();


//The function for the deletion update when using only one HDR tree for the users for comparison with the optimization method, using computeKNN_sort.

//The function for the distance between item and user cluster, combination of dimension reduction and distance calculation


    float distance_u_ucluster(Item* i, Cluster* c);



//The function for the distance between item and user cluster, combination of dimension reduction and distance calculation


//The function for constructing the little tree The function for constructing the little tree The function for constructing the little tree The function for constructing the little tree

    int construct_littletree(std::vector<User*>* tree_user, LeafNode* tree_node, NonLeafNode* tree_root, DataBase* tree_data, long fan, long thresh);

//The function for constructing the little tree The function for constructing the little tree The function for constructing the little tree The function for constructing the little tree

//The clusters for constructing the little trees The clusters for constructing the little trees The clusters for constructing the little trees The clusters for constructing the little trees

    Clustersu* clusters_user_tree;

//The clusters for constructing the little trees The clusters for constructing the little trees The clusters for constructing the little trees The clusters for constructing the little trees

//The function for constructing the clusters for building the trees The function for constructing the clusters for building the trees The function for constructing the clusters for building the trees

    void build_clustersu (long n_c);

//The function for constructing the clusters for building the trees The function for constructing the clusters for building the trees The function for constructing the clusters for building the trees

    void complete_little_trees(long n_t, long l_f, long l_thresh);

//The function for constructing the clusters for building the trees The function for constructing the clusters for building the trees The function for constructing the clusters for building the trees

//A kind of projection for projecting the item by the little data A kind of projection for projecting the item by the little data A kind of projection for projecting the item by the little data

    void project_item_little(Item* i_p, DataBase* db_l, int height_in);

//A kind of projection for projecting the item by the little data A kind of projection for projecting the item by the little data A kind of projection for projecting the item by the little data

//Delete item using multiple hdr trees. Delete item using multiple hdr trees. Delete item using multiple hdr trees. Delete item using multiple hdr trees.

    void update_item_multipletree();

//Delete item using multiple hdr trees. Delete item using multiple hdr trees. Delete item using multiple hdr trees. Delete item using multiple hdr trees.

//Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees.

    void update_item_add_multipletree();

//Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees. Add item using multiple hdr trees.

//Storage of the roots of the influenced trees. Storage of the roots of the influenced trees. Storage of the roots of the influenced trees. Storage of the roots of the influenced trees.

    std::vector<Node*> root_influenced;

//Storage of the roots of the influenced trees. Storage of the roots of the influenced trees. Storage of the roots of the influenced trees. Storage of the roots of the influenced trees.


//The features for cross optimization; The features for cross optimization; The features for cross optimization; The features for cross optimization;


    std::vector<vector<itemCluster*>*> dictionary_for_layer_itemcluster;
    std::vector<int*> record_sequence;





//The features for cross optimization; The features for cross optimization; The features for cross optimization; The features for cross optimization;





private:


	std::vector<Eigen::VectorXf> iTrans;
	std::vector<Eigen::VectorXf> uTrans;

	long itemIndex;



	long  threshold;
	long  fix_threshold=5;

	float dist(Item* I, Cluster* C);
	float dist(User* U, Cluster* C);
	float dist(Item* I, User* U);
	float dist(VectorRef A, VectorRef B);

	void Search(Node* node, Item* I);

	//For searching on the blocks, for searching on the blocks, for searching on the blocks, for searching on the blocks

    void Search_block(Node* node, Item* I, Littletree* lt_pointer);

    //For searching on the blocks, for searching on the blocks, for searching on the blocks, for searching on the blocks

	void updateUser(Node* node, User* U);

	std::chrono::steady_clock::time_point begin;

	void initUsersSerial();
	void UsersParalell(int o, int n);

	void initUsersParalell();

	void insert(Item* I);
	void userInsert(User* U);
	void fillTree();
	void update(Item* I, Mode m);
	void userUpdate(User* U, Mode m);
	void UpdateNaiveRKNN(Item* I, Mode m);
	void UpdateHdrKNN(Item* I, Mode m);
	void userUpdateHdrKNN(User* U, Mode m);

	float AdjustMaxdknnPrev(Node* n);
	bool AdjustMaxdknnPaper(Node* n);

//For adjusting the maxdknn of the little trees For adjusting the maxdknn of the little trees For adjusting the maxdknn of the little trees

    bool AdjustMaxdknnPaper_littletree(Node* n, Littletree* lpt_pointer);


//For adjusting the maxdknn of the little trees For adjusting the maxdknn of the little trees For adjusting the maxdknn of the little trees





	void updateUsers(Item* I, Mode m, int o, int n);
	void updateUsersParalell(Item* I, Mode m);
	void updateUsersSerial(Item* I, Mode m);

	void userComputeKNN(itemNode* node, User* user);

	//float idist(float& D, itemCluster* C);

	/*void dist(Item* I, float& D);*/
	float dist(User* U, struct itemCluster* C);
	float dist(User* U, Item* I);

	long L;

	Node* root;
	NonLeafNode* parent;

	void init();

	void streamLinear();
	void streamRandom();
	void userStreamRandom();
	void streamIndex(long ind);
	void userStreamIndex(long ind);

	void syncUsers(int o, int n);

	void projectItem(Item* I);
	void projectUser(User* U);
};

