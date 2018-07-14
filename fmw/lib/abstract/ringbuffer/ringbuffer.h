#ifndef RINGBUFFER_H
#define RINGBUFFER_H

typedef struct
{
  void* buff;
  size_t size;
  size_t idx_in;
  size_t idx_out;

}ring_buffer_t;

ring_buffer_t* ring_buffer_create(size_t sz);
void ring_buffer_delete(ring_buffer_t* prb);

size_t ring_buffer_read(ring_buffer_t* prb, void* dis, size_t sz);
size_t ring_buffer_write(ring_buffer_t* prb, void* src, size_t sz);

size_t ring_buffer_get_size(ring_buffer_t* prb);
size_t ring_buffer_get_free_size(ring_buffer_t* prb);
size_t ring_buffer_get_full_size(ring_buffer_t* prb);

#endif