#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include "bit.h"

// default size of the buffer (in bytes)
#define BIT_BUFFER 1024

//----------------------------------------------------------------------------
// bit_open		opens a file as bit-stream
//				returns a handle to the bit_file
// name			filename of the file (if mode != 'm') or pointer to the
//				memory block to read from
// mode			mode[0] = 'r' or 'w' (read / write), mode[1] = 'b' or 'm'
//				(disk or memory)
// mem_size		size of the buffer (in bits) (when reading from mem) or
//				maximum number of bits_to read from disk (0 reads all bits)
//----------------------------------------------------------------------------
WAVELET_DLL_API t_bit_file* WAVELET_DLL_CC bit_open(char* name, const char *mode, const int mem_size)
{
    t_bit_file* f = NULL;
    int mem_bytes = mem_size ? mem_size / 8 + (mem_size % 8 != 0) : 0;

    if (mode[1] == 'm' || mode[1] == 'M')
    { // from / to mem
        if (((mode[0] == 'r' || mode[0] == 'R') && name && mem_size > 0) || (mode[0] == 'w' || mode[0] == 'W'))
        {
            f = (t_bit_file*)malloc(sizeof (t_bit_file));
            f->file = NULL;
            f->bits_left = 0;
            if ((mode[0] == 'r' || mode[0] == 'R'))
            {
                f->bufsize = f->bufleft = mem_bytes;
                f->buffer = (unsigned char*)name;
                f->bits_left = mem_size;
            }
            else
            {
                f->bufsize = f->bufleft = mem_bytes ? mem_bytes : BIT_BUFFER;
                f->buffer = (unsigned char *)malloc(f->bufsize * sizeof (unsigned char));
            }
        }
    }
    else
    {
        FILE* file = fopen(name, mode);

        if (file)
        {
            f = (t_bit_file*)malloc(sizeof (t_bit_file));

            f->file = file;
            f->bits_left = 0;
            f->bufsize = f->bufleft = mem_bytes ? mem_bytes : BIT_BUFFER;
            f->buffer = (unsigned char *)malloc(f->bufsize * sizeof (unsigned char));
            if (mode[0] == 'r' || mode[0] == 'R')
            {
                f->bufleft = fread(f->buffer, sizeof *f->buffer, f->bufsize, f->file);
                f->bits_left = mem_size ? mem_size : (1 << 30);
            }
        }
    }
    if (f)
    {
        f->idx = 0;
        f->mask = 0x80;
        f->mode[0] = toupper(mode[0]);
        f->mode[1] = toupper(mode[1]);
        if (f->mode[0] == 'W')
            memset(f->buffer, 0, f->bufsize * sizeof *f->buffer);
    }
    return f;
}

// flushes the contents of the buffer into the file
static void bit_flush(t_bit_file* f)
{
    if (f && f->file)
    {
        int num = f->idx + (f->mask != 0x80 && f->mask != 0x00);

        if (num > 0)
        {
            fwrite(f->buffer, sizeof *f->buffer, num, f->file);
            f->idx = 0;
            f->mask = 0x80;
            f->bufleft = f->bufsize;
            memset(f->buffer, 0, f->bufsize * sizeof *f->buffer);
        }
    }
}


//----------------------------------------------------------------------------
// bit_close	closes a bit-stream opened with bit_open
//				returns the number fo bits written (in WRITE-mode)
// f			handle to the bit_file
// mem			when writing to memory and mem != NULL,
//				then mem receives the pointer to the write-buffer
//				when reading from memory and mem == the pointer passed
//				on bit_open, the buffer is freed
//----------------------------------------------------------------------------
WAVELET_DLL_API int WAVELET_DLL_CC bit_close(t_bit_file* f, unsigned char** mem)
{
    int ret = 0;

    if (f)
    {
        if (f->file)
        {
            if (f->mode[0] == 'W')
            {
                ret = f->bits_left;
                bit_flush(f);
            }
            free(f->buffer);
            fclose(f->file);
        }
        else
        {
            if (f->mode[0] == 'W')
            {
                ret = f->bits_left;
                if (mem)
                    *mem = f->buffer;
                else
                    free(f->buffer);
            }
            else if (f->mode[0] == 'R' && f->buffer == (unsigned char*)mem)
                free(f->buffer);
        }
        free(f);
    }
    return ret;
}


//----------------------------------------------------------------------------
// bit_read		reads the number of given bits from a bit_file
//				(which has to be opened in READ-mode)
//				returns the bits as an integer
//				(bits read first are in higher bit-positions)
// num			number of bits to read
// f			handle to the bit_file (READ-mode)
//----------------------------------------------------------------------------
WAVELET_DLL_API int WAVELET_DLL_CC bit_read(const int num, t_bit_file* f)
{
    int out = 0;

    if (num > 0 && f)
    {
        int i;

        for (i = 0; i < num; i++)
        {
            out <<= 1;
            if (f->bits_left-- > 0 && f->idx < f->bufleft)
            {
                out |= (f->buffer[f->idx] & f->mask) != 0x00;
                f->mask >>= 1;
                if (f->mask == 0x00)
                { // read a byte
                    f->mask = 0x80;
                    if (++f->idx >= f->bufleft)
                    {
                        f->idx = 0;
                        f->bufleft = 0;
                        if (f->file)
                            f->bufleft = fread(f->buffer, 1, f->bufsize * sizeof *f->buffer, f->file);
                    }
                }
            }
        }
    }
    return out;
}


//----------------------------------------------------------------------------
// bit_write	writes the number of given bits to a bit_file
//				(which has to be opened in WRITE-mode)
//				returns number of bits written
// bits			bits to be written (most significant bits are written first)
// num			number of bits to write
// f			handle to the bit_file (WRITE-mode)
//----------------------------------------------------------------------------
WAVELET_DLL_API int WAVELET_DLL_CC bit_write(const int bits, const int num, t_bit_file* f)
{
    if (f && num > 0)
    {
        int in_mask = 1 << (num - 1);
        int i;

        for (i = 0; i < num; i++, in_mask >>= 1)
        {
            f->buffer[f->idx] |= (bits & in_mask) ? f->mask : 0;
            f->bits_left++;
            f->mask >>= 1;
            if (f->mask == 0x00)
            { // filled a byte
                f->mask = 0x80;
                if (++f->idx >= f->bufsize)
                {
                    if (f->file)
                        bit_flush(f);
                    else
                    {
                        f->bufsize *= 2;
                        f->buffer = (unsigned char *)realloc(f->buffer, f->bufsize);
                        memset(f->buffer + f->bufsize / 2, 0, (f->bufsize / 2) * sizeof *f->buffer);
                    }
                }
            }
        }
    }
    return num;
}
