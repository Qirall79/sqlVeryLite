# SQLite Clone

A minimal SQLite database implementation built from scratch. This project demonstrates database internals, file format parsing, and query processing by creating a working SQLite clone that can read and query real SQLite database files.

## Features

### Core Database Operations
- **Database Introspection**: Query database metadata (`.dbinfo`, `.tables`)
- **Table Schema Parsing**: Parse and understand SQLite table definitions
- **Data Retrieval**: Execute `SELECT` queries with column selection
- **Conditional Queries**: Support for `WHERE` clause filtering
- **Aggregate Functions**: `COUNT(*)` queries for row counting
- **Wildcard Selection**: `SELECT *` for all columns

### Advanced Features
- **B-Tree Navigation**: Traverse SQLite's B-tree page structure
- **Index Utilization**: Intelligent index scanning for optimized queries
- **Multi-page Support**: Handle databases with interior and leaf pages
- **Binary Search**: Efficient row lookup using binary search algorithms
- **Large Database Support**: Optimized for databases up to 1GB+ in size

### Supported SQL Syntax
```sql
-- Database introspection
.dbinfo
.tables

-- Basic queries
SELECT column1, column2 FROM table_name;
SELECT * FROM table_name;
SELECT COUNT(*) FROM table_name;

-- Conditional queries
SELECT id, name FROM apples WHERE country = 'eritrea';
```

## Technical Implementation

**Language**: C++23
**Dependencies**: Standard C++ library only  
**File I/O**: Binary file parsing with custom readers

### Architecture Components

- **File Parser**: Binary file reading utilities, varint parsing, data type handling
- **B-Tree Navigation**: Page type detection, cell parsing, multi-level tree traversal
- **Query Engine**: SQL command parsing, query classification, result formatting
- **Index Optimization**: Smart query execution path selection and optimized row retrieval
- **Metadata Management**: Table schema storage and index mapping

## Usage

### Build
```bash
make
```

### Run Queries
```bash
# Database information
./sqlite sample.db ".dbinfo"

# List tables
./sqlite sample.db ".tables"

# Count rows
./sqlite sample.db "SELECT COUNT(*) FROM apples"

# Select with conditions (uses index when available)
./sqlite sample.db "SELECT id, name FROM companies WHERE country = 'eritrea'"
```

### Sample Output
```
121311|unilink s.c.
2102438|orange asmara it solutions
5729848|zara mining share company
6634629|asmara rental
```

## Performance Benchmarks

- **Query Execution**: Under 3 seconds on 1GB database
- **Index Utilization**: 100x+ speedup over full table scans
- **Memory Usage**: Minimal heap allocation during execution

## Learning Outcomes

This project provided hands-on experience with:

- Database internals and data storage organization
- Binary file format parsing and manipulation
- B-tree algorithms and tree traversal techniques
- Query optimization and index usage strategies
- Performance tuning for large-scale data processing
- Low-level file I/O and binary data conversion