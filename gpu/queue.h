/// \file queue.h
/// \author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
/// \date May 2023
/// \brief This header file contains definitions for simple dynamicly allocated circular queue of pointers.
/// \copyright GNU Public License.

#pragma once

#ifndef GLES3COMPUTELIB_QUEUE_H
#define GLES3COMPUTELIB_QUEUE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/// Default minimum size of the queue that can be allocated.
#define QUEUE_DEFAULT_MIN_SIZE 16
/// Default expansion factor for the queue size.
#define QUEUE_DEFAULT_FACTOR_EXPANSION 2.0f
/// Default reduction factor for the queue size.
#define QUEUE_DEFAULT_FACTOR_REDUCTION 0.0f

/// Structure of queue instance.
typedef struct queue_s {
    /// Storage index of the first element in the queue.
    int start_pos;
    /// Current number of elements in the queue.
    int size;
    /// Virtual maximum capacity of the queue.
    int max_size;
    /// Pointer to the internal storage of queue elements (pointers).
    void** content;
    /// Minimum size of the queue that can be allocated.
    int min_size;
    /// Expansion factor of the queue size, set to <=1.0 for static queue size (no expansion).
    float factor_expansion;
    /// Reduction factor of the queue size, set to 0.0 for no reduction.
    float factor_reduction;
} queue_t;

/// Allocates and initializes a queue instance for void* pointers.
/// \param capacity Initial number of pointers for allocation of internal storage.
/// \return Initialized queue_t* instance.
queue_t* queue_create(int capacity);

/// Deallocates queue resources (using internal storage and the instance pointers).
/// \param queue Queue instance that shall be deallocated.
void queue_delete(queue_t* queue);

/// Pushes new pointer into the queue.
/// \param queue Queue instance.
/// \param data Pointer to the new queue element.
/// \return True on success.
bool queue_push(queue_t* queue, void* data);

/// Pops the first element from the queue.
/// \param queue Queue instance.
/// \return Pointer to the first element of the queue or NULL if the queue is empty.
void* queue_pop(queue_t* queue);

/// Reads i-th element in the queue.
/// \param queue Queue instance.
/// \param i Index of the element in the queue.
/// \return Pointer to the i-th element in the queue or NULL if the i-th element does not exist.
void* queue_get(queue_t* queue, int i);


#endif // GLES3COMPUTELIB_QUEUE_H
