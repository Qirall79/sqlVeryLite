#include "b_tree_node.hpp"

BTreeNode::BTreeNode() {}

void BTreeNode::parseHeader(std::ifstream &file, Metadata& metadata)
{
    (void) metadata;

    byte b = Util::readByte(file);

    if (b == INTERIOR_BTREE_PAGE)
        type = INTERIOR_NODE;
    else if (b == LEAF_BTREE_PAGE)
        type = LEAF_NODE;
    else if (b == LEAF_INDEX_PAGE)
        type = LEAF_INDEX_NODE;
    else if (b == INTERIOR_INDEX_PAGE)
        type = INTERIOR_INDEX_NODE;

    Util::readShort(file);

    numberOfCells = Util::readShort(file);

    cellContentStart = Util::readShort(file);

    if (cellContentStart == 0)
        cellContentStart = 65536;
    freeBytesCount = Util::readByte(file);

    if (type == INTERIOR_NODE || type == INTERIOR_INDEX_NODE)
    {
        rightMostPointer = Util::readInt(file);
    }
    else
        rightMostPointer = 0;
}

std::vector<unsigned long> parseRecordHeader(std::ifstream &file) {
    std::streampos headerStart = file.tellg();
    unsigned long headerSize = Util::readVarInt(file);
    std::streampos headerEnd = headerStart + static_cast<std::streamoff>(headerSize);

    std::vector<unsigned long> serialTypes;
    while (file.tellg() < headerEnd)
    {
        serialTypes.push_back(Util::readVarInt(file));
    }

    return serialTypes;
}

std::vector<std::string> parseValues(std::ifstream& file, std::vector<unsigned long>& serialTypes) {
    std::vector<std::string> values;
    
    for (auto serialType: serialTypes) {
        unsigned long columnLength;
        std::string columnType;

        if (serialType >= 13 && serialType % 2 == 1)
        {
            columnLength = (serialType - 13) / 2;
            columnType = "string";
        }
        else if (serialType >= 12 && serialType % 2 == 0)
        {
            columnLength = (serialType - 12) / 2;
            columnType = "blob";
        }
        else
        {
            columnLength = serialType;
            columnType = "int";

            if (serialType == 5)
                columnLength = 6;
            if (serialType == 6)
                columnLength = 8;
        }

        std::string value;
        if (columnType == "int")
        {
            unsigned long val;

            if (columnLength == 1)
                val = Util::readByte(file);
            else if (columnLength == 2)
                val = Util::readShort(file);
            else if (columnLength == 3)
                val = Util::readMidInt(file);
            else if (columnLength == 4)
                val = Util::readInt(file);
            else if (columnLength == 6)
                val = Util::readMidLong(file);
            else if (columnLength == 8)
                val = Util::readLong(file);
            else
                val = 0;
            value = std::to_string(val);
        }
        else
        {
            value = Util::readString(file, columnLength);
        }

        values.push_back(value);
    }
    return values;
}

