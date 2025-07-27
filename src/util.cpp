#include "util.hpp"

Util::Util() {}

byte Util::readByte(std::ifstream& file) {
    byte b;

    file.read(reinterpret_cast<char *>(&b), 1);
    return b;
}

unsigned short Util::readShort(std::ifstream& file) {
    byte buffer[2];
    unsigned short res = 0;

    file.read(reinterpret_cast<char *>(&buffer), 2);

    res = (buffer[0] << 8) | buffer[1];

    return res;
}

unsigned long Util::readMidInt(std::ifstream& file) {
    byte buffer[3];
    unsigned long res = 0;

    file.read(reinterpret_cast<char *>(&buffer), 3);

    res = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];

    return res;
}

unsigned long Util::readInt(std::ifstream& file) {
    byte buffer[4];
    unsigned long res = 0;

    file.read(reinterpret_cast<char *>(&buffer), 4);

    res = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];

    return res;
}

unsigned long Util::readMidLong(std::ifstream& file) {
    byte buffer[6];
    unsigned long res = 0;

    file.read(reinterpret_cast<char *>(&buffer), 6);

    for (int i = 0; i < 6; i++) {
        res = (res << 8) | buffer[i];
    }

    // res = (buffer[0] << 40) | (buffer[1] << 32) | (buffer[2] << 24) | (buffer[3] << 16) | (buffer[4] << 8) | buffer[5];

    return res;
}

unsigned long Util::readLong(std::ifstream& file) {
    byte buffer[8];
    unsigned long res = 0;

    file.read(reinterpret_cast<char *>(&buffer), 8);

    for (int i = 0; i < 8; i++) {
        res = (res << 8) | buffer[i];
    }

    // res = (buffer[0] << 56) | (buffer[1] << 48) | (buffer[2] << 40) | (buffer[3] << 32) | (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];

    return res;
}

unsigned long Util::readVarInt(std::ifstream& file) {
    unsigned long value = 0;
    byte currentByte = 0;
    int i = 0;
    while (i < 9) {
        file.read(reinterpret_cast<char*>(&currentByte), 1);
        value = (value << 7) | (currentByte & 0x7F);
        if ((currentByte & 0x80) == 0) // If high bit isn't set, this is the last byte
            return value;
        i++;
    }
    // The 9th byte uses all 8 bits
    file.read(reinterpret_cast<char*>(&currentByte), 1);
    value = (value << 8) | currentByte;
    return value;
}

std::string Util::readString(std::ifstream& file, unsigned long size) {
    char buffer[1025];
    std::string res = "";
    int s = (int) size;
    
    while (s > 0) {
        file.read(reinterpret_cast<char *>(&buffer), s > 1024 ? 1024 : s);

        if (s > 1024)
            buffer[1024] = 0;
        else
            buffer[s] = 0;

        std::string tmp(buffer);
        res += tmp;
        s -= 1024;
    }

    return res;
}

// only root page
void Util::parseMetadata(std::ifstream& file, Metadata& metadata) {
    file.seekg(100);

    // skip unnecessary bytes
    byte b = Util::readByte(file);
    Util::readShort(file);

    metadata.numberOfTables = Util::readShort(file);

    Util::readShort(file);
    Util::readByte(file);

    // skip page number if it's an internal node
    if (b == 0x05)
        Util::readInt(file);

    std::vector<unsigned short> offsets;

    for (size_t i = 0; i < metadata.numberOfTables; i++)
    {
        offsets.push_back(Util::readShort(file));
    }

    for (auto offset : offsets)
    {
        file.seekg(offset);
        Util::readVarInt(file);
        Util::readVarInt(file);

        std::streampos headerStart = file.tellg();
        unsigned long headerSize = Util::readVarInt(file);
        std::streampos headerEnd = headerStart + static_cast<std::streamoff>(headerSize);

        std::vector<unsigned long> serialTypes;
        while (file.tellg() < headerEnd)
        {
            serialTypes.push_back(Util::readVarInt(file));
        }

        int i = 0;
        std::pair<unsigned long, std::string> table;
        std::vector<std::string> values;
        std::string type = "table";
        std::string indexTargetTable = "";

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

                    if (i == 3)
                        table.first = val;
                    value = std::to_string(val);
                }
            else
                value = Util::readString(file, columnLength);

            values.push_back(value);

            i++;
        }

        type = values[0];

        table.second = values[1];
        metadata.columns[table.second] = parseTableSchema(values[4]);

        
        if (type == "index") {
            std::pair<std::string, unsigned long> index;
            
            index.first = metadata.columns[table.second][0];
            index.second = table.first;
            
            metadata.indexes[values[2]] = index;
        }

        metadata.tables.push_back(table);
    }
}


// trim from end of string (right)
std::string& Util::rtrim(std::string& s, const char* t)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
std::string& Util::ltrim(std::string& s, const char* t)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (right then left)
std::string& Util::trim(std::string& s, const char* t)
{
    return ltrim(rtrim(s, t), t);
}

std::vector<std::string> Util::parseTableSchema(std::string schema) {
    std::vector<std::string> columns;
    std::vector<std::string> tmp;
    std::vector<std::string> components = split(schema, "(");
    std::string content = rtrim(components[1], ")");

    components = split(content, ",");

    for (size_t i = 0; i < components.size(); i++) {
        trim(components[i], "\t\n\r ");

        tmp = split(components[i], " ");

        columns.push_back(tmp[0]);
    }

    return columns;
}

std::vector<std::string> Util::parseIndexSchema(std::string schema) {
    std::vector<std::string> columns;
    std::vector<std::string> components = split(schema, "(");
    std::string content = rtrim(components[1], ")");

    columns.push_back(content);

    // components = split(content, ",");

    // for (size_t i = 0; i < components.size(); i++) {
    //     trim(components[i], "\t\n\r ");

    //     tmp = split(components[i], " ");

    //     columns.push_back(tmp[0]);
    // }

    return columns;
}


std::vector<std::string> Util::split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;
    
    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

std::string Util::toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {return std::toupper(c);});
    return str;
}

std::string Util::toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {return std::tolower(c);});
    return str;
}

Util::~Util() {}