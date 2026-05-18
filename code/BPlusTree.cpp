#include "BPlusTree.h"
#include <queue>
using namespace std;
#include <iostream>


BPlusTree::BPlusTree()
{
	m_pRoot = nullptr;
	m_pFirst = nullptr;
	m_pLast = nullptr;
}


BPlusTree::~BPlusTree()
= default;

bool BPlusTree::Insert(double nKey, Item* number)   //To change the value in the key value pair from double to item pointer
{
	int i = 0;
	int m = 0;

	//根结点为空
	if (m_pRoot==nullptr)
	{
	    //新建根结点
        auto * pNew = new bpLeafNode();
		//申请空间失败，返回false
        pNew->m_bLeaf = true;
		pNew->m_nKeyNum = 1;
        pNew->m_pParent = nullptr;
		pNew->leafNodeData->m_Keys[0]=nKey;
        pNew->leafNodeData->m_number[0]=number;
        m_pRoot = pNew;
		m_pFirst = pNew;
		return true;
	}

	//找到插入的叶节点
	bpNode * pTmp = m_pRoot;

	//当前结点不是叶结点，向下寻找
	while (!pTmp->m_bLeaf)
	{
		for (i = 0; i < pTmp->m_nKeyNum; i++)
		{

			if (nKey <  ((InternalNode*)pTmp)->internalNodeData->m_Keys[i])
				break;
		}
		if (i == 0)
			pTmp = (bpNode *) (((InternalNode*)pTmp)->internalNodeData->m_Pointers[0]);
		else
			pTmp = (bpNode *)(((InternalNode*)pTmp)->internalNodeData->m_Pointers[i]);
	}

	//判断叶节点中是否存在
	for (i = 0; i < ((bpLeafNode *)pTmp)->m_nKeyNum; i++)
	{
	    //想要插入的叶子结点已经存在
		if (nKey == ((bpLeafNode *)pTmp)->leafNodeData->m_Keys[i])    //Here, should not use element in m_number to make comparison with mkey, but in mkeys
		{
		    std::cout<<"The key has existed"<<std::endl;
			return false;
		}
	}

	//判断叶子节点的数量
	if (((bpLeafNode *)pTmp)->m_nKeyNum < LEAF_ORDER)
	{
		for (i = 0; i < ((bpLeafNode *)pTmp)->m_nKeyNum; i++)
		{
			if (nKey < ((bpLeafNode *)pTmp)->leafNodeData->m_Keys[i])     //Here, should not use element in m_number to make comparison with mkey, but in mkeys
            {
                break;
            }
		}


		//移动
		for (m = ((bpLeafNode *)pTmp)->m_nKeyNum - 1; m >= i; m--)
		{
            ((bpLeafNode *)pTmp)->leafNodeData->m_Keys[m + 1] = ((bpLeafNode *)pTmp)->leafNodeData->m_Keys[m];
            ((bpLeafNode *)pTmp)->leafNodeData->m_number[m + 1] = ((bpLeafNode *)pTmp)->leafNodeData->m_number[m];
		}
        ((bpLeafNode *)pTmp)->leafNodeData->m_Keys[i] = nKey;
        ((bpLeafNode *)pTmp)->leafNodeData->m_number[i] = number;
        Item* x1 = ((bpLeafNode *)pTmp)->leafNodeData->m_number[i];
        ((bpLeafNode *)pTmp)->m_nKeyNum++;
		return true;
	}

	//叶子节点已经满了
	int nMid = (LEAF_ORDER + 1) / 2;

	double * pTmpKeys = new double[LEAF_ORDER + 1];    //The temporary list of keys should be float instead of integer
	auto * pTmpnumbers = new Item*[LEAF_ORDER + 1];  //Be careful, here, maybe the data type of the elements in the list should be item pointer


	for (i = 0; i < pTmp->m_nKeyNum; i++)
	{
		if (((bpLeafNode *)pTmp)->leafNodeData->m_Keys[i] > nKey)  //Here the value should not be used for comparison but the key
			break;
	}
    //叶子结点加入新插入的值
	for (m = pTmp->m_nKeyNum - 1; m >= i; m--)
	{
		pTmpKeys[m + 1] = ((bpLeafNode *)pTmp)->leafNodeData->m_Keys[m];
        pTmpnumbers[m + 1] = ((bpLeafNode *)pTmp)->leafNodeData->m_number[m];
	}

	for (m = 0; m < i; m++)
	{
		pTmpKeys[m] = ((bpLeafNode *)pTmp)->leafNodeData->m_Keys[m];
        pTmpnumbers[m] = ((bpLeafNode *)pTmp)->leafNodeData->m_number[m];
        Item* x = ((bpLeafNode *)pTmp)->leafNodeData->m_number[m];
        Item* y = pTmpnumbers[m];
	}
	pTmpKeys[i] = nKey;
    pTmpnumbers[i] = number;


    //新建叶子结点，用于分裂
    auto * pNew = new bpLeafNode;

	for (m = 0, i = nMid; i < LEAF_ORDER + 1; i++,m++)
	{
		pNew->leafNodeData->m_Keys[m] =pTmpKeys[i];
		pNew->leafNodeData->m_number[m] = pTmpnumbers[i];
		pNew->m_nKeyNum++;
	}
    pNew->m_pParent = pTmp->m_pParent;

	pNew->m_bLeaf = pTmp->m_bLeaf;

	for (i = 0; i < pTmp->m_nKeyNum; i++)
	{
        ((bpLeafNode*)pTmp)->leafNodeData->m_Keys[i] = 0;
        ((bpLeafNode*)pTmp)->leafNodeData->m_number[i] = nullptr;       //Maybe here the elements should be void pointer
	}

	pTmp->m_nKeyNum = 0;

	for (i = 0; i < nMid; i++)
	{
        ((bpLeafNode*)pTmp)->leafNodeData->m_Keys[i] = pTmpKeys[i];
        ((bpLeafNode*)pTmp)->leafNodeData->m_number[i] = pTmpnumbers[i];
		pTmp->m_nKeyNum++;
	}
	pNew->leafNodeData->m_next = ((bpLeafNode*)pTmp)->leafNodeData->m_next;
    ((bpLeafNode*)pTmp)->leafNodeData->m_next= pNew;


	if (!InsertKeyAndPointer(pTmp->m_pParent, pTmp, pTmpKeys[nMid], pNew))
	{
	    std::cout<<"Insertion of an internode fails"<<std::endl;
		return false;
	}
	return true;
}

