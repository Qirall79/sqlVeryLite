#include <cstring>
#include <iostream>
#include <fstream>

#include "b_tree_node.hpp"
#include "metadata.hpp"
#include "command.hpp"

void printVector(std::vector<unsigned long>& validIndexes) {
    bool isFirst = true;
    
    for (auto i: validIndexes) {
        if (!isFirst)
            std::cout << ", ";
        else
            isFirst = false;
        std::cout << i;
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    Command command;
    std::string database_file_path = argv[1];
    std::string strCommand = argv[2];
    Metadata metadata;
    
    command.parse(Util::toLower(strCommand));

    std::ifstream database_file(database_file_path, std::ios::binary);
    BTreeNode page;

    if (!database_file) {
        std::cerr << "Failed to open the database file" << std::endl;
        return 1;
    }
    database_file.seekg(16);  // skip the first 16 bytes of the header
    char buffer[2];
    database_file.read(buffer, 2);
    
    metadata.pageSize = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));
    Util::parseMetadata(database_file, metadata);


    if (command.type == INFO) {
        std::cout << "database page size: " << metadata.pageSize << std::endl;
        std::cout << "number of tables: " << metadata.numberOfTables << std::endl;
    }
    else if (command.type == TABLES) {
        for (auto table: metadata.tables) {
            std::cout << table.second << " ";
        }
        std::cout << std::endl;
    }

    else {
        if (command.type == UNKNOWN) {
            std::cerr << "Unknown query !" << std::endl;
            exit(1);
        }
        
        if (command.queryType == COUNT) {
            for (auto t: metadata.tables) {
                if (t.second == command.table) {
                    // go to table page
                    database_file.seekg(metadata.pageSize * (t.first - 1));
            
                    page.parseHeader(database_file, metadata);
    
                    std::cout << page.numberOfCells << std::endl;
                    break ;
                }
            }
        } else {            

            if (command.columns[0] == "*") {
                command.columns.clear();
                // read all
                for (size_t i = 0; i < metadata.columns[command.table].size(); i++) {
                    command.columns.push_back(metadata.columns[command.table][i]);
                }
            }

            // for (size_t i = 0; i < metadata.columns[command.table].size(); i++) {
            //     std::cout << metadata.columns[command.table][i] << std::endl;
            // }

            // display labels
            // for (size_t i = 0; i < command.columns.size(); i++) {
            //     if (i != 0)
            //         std::cout << "|";
            //     std::cout << command.columns[i];
            // }
            // std::cout << std::endl;

            std::pair<std::string, std::string> pair;

            
            for (auto t: metadata.tables) {
                if (t.second == command.table) {
                    pair.first = command.table;
                    pair.second = command.condition.first;

                    // go to table page
                    database_file.seekg(metadata.pageSize * (t.first - 1));
                    
                    // std::cout << command.table << std::endl;

                    if (metadata.indexes[command.table].second && command.condition.first == metadata.indexes[command.table].first) {
                        // go to index
                        database_file.seekg(metadata.pageSize * (metadata.indexes[command.table].second - 1));
                        page.parseHeader(database_file, metadata);

                        std::vector<unsigned long> validIndexes = page.scanIndex(database_file, metadata, metadata.indexes[command.table].second, command);
                        
                        // std::sort(validIndexes.begin(), validIndexes.end());

                        for (auto id: validIndexes) {
                            // std::cout << id << std::endl;
                            
                            database_file.seekg(metadata.pageSize * (t.first - 1));
                            page.parseHeader(database_file, metadata);
                            page.readRowById(database_file, metadata, t.first, command, id);
                        }
                    } else {
                        page.parseHeader(database_file, metadata);
                        page.readData(database_file, metadata, t.first, command);
                    }

                    break ;
                }
            }
        }

    }

    return 0;
}
