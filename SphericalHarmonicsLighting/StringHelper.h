#pragma once
#include "ChunkBuffer.h"

namespace StringHelper {
	// Finds the index of the first occurence of needle in haystack
	// Searching begins at the given start index and moves forward through the string
	// Returns -1 when no match is found
	int first_index_of(const char * needle, const char * haystack, int start = 0);

	// Finds the index of the last occurence of needle in haystack
	// Searching begins at the given start index and moves backward through the string
	// Default start index = -1, which makes the search start at strlen(haystack) - strlen(needle)
	// Returns -1 when no match is found
	int last_index_of(const char * needle, const char * haystack, int start = -1);

	// Memcopies length chars from src + start to dst, then null terminates the dst string
	void substr(char * dst, const char * src, int start, int length);
	
	// Splits the given input string str on the given split char
	// The result is stored in a ChunkBuffer of chars, where every substring is null-terminated.
	// The returned integer is the amount of splitted substrings
	int split(const char * str, char split_char, ChunkBuffer<char>& result);
}