void BPlusTree::PrintLeaves()
{
    int num=0;
	int i = 0;
	if (!m_pFirst)
		return;
	bpLeafNode * pCur =(bpLeafNode*)m_pFirst;
	printf("------print leave-------\n");
	while(1){
        for(int i=0;i<pCur->m_nKeyNum;i++){
            printf("<key:%f, number:%lf>\n", pCur->leafNodeData->m_Keys[i], pCur->leafNodeData->m_number[i]);
            num=num+1;
        }
        if(pCur->leafNodeData->m_next==NULL){
            break;
        }else{
            pCur = static_cast<bpLeafNode *>(pCur->leafNodeData->m_next);
        }
        printf("\n------------------------------------------------------\n");
	}
	std::cout<<num<<std::endl;
}

bool BPlusTree::InsertKeyAndPointer(bpNode * pParent, bpNode * pOld, double nKey, bpNode * pNew)
{
	if (!pOld)
		return false;

	if (!pNew)
		return false;

	int i = 0;

	int m = 0;

	int k = 0;

	//父结点不存在，就是根结点的父亲
	if (pParent == nullptr)
	{
		//产生新根
		auto * pNewRoot = new InternalNode;
		//没有申请成功
        pNewRoot->m_bLeaf = false;



		pNewRoot->internalNodeData->m_Keys[0] = nKey;
		pNewRoot->m_nKeyNum = 1;
        pNewRoot->internalNodeData->m_Pointers[0] = pOld;
        pNewRoot->internalNodeData->m_Pointers[1] = pNew;
		pNewRoot->m_pParent = nullptr;
		pOld->m_pParent = pNewRoot;
        pNew->m_pParent = pNewRoot;
		m_pRoot = pNewRoot;






		return true;
	}

	//挪一下
	for (i = 0; i < pParent->m_nKeyNum; i++)
	{
	    //不可以重复添加相同的值
		if (nKey == ((InternalNode*)pParent)->internalNodeData->m_Keys[i]) {
		    std::cout<<"Trying to inserting an existing key"<<std::endl;
            return false;
        }
		if (nKey < ((InternalNode*)pParent)->internalNodeData->m_Keys[i]) {
            break;
        }
	}

	//判断父结点是否已经满了
	if (pParent->m_nKeyNum < ORDER)
	{
		//移动关键字
		for (m = pParent->m_nKeyNum - 1; m >= i; m--)
		{
            ((InternalNode*)pParent)->internalNodeData->m_Keys[m + 1] = ((InternalNode*)pParent)->internalNodeData->m_Keys[m];
		}

		//移动孩子
		for (m = pParent->m_nKeyNum; m > i; m--)
		{
            ((InternalNode*)pParent)->internalNodeData->m_Pointers[m + 1] = ((InternalNode*)pParent)->internalNodeData->m_Pointers[m];
		}

		//插进去
        ((InternalNode*)pParent)->internalNodeData->m_Keys[i] = nKey;
        ((InternalNode*)pParent)->internalNodeData->m_Pointers[i + 1] = pNew;
		pParent->m_nKeyNum++;

		pNew->m_pParent = pParent;
		return true;
	}

	//父亲结点已经满了
	double * pTmpKeys = new double[ORDER + 1];   //The temporary list of keys should contain elements of data type of float instead of integer

	void ** pTmpPointers = new void *[ORDER + 2];

	//找到插入的地点
	for (i = 0; i < pParent->m_nKeyNum; i++)
	{
		if (nKey < ((InternalNode*)pParent)->internalNodeData->m_Keys[i])
			break;
	}

	//键插入
	for (m = 0; m < i; m++)
	{
		pTmpKeys[m] = ((InternalNode*)pParent)->internalNodeData->m_Keys[m];
	}

	pTmpKeys[m] = nKey;
	m++;

	for (k = i; k < pParent->m_nKeyNum; k++,m++)
	{
		pTmpKeys[m] = ((InternalNode*)pParent)->internalNodeData->m_Keys[k];
	}

	//孩子插入
	for (m = 0; m <= i; m++)
	{
		pTmpPointers[m] = ((InternalNode*)pParent)->internalNodeData->m_Pointers[m];
	}

	pTmpPointers[m] = pNew;
	m++;

	for (k = i + 1; k <= pParent->m_nKeyNum; k++,m++)
	{
		pTmpPointers[m] = ((InternalNode*)pParent)->internalNodeData->m_Pointers[k];
	}

	//父亲分裂，分裂出来的新结点初始化
	bpNode * pNewSplit = new InternalNode;

	//设置是不是叶子结点
	pNewSplit->m_bLeaf = pParent->m_bLeaf;

	//新分裂出来的结点
	pNewSplit->m_pParent = pParent->m_pParent;

	//从这分开
	int nMid = (ORDER + 1) / 2;

	//用来提到爸爸那里
	double nMidKey = pTmpKeys[nMid]; //The key obtained from key in the middle position of the temporary list should be float instead of integer

	//右边的结点
	for (m = 0, i = nMid + 1; i < ORDER + 1; i++, m++)
	{
        ((InternalNode*)pNewSplit)->internalNodeData->m_Keys[m] = pTmpKeys[i];
        ((InternalNode*)pNewSplit)->internalNodeData->m_Pointers[m] = pTmpPointers[i];
		if (!pNewSplit->m_bLeaf)
		{
			bpNode * pCur = static_cast<bpNode *>(((InternalNode*)pNewSplit)->internalNodeData->m_Pointers[m]);
			if (pCur)
			{
				pCur->m_pParent = pNewSplit;
			}
		}
		pNewSplit->m_nKeyNum++;
	}

    ((InternalNode*)pNewSplit)->internalNodeData->m_Pointers[m] = pTmpPointers[i];

	if (!pNewSplit->m_bLeaf)
	{
		bpNode * pCur = static_cast<bpNode *>(((InternalNode*)pNewSplit)->internalNodeData->m_Pointers[m]);
		if (pCur)
		{
			pCur->m_pParent = pNewSplit;
		}
	}

	pParent->m_nKeyNum = 0;


	//左面结点
	for (i = 0; i < nMid; i++)
	{
        ((InternalNode*)pParent)->internalNodeData->m_Keys[i] = pTmpKeys[i];
        ((InternalNode*)pParent)->m_nKeyNum++;
	}

	for (i = 0; i <= nMid; i++)
	{
        ((InternalNode*)pParent)->internalNodeData->m_Pointers[i] = pTmpPointers[i];
	}

    delete[] pTmpKeys;

    delete[] pTmpPointers;

	return InsertKeyAndPointer(pParent->m_pParent, pParent, nMidKey, pNewSplit);
}


