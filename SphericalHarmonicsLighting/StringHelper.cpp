#include "StringHelper.h"

#include <cstring>

bool StringHelper::starts_with(const char * str, const char * start_str) {
	int start_index = 0;

	// Iterate while str and start_str are the same
	while (str[start_index] == start_str[start_index]) {
		start_index++;

		// If we manage to reach the end of the start_str it was matched succesfully
		if (start_str[start_index] == NULL) {
			return true;
		}
	}

	return false;
}

bool StringHelper::contains(const char * needle, const char * haystack){
	int haystack_index = 0;

	// Iterate over the haystack while index < length of the string
	while (haystack[haystack_index] != NULL) {
		if (starts_with(haystack + haystack_index, needle)) {
			return true;
		}

		haystack_index++;
	}

	// No match was found
	return false;
}

// Finds the index of the first occurence of needle in haystack
// Searching begins at the given start index and moves forward through the string
// Returns -1 when no match is found
int StringHelper::first_index_of(const char * needle, const char * haystack, int start) {
	int haystack_index = start;

	// Iterate over the haystack while index < length of the string
	while (haystack[haystack_index] != NULL) {
		if (starts_with(haystack + haystack_index, needle)) {
			return haystack_index;
		}

		haystack_index++;
	}

	// Return -1 if no match was found
	return -1;
}

// Finds the index of the last occurence of needle in haystack
// Searching begins at the given start index and moves backward through the string
// Default start index = -1, which makes the search start at strlen(haystack) - strlen(needle)
// Returns -1 when no match is found
int StringHelper::last_index_of(const char * needle, const char * haystack, int start) {
	if (start == -1) {
		start = strlen(haystack) - strlen(needle);
	}

	int haystack_index = start;
	
	// Iterate over the haystack while index < length of the string
	while (haystack_index >= 0) {
		if (starts_with(haystack + haystack_index, needle)) {
			return haystack_index;
		}

		haystack_index--;
	}

	// Return -1 if no match was found
	return -1;
}

// Memcopies length chars from src + start to dst, then null terminates the dst string
void StringHelper::substr(char * dst, const char * src, int start, int length) {
	memcpy(dst, src + start, length);
	dst[length] = NULL;
}

int StringHelper::split(const char * str, char split_char, ChunkBuffer<char>& result) {
	int index = 0;
	int index_after_last_split = 0;

	int count = 1;

	while (str[index] != NULL) {
		if (str[index] == split_char) {
			result.add(str + index_after_last_split, index - index_after_last_split);
			result.add(NULL);

			index_after_last_split = index + 1;

			count++;
		}

		index++;
	}

	result.add(str + index_after_last_split, index - index_after_last_split);
	result.add(NULL);

	return count;
}

void StringHelper::to_lower(char * str) {
	int index = 0;

	while (str[index] != NULL) {
		if (str[index] >= 'A' && str[index] <= 'Z') {
			str[index] += 'a' - 'A';
		}

		index++;
 	}
}

void StringHelper::to_upper(char * str) {
	int index = 0;

	while (str[index] != NULL) {
		if (str[index] >= 'a' && str[index] <= 'z') {
			str[index] += 'A' - 'a';
		}

		index++;
 	}
}