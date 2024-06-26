/*
 *  Copyright (C) 2010-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "AEPackIEC61937.h"

#include <cassert>
#include <string.h>

#define IEC61937_PREAMBLE1  0xF872
#define IEC61937_PREAMBLE2  0x4E1F

inline void SwapEndian(uint16_t *dst, uint16_t *src, unsigned int size)
{
  for (unsigned int i = 0; i < size; ++i, ++dst, ++src)
    *dst = ((*src & 0xFF00) >> 8) | ((*src & 0x00FF) << 8);
}

int CAEPackIEC61937::PackAC3(uint8_t *data, unsigned int size, uint8_t *dest)
{
  assert(size <= OUT_FRAMESTOBYTES(AC3_FRAME_SIZE));
  struct IEC61937Packet *packet = (struct IEC61937Packet*)dest;

  packet->m_preamble1 = IEC61937_PREAMBLE1;
  packet->m_preamble2 = IEC61937_PREAMBLE2;
  packet->m_length    = size << 3;

  if (data == NULL)
    data = packet->m_data;
#ifdef __BIG_ENDIAN__
  else
    memcpy(packet->m_data, data, size);
#else

  int bitstream_mode  = data[5] & 0x7;
  packet->m_type      = IEC61937_TYPE_AC3 | (bitstream_mode << 8);

  size += size & 0x1;
  SwapEndian((uint16_t*)packet->m_data, (uint16_t*)data, size >> 1);
#endif

  memset(packet->m_data + size, 0, OUT_FRAMESTOBYTES(AC3_FRAME_SIZE) - IEC61937_DATA_OFFSET - size);
  return OUT_FRAMESTOBYTES(AC3_FRAME_SIZE);
}

int CAEPackIEC61937::PackEAC3(uint8_t *data, unsigned int size, uint8_t *dest)
{
  assert(size <= OUT_FRAMESTOBYTES(EAC3_FRAME_SIZE));
  struct IEC61937Packet *packet = (struct IEC61937Packet*)dest;

  packet->m_preamble1 = IEC61937_PREAMBLE1;
  packet->m_preamble2 = IEC61937_PREAMBLE2;
  packet->m_type      = IEC61937_TYPE_EAC3;
  packet->m_length    = size;

  if (data == NULL)
    data = packet->m_data;
#ifdef __BIG_ENDIAN__
  else
    memcpy(packet->m_data, data, size);
#else
  size += size & 0x1;
  SwapEndian((uint16_t*)packet->m_data, (uint16_t*)data, size >> 1);
#endif

  memset(packet->m_data + size, 0, OUT_FRAMESTOBYTES(EAC3_FRAME_SIZE) - IEC61937_DATA_OFFSET - size);
  return OUT_FRAMESTOBYTES(EAC3_FRAME_SIZE);
}

int CAEPackIEC61937::PackDTS_512(uint8_t *data, unsigned int size, uint8_t *dest, bool littleEndian)
{
  return PackDTS(data, size, dest, littleEndian, OUT_FRAMESTOBYTES(DTS1_FRAME_SIZE), IEC61937_TYPE_DTS1);
}

int CAEPackIEC61937::PackDTS_1024(uint8_t *data, unsigned int size, uint8_t *dest, bool littleEndian)
{
  return PackDTS(data, size, dest, littleEndian, OUT_FRAMESTOBYTES(DTS2_FRAME_SIZE), IEC61937_TYPE_DTS2);
}

int CAEPackIEC61937::PackDTS_2048(uint8_t *data, unsigned int size, uint8_t *dest, bool littleEndian)
{
  return PackDTS(data, size, dest, littleEndian, OUT_FRAMESTOBYTES(DTS3_FRAME_SIZE), IEC61937_TYPE_DTS3);
}

int CAEPackIEC61937::PackTrueHD(const uint8_t* data, unsigned int size, uint8_t* dest)
{
  if (size == 0)
    return OUT_FRAMESTOBYTES(TRUEHD_FRAME_SIZE);

  assert(size <= OUT_FRAMESTOBYTES(TRUEHD_FRAME_SIZE));
  struct IEC61937Packet *packet = (struct IEC61937Packet*)dest;
  packet->m_preamble1 = IEC61937_PREAMBLE1;
  packet->m_preamble2 = IEC61937_PREAMBLE2;
  packet->m_type = IEC61937_TYPE_TRUEHD;
  packet->m_length = 61424;

  if (data == NULL)
    data = packet->m_data;
#ifdef __BIG_ENDIAN__
  else
    memcpy(packet->m_data, data, size);
#else
  size += size & 0x1;
  SwapEndian((uint16_t*)packet->m_data, (uint16_t*)data, size >> 1);
#endif

  memset(packet->m_data + size, 0, OUT_FRAMESTOBYTES(TRUEHD_FRAME_SIZE) - IEC61937_DATA_OFFSET - size);
  return OUT_FRAMESTOBYTES(TRUEHD_FRAME_SIZE);
}

int CAEPackIEC61937::PackDTSHD(uint8_t *data, unsigned int size, uint8_t *dest, unsigned int period)
{
  unsigned int subtype;
  switch (period)
  {
    case   512: subtype = 0; break;
    case  1024: subtype = 1; break;
    case  2048: subtype = 2; break;
    case  4096: subtype = 3; break;
    case  8192: subtype = 4; break;
    case 16384: subtype = 5; break;

    default:
      return 0;
  }

  struct IEC61937Packet *packet = (struct IEC61937Packet*)dest;
  packet->m_preamble1 = IEC61937_PREAMBLE1;
  packet->m_preamble2 = IEC61937_PREAMBLE2;
  packet->m_type      = IEC61937_TYPE_DTSHD | (subtype << 8);

  /* Align so that (length_code & 0xf) == 0x8. This is reportedly needed
   * with some receivers, but the exact requirement is unconfirmed. */
  packet->m_length    = ((size + 0x17) &~ 0x0f) - 0x08;

  if (data == NULL)
    data = packet->m_data;
