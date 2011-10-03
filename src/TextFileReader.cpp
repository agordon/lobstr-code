/*
 Copyright (C) 2011 Melissa Gymrek <mgymrek@mit.edu>
*/

#include <err.h>
#include <error.h>
#include <fstream>
#include <iostream>

#include "IFileReader.h"
#include "TextFileReader.h"

using namespace std;

TextFileReader::TextFileReader(const std::string& _filename) :
  current_line(0), filename(_filename),
  input_file_stream(_filename.empty() ? NULL : create_file_stream(filename)),
  input_stream(_filename.empty() ? cin : *input_file_stream ) {}

std::ifstream* TextFileReader::create_file_stream(const std::string &filename) {
  std::ifstream *input_stream = new std::ifstream(filename.c_str(), std::ios_base::in);
  if (input_stream==NULL)
    err(1, "Failed to allocate memory for ifstream");
  if (! (*input_stream))
    err(1, "Failed to open file '%s'", filename.c_str());
  
  return input_stream;
}

TextFileReader::~TextFileReader() {
  if (input_file_stream!=NULL) {
    delete input_file_stream;
    input_file_stream = NULL ;
  }
}

bool TextFileReader::GetNextLine(string* line) {
  current_line++;
  if (!getline(input_stream, *line))
    return false;
  return true;
}

bool TextFileReader::GetNextRecord(MSReadRecord* read) {
  return false; // do nothing
}
