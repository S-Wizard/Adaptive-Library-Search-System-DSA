#!/bin/bash

# Exit on error
set -e

echo "--- Compiling C++ Backend for Linux ---"

# Create output directory for Vercel (satisfies the 'public' folder requirement)
mkdir -p public
echo "Build complete" > public/_keep

# Compile the backend
# Vercel environment usually has g++ available
g++ -std=c++17 -Ibackend/include \
    backend/main.cpp \
    backend/library_engine.cpp \
    backend/avl_tree.cpp \
    backend/trie.cpp \
    backend/recommendation_graph.cpp \
    -o backend/library

# Make it executable
chmod +x backend/library

echo "--- Compilation Complete ---"
