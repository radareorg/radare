#ifndef _INCLUDE_R_HASH_H_
#define _INCLUDE_R_HASH_H_

u16 r_hash_crc16(u16 crc, u8 const *buffer, u64 len);
u32 r_hash_crc32(const u8 *buf, u64 len);
u16 r_hash_xorpair(const u8 *a, u64 len);
int r_hash_par(u8 *buf, u64 len);
int r_hash_pcprint(u8 *buffer, u64 len);
u8  r_hash_xor(const u8 *b, u64 len);
u8  r_hash_mod255(const u8 *b, u64 len);
u8  r_hash_hamdist(const u8 *buf, u64 len);
double r_hash_entropy(const u8 *data, u64 size);

#endif