void BTreeNode::readData(std::ifstream &file, Metadata& metadata, unsigned long pageNumber, Command& command) {
    std::vector<unsigned short> offsets;
    unsigned long pageOffset = (pageNumber - 1) * metadata.pageSize;

    for (int i = 0; i < numberOfCells; i++)
    {
        offsets.push_back(Util::readShort(file));
    }

    for (auto offset : offsets)
    {
        file.seekg(pageOffset + offset);
        
        if (type == LEAF_NODE) {
            // payload size
            Util::readVarInt(file);
            
            // row id
            unsigned long rowId = Util::readVarInt(file);

            
            std::vector<unsigned long> serialTypes = parseRecordHeader(file);

            int i = 0;
            bool conditionFailed = false;
            std::map<std::string, std::string> values;



            for (auto serialType : serialTypes)
            {
                unsigned long columnLength;
                std::string columnType;

                if (serialType >= 13 && serialType % 2 == 1)
                {
                    columnLength = (serialType - 13) / 2;
                    columnType = "string";
                }
                else if (serialType >= 12 && serialType % 2 == 0)
                {
                    columnLength = (serialType - 12) / 2;
                    columnType = "blob";
                }
                else
                {
                    columnLength = serialType;
                    columnType = "int";

                    if (serialType == 5)
                        columnLength = 6;
                    if (serialType == 6)
                        columnLength = 8;
                }

                std::string value;
                if (columnType == "int")
                {
                    unsigned long val;

                    if (columnLength == 1)
                        val = Util::readByte(file);
                    else if (columnLength == 2)
                        val = Util::readShort(file);
                    else if (columnLength == 3)
                        val = Util::readMidInt(file);
                    else if (columnLength == 4)
                        val = Util::readInt(file);
                    else if (columnLength == 6)
                        val = Util::readMidLong(file);
                    else if (columnLength == 8)
                        val = Util::readLong(file);
                    else
                        val = 0;
                    value = std::to_string(val);
                }
                else
                {
                    value = Util::readString(file, columnLength);
                }

                if (i < (int) metadata.columns[command.table].size()) {
                    values[metadata.columns[command.table][i]] = value;
    
                    if (command.queryType == CONDITION_SELECT 
                        && command.condition.first != "id"
                        && command.condition.first == metadata.columns[command.table][i] && command.condition.second != value) {
                        conditionFailed = true;
                        break ;
                    }
                } else {
                    std::cout << "Something went wrong" << std::endl;
                }

                i++;
            }

            bool isFirst = true;
            values["id"] = std::to_string(rowId);

            if (command.condition.first == "id" && values["id"] != command.condition.second) {
                conditionFailed = true;
            }  
            
            if (!conditionFailed) {
                // display values
                for (auto column: command.columns) {
                    if (!isFirst)
                        std::cout << "|";
                    else
                        isFirst = false;
                    std::cout << values[column];
                }
                std::cout << std::endl;
            }
        }
        else if (type == INTERIOR_NODE) {
            unsigned long childPageNumber = Util::readInt(file);
            Util::readVarInt(file);

            BTreeNode childPage;

            file.seekg((childPageNumber - 1) * metadata.pageSize);
            childPage.parseHeader(file, metadata);
            childPage.readData(file, metadata, childPageNumber, command);
        }
    }
}
void BTreeNode::readRowById(std::ifstream &file, Metadata& metadata, unsigned long pageNumber, Command& command, unsigned long id) {
    std::vector<unsigned short> offsets;
    unsigned long pageOffset = (pageNumber - 1) * metadata.pageSize;

    int headerSize = (type == LEAF_NODE || type == LEAF_INDEX_NODE) ? 8 : 12;
    file.seekg(pageOffset + headerSize);

    for (int i = 0; i < numberOfCells; i++) {
        offsets.push_back(Util::readShort(file));
    }

    if (type == INTERIOR_NODE) {
        int left = 0, right = numberOfCells - 1;
        unsigned long targetChild = 0;

        // binary search
        while (left <= right) {
            int mid = (left + right) / 2;
            file.seekg(pageOffset + offsets[mid]);

            unsigned long childPageNumber = Util::readInt(file);
            unsigned long key = Util::readVarInt(file);

            if (id <= key) { 
                targetChild = childPageNumber;

                // searching left for the leftmost suitable child
                right = mid - 1; 
            } else {
                left = mid + 1;
            }
        }

        // If no suitable child found, use rightmost pointer
        if (targetChild == 0 && rightMostPointer > 0) {
            targetChild = rightMostPointer;
        }

        if (targetChild > 0) {
            BTreeNode childPage;
            file.seekg((targetChild - 1) * metadata.pageSize);
            childPage.parseHeader(file, metadata);
            childPage.readRowById(file, metadata, targetChild, command, id);
        }
        return;
    }

    for (auto offset : offsets) {
        file.seekg(pageOffset + offset);

        Util::readVarInt(file); // payload size
        unsigned long rowId = Util::readVarInt(file);

        if (rowId != id) continue;

        
        
        std::vector<unsigned long> serialTypes = parseRecordHeader(file);
        std::map<std::string, std::string> values;
        
        values["id"] = std::to_string(rowId);

        for (size_t col = 0; col < serialTypes.size(); col++) {
            unsigned long serialType = serialTypes[col];
            unsigned long columnLength;
            std::string columnType;

            if (serialType >= 13 && serialType % 2 == 1) {
                columnLength = (serialType - 13) / 2;
                columnType = "string";
            } else if (serialType >= 12 && serialType % 2 == 0) {
                columnLength = (serialType - 12) / 2;
                columnType = "blob";
            } else {
                columnLength = serialType;
                columnType = "int";
                if (serialType == 5) columnLength = 6;
                else if (serialType == 6) columnLength = 8;
            }

            std::string value;
            if (columnType == "int") {
                unsigned long val = 0;
                if (columnLength == 1) val = Util::readByte(file);
                else if (columnLength == 2) val = Util::readShort(file);
                else if (columnLength == 3) val = Util::readMidInt(file);
                else if (columnLength == 4) val = Util::readInt(file);
                else if (columnLength == 6) val = Util::readMidLong(file);
                else if (columnLength == 8) val = Util::readLong(file);
                value = std::to_string(val);
            } else {
                value = Util::readString(file, columnLength);
            }

            if (col < metadata.columns[command.table].size()) {
                values[metadata.columns[command.table][col]] = value;
            }
        }


        bool isFirst = true;
        for (const std::string &column : command.columns) {
            if (!isFirst) std::cout << "|";
            std::cout << values[column];
            isFirst = false;
        }
        std::cout << std::endl;
        return;
    }
}

