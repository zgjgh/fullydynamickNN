//
// Created by HP on 2022/8/25.
//

//#ifndef HDR_TREE_ITEM_4_USERNODE_H
//#define HDR_TREE_ITEM_4_USERNODE_H

//#endif //HDR_TREE_ITEM_4_USERNODE_H


#pragma once

enum class userNodeType {userLeafNode, userNonLeafNode};

class userNode
{
public:

    userNodeType type;

    virtual ~userNode();
};