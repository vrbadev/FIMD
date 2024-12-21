/*
 * \file queue.c
 * \author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
 * \date December 2024
 * \brief Source file for implementation of simple dynamically allocated circular queue of pointers.
 * \copyright GNU Public License.
 */

#include "queue.h"


queue_t* queue_create(int capacity)
{
    queue_t* new = (queue_t*) malloc(sizeof(queue_t));
    new->start_pos = 0;
    new->size = 0;
    new->max_size = capacity;
    new->content = (void**) malloc(capacity * sizeof(void*));
    new->min_size = QUEUE_DEFAULT_MIN_SIZE;
    new->factor_expansion = QUEUE_DEFAULT_FACTOR_EXPANSION;
    new->factor_reduction = QUEUE_DEFAULT_FACTOR_REDUCTION;
    return new;
}

void queue_delete(queue_t* queue)
{
    free(queue->content);
    free(queue);
}

/// Dynamically reallocates the queue internal pointer storage to a new size. All pointers are at first copied to the beginning of the queue storage.
/// \param queue Queue instance that shall be resized.
/// \param new_capacity New number of pointers to be stored in the queue.
/// \return true on success.
static bool queue_resize(queue_t* queue, int new_capacity)
{
    void** new_content = (void**) malloc(new_capacity * sizeof(void*));
    if (new_content == NULL) {
        return false;
    }

    // transform circular arrangement of the elements to linear
    int elems_from_start = queue->max_size - queue->start_pos;
    if (elems_from_start >= queue->size) {
        memcpy(new_content, &(queue->content[queue->start_pos]), queue->size * sizeof(void*));
    } else {
        memcpy(new_content, &(queue->content[queue->start_pos]), elems_from_start * sizeof(void*));
        memcpy(&(new_content[elems_from_start]), queue->content, (queue->size - elems_from_start) * sizeof(void*));
    }

    // swap buffers
    free(queue->content);
    queue->content = new_content;
    queue->max_size = new_capacity;
    queue->start_pos = 0;

    return true;
}

bool queue_push(queue_t* queue, void* data)
{
    if (queue->size == queue->max_size) {
        int new_capacity = (int) ((float) queue->size * queue->factor_expansion);
        if (new_capacity <= queue->max_size) {
            return false; // no expansion possible (static queue size)
        }
        if (!queue_resize(queue, new_capacity)) {
            return false; // could not expand the queue
        }
    }
    queue->content[(queue->start_pos + queue->size) % queue->max_size] = data;
    queue->size++;
    return true;
}

void* queue_pop(queue_t* queue)
{
    if (queue->size > 0) {
        void* elem = queue->content[queue->start_pos];
        queue->start_pos = (queue->start_pos + 1) % queue->max_size;
        queue->size--;
        if (queue->size >= queue->min_size) {
            float factor = ((float) queue->size) / ((float) queue->max_size);
            if (factor < queue->factor_reduction) {
                queue_resize(queue, queue->size);
            }
        }
        return elem;
    }
    return NULL;
}

void* queue_get(queue_t* queue, int i)
{
    if (i >= 0 && i < queue->size) {
        return queue->content[(queue->start_pos + i) % queue->max_size];
    }
    return NULL;
}