//查找某个值是否存在
bpLeafNode* BPlusTree::search(double nKey){    //The data type of the variable should be float instead of integer
    //向下直到找到叶子结点

    int i=0;

    //树上没有结点
    if (!m_pRoot){
        std::cout<<"The tree is empty"<<std::endl;
        return nullptr;
    }


    bpNode * pTmp = m_pRoot;
    while (!pTmp->m_bLeaf)
    {
        for (i = 0; i < pTmp->m_nKeyNum; i++)
        {
            if (nKey < ((InternalNode*)pTmp)->internalNodeData->m_Keys[i])
            {
                break;
            }
        }

        pTmp = (bpNode*)(((InternalNode*)pTmp)->internalNodeData->m_Pointers[i]);
    }

    auto* leafNode = (bpLeafNode *)pTmp;
    /*for(int i=0;i<leafNode->m_nKeyNum;i++){
        if(nKey == leafNode->leafNodeData->m_Keys[i]){
            std::cout<<nKey<<" Exist"<<std::endl;
            return leafNode;
        }
    }
    std::cout<<nKey<<"The leafnode that includes the key does not exist"<<std::endl;*/
    return leafNode;
}








void BPlusTree::PrintLayerTree()
{
    int i = 0;

    queue<bpNode *> q;

    if (m_pRoot == NULL)
    {
        printf("b+tree is null.\n");
        return;
    }

    bpNode* node;

    node = m_pRoot;
    q.push(node);

    double test_layer=0;      // A variable to check if the node is the first node of a new line
    int test_leaf=0;       // A variable to Check if the node is the first node of the leaf nodes



    while (q.empty() == false)
    {
        bpNode * nodeTmp = q.front();

        if (nodeTmp->m_bLeaf == false)
        {
            if(((InternalNode*)nodeTmp) ->internalNodeData->m_Keys[0]<test_layer){        //if we need to change a new line
                std::cout<<" "<<std::endl;
            }
            for (i = 0;
            i < nodeTmp->m_nKeyNum; i++)
            {
                //printf("%d",((InternalNode*)nodeTmp) ->internalNodeData->m_Keys[i]);

                std::cout<<((InternalNode*)nodeTmp) ->internalNodeData->m_Keys[i]<<" ";   // A substitute one of output

                q.push((bpNode*)((InternalNode*)nodeTmp) ->internalNodeData->m_Pointers[i]);

                //printf(" ");


            }
            q.push((bpNode*)((InternalNode*)nodeTmp) ->internalNodeData->m_Pointers[i]);
            std::cout<<"|| ";           // A substitute one of output
            test_layer=((InternalNode*)nodeTmp) ->internalNodeData->m_Keys[nodeTmp->m_nKeyNum-1];
        }
        else
        {
            if(test_leaf==0){                  //Check if it is the first node of the leaf nodes
                std::cout<<" "<<std::endl;
            }
            for (i = 0; i < nodeTmp->m_nKeyNum; i++)
            {
                //printf("%d", ((LeafNode*)nodeTmp)->leafNodeData->m_Keys[i]);
                //printf(" ");

                std::cout<<((bpLeafNode*)nodeTmp) ->leafNodeData->m_Keys[i]<<" ";   // A substitute one of output


            }
            test_leaf=test_leaf+1;
            std::cout<<"|| ";           // A substitute one of output
        }

        //printf("\n");

        //printf("-------------------------------------------------\n");

        q.pop();
    }

    std::cout<<" "<<std::endl;

    return;
}

