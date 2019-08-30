#pragma once

template<typename T, int ChunkSize = 4096>
struct ChunkBuffer {
private:
	T data[ChunkSize];
	int count;
	ChunkBuffer<T, ChunkSize> * next;

public:
	inline ChunkBuffer() {
		count = 0;
		next  = nullptr;
	}

	inline ~ChunkBuffer() {
		if (next) delete next;
	}

	// Computes the total size of the linked list of Chunks
	inline int size() const {
		int full_chunk_count = 0;

		const ChunkBuffer<T, ChunkSize> * buffer = this;

		while (buffer->next) {
			full_chunk_count++;

			buffer = buffer->next;
		}

		return full_chunk_count * ChunkSize + buffer->count;
	}

	// Adds a new item to the linked list of Chunks
	inline void add(const T& item) {
		ChunkBuffer<T, ChunkSize> * buffer = this;

		// Iterate until we find the last Chunk in the linked list
		while (buffer->next) buffer = buffer->next;

		assert(buffer->count < ChunkSize);
		buffer->data[buffer->count++] = item;

		if (buffer->count == ChunkSize) {
			buffer->next = new ChunkBuffer<T, ChunkSize>();
		}
	}

	// Adds an array of new items to the linked list of Chunks
	inline void add(int items_length, const T items[]) {
		ChunkBuffer<T, ChunkSize> * buffer = this;

		// Iterate until we find the last Chunk in the linked list
		while (buffer->next) buffer = buffer->next;

		if (buffer->count + items_length < ChunkSize) {
			memcpy(buffer->data + buffer->count, items, items_length);
			buffer->count += items_length;

			return;
		}

		// Fill the rest of the current Chunk
		int copy_length = ChunkSize - buffer->count;
		memcpy(buffer->data + buffer->count, items, copy_length);
		buffer->count = ChunkSize;

		items        += copy_length;
		items_length -= copy_length;

		buffer->next = new ChunkBuffer<T, ChunkSize>();
		buffer       = buffer->next;

		// While the current Chunk must be filled entirely
		while (items_length >= ChunkSize) {
			memcpy(buffer->data, items, ChunkSize);
			buffer->count = ChunkSize;

			items        += ChunkSize;
			items_length -= ChunkSize;	

			buffer->next = new ChunkBuffer<T, ChunkSize>();
			buffer       = buffer->next;
		}

		memcpy(buffer->data, items, items_length);
		buffer->count = items_length;
	}

	// Copy into Chunk Buffer as if it were contiguous memory
	inline void copy_in(const T* src, int start_index, int length) {
		ChunkBuffer<T, ChunkSize> * buffer = this;

		// Iterate over the linked list of Chunks until we find the one that contains the start index
		while (start_index >= ChunkSize) {
			start_index -= ChunkSize;

			buffer = buffer->next;
		}
		assert(buffer);

		// If the desired copy length fits within the current Chunk, simply perform 1 memcpy and return
		if (start_index + length < ChunkSize) {
			memcpy(buffer->data + start_index, src, length);

			return;
		}

		// Copy from start index until the end of the Chunk
		int copy_length = ChunkSize - start_index;
		memcpy(buffer->data + start_index, src, copy_length);

		src    += copy_length;
		length -= copy_length;
		buffer = buffer->next;

		// While the current Chunk must be copied entirely, copy it entirely
		while (length >= ChunkSize) {
			assert(buffer->count == ChunkSize);
			memcpy(buffer->data, src, ChunkSize);

			src    += ChunkSize;
			length -= ChunkSize;	
			buffer = buffer->next;
		}

		// Copy from the start of the Chunk until whats left of the desired copy length
		assert(length <= buffer->count);
		memcpy(buffer->data, src, length);
	}

	// Copy out of ChunkBuffer as if it were contiguous memory
	inline void copy_out(T* dst, int start_index, int length) const {
		const ChunkBuffer<T, ChunkSize> * buffer = this;

		// Iterate over the linked list of Chunks until we find the one that contains the start index
		while (start_index >= ChunkSize) {
			start_index -= ChunkSize;

			buffer = buffer->next;
		}

		// If the desired copy length fits within the current Chunk, simply perform 1 memcpy and return
		if (start_index + length < ChunkSize) {
			memcpy(dst, buffer->data + start_index, length);

			return;
		}

		// Copy from start index until the end of the Chunk
		int copy_length = ChunkSize - start_index;
		memcpy(dst, buffer->data + start_index, copy_length);

		dst    += copy_length;
		length -= copy_length;
		buffer = buffer->next;

		// While the current Chunk must be copied entirely, copy it entirely
		while (length >= ChunkSize) {
			assert(buffer->count == ChunkSize);
			memcpy(dst, buffer->data, ChunkSize);

			dst    += ChunkSize;
			length -= ChunkSize;	
			buffer = buffer->next;
		}

		// Copy from the start of the Chunk until whats left of the desired copy length
		assert(length <= buffer->count);
		memcpy(dst, buffer->data, length);
	}

	inline T& operator[](int index) const {
		ChunkBuffer<T, ChunkSize> * buffer = this;

		while (index >= ChunkSize) {
			index -= ChunkSize;

			buffer = buffer->next;
		}

		assert(buffer);
		assert(index < buffer->count);

		return buffer->data[index];
	}
};
