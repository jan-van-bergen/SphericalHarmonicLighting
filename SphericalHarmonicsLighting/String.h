#pragma once
#include <cstring>

// Finds the index of the first occurence of needle in haystack
// Searching begins at the given start index and moves forward through the string
// Returns -1 when no match is found
inline int first_index_of(const char * needle, const char * haystack, int start = 0) {
	int index = start;

	// Iterate over the haystack while index < length of the string
	while (haystack[index] != NULL) {
		int i = 0;

		// Iterate while needle and haystack are the same
		while (haystack[index + i] == needle[i]) {
			i++;

			// If we manage to reach the end of the needle it was matched succesfully
			if (needle[i] == NULL) {
				return index;
			}
		}

		index++;
	}

	// Return -1 if no match was found
	return -1;
}

// Finds the index of the last occurence of needle in haystack
// Searching begins at the given start index and moves backward through the string
// Default start index = -1, which makes the search start at strlen(haystack) - strlen(needle)
// Returns -1 when no match is found
inline int last_index_of(const char * needle, const char * haystack, int start = -1) {
	if (start == -1) {
		start = strlen(haystack) - strlen(needle);
	}

	int index = start;
	
	// Iterate over the haystack while index < length of the string
	while (index >= 0) {
		int i = 0;

		// Iterate while needle and haystack are the same
		while (haystack[index + i] == needle[i]) {
			i++;

			// If we manage to reach the end of the needle it was matched succesfully
			if (needle[i] == NULL) {
				return index;
			}
		}

		index--;
	}

	// Return -1 if no match was found
	return -1;
}

// Memcopies length chars from src + start to dst, then null terminates the dst string
inline void substr(char * dst, const char * src, int start, int length) {
	memcpy(dst, src + start, length);
	dst[length] = NULL;
}
