cmake_minimum_required(VERSION 3.17)
project(HDR_Tree_item_4)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(hdr_tree_same_pca main.cpp Cluster.h Clusters.cpp Clusters.h DataBase.cpp DataBase.h HDR_Tree.cpp HDR_Tree.h Item.cpp Item.h itemCluster.h itemClusters.cpp itemClusters.h itemDataBase.cpp itemDataBase.h itemHDR_Tree.cpp itemHDR_Tree.h itemLeafNode.cpp itemLeafNode.h itemNode.cpp itemNode.h itemNonLeafNode.cpp itemNonLeafNode.h LeafNode.cpp LeafNode.h Node.cpp Node.h NonLeafNode.cpp NonLeafNode.h User.cpp User.h Clusteri.h Clustersi.h Clustersi.cpp BPlusTree.h BPlusTree.cpp Clusteru.h Clustersu.h Clustersu.cpp userNode.h userNode.cpp userLeafNode.h userLeafNode.cpp userNonLeafNode.h userNonLeafNode.cpp Littletree.h Littletree.cpp Cluster_origin.h Cluster_origin.cpp)

add_executable(benchmark_runtime_memory benchmark_runtime_memory.cpp Cluster.h Clusters.cpp Clusters.h DataBase.cpp DataBase.h HDR_Tree.cpp HDR_Tree.h Item.cpp Item.h itemCluster.h itemClusters.cpp itemClusters.h itemDataBase.cpp itemDataBase.h itemHDR_Tree.cpp itemHDR_Tree.h itemLeafNode.cpp itemLeafNode.h itemNode.cpp itemNode.h itemNonLeafNode.cpp itemNonLeafNode.h LeafNode.cpp LeafNode.h Node.cpp Node.h NonLeafNode.cpp NonLeafNode.h User.cpp User.h Clusteri.h Clustersi.h Clustersi.cpp BPlusTree.h BPlusTree.cpp Clusteru.h Clustersu.h Clustersu.cpp userNode.h userNode.cpp userLeafNode.h userLeafNode.cpp userNonLeafNode.h userNonLeafNode.cpp Littletree.h Littletree.cpp Cluster_origin.h Cluster_origin.cpp)


# 设置 GSL 的路径
include_directories(D:/msys64/mingw64/include)
link_directories(D:/msys64/mingw64/lib)

# 添加 GSL 库
#add_executable(MyGSLProject main.cpp)
#target_link_libraries(hdr_tree_same_pca gsl gslcblas)
target_link_libraries(hdr_tree_same_pca PRIVATE
        "D:/msys64/mingw64/lib/libgsl.a"
        "D:/msys64/mingw64/lib/libgslcblas.a"
        )

target_link_libraries(benchmark_runtime_memory PRIVATE
        "D:/msys64/mingw64/lib/libgsl.a"
        "D:/msys64/mingw64/lib/libgslcblas.a"
        psapi
        )

message(STATUS "GSL library path: D:/msys64/mingw64/lib")
message(STATUS "GSL include path: D:/msys64/mingw64/include")
# 因为大量使用模板,导致代码增大,MSVC下要用/bigobj选项,否则会报错:error C1128
# GNU compiler 要用 -Wa,-mbig-obj 选项
#target_compile_options(HDR_Tree_item_4
#PRIVATE
#$<$<CXX_COMPILER_ID:GNU>:-Wa,-mbig-obj>)




set(CMAKE_BUILD_TYPE "Release")