bool BPlusTree::Remove(double nKey)    //Here, the data type of the variable should be float instead of integer
{
    //树上没有结点
	if (!m_pRoot)
		return false;

	int i = 0;

	int j=-1;

	int m = 0;

	bpNode * pTmp = m_pRoot;

	//向下直到找到叶子结点
	while (!pTmp->m_bLeaf)
	{
	    bool flag=false;
		for (i = 0; i < pTmp->m_nKeyNum; i++)
		{
            if (nKey < ((InternalNode*)pTmp)->internalNodeData->m_Keys[i])
            {
                flag = true;
                //j表示当前结点在在父结点中的位置
                j=i;
                break;
            }
		}
		if(flag==false){
		    j = pTmp->m_nKeyNum;
		}

        pTmp = (bpNode*)(((InternalNode*)pTmp)->internalNodeData->m_Pointers[i]);
	}

    //找到应该被删除的结点
	for (i = 0; i < pTmp->m_nKeyNum; i++)
	{
		if (nKey == ((bpLeafNode*)pTmp)->leafNodeData->m_Keys[i]) {
            break;
        }
	}

	//没有找到应该被删除的结点
	if (i == pTmp->m_nKeyNum) {
	    printf("Wrong! %d node not exists",nKey);
        return false;
    }

	auto * pCur = (bpLeafNode *)pTmp;


	//剩下的结点向前挪，挤掉删除的结点
	for (m = i + 1; m < pTmp->m_nKeyNum; m++)
	{
        pCur->leafNodeData->m_Keys[m-1] = pCur->leafNodeData->m_Keys[m];
        pCur->leafNodeData->m_number[m-1] = pCur->leafNodeData->m_number[m];
	}

	pTmp->m_nKeyNum--;

	//当节点的数目少于小面的值时，就应该进行之后的操作了
	int nLowNum = (LEAF_ORDER+1)  / 2;

	if (pTmp->m_nKeyNum >= nLowNum)
	{
		return true;
	}

	//下溢出

	bpNode * pParent = pTmp->m_pParent;

	//父亲是空的，说明当前叶子结点就是根节点
	if (!pParent) {
        //根结点，删空了
        if (pTmp->m_nKeyNum < 1) {
            m_pRoot = NULL;
            delete pTmp;
            m_pFirst = m_pLast = NULL;
        }
        return true;
    }

	bpNode * pNeighbor = NULL;
	int nNeighbor = -1;
	int nIndex = -1;

	//当前结点位置为头节点，邻居结点为1号结点
	if (j == 0)
	{
		pNeighbor = (bpNode*)(((InternalNode *)pParent)->internalNodeData->m_Pointers[1]);
		nNeighbor = 1;
		nIndex = 0;
	}
	else
	{
        pNeighbor = (bpNode*)(((InternalNode *)pParent)->internalNodeData->m_Pointers[j-1]);
		nNeighbor = j - 1;
		nIndex = j;
	}

    //若兄弟结点key有富余（大于Math.ceil(m-1)/2 – 1），向兄弟结点借一个记录，同时用借到的key替换父结（指当前结点和兄弟结点共同的父结点）点中的key
	if (pNeighbor->m_nKeyNum > nLowNum)
	{
		//向左邻居借一个结点
		if (nNeighbor < nIndex)
		{
		    //左邻居借出最右面的结点，上升到父亲结点          //Here, the original variable is nNeighbor???
            ((InternalNode*)pParent)->internalNodeData->m_Keys[nNeighbor] = ((bpLeafNode*)pNeighbor)->leafNodeData->m_Keys[pNeighbor->m_nKeyNum - 1];    //Here, be careful, it seems that the key for the original node should be changed, instead of the neighbour node???
            //该结点为接过来的结点挪地方
			for (i = pTmp->m_nKeyNum - 1; i >= 0; i--)
			{         //Here, originally pneighbor                   //Here, originally pneighbor      //Looking the four lines below
                ((bpLeafNode*)pTmp)->leafNodeData->m_Keys[i + 1] = ((bpLeafNode*)pTmp)->leafNodeData->m_Keys[i];  //Here, be careful, the node that needs to be offset keys should be the node(pTmp) that lose key, instead of its neighbor(left) node???
                ((bpLeafNode*)pTmp)->leafNodeData->m_number[i + 1] = ((bpLeafNode*)pTmp)->leafNodeData->m_number[i];   //Here, be careful, the node that needs to be offset keys should be the node(pTmp) that lose key, instead of its neighbor(left) node???
			}
            ((bpLeafNode*)pTmp)->leafNodeData->m_Keys[i + 1] = ((bpLeafNode*)pNeighbor)->leafNodeData->m_Keys[pNeighbor->m_nKeyNum - 1];
            ((bpLeafNode*)pTmp)->leafNodeData->m_number[0] = ((bpLeafNode*)pNeighbor)->leafNodeData->m_number[pNeighbor->m_nKeyNum - 1];
			pTmp->m_nKeyNum++;
			pNeighbor->m_nKeyNum--;
		}
		//向右邻居借一个结点
		else
		{
            ((InternalNode*)pParent)->internalNodeData->m_Keys[nIndex] = ((bpLeafNode*)pNeighbor)->leafNodeData->m_Keys[1];
            ((bpLeafNode*)pTmp)->leafNodeData->m_Keys[pTmp->m_nKeyNum] = ((bpLeafNode*)pNeighbor)->leafNodeData->m_Keys[0];   //Here, the key borrowed should be the first key from the right brother node instead of from the node itself
            ((bpLeafNode*)pTmp)->leafNodeData->m_number[pTmp->m_nKeyNum] = ((bpLeafNode*)pNeighbor)->leafNodeData->m_number[0];   //Here, the number borrowed should be the first number from the right brother node instead of from the node itself
			pTmp->m_nKeyNum++;
			for (i = 1; i <= pNeighbor->m_nKeyNum - 1; i++)
			{
                ((bpLeafNode*)pNeighbor)->leafNodeData->m_Keys[i - 1] = ((bpLeafNode*)pNeighbor)->leafNodeData->m_Keys[i];
                ((bpLeafNode*)pNeighbor)->leafNodeData->m_number[i - 1] = ((bpLeafNode*)pNeighbor)->leafNodeData->m_number[i];
			}
			pNeighbor->m_nKeyNum--;
		}
		return true;
	}

	//3）若兄弟结点中没有富余的key,则当前结点和兄弟结点合并成一个新的叶子结点，并删除父结点中的key（父结点中的这个key两边的孩子指针就变成了一个指针，正好指向这个新的叶子结点），
	// 将当前结点指向父结点（必为索引结点）。
	else
	{
		//合并左邻居和自己
		if (nNeighbor < nIndex)
            {
			for (i = 0; i < pTmp->m_nKeyNum; i++)
			{
                ((bpLeafNode*)pNeighbor)->leafNodeData->m_Keys[pNeighbor->m_nKeyNum] = ((bpLeafNode*)pTmp)->leafNodeData->m_Keys[i];
                ((bpLeafNode*)pNeighbor)->leafNodeData->m_number[pNeighbor->m_nKeyNum] = ((bpLeafNode*)pTmp)->leafNodeData->m_number[i];
				pNeighbor->m_nKeyNum++;
			}

			//删除父节点中的一个键
			for (i = nIndex; i < pParent->m_nKeyNum; i++)
			{
                ((InternalNode*)pParent)->internalNodeData->m_Keys[i - 1] = ((InternalNode*)pParent)->internalNodeData->m_Keys[i];
			}

			for (i = nIndex + 1; i <= pParent->m_nKeyNum; i++)
			{
                ((InternalNode*)pParent)->internalNodeData->m_Pointers[i - 1] = ((InternalNode*)pParent)->internalNodeData->m_Pointers[i];
			}

			pParent->m_nKeyNum--;

            ((bpLeafNode*)pNeighbor)->leafNodeData->m_next = ((bpLeafNode*)pTmp)->leafNodeData->m_next;

			delete pTmp;

		}
		//合并右邻居和自己
		else
		{
			for (i = 0; i < pNeighbor->m_nKeyNum; i++)
			{
                ((bpLeafNode*)pTmp)->leafNodeData->m_Keys[pTmp->m_nKeyNum] = ((bpLeafNode*)pNeighbor)->leafNodeData->m_Keys[i];
                ((bpLeafNode*)pTmp)->leafNodeData->m_number[pTmp->m_nKeyNum] = ((bpLeafNode*)pNeighbor)->leafNodeData->m_number[i];
				pTmp->m_nKeyNum++;
			}

			for (i = nNeighbor; i < pParent->m_nKeyNum; i++)
			{
                ((InternalNode*)pParent)->internalNodeData->m_Keys[i - 1] = ((InternalNode*)pParent)->internalNodeData->m_Keys[i];
			}
			for (i = nNeighbor + 1; i <= pParent->m_nKeyNum; i++)
			{
                ((InternalNode*)pParent)->internalNodeData->m_Pointers[i - 1] = ((InternalNode*)pParent)->internalNodeData->m_Pointers[i];
			}

			pParent->m_nKeyNum--;
            ((bpLeafNode*)pTmp)->leafNodeData->m_next = ((bpLeafNode*)pNeighbor)->leafNodeData->m_next;
			delete pNeighbor;
		}

		//当前节点指向父节点
		bpNode * pCurTmp = pParent;
		int nInternalLowNum = (ORDER + 1) / 2;

		//分支结点重复操作
		while (pCurTmp)
		{
		    //若索引结点的key的个数大于等于Math.ceil(m-1)/2 – 1，则删除操作结束。
			if (pCurTmp->m_nKeyNum >= nInternalLowNum)
			{
				break;
			}

			//借或者合并
			bpNode * pCurParent = pCurTmp->m_pParent;

			bpNode * pCurNeighbor = NULL;

			int nCurIndex = 0;

			int nNeighborIndex = 0;

			double nTmp = 0;    //It is a key which should be changed to float

			//当前节点为根节点
			if (!pCurParent)
			{
				//根结点
				if (pCurTmp->m_nKeyNum < 1)
				{
                    ((bpNode*)(((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[0]))->m_pParent= NULL;
					m_pRoot = (bpNode *)(((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[0]);
					delete pCurTmp;
				}
				break;
			}
			else
			{
				//非根结点
				//找到当前结点的位置
				for (i = 0; i <= pCurParent->m_nKeyNum; i++)
				{
					if (pCurTmp == ((InternalNode*)pCurParent)->internalNodeData->m_Pointers[i])
					{
						break;
					}
				}

				if (i == 0)
				{
					pCurNeighbor = (bpNode *)((InternalNode*)pCurParent)->internalNodeData->m_Pointers[1];
					nCurIndex = 0;
					nNeighborIndex = 1;
				}
				else
				{
					pCurNeighbor = (bpNode *)((InternalNode*)pCurParent)->internalNodeData->m_Pointers[i-1];
					nCurIndex = i;
					nNeighborIndex = i-1;
				}

				//兄弟结点有能力借
				if (pCurNeighbor->m_nKeyNum > nInternalLowNum)
				{
					//借
					if (nNeighborIndex < nCurIndex)
					{
						//左借
						nTmp = ((InternalNode*)pCurParent)->internalNodeData->m_Keys[nNeighborIndex];
						//父亲结点键值更改
                        ((InternalNode*)pCurParent)->internalNodeData->m_Keys[nNeighborIndex] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Keys[pCurNeighbor->m_nKeyNum - 1];
						for (i = pCurTmp->m_nKeyNum - 1; i >= 0; i--)
						{
                            ((InternalNode*)pCurTmp)->internalNodeData->m_Keys[i + 1] = ((InternalNode*)pCurTmp)->internalNodeData->m_Keys[i];
						}
						for (i = pCurTmp->m_nKeyNum; i >= 0; i--)
						{
						    ((InternalNode *) pCurTmp)->internalNodeData->m_Pointers[i + 1] = ((InternalNode *) pCurTmp)->internalNodeData->m_Pointers[i];
						}
                        ((InternalNode*)pCurTmp)->internalNodeData->m_Keys[0] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Keys[pCurNeighbor->m_nKeyNum-1];   //It seems that the nTmp could not mean the last key of the neighbour
						if (((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[pCurNeighbor->m_nKeyNum])
						{

                            static_cast<bpNode *>(((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[pCurNeighbor->m_nKeyNum])->m_pParent = pCurTmp;
						}
                        ((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[0] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[pCurNeighbor->m_nKeyNum];
						pCurTmp->m_nKeyNum++;
						pCurNeighbor->m_nKeyNum--;
					}
					else
					{
						//右借
						nTmp =  ((InternalNode*)pCurParent)->internalNodeData->m_Keys[nCurIndex];
                        ((InternalNode*)pCurParent)->internalNodeData->m_Keys[nCurIndex] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Keys[1];  //Here, it seems that the key at the right of the parent in the grandparent should be the second key of the uncle
                        ((InternalNode*)pCurTmp)->internalNodeData->m_Keys[pCurTmp->m_nKeyNum] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Keys[0];  //Here, it seems that the new key for the parent should be the first key of the uncle

                        //右面结点不是个空结点
						if (((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[0])
						{
							static_cast<bpNode *>(((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[0])->m_pParent = pCurTmp;
						}

                        ((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[pCurTmp->m_nKeyNum + 1] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[0];
						pCurTmp->m_nKeyNum++;

						for (i = 1; i < pCurNeighbor->m_nKeyNum; i++)
						{
                            ((InternalNode*)pCurNeighbor)->internalNodeData->m_Keys[i - 1] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Keys[i];
						}
						for (i = 1; i <= pCurNeighbor->m_nKeyNum; i++)
						{
                            ((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[i-1] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[i];
						}

						pCurNeighbor->m_nKeyNum--;
					}
					break;
				}
				//兄弟结点没有能力借，当前结点和兄弟结点及父结点下移key合并成一个新的结点。将当前结点指向父结点，重复第4步。
				else
				{
				    //左借借不到
					if (nNeighborIndex < nCurIndex)
					{
						//左合并

						//合并出一个新的结点
                        ((InternalNode*)pCurNeighbor)->internalNodeData->m_Keys[pCurNeighbor->m_nKeyNum] = ((InternalNode*)pCurParent)->internalNodeData->m_Keys[nNeighborIndex];
						pCurNeighbor->m_nKeyNum++;
						for (i = 0; i < pCurTmp->m_nKeyNum; i++)
						{
                            ((InternalNode*)pCurNeighbor)->internalNodeData->m_Keys[pCurNeighbor->m_nKeyNum] = ((InternalNode*)pCurTmp)->internalNodeData->m_Keys[i];
                            //pCurNeighbor->m_nKeyNum++;
                            ((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[pCurNeighbor->m_nKeyNum] = ((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[i];
							bpNode * pChild = (bpNode *)((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[pCurNeighbor->m_nKeyNum];
							if (pChild)
								pChild->m_pParent = pCurNeighbor;
							pCurNeighbor->m_nKeyNum++;
						}

                        ((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[pCurNeighbor->m_nKeyNum] = ((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[i];

						if ( ((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[pCurNeighbor->m_nKeyNum])
						{
							static_cast<bpNode *>( ((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[pCurNeighbor->m_nKeyNum])->m_pParent = pCurNeighbor;
						}

						//父结点调整
						for (i = nNeighborIndex + 1; i < pCurParent->m_nKeyNum; i++)
						{
                            ((InternalNode*)pCurParent)->internalNodeData->m_Keys[i - 1] = ((InternalNode*)pCurParent)->internalNodeData->m_Keys[i];
						}

						for (i = nCurIndex + 1; i <= pCurParent->m_nKeyNum; i++)
						{
                            ((InternalNode*)pCurParent)->internalNodeData->m_Pointers[i - 1] = ((InternalNode*)pCurParent)->internalNodeData->m_Pointers[i];
						}

						pCurParent->m_nKeyNum--;

						delete pCurTmp;
					}
					//右借借不到
					else
					{
						//右合并
						//形成合并节点
                        ((InternalNode*)pCurTmp)->internalNodeData->m_Keys[pCurTmp->m_nKeyNum] = ((InternalNode*)pCurParent)->internalNodeData->m_Keys[nCurIndex];
						pCurTmp->m_nKeyNum++;

						for (i = 0; i < pCurNeighbor->m_nKeyNum; i++)
						{
                            ((InternalNode*)pCurTmp)->internalNodeData->m_Keys[pCurTmp->m_nKeyNum] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Keys[i];
                            ((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[pCurTmp->m_nKeyNum] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[i];
							bpNode * pChild = (bpNode *)(((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[pCurTmp->m_nKeyNum]);
							if (pChild)
								pChild->m_pParent = pCurTmp;
							pCurTmp->m_nKeyNum++;
						}

                        ((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[pCurTmp->m_nKeyNum] = ((InternalNode*)pCurNeighbor)->internalNodeData->m_Pointers[i];
						if (((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[pCurTmp->m_nKeyNum])
						{
							((bpNode *)(((InternalNode*)pCurTmp)->internalNodeData->m_Pointers[pCurTmp->m_nKeyNum]))->m_pParent = pCurTmp;
						}

						//父结点调整
						for (i = nCurIndex + 1; i < pCurParent->m_nKeyNum; i++)
						{
							((InternalNode*)pCurParent)->internalNodeData->m_Keys[i - 1] = ((InternalNode*)pCurParent)->internalNodeData->m_Keys[i];
						}

						for (i = nNeighborIndex + 1; i <= pCurParent->m_nKeyNum; i++)
						{
							((InternalNode*)pCurParent)->internalNodeData->m_Pointers[i - 1] = ((InternalNode*)pCurParent)->internalNodeData->m_Pointers[i];
						}

						pCurParent->m_nKeyNum--;

						delete pCurNeighbor;
					}

					pCurTmp = pCurParent;
				}
			}
		}

		return true;
	}
}


void BPlusTree::get_range_item(std::vector<Item*>* pointer,  double a, double b, bool left_bool, bool right_bool, bool print_if){
    if(print_if==false){
        if((left_bool==true)&&(right_bool==true)){
            if(a>b){
                //std::cout<<"The floor of the range should be smaller than the cell"<<std::endl;
            }else{
                bpLeafNode* left=search(a);
                bpLeafNode* right=search(b);
                bpLeafNode* pointer_dynamic=left;
                int i;
                while(1==1){
                    for(i=0; i<pointer_dynamic->m_nKeyNum;i++){
                        if((pointer_dynamic->leafNodeData->m_Keys[i]>=a)&&(pointer_dynamic->leafNodeData->m_Keys[i]<=b)){
                            (*pointer).emplace_back(pointer_dynamic->leafNodeData->m_number[i]);
                        }
                    }
                    if(pointer_dynamic==right){
                        break;
                    }else{
                        pointer_dynamic=(bpLeafNode*)(pointer_dynamic->leafNodeData->m_next);
                    }
                }
            }
        }
        if((left_bool==true)&&(right_bool==false)){
            if(a>=b){
                //std::cout<<"The floor of the range should be smaller than the cell"<<std::endl;
            }else{
                bpLeafNode* left=search(a);
                bpLeafNode* right=search(b);
                bpLeafNode* pointer_dynamic=left;
                int i;
                while(1==1){
                    for(i=0; i<pointer_dynamic->m_nKeyNum;i++){
                        if((pointer_dynamic->leafNodeData->m_Keys[i]>=a)&&(pointer_dynamic->leafNodeData->m_Keys[i]<b)){
                            (*pointer).emplace_back(pointer_dynamic->leafNodeData->m_number[i]);
                        }
                    }
                    if(pointer_dynamic==right){
                        break;
                    }else{
                        pointer_dynamic=(bpLeafNode*)(pointer_dynamic->leafNodeData->m_next);
                    }
                }
            }
        }
        if((left_bool==false)&&(right_bool==false)){
            if(a>=b){
                //std::cout<<"The floor of the range should be smaller than the cell"<<std::endl;
            }else{
                bpLeafNode* left=search(a);
                bpLeafNode* right=search(b);
                bpLeafNode* pointer_dynamic=left;
                int i;
                while(1==1){
                    for(i=0; i<pointer_dynamic->m_nKeyNum;i++){
                        if((pointer_dynamic->leafNodeData->m_Keys[i]>a)&&(pointer_dynamic->leafNodeData->m_Keys[i]<b)){
                            (*pointer).emplace_back(pointer_dynamic->leafNodeData->m_number[i]);
                        }
                    }
                    if(pointer_dynamic==right){
                        break;
                    }else{
                        pointer_dynamic=(bpLeafNode*)(pointer_dynamic->leafNodeData->m_next);
                    }
                }
            }
        }
        if((left_bool==false)&&(right_bool==true)){
            if(a>=b){
                //std::cout<<"The floor of the range should be smaller than the cell"<<std::endl;
            }else{
                bpLeafNode* left=search(a);
                bpLeafNode* right=search(b);
                bpLeafNode* pointer_dynamic=left;
                int i;
                while(1==1){
                    for(i=0; i<pointer_dynamic->m_nKeyNum;i++){
                        if((pointer_dynamic->leafNodeData->m_Keys[i]>a)&&(pointer_dynamic->leafNodeData->m_Keys[i]<=b)){
                            (*pointer).emplace_back(pointer_dynamic->leafNodeData->m_number[i]);
                        }
                    }
                    if(pointer_dynamic==right){
                        break;
                    }else{
                        pointer_dynamic=(bpLeafNode*)(pointer_dynamic->leafNodeData->m_next);
                    }
                }
            }
        }





    }else{
        if((left_bool==true)&&(right_bool==true)){
            if(a>b){
                std::cout<<"The floor of the range should be smaller than the cell"<<std::endl;
            }else{
                bpLeafNode* left=search(a);
                bpLeafNode* right=search(b);
                bpLeafNode* pointer_dynamic=left;
                int i;
                int count_get=0;
                while(1==1){
                    for(i=0; i<pointer_dynamic->m_nKeyNum;i++){
                        if((pointer_dynamic->leafNodeData->m_Keys[i]>=a)&&(pointer_dynamic->leafNodeData->m_Keys[i]<=b)){
                            std::cout<<pointer_dynamic->leafNodeData->m_Keys[i]<<" ";
                            //printf("<key:%f>\n", pointer_dynamic->leafNodeData->m_Keys[i]);
                            (*pointer).emplace_back(pointer_dynamic->leafNodeData->m_number[i]);
                            count_get=count_get+1;
                        }
                    }
                    if(pointer_dynamic==right){
                        break;
                    }else{
                        pointer_dynamic=(bpLeafNode*)(pointer_dynamic->leafNodeData->m_next);
                    }
                }
                std::cout<<'\n'<<std::endl;
                std::cout<<"Get "<<count_get<<" keys"<<std::endl;
            }
        }
        if((left_bool==true)&&(right_bool==false)){
            if(a>=b){
                std::cout<<"The floor of the range should be smaller than the cell"<<std::endl;
            }else{
                bpLeafNode* left=search(a);
                bpLeafNode* right=search(b);
                bpLeafNode* pointer_dynamic=left;
                int i;
                int count_get=0;
                while(1==1){
                    for(i=0; i<pointer_dynamic->m_nKeyNum;i++){
                        if((pointer_dynamic->leafNodeData->m_Keys[i]>=a)&&(pointer_dynamic->leafNodeData->m_Keys[i]<b)){
                            (*pointer).emplace_back(pointer_dynamic->leafNodeData->m_number[i]);
                            std::cout<<pointer_dynamic->leafNodeData->m_Keys[i]<<" ";
                            //printf("<key:%f>\n", pointer_dynamic->leafNodeData->m_Keys[i]);
                            count_get=count_get+1;
                        }
                    }
                    if(pointer_dynamic==right){
                        break;
                    }else{
                        pointer_dynamic=(bpLeafNode*)(pointer_dynamic->leafNodeData->m_next);
                    }
                }
                std::cout<<'\n'<<std::endl;
                std::cout<<"Get "<<count_get<<" keys"<<std::endl;
            }
        }
        if((left_bool==false)&&(right_bool==false)){
            if(a>=b){
                std::cout<<"The floor of the range should be smaller than the cell"<<std::endl;
            }else{
                bpLeafNode* left=search(a);
                bpLeafNode* right=search(b);
                bpLeafNode* pointer_dynamic=left;
                int i;
                int count_get=0;
                while(1==1){
                    for(i=0; i<pointer_dynamic->m_nKeyNum;i++){
                        if((pointer_dynamic->leafNodeData->m_Keys[i]>a)&&(pointer_dynamic->leafNodeData->m_Keys[i]<b)){
                            (*pointer).emplace_back(pointer_dynamic->leafNodeData->m_number[i]);
                            std::cout<<pointer_dynamic->leafNodeData->m_Keys[i]<<" ";
                            //printf("<key:%f>\n", pointer_dynamic->leafNodeData->m_Keys[i]);
                            count_get=count_get+1;
                        }
                    }
                    if(pointer_dynamic==right){
                        break;
                    }else{
                        pointer_dynamic=(bpLeafNode*)(pointer_dynamic->leafNodeData->m_next);
                    }
                }
                std::cout<<'\n'<<std::endl;
                std::cout<<"Get "<<count_get<<" keys"<<std::endl;
            }
        }
        if((left_bool==false)&&(right_bool==true)){
            if(a>=b){
                std::cout<<"The floor of the range should be smaller than the cell"<<std::endl;
            }else{
                bpLeafNode* left=search(a);
                bpLeafNode* right=search(b);
                bpLeafNode* pointer_dynamic=left;
                int i;
                int count_get=0;
                while(1==1){
                    for(i=0; i<pointer_dynamic->m_nKeyNum;i++){
                        if((pointer_dynamic->leafNodeData->m_Keys[i]>a)&&(pointer_dynamic->leafNodeData->m_Keys[i]<=b)){
                            (*pointer).emplace_back(pointer_dynamic->leafNodeData->m_number[i]);
                            std::cout<<pointer_dynamic->leafNodeData->m_Keys[i]<<" ";
                            //printf("<key:%f>\n", pointer_dynamic->leafNodeData->m_Keys[i]);
                            count_get=count_get+1;
                        }
                    }
                    if(pointer_dynamic==right){
                        break;
                    }else{
                        pointer_dynamic=(bpLeafNode*)(pointer_dynamic->leafNodeData->m_next);
                    }
                }
                std::cout<<'\n'<<std::endl;
                std::cout<<"Get "<<count_get<<" keys"<<std::endl;
            }
        }
    }


}
