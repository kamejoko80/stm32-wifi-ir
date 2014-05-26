#include <stddef.h>
#include "ring_buffer.h"

#ifndef min
#define min(a,b) ( ((a) < (b)) ? (a) : (b) )
#endif

void ring_buffer_u8_init(ring_buffer_u8* ring, uint8_t* storage, uint16_t size) {
  ring->storage = storage;
  ring->size = size;
  ring->end = ring->storage + ring->size;
  ring->read = ring->storage;
  ring->write = ring->storage;
  ring->available = 0;
}

uint16_t ring_buffer_u8_available(ring_buffer_u8* ring) {
  return ring->available;
}

uint16_t ring_buffer_u8_free(ring_buffer_u8* ring) {
  return ring->size - ring->available;
}

uint8_t ring_buffer_u8_read_byte(ring_buffer_u8* ring) {
  if (ring->available == 0) {
    return 0;
  }
  uint8_t ret = *ring->read++;
  ring->available--;
  if (ring->read >= ring->end) {
    ring->read = ring->storage;
  }
  return ret;
}

void ring_buffer_u8_read(ring_buffer_u8* ring, uint8_t* buffer, uint16_t size) {
  uint16_t i;

  // TODO can be optimized
  for (i = 0; i < size; i++) {
    buffer[i] = ring_buffer_u8_read_byte(ring);
  }
}

void ring_buffer_u8_write_byte(ring_buffer_u8* ring, uint8_t b) {
  if (ring->available >= ring->size) {
    ring_buffer_u8_read_byte(ring);
  }

  *ring->write = b;
  ring->write++;
  ring->available++;
  if (ring->write >= ring->end) {
    ring->write = ring->storage;
  }
}

void ring_buffer_u8_write(ring_buffer_u8* ring, const uint8_t* buffer, uint16_t size) {
  uint16_t i;

  // TODO can be optimized
  for (i = 0; i < size; i++) {
    ring_buffer_u8_write_byte(ring, buffer[i]);
  }
}

uint16_t ring_buffer_u8_readline(ring_buffer_u8* ring, char* buffer, uint16_t size) {
  uint8_t b;
  uint16_t i;
  for (i = 0; i < min(ring->available, size - 1); i++) {
    b = ring_buffer_u8_peekn(ring, i);
    if (b == '\n') {
      i++;
      ring_buffer_u8_read(ring, (uint8_t*) buffer, i);
      buffer[i] = '\0';
      return i;
    }
  }
  buffer[0] = '\0';
  return 0;
}

uint8_t ring_buffer_u8_peek(ring_buffer_u8* ring) {
  return ring_buffer_u8_peekn(ring, 0);
}

uint8_t ring_buffer_u8_peekn(ring_buffer_u8* ring, uint16_t i) {
  if (i >= ring->available) {
    return 0;
  }

  uint8_t* read = (uint8_t*)ring->read;
  uint8_t* p = read + i;
  if (p >= ring->end) {
    p -= ring->size;
  }
  return *p;
}

void ring_buffer_voidptr_init(ring_buffer_voidptr* ring, void** storage, uint16_t size) {
  ring->storage = storage;
  ring->size = size;
  ring->end = ring->storage + ring->size;
  ring->read = (volatile void**)ring->storage;
  ring->write = (volatile void**)ring->storage;
  ring->available = 0;
}

uint16_t ring_buffer_voidptr_available(ring_buffer_voidptr* ring) {
  return ring->available;
}

uint16_t ring_buffer_voidptr_free(ring_buffer_voidptr* ring) {
  return ring->size - ring->available;
}

void* ring_buffer_voidptr_read(ring_buffer_voidptr* ring) {
  if (ring->available == 0) {
    return NULL;
  }
  void* ret = (void*)*ring->read++;
  ring->available--;
  if (ring->read >= (volatile void**)ring->end) {
    ring->read = (volatile void**)ring->storage;
  }
  return ret;
}

void ring_buffer_voidptr_write(ring_buffer_voidptr* ring, void* ptr) {
  if (ring->available >= ring->size) {
    ring_buffer_voidptr_read(ring);
  }

  *ring->write = ptr;
  ring->write++;
  ring->available++;
  if (ring->write >= (volatile void**)ring->end) {
    ring->write = (volatile void**)ring->storage;
  }
}