#ifdef __BIG_ENDIAN__
  else
    memcpy(packet->m_data, data, size);
#else
  size += size & 0x1;
  SwapEndian((uint16_t*)packet->m_data, (uint16_t*)data, size >> 1);
#endif

  unsigned int burstsize = period << 2;
  memset(packet->m_data + size, 0, burstsize - IEC61937_DATA_OFFSET - size);
  return burstsize;
}

int CAEPackIEC61937::PackDTS(uint8_t *data, unsigned int size, uint8_t *dest, bool littleEndian,
                             unsigned int frameSize, uint16_t type)
{
  assert(size <= frameSize);

  /* BE is the standard endianness, byteswap needed if LE */
  bool byteSwapNeeded = littleEndian;

#ifndef __BIG_ENDIAN__
  /* on LE systems we want LE output, byteswap needed */
  byteSwapNeeded ^= true;
#endif

  struct IEC61937Packet *packet = (struct IEC61937Packet*)dest;
  uint8_t *dataTo;

  if (size == frameSize)
  {
    /* No packing possible or needed, DTS stream is suitable for direct output */
    dataTo = dest;
  }
  else if (size <= frameSize - IEC61937_DATA_OFFSET)
  {
    /* Fits to IEC61937, perform packing */
    packet->m_preamble1 = IEC61937_PREAMBLE1;
    packet->m_preamble2 = IEC61937_PREAMBLE2;
    packet->m_type      = type;
    packet->m_length    = size << 3;

    dataTo = packet->m_data;
  }
  else
  {
    /* Stream is unsuitable for both packing and direct output */
    return 0;
  }

  if (data == NULL)
    data = dataTo;
  else if (!byteSwapNeeded)
    memcpy(dataTo, data, size);

  if (byteSwapNeeded)
  {
    size += size & 0x1;
    SwapEndian((uint16_t*)dataTo, (uint16_t*)data, size >> 1);
  }

  if (size != frameSize)
    memset(packet->m_data + size, 0, frameSize - IEC61937_DATA_OFFSET - size);

  return frameSize;
}

int CAEPackIEC61937::PackPause(uint8_t *dest, unsigned int millis, unsigned int framesize, unsigned int samplerate, unsigned int rep_period, unsigned int encodedRate)
{
  int periodInBytes = rep_period * framesize;
  double periodInTime = (double)rep_period / samplerate * 1000;
  int periodsNeeded = millis / periodInTime;
  int maxPeriods = MAX_IEC61937_PACKET / periodInBytes;
  if (periodsNeeded > maxPeriods)
    periodsNeeded = maxPeriods;
  uint16_t gap = encodedRate * millis / 1000;

  struct IEC61937Packet *packet = (struct IEC61937Packet*)dest;
  packet->m_preamble1 = IEC61937_PREAMBLE1;
  packet->m_preamble2 = IEC61937_PREAMBLE2;
  packet->m_type = 3;
  packet->m_length = 32;
  memset(packet->m_data, 0, periodInBytes - 8);

  for (int i=1; i<periodsNeeded; i++)
  {
    memcpy(dest+i*periodInBytes, dest, periodInBytes);
  }

  uint16_t *gapPtr = reinterpret_cast<uint16_t*>(packet->m_data);
  *gapPtr = gap;

  return periodsNeeded * periodInBytes;
}