std::vector<unsigned long> BTreeNode::scanIndex(std::ifstream &file, Metadata& metadata, unsigned long pageNumber, Command& command) {
    std::vector<unsigned short> offsets;
    std::vector<unsigned long> validIndexes;
    unsigned long pageOffset = (pageNumber - 1) * metadata.pageSize;

    // get cell offsets
    int headerSize = (type == LEAF_INDEX_NODE) ? 8 : 12;
    file.seekg(pageOffset + headerSize);
    
    for (int i = 0; i < numberOfCells; i++) {
        offsets.push_back(Util::readShort(file));
    }
    
    if (type == LEAF_INDEX_NODE) {
        int matchCount = 0;
        
        for (auto offset : offsets) {
            file.seekg(pageOffset + offset);
            Util::readVarInt(file); // payload size
            
            std::vector<unsigned long> serialTypes = parseRecordHeader(file);
            std::vector<std::string> values = parseValues(file, serialTypes);
            
            if (values[0] == command.condition.second) {
                validIndexes.push_back(std::stoul(values[1]));
                matchCount++;
            }
        }
    }
    else if (type == INTERIOR_INDEX_NODE) {
        
        std::vector<unsigned long> childrenToVisit;
        
        bool foundExactMatches = false;
        int lastExactMatchIndex = -1;
        
        for (int i = 0; i < numberOfCells; i++) {
            file.seekg(pageOffset + offsets[i]);
            unsigned long childPageNumber = Util::readInt(file);
            Util::readVarInt(file);
            
            std::vector<unsigned long> serialTypes = parseRecordHeader(file);
            std::vector<std::string> values = parseValues(file, serialTypes);
            
            if (values[0] == command.condition.second) {
                validIndexes.push_back(std::stoul(values[1]));
                
                childrenToVisit.push_back(childPageNumber);

                foundExactMatches = true;
                lastExactMatchIndex = i;
            }
            else if (command.condition.second < values[0]) {
                childrenToVisit.push_back(childPageNumber);
                break;
            }
        }
        
        // if we found exact matches but didn't encounter a larger separator, we need to check the rightmost pointer
        if (foundExactMatches && lastExactMatchIndex == numberOfCells - 1 && rightMostPointer > 0) {
            childrenToVisit.push_back(rightMostPointer);
        }
        
        // if we went through all cells and didn't find a "less than" condition, and we haven't collected any children yet, use the rightmost pointer
        if (childrenToVisit.empty() && rightMostPointer > 0) {
            childrenToVisit.push_back(rightMostPointer);
        }
        
        for (unsigned long childPage : childrenToVisit) {
            BTreeNode childNode;
            file.seekg((childPage - 1) * metadata.pageSize);
            childNode.parseHeader(file, metadata);
            
            std::vector<unsigned long> childIndexes = childNode.scanIndex(file, metadata, childPage, command);
            validIndexes.insert(validIndexes.end(), childIndexes.begin(), childIndexes.end());
        }
    }
    
    return validIndexes;
}

BTreeNode::~BTreeNode() {}