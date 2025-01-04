# Directory File Traversal (DFT)

## Overview
DFT is a C program that recursively traverses a directory structure and performs various file operations based on command-line arguments. The tool leverages the `nftw()` system call to explore file trees and provides functionalities like counting files and directories, calculating file sizes, copying directory structures, and moving directories.

## Features
- **Count Files**: Lists the total number of files present in a given directory and its subdirectories.
- **Count Directories**: Lists the total number of directories present in a given directory and its subdirectories.
- **Calculate File Size**: Computes the total size of all files in a directory tree (in bytes).
- **Copy Directory Structure**: Copies an entire subdirectory, maintaining its structure at the destination. Optionally, you can exclude files by their extension.
- **Move Directory**: Moves an entire subdirectory to another location, deleting the source after the move.

## Command-Line Arguments

### Count Files
Counts and lists the total number of files in the directory tree rooted at `root_dir`.
- **Command:**
  $ dftw -nf /home/user/documents
- **Output:**
  Total number of files: 42

### Count Directories
Counts and lists the total number of directories in the directory tree rooted at root_dir.
- **Command:**
  $ dftw -nd /home/user/documents
- **Output:**
  Total number of directories: 7
  
### Calculate File Size
Lists the total size (in bytes) of all files present in the directory tree rooted at root_dir.
- **Command:**
  $ dftw -sf /home/user/documents
- **Output:**
  Size of all files: 204800 bytes
  
### Copy Directory Structure
Copies the subdirectory from source_dir to destination_dir, maintaining the directory structure. If a file extension (.c, .txt, .pdf) is provided, files of that type will be excluded from the copy.
- **Command:**
  $ dftw -cpx /home/user/source /home/user/destination .txt
  
### Move Directory
Moves the entire subdirectory from source_dir to destination_dir and deletes the original directory after the move.
- **Command:**
$ dftw -mv /home/user/source /home/user/destination