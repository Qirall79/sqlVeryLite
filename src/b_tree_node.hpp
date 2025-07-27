#pragma once

# include<vector>
# include<utility>
# include<string>
# include<set>
# include<algorithm>

# include "util.hpp"
# include "command.hpp"

# define INTERIOR_BTREE_PAGE 0x05
# define INTERIOR_INDEX_PAGE 0x02
# define LEAF_BTREE_PAGE 0x0d
# define LEAF_INDEX_PAGE 0x0a


class BTreeNode {
    public:
        BTreeNode();
        ~BTreeNode();

        void parseHeader(std::ifstream&, Metadata&);
        void readData(std::ifstream&, Metadata&, unsigned long, Command&);
        void readRowById(std::ifstream&, Metadata&, unsigned long, Command&, unsigned long);
        std::vector<unsigned long> scanIndex(std::ifstream&, Metadata&, unsigned long, Command&);

        NodeType type;
        unsigned short numberOfCells;
        int cellContentStart;
        byte freeBytesCount;
        int rightMostPointer;
        std::vector<vbyte> keys;
        std::vector<vbyte> values;
        std::vector<BTreeNode*> kids;
        std::vector<std::string> tables;
};