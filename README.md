# DBMS.c
Database server and client in C 
# Test Store Client

This program demonstrates the usage of the mystore client library to create and read registers using the store_engine server.

## Prerequisites

Before running this program, make sure you have the following libraries installed:

- mycache.h
- mystore_cli.h
- debug.h

## Functionality

The program performs two tests: write test and read test.

### Write Test

In the write test, the program initializes the client API and proceeds to create or update registers. It fills the registers with data and writes them into storage. The process is repeated multiple times to create a sufficient number of registers.

### Read Test

In the read test, the program opens the API again and reads the registers from storage. It verifies the integrity of the data by comparing the index of each record with its position. If the index and position match, the record is considered valid.

## Usage

1. Compile the program with the required libraries.
2. Ensure that the store_engine server is running.
3. Run the compiled program.
