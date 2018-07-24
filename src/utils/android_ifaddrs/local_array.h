/*
	bumo is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	bumo is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with bumo.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LOCAL_ARRAY_H_included
#define LOCAL_ARRAY_H_included

#ifdef OS_ANDROID

#include <cstddef>
#include <new>

/**
 * A fixed-size array with a size hint. That number of bytes will be allocated
 * on the stack, and used if possible, but if more bytes are requested at
 * construction time, a buffer will be allocated on the heap (and deallocated
 * by the destructor).
 *
 * The API is intended to be a compatible subset of C++0x's std::array.
 */
template <size_t STACK_BYTE_COUNT>
class LocalArray {
public:
    /**
     * Allocates a new fixed-size array of the given size. If this size is
     * less than or equal to the template parameter STACK_BYTE_COUNT, an
     * internal on-stack buffer will be used. Otherwise a heap buffer will
     * be allocated.
     */
    LocalArray(size_t desiredByteCount) : mSize(desiredByteCount) {
        if (desiredByteCount > STACK_BYTE_COUNT) {
            mPtr = new char[mSize];
        } else {
            mPtr = &mOnStackBuffer[0];
        }
    }

    /**
     * Frees the heap-allocated buffer, if there was one.
     */
    ~LocalArray() {
        if (mPtr != &mOnStackBuffer[0]) {
            delete[] mPtr;
        }
    }

    // Capacity.
    size_t size() { return mSize; }
    bool empty() { return mSize == 0; }

    // Element access.
    char& operator[](size_t n) { return mPtr[n]; }
    const char& operator[](size_t n) const { return mPtr[n]; }

private:
    char mOnStackBuffer[STACK_BYTE_COUNT];
    char* mPtr;
    size_t mSize;

    // Disallow copy and assignment.
    LocalArray(const LocalArray&);
    void operator=(const LocalArray&);
};

#endif // LOCAL_ARRAY_H_included

#endif
