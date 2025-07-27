#pragma once

#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <fstream>

#include "metadata.hpp"

#define BTREE_PAGE_SIZE 4096
#define BTREE_MAX_KEY_SIZE 1000
#define BTREE_MAX_VAL_SIZE 3000

typedef unsigned char byte;
typedef std::vector<byte> vbyte;

enum NodeType
{
    LEAF_NODE,
    INTERIOR_NODE,
    LEAF_INDEX_NODE,
    INTERIOR_INDEX_NODE
};

class Util
{
public:
    Util();
    ~Util();

    static std::vector<std::string> split(std::string, std::string);
    static std::string toUpper(std::string);
    static std::string toLower(std::string);
    static std::string& trim(std::string& s, const char* t);
    static std::string& ltrim(std::string& s, const char* t);
    static std::string& rtrim(std::string& s, const char* t);

    static byte readByte(std::ifstream &);
    static unsigned short readShort(std::ifstream &);
    static unsigned long readMidInt(std::ifstream &);
    static unsigned long readInt(std::ifstream &);
    static unsigned long readMidLong(std::ifstream &);
    static unsigned long readLong(std::ifstream &);
    static unsigned long readVarInt(std::ifstream &);
    static std::string readString(std::ifstream&, unsigned long);

    static void parseMetadata(std::ifstream&, Metadata&);
    static std::vector<std::string> parseTableSchema(std::string);
    static std::vector<std::string> parseIndexSchema(std::string);
};