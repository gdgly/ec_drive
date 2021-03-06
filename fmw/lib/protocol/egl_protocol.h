#ifndef EGL_PROTOCOL_H
#define EGL_PROTOCOL_H

typedef egl_result_t (*egl_decode_func_t)(void *meta, uint8_t *in, size_t *len);
typedef egl_result_t (*egl_handle_func_t)(void *meta, uint8_t *out, size_t *len);
typedef egl_result_t (*egl_encode_func_t)(void *meta, const void *data, size_t data_len, uint8_t *out, size_t *out_len);
typedef egl_result_t (*egl_setup_func_t)(void *meta, const void *data, size_t len);

typedef struct
{
  void                 *meta;
  egl_decode_func_t    decode;
  egl_handle_func_t    handle;
  egl_encode_func_t    encode;
  egl_setup_func_t     setup;
}egl_ptc_t;

egl_result_t egl_ptc_decode(egl_ptc_t *ptc, uint8_t *in, size_t *len_in, uint8_t *out, size_t *len_out);
egl_result_t egl_ptc_encode(egl_ptc_t *ptc, const void *data, size_t data_size, uint8_t *out, size_t *out_len);
egl_result_t egl_ptc_setup(egl_ptc_t *ptc, const void *data, size_t len);

#endif
