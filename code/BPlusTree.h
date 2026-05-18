#pragma once
//每个叶结点所能存储的最大数量
#define LEAF_ORDER 300
#define ORDER 300
#define MAX_NUM 301

#include <string>
using namespace std;
#include "Item.h"
#include <vector>

/*
在B+树中，每个分支结点的关键字和子树之间的关系是：
第0颗子树的关键字都小于第0个关键字，
之后索引从i开始，i>=0，i+1的子树的关键字
都大于等于i的关键字，小于i+1关键字。
注意关键字和子树的索引都是从0开始的。
*/

//模拟40byte的内存单元
struct InternalNodeData{
    //键值数组，指针数组
    double m_Keys[ORDER];      //1. Change the keys of the internodes from integer to float to fit the data type of the distances between points
    void * m_Pointers[MAX_NUM];
};

//模拟40字节的内存单元
struct LeafNodeData{
    double m_Keys[LEAF_ORDER];
    Item* m_number[LEAF_ORDER];      //To change the data type of the elements in m_number
    void* m_next;
};

class bpNode{
public:
    //键的数量
    int m_nKeyNum;
    //是不是叶结点
    bool m_bLeaf;
    //父亲结点
    bpNode *m_pParent{};
    bpNode()
    {
        m_nKeyNum = 0;
        m_bLeaf = false;
    }
};

class InternalNode:public bpNode{
public:
    InternalNodeData* internalNodeData;
    InternalNode(){
        internalNodeData = new InternalNodeData;
        for(double & m_Key : internalNodeData->m_Keys){
            m_Key=0;
        }
        for(int i=0;i<LEAF_ORDER;i++){
            internalNodeData->m_Pointers[i]=nullptr;
        }
    };
};

class bpLeafNode:public bpNode{
public:
    LeafNodeData* leafNodeData;
    bpLeafNode(){
        leafNodeData = new LeafNodeData;
        for(int i=0;i<ORDER;i++){
            leafNodeData->m_Keys[i]=0;
        }
        for(int i=0;i<MAX_NUM;i++){
            leafNodeData->m_number[i]= nullptr;     //To change the data type of the elements in m_number
        }
        leafNodeData->m_next = nullptr;
    };
};
class BPlusTree
{
public:
	BPlusTree();
	~BPlusTree();
private:
    //根结点
	bpNode * m_pRoot;
    //第一个叶子结点
	bpNode * m_pFirst;
	bpNode * m_pLast;
public:
    //叶子结点插入
	bool Insert(double nKey, Item* number);     //To change the value in the key-value pair from double to item pointer
    //删除
	bool Remove(double nKey);
	//内部结点插入
	bool InsertKeyAndPointer(bpNode * pParent, bpNode * pOld, double nKey, bpNode * pNew);
	//打印叶子结点值
	void PrintLeaves();
	//查找某个值是否存在
    bpLeafNode * search(double nKey);
    //分层打印
    void PrintLayerTree();
    //以一个leafnode为起点，将起点及其后点中的符合key在a,b之间的item*加入参数指针所指容器中
    void get_range_item(std::vector<Item*>* pointer,  double a, double b, bool left_bool, bool right_bool, bool print_if );

    //std::vector<std::tuple<float, Item*>> Items;



};

