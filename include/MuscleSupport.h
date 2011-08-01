/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000 - 2001 ^Enigma^
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** NeoStats CVS Identification
** $Id: MuscleSupport.h 3294 2008-02-24 02:45:41Z Fish $
*/

/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

/******************************************************************************
/
/     File:     MuscleSupport.h
/
/     Description:  Standard types, macros, etc, for MUSCLE.
/                   Many of them are suspiciously BeOS-like.  ;^)
/
*******************************************************************************/

#ifndef MuscleSupport_h
#define MuscleSupport_h

#define MUSCLE_VERSION_STRING "3.25b"

#include <string.h>  /* for memcpy() */

/* Define this if the default FD_SETSIZE is too small for you (i.e. under Windows it's only 64) */
#if defined(MUSCLE_FD_SETSIZE)
# if defined(FD_SETSIZE)
#  error "MuscleSupport.h:  Can't redefine FD_SETSIZE, someone else has already defined it!  You need to include MuscleSupport.h before including any other header files that define FD_SETSIZE."
# else
#  define FD_SETSIZE MUSCLE_FD_SETSIZE
# endif
#endif

/* If we are in an environment where known assembly is available, make a note of that fact */
#ifndef MUSCLE_AVOID_INLINE_ASSEMBLY
# if defined(__GNUC__)
#  if defined(__i386__)
#   define MUSCLE_USE_X86_INLINE_ASSEMBLY 1
#  elif defined(__PPC__) || defined(__POWERPC__)
#   define MUSCLE_USE_POWERPC_INLINE_ASSEMBLY 1
#  endif
# endif
# if defined(_MSC_VER) && defined(_X86_)
#  define MUSCLE_USE_X86_INLINE_ASSEMBLY 1
# endif
#endif

#ifndef __cplusplus
# define MUSCLE_AVOID_NAMESPACES
# define NEW_H_NOT_AVAILABLE
#endif

/* Since certain antique compilers don't support namespaces, we
 * do all namespace-related declarations via macros which can
 * be no-op'd out by declaring -DMUSCLE_AVOID_NAMESPACES in the Makefile.
 */
#ifdef MUSCLE_AVOID_NAMESPACES
# define DECLARE_NAMESPACE(x)
# define BEGIN_NAMESPACE(x)
# define END_NAMESPACE(x)
# define USING_NAMESPACE(x)
#else
# define DECLARE_NAMESPACE(x) namespace x {};
# define BEGIN_NAMESPACE(x) namespace x {
# define END_NAMESPACE(x) };
# define USING_NAMESPACE(x) using namespace x;
#endif

/* Just declare the muscle namespace as existing.
 * If we ever decide to make the muscle namespace a superset
 * of another namespace, we would add a 'using namespace' line here.
 */
DECLARE_NAMESPACE(muscle);

/* these CPUs can't handle non-aligned word reads, so we'll accomodate them by using memcpy() instead. */
#if defined(MIPS) || defined(mc68000) || defined(sparc) || defined(__sparc) || defined(m68k) || defined(__68k__) || defined(__sparc__)
# define MUSCLE_CPU_REQUIRES_DATA_ALIGNMENT
#endif

/* Borland C++ builder also runs under Win32, but it doesn't set this flag So we'd better set it ourselves. */
#if defined(__BORLANDC__) || defined(__WIN32__) || defined(_MSC_VER)
# ifndef WIN32
#  define WIN32 1
# endif
#endif

/* Win32 can't handle this stuff, it's too lame */
#ifdef WIN32
# define UNISTD_H_NOT_AVAILABLE
# ifndef _MSC_VER  /* 7/3/2006: Mika's patch allows VC++ to use newnothrow */
#  define NEW_H_NOT_AVAILABLE
# endif
#endif

#ifndef UNISTD_H_NOT_AVAILABLE
# include <unistd.h>
#endif

#ifndef NEW_H_NOT_AVAILABLE
# include <new>
# ifndef MUSCLE_AVOID_NAMESPACES
#  ifndef __MWERKS__
using std::bad_alloc;
using std::nothrow_t;
using std::nothrow;
#   if (defined(_MSC_VER))
// VC++ 6.0 and earlier lack this definition
#    if (_MSC_VER <= 1200)
inline void __cdecl operator delete(void *p, const std::nothrow_t&) _THROW0() {delete(p);}
#    endif
#   else
using std::new_handler;
using std::set_new_handler;
#   endif
#  endif
# endif
#else
# define MUSCLE_AVOID_NEWNOTHROW
#endif

#ifndef newnothrow
# ifdef MUSCLE_AVOID_NEWNOTHROW
#  define newnothrow new
# else
#  define newnothrow new (nothrow)
# endif
#endif

/* Unfortunately, the 64-bit printf() format specifier is different for different compilers :^P */
#if defined(__MWERKS__) || defined(WIN32) || defined(__BORLANDC__) || defined(__BEOS__)
# if (_MSC_VER == 1200)
#  define  INT64_FORMAT_SPEC "%I64i"
#  define UINT64_FORMAT_SPEC "%I64u"
# else
#  define  INT64_FORMAT_SPEC "%Li"
#  define UINT64_FORMAT_SPEC "%Lu"
# endif
#else
#  define  INT64_FORMAT_SPEC "%lli"
#  define UINT64_FORMAT_SPEC "%llu"
#endif

#ifdef __BEOS__
# include <kernel/debugger.h>
# define MCRASH_IMPL debugger("muscle assertion failure")
#elif defined(WIN32)
# if defined(UNICODE)
#  define MCRASH_IMPL FatalAppExit(0, L"muscle assertion failure")
# else
#  define MCRASH_IMPL FatalAppExit(0, "muscle assertion failure")
# endif
#else
# define MCRASH_IMPL *((uint32*)NULL) = 0x666
#endif

#ifdef MUSCLE_AVOID_ASSERTIONS
# define MASSERT(x,msg)
#else
# define MASSERT(x,msg) {if(!(x)) MCRASH(msg)}
#endif

#ifdef MUSCLE_AVOID_NAMESPACES
# define MCRASH(msg) {LogTime(MUSCLE_LOG_CRITICALERROR, "ASSERTION FAILED: (%s:%i) %s\n", __FILE__,__LINE__,msg); LogStackTrace(MUSCLE_LOG_CRITICALERROR); MCRASH_IMPL;}
# define WARN_OUT_OF_MEMORY LogTime(MUSCLE_LOG_CRITICALERROR, "ERROR--OUT OF MEMORY!  (%s:%i)\n",__FILE__,__LINE__)
# define MCHECKPOINT LogTime(MUSCLE_LOG_WARNING, "Reached checkpoint at %s:%i\n", __FILE__, __LINE__)
#else
# define MCRASH(msg) {muscle::LogTime(muscle::MUSCLE_LOG_CRITICALERROR, "ASSERTION FAILED: (%s:%i) %s\n", __FILE__,__LINE__,msg); muscle::LogStackTrace(MUSCLE_LOG_CRITICALERROR); MCRASH_IMPL;}
# define WARN_OUT_OF_MEMORY muscle::LogTime(muscle::MUSCLE_LOG_CRITICALERROR, "ERROR--OUT OF MEMORY!  (%s:%i)\n",__FILE__,__LINE__)
# define MCHECKPOINT muscle::LogTime(muscle::MUSCLE_LOG_WARNING, "Reached checkpoint at %s:%i\n", __FILE__, __LINE__)
#endif

#define UNLESS(x) if(!(x))
#define ARRAYITEMS(x) (sizeof(x)/sizeof(x[0]))  /* returns # of items in array */

typedef void * muscleVoidPointer;  /* it's a bit easier, syntax-wise, to use this type than (void *) directly in some cases. */

#ifdef __BEOS__
# include <support/Errors.h>
# include <support/ByteOrder.h>  /* might as well use the real thing (and avoid complaints about duplication) */
# include <support/SupportDefs.h>
# include <support/TypeConstants.h>
# ifdef BONE
#  define closesocket close
# else
#  define BEOS_OLD_NETSERVER
# endif
#else
# define B_ERROR    -1
# define B_NO_ERROR 0
# define B_OK       B_NO_ERROR
# ifndef WIN32
#  define closesocket close
# endif
# ifdef __ATHEOS__
#  include </ainc/atheos/types.h>
# else
#  ifndef MUSCLE_TYPES_PREDEFINED  /* certain (ahem) projects already set these themselves... */
#   define true                     1
#   define false                    0
    typedef signed char             int8;
    typedef unsigned char           uint8;
    typedef short                   int16;
    typedef unsigned short          uint16;
#   if defined(__osf__)     /* some 64bit systems will have long=64-bit, int=32-bit */
     typedef int                    int32;
     typedef unsigned int           uint32;
#   elif defined(__amd64__) /* some 64bit systems will have long=64-bit, int=32-bit */
     typedef int                    int32;
     typedef unsigned int           uint32;
#   else
     typedef long                   int32;
     typedef unsigned long          uint32;
#   endif
#   if defined(WIN32) && !defined(__GNUWIN32__)
     typedef __int64                int64;
     typedef unsigned __int64       uint64;
#   else
     typedef long long              int64;
     typedef unsigned long long     uint64;
#   endif
    typedef unsigned char           uchar;
    typedef unsigned short          unichar;
    typedef int32                   status_t;
#  endif  /* !MUSCLE_TYPES_PREDEFINED */
# endif  /* !__ATHEOS__*/
#endif  /* __BEOS__*/

#define MAKETYPE(x) ((((unsigned long)(x[0])) << 24) | \
                     (((unsigned long)(x[1])) << 16) | \
                     (((unsigned long)(x[2])) <<  8) | \
                     (((unsigned long)(x[3])) <<  0))

#ifndef __BEOS__
/* Be-style message-field type codes.
 * I've calculated the integer equivalents for these codes
 * because gcc whines like a little girl about the four-byte
 * constants when compiling under Linux --jaf
 */
enum {
   B_ANY_TYPE     = 1095653716, /* 'ANYT' = wild card                                   */
   B_BOOL_TYPE    = 1112493900, /* 'BOOL' = boolean (1 byte per bool)                   */
   B_DOUBLE_TYPE  = 1145195589, /* 'DBLE' = double-precision float (8 bytes per double) */
   B_FLOAT_TYPE   = 1179406164, /* 'FLOT' = single-precision float (4 bytes per float)  */
   B_INT64_TYPE   = 1280069191, /* 'LLNG' = long long integer (8 bytes per int)         */
   B_INT32_TYPE   = 1280265799, /* 'LONG' = long integer (4 bytes per int)              */
   B_INT16_TYPE   = 1397248596, /* 'SHRT' = short integer (2 bytes per int)             */
   B_INT8_TYPE    = 1113150533, /* 'BYTE' = byte integer (1 byte per int)               */
   B_MESSAGE_TYPE = 1297303367, /* 'MSGG' = sub Message objects (reference counted)     */
   B_POINTER_TYPE = 1347310674, /* 'PNTR' = pointers (will not be flattened)            */
   B_POINT_TYPE   = 1112559188, /* 'BPNT' = Point objects (each Point has two floats)   */
   B_RECT_TYPE    = 1380270932, /* 'RECT' = Rect objects (each Rect has four floats)    */
   B_STRING_TYPE  = 1129534546, /* 'CSTR' = String objects (variable length)            */
   B_OBJECT_TYPE  = 1330664530, /* 'OPTR' = Flattened user objects (obsolete)           */
   B_RAW_TYPE     = 1380013908, /* 'RAWT' = Raw data (variable number of bytes)         */
   B_MIME_TYPE    = 1296649541  /* 'MIME' = MIME strings (obsolete)                     */
};
#endif

/* This one isn't defined by BeOS, so we have to enumerate it separately.               */
enum {
   B_TAG_TYPE     = 1297367367  /* 'MTAG' = new for v2.00; for in-mem-only tags         */
};

/* This constant is used in various places to mean 'as much as you want' */
#define MUSCLE_NO_LIMIT ((uint32)-1)

#ifdef __cplusplus

/** A handy little method to swap the bytes of any int-style datatype around */
template<typename T> inline T muscleSwapBytes(T swapMe)
{
   union {T _iWide; uint8 _i8[sizeof(T)];} u1, u2;
   u1._iWide = swapMe; 

   int i = 0;
   int numBytes = sizeof(T);
   while(numBytes>0) u2._i8[i++] = u1._i8[--numBytes];
   return u2._iWide;
}

/* This template safely copies a value in from an untyped byte buffer to a typed value.
 * (Make sure MUSCLE_CPU_REQUIRES_DATA_ALIGNMENT is defined if you are on a CPU
 * that doesn't like non-word-aligned data reads and writes)
 */
template<typename T> inline void muscleCopyIn(T & dest, const void * source)
{
#ifdef MUSCLE_CPU_REQUIRES_DATA_ALIGNMENT
   memcpy(&dest, source, sizeof(dest));
#else
   dest = *((const T*)source);
#endif
}

/** This template safely copies a value in from a typed value to an untyped byte buffer.
  * (Make sure MUSCLE_CPU_REQUIRES_DATA_ALIGNMENT is defined if you are on a CPU
  *  that doesn't like non-word-aligned data reads and writes)
  */
template<typename T> inline void muscleCopyOut(void * dest, const T & source)
{
#ifdef MUSCLE_CPU_REQUIRES_DATA_ALIGNMENT
   memcpy(dest, &source, sizeof(source));
#else
   *((T*)dest) = source;
#endif
}

/** This macro should be used instead of "newnothrow T[count]".  It works the 
  * same, except that it hacks around an ugly bug in gcc 3.x where newnothrow 
  * would return ((T*)0x4) on memory failure instead of NULL.
  * See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=10300
  */
#if __GNUC__ == 3
template <typename T> inline T * broken_gcc_newnothrow_array(size_t count)
{
   T * ret = newnothrow T[count];
   return (ret <= (T *)(sizeof(void *))) ? NULL : ret;
}
# define newnothrow_array(T, count) broken_gcc_newnothrow_array<T>(count)
#else
# define newnothrow_array(T, count) newnothrow T[count]
#endif

/** Returns the smallest of the two arguments */
template<typename T> inline const T & muscleMin(const T & p1, const T & p2) {return (p1 < p2) ? p1 : p2;}

/** Returns the smallest of the three arguments */
template<typename T> inline const T & muscleMin(const T & p1, const T & p2, const T & p3) {return muscleMin(p3, muscleMin(p1, p2));}

/** Returns the smallest of the four arguments */
template<typename T> inline const T & muscleMin(const T & p1, const T & p2, const T & p3, const T & p4) {return muscleMin(p3, p4, muscleMin(p1, p2));}

/** Returns the smallest of the five arguments */
template<typename T> inline const T & muscleMin(const T & p1, const T & p2, const T & p3, const T & p4, const T & p5) {return muscleMin(p3, p4, p5, muscleMin(p1, p2));}

/** Returns the largest of the two arguments */
template<typename T> inline const T & muscleMax(const T & p1, const T & p2) {return (p1 < p2) ? p2 : p1;}

/** Returns the largest of the three arguments */
template<typename T> inline const T & muscleMax(const T & p1, const T & p2, const T & p3) {return muscleMax(p3, muscleMax(p1, p2));}

/** Returns the largest of the four arguments */
template<typename T> inline const T & muscleMax(const T & p1, const T & p2, const T & p3, const T & p4) {return muscleMax(p3, p4, muscleMax(p1, p2));}

/** Returns the largest of the five arguments */
template<typename T> inline const T & muscleMax(const T & p1, const T & p2, const T & p3, const T & p4, const T & p5) {return muscleMax(p3, p4, p5, muscleMax(p1, p2));}

/** Swaps the two arguments */
template<typename T> inline void muscleSwap(T & p1, T & p2) {T t = p1; p1 = p2; p2 = t;}

/** Returns the value nearest to (v) that is still in the range [lo, hi]. */
template<typename T> inline const T & muscleClamp(const T & v, const T & lo, const T & hi) {return (v < lo) ? lo : ((v > hi) ? hi : v);}

/** Returns true iff (v) is in the range [lo,hi]. */
template<typename T> inline bool muscleInRange(const T & v, const T & lo, const T & hi) {return ((v >= lo)&&(v <= hi));}

/** Returns -1 if arg1 is larger, or 1 if arg2 is larger, or 0 if they are equal. */
template<typename T> inline int muscleCompare(const T & arg1, const T & arg2) {return (arg1>arg2) ? 1 : ((arg1<arg2) ? -1 : 0);}

/** Returns the absolute value of (arg) */
template<typename T> inline T muscleAbs(const T & arg) {return (arg<0)?(-arg):arg;}

/** Rounds the given float to the nearest integer value. */
inline int muscleRintf(float f) {return (f>=0.0f) ? ((int)(f+0.5f)) : -((int)((-f)+0.5f));}

/** Returns -1 if the value is less than zero, +1 if it is greater than zero, or 0 otherwise. */
template<typename T> inline int muscleSgn(const T & arg) {return (arg<0)?-1:((arg>0)?1:0);}

#endif  /* __cplusplus */

#ifndef __BEOS__

/*
 * Copyright(c) 1983,   1989
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 *      from nameser.h  8.1 (Berkeley) 6/2/93
 */

#ifndef BYTE_ORDER
  #if (BSD >= 199103)
    #include <machine/endian.h>
  #else
    #ifdef linux
      #include <endian.h>
    #else
      #define LITTLE_ENDIAN   1234    /* least-significant byte first (vax, pc) */
      #define BIG_ENDIAN      4321    /* most-significant byte first (IBM, net) */

      #if defined(vax) || defined(ns32000) || defined(sun386) || defined(i386) || \
              defined(__i386) || defined(__ia64) || \
              defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
              defined(__alpha__) || defined(__alpha) || defined(__CYGWIN__) || \
              defined(_M_IX86) || defined(__GNUWIN32__) || defined(__LITTLEENDIAN__) || \
              (defined(__Lynx__) && defined(__x86__))
        #define BYTE_ORDER      LITTLE_ENDIAN
      #endif

      #if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
          defined(__sparc) || \
          defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
          defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
          defined(apollo) || defined(__convex__) || defined(_CRAY) || \
          defined(__hppa) || defined(__hp9000) || \
          defined(__hp9000s300) || defined(__hp9000s700) || \
          defined(__hp3000s900) || defined(MPE) || \
          defined(BIT_ZERO_ON_LEFT) || defined(m68k) || \
              (defined(__Lynx__) && \
              (defined(__68k__) || defined(__sparc__) || defined(__powerpc__)))
        #define BYTE_ORDER      BIG_ENDIAN
      #endif
    #endif /* linux */
  #endif /* BSD */
#endif /* BYTE_ORDER */

#if !defined(BYTE_ORDER) || (BYTE_ORDER != BIG_ENDIAN && BYTE_ORDER != LITTLE_ENDIAN) 
        /*
         * you must determine what the correct bit order is for
         * your compiler - the next line is an intentional error
         * which will force your compiles to bomb until you fix
         * the above macros.
         */
#       error "Undefined or invalid BYTE_ORDER -- you will need to modify MuscleSupport.h to correct this";
#endif

/* End replacement code from Sun/University of California */

# if defined(MUSCLE_USE_POWERPC_INLINE_ASSEMBLY)
static inline uint16 MusclePowerPCSwapInt16(uint16 val)
{
   uint16 a;
   uint16 * addr = &a;
   __asm__ ("sthbrx %1,0,%2" : "=m" (*addr) : "r" (val), "r" (addr));
   return a;
}
static inline uint32 MusclePowerPCSwapInt32(uint32 val)
{
   uint32 a;
   uint32 * addr = &a;
   __asm__ ("stwbrx %1,0,%2" : "=m" (*addr) : "r" (val), "r" (addr));
   return a;
}
static inline float MusclePowerPCSwapFloat(float val)
{
   float a;
   float * addr = &a;
   __asm__ ("stwbrx %1,0,%2" : "=m" (*addr) : "r" (val), "r" (addr));
   return a;
}
static inline uint64 MusclePowerPCSwapInt64(uint64 val)
{
   return ((uint64)(MusclePowerPCSwapInt32((uint32)((val>>32)&0xFFFFFFFF))))|(((uint64)(MusclePowerPCSwapInt32((uint32)(val&0xFFFFFFFF))))<<32);
}
static inline double MusclePowerPCSwapDouble(double val)
{
   union {
      double _dv;
      uint64 _iv;
   } u;
   u._dv = val;
   u._iv = MusclePowerPCSwapInt64(u._iv);
   return u._dv;
}
#  define B_SWAP_DOUBLE(arg)   MusclePowerPCSwapDouble((double)(arg))
#  define B_SWAP_FLOAT(arg)    MusclePowerPCSwapFloat((float)(arg))
#  define B_SWAP_INT64(arg)    MusclePowerPCSwapInt64((uint64)(arg))
#  define B_SWAP_INT32(arg)    MusclePowerPCSwapInt32((uint32)(arg))
#  define B_SWAP_INT16(arg)    MusclePowerPCSwapInt16((uint16)(arg))
# elif defined(MUSCLE_USE_X86_INLINE_ASSEMBLY)
static inline uint16 MuscleX86SwapInt16(uint16 val)
{
#ifdef _MSC_VER
   __asm {
      mov ax, val;
      xchg al, ah;
      mov val, ax;
   };
#else
   __asm__ ("xchgb %b0,%h0" : "=q" (val) : "0" (val));
#endif
   return val;
}
static inline uint32 MuscleX86SwapInt32(uint32 val)
{
#ifdef _MSC_VER
   __asm {
      mov eax, val;
      bswap eax;
      mov val, eax;
   };
#else
   __asm__ ("bswap %0" : "+r" (val));
#endif
   return val;
}
static inline float MuscleX86SwapFloat(float val)
{
#ifdef _MSC_VER
   __asm {
      mov eax, val;
      bswap eax;
      mov val, eax;
   };
#else
   __asm__ ("bswap %0" : "+r" (val));
#endif
   return val;
}
static inline uint64 MuscleX86SwapInt64(uint64 val)
{
#ifdef _MSC_VER
   __asm {
      mov eax, DWORD PTR val;
      mov edx, DWORD PTR val + 4;
      bswap eax;
      bswap edx;
      mov DWORD PTR val, edx;
      mov DWORD PTR val + 4, eax;
   };
   return val;
#else
   return ((uint64)(MuscleX86SwapInt32((uint32)((val>>32)&0xFFFFFFFF))))|(((uint64)(MuscleX86SwapInt32((uint32)(val&0xFFFFFFFF))))<<32);
#endif
}
static inline double MuscleX86SwapDouble(double val)
{
#ifdef _MSC_VER
   __asm {
      mov eax, DWORD PTR val;
      mov edx, DWORD PTR val + 4;
      bswap eax;
      bswap edx;
      mov DWORD PTR val, edx;
      mov DWORD PTR val + 4, eax;
   };
   return val;
#else
   union {
      double _dv;
      uint64 _iv;
   } u;
   u._dv = val;
   u._iv = MuscleX86SwapInt64(u._iv);
   return u._dv;
#endif
}
#  define B_SWAP_DOUBLE(arg)   MuscleX86SwapDouble((double)(arg))
#  define B_SWAP_FLOAT(arg)    MuscleX86SwapFloat((float)(arg))
#  define B_SWAP_INT64(arg)    MuscleX86SwapInt64((uint64)(arg))
#  define B_SWAP_INT32(arg)    MuscleX86SwapInt32((uint32)(arg))
#  define B_SWAP_INT16(arg)    MuscleX86SwapInt16((uint16)(arg))
# else

// No assembly language available... so we'll use inline C

# if defined(__cplusplus)
#  define MUSCLE_INLINE inline
# else
#  define MUSCLE_INLINE static inline
# endif

MUSCLE_INLINE int64 B_SWAP_INT64(int64 arg)
{
   union {int64 _i64; uint8 _i8[8];} u1, u2;
   u1._i64   = arg; 
   u2._i8[0] = u1._i8[7];
   u2._i8[1] = u1._i8[6];
   u2._i8[2] = u1._i8[5];
   u2._i8[3] = u1._i8[4];
   u2._i8[4] = u1._i8[3];
   u2._i8[5] = u1._i8[2];
   u2._i8[6] = u1._i8[1];
   u2._i8[7] = u1._i8[0];
   return u2._i64;
}
MUSCLE_INLINE int32 B_SWAP_INT32(int32 arg)
{
   union {int32 _i32; uint8 _i8[4];} u1, u2;
   u1._i32   = arg; 
   u2._i8[0] = u1._i8[3];
   u2._i8[1] = u1._i8[2];
   u2._i8[2] = u1._i8[1];
   u2._i8[3] = u1._i8[0];
   return u2._i32;
}
MUSCLE_INLINE int16 B_SWAP_INT16(int16 arg) 
{
   union {int16 _i16; uint8 _i8[2];} u1, u2;
   u1._i16   = arg; 
   u2._i8[0] = u1._i8[1];
   u2._i8[1] = u1._i8[0];
   return u2._i16;
}
MUSCLE_INLINE double B_SWAP_DOUBLE(double arg)
{
   union {double _f64; uint8 _i8[8];} u1, u2;
   u1._f64   = arg; 
   u2._i8[0] = u1._i8[7];
   u2._i8[1] = u1._i8[6];
   u2._i8[2] = u1._i8[5];
   u2._i8[3] = u1._i8[4];
   u2._i8[4] = u1._i8[3];
   u2._i8[5] = u1._i8[2];
   u2._i8[6] = u1._i8[1];
   u2._i8[7] = u1._i8[0];
   return u2._f64;
}
MUSCLE_INLINE float B_SWAP_FLOAT(float arg)    
{
   union {float _f32; uint8 _i8[4];} u1, u2;
   u1._f32   = arg; 
   u2._i8[0] = u1._i8[3];
   u2._i8[1] = u1._i8[2];
   u2._i8[2] = u1._i8[1];
   u2._i8[3] = u1._i8[0];
   return u2._f32;
}
# endif
# if BYTE_ORDER == LITTLE_ENDIAN
#  define B_HOST_IS_LENDIAN 1
#  define B_HOST_IS_BENDIAN 0
#  define B_HOST_TO_LENDIAN_DOUBLE(arg) ((double)(arg))
#  define B_HOST_TO_LENDIAN_FLOAT(arg)  ((float)(arg))
#  define B_HOST_TO_LENDIAN_INT64(arg)  ((uint64)(arg))
#  define B_HOST_TO_LENDIAN_INT32(arg)  ((uint32)(arg))
#  define B_HOST_TO_LENDIAN_INT16(arg)  ((uint16)(arg))
#  define B_HOST_TO_BENDIAN_DOUBLE(arg) B_SWAP_DOUBLE(arg)
#  define B_HOST_TO_BENDIAN_FLOAT(arg)  B_SWAP_FLOAT(arg)
#  define B_HOST_TO_BENDIAN_INT64(arg)  B_SWAP_INT64(arg)
#  define B_HOST_TO_BENDIAN_INT32(arg)  B_SWAP_INT32(arg)
#  define B_HOST_TO_BENDIAN_INT16(arg)  B_SWAP_INT16(arg)
#  define B_LENDIAN_TO_HOST_DOUBLE(arg) ((double)(arg))
#  define B_LENDIAN_TO_HOST_FLOAT(arg)  ((float)(arg))
#  define B_LENDIAN_TO_HOST_INT64(arg)  ((uint64)(arg))
#  define B_LENDIAN_TO_HOST_INT32(arg)  ((uint32)(arg))
#  define B_LENDIAN_TO_HOST_INT16(arg)  ((uint16)(arg))
#  define B_BENDIAN_TO_HOST_DOUBLE(arg) B_SWAP_DOUBLE(arg)
#  define B_BENDIAN_TO_HOST_FLOAT(arg)  B_SWAP_FLOAT(arg)
#  define B_BENDIAN_TO_HOST_INT64(arg)  B_SWAP_INT64(arg)
#  define B_BENDIAN_TO_HOST_INT32(arg)  B_SWAP_INT32(arg)
#  define B_BENDIAN_TO_HOST_INT16(arg)  B_SWAP_INT16(arg)
# else /* LITTLE_ENDIAN */
#  define B_HOST_IS_LENDIAN 0
#  define B_HOST_IS_BENDIAN 1
#  define B_HOST_TO_LENDIAN_DOUBLE(arg) B_SWAP_DOUBLE(arg)
#  define B_HOST_TO_LENDIAN_FLOAT(arg)  B_SWAP_FLOAT(arg)
#  define B_HOST_TO_LENDIAN_INT64(arg)  B_SWAP_INT64(arg)
#  define B_HOST_TO_LENDIAN_INT32(arg)  B_SWAP_INT32(arg)
#  define B_HOST_TO_LENDIAN_INT16(arg)  B_SWAP_INT16(arg)
#  define B_HOST_TO_BENDIAN_DOUBLE(arg) ((double)(arg))
#  define B_HOST_TO_BENDIAN_FLOAT(arg)  ((float)(arg))
#  define B_HOST_TO_BENDIAN_INT64(arg)  ((uint64)(arg))
#  define B_HOST_TO_BENDIAN_INT32(arg)  ((uint32)(arg))
#  define B_HOST_TO_BENDIAN_INT16(arg)  ((uint16)(arg))
#  define B_LENDIAN_TO_HOST_DOUBLE(arg) B_SWAP_DOUBLE(arg)
#  define B_LENDIAN_TO_HOST_FLOAT(arg)  B_SWAP_FLOAT(arg)
#  define B_LENDIAN_TO_HOST_INT64(arg)  B_SWAP_INT64(arg)
#  define B_LENDIAN_TO_HOST_INT32(arg)  B_SWAP_INT32(arg)
#  define B_LENDIAN_TO_HOST_INT16(arg)  B_SWAP_INT16(arg)
#  define B_BENDIAN_TO_HOST_DOUBLE(arg) ((double)(arg))
#  define B_BENDIAN_TO_HOST_FLOAT(arg)  ((float)(arg))
#  define B_BENDIAN_TO_HOST_INT64(arg)  ((uint64)(arg))
#  define B_BENDIAN_TO_HOST_INT32(arg)  ((uint32)(arg))
#  define B_BENDIAN_TO_HOST_INT16(arg)  ((uint16)(arg))
# endif /* !LITTLE_ENDIAN */
#endif /* !__BEOS__ */

/* Macro to turn a type code into a string representation.
 * (typecode) is the type code to get the string for
 * (buf) is a (char *) to hold the output string; it must be >= 5 bytes long.
 */
#define MakePrettyTypeCodeString(typecode, buf)                     \
   {                                                                \
      uint32 __bigEndian = B_HOST_TO_BENDIAN_INT32(typecode);       \
      memcpy(buf, (const char *)&__bigEndian, sizeof(__bigEndian)); \
      buf[sizeof(__bigEndian)] = '\0';                              \
   }

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>     /* for errno */

#ifdef WIN32
# include <windows.h>
# include <winsock.h>  /* for WSAGetLastError() */
#endif

#ifdef __cplusplus
# include "syslog/SysLog.h"  /* for LogTime() */
#endif  /* __cplusplus */

/** Checks errno and returns true iff the last I/O operation
  * failed because it would have had to block otherwise.
  * NOTE:  Returns int so that it will compile even in C environments where no bool type is defined.
  */
static inline int PreviousOperationWouldBlock()
{
#ifdef WIN32
   return (WSAGetLastError() == WSAEWOULDBLOCK);
#else
   return (errno == EWOULDBLOCK);
#endif
}

/** Checks errno and returns true iff the last I/O operation
  * failed because it was interrupted by a signal or etc.
  * NOTE:  Returns int so that it will compile even in C environments where no bool type is defined.
  */
static inline int PreviousOperationWasInterrupted()
{
#ifdef WIN32
   return (WSAGetLastError() == WSAEINTR);
#else
   return (errno == EINTR);
#endif
}

/** This function applies semi-standard logic to convert the return value
  * of a system I/O call and (errno) into a proper MUSCLE-standard return value.
  * (A MUSCLE-standard return value's semantics are:  Negative on error,
  * otherwise the return value is the number of bytes that were transferred)
  * @param origRet The return value of the original system call (e.g. to read()/write()/send()/recv())
  * @param maxSize The maximum number of bytes that the system call was permitted to send during that call.
  * @param blocking True iff the socket/file descriptor is in blocking I/O mode.  (Type is int for C compatibility -- it's really a boolean parameter)
  * @returns The system call's return value equivalent in MUSCLE return value semantics.
  */
static inline int32 ConvertReturnValueToMuscleSemantics(int origRet, uint32 maxSize, int blocking)
{
   int32 retForBlocking = ((origRet > 0)||(maxSize == 0)) ? origRet : -1;
   return blocking ? retForBlocking : ((origRet<0)&&((PreviousOperationWouldBlock())||(PreviousOperationWasInterrupted()))) ? 0 : retForBlocking;
}

BEGIN_NAMESPACE(muscle);

#if MUSCLE_TRACE_CHECKPOINTS > 0

/** Exposed as an implementation detail.  Please ignore! */
extern volatile uint32 * _muscleTraceValues;

/** Exposed as an implementation detail.  Please ignore! */
extern uint32 _muscleNextTraceValueIndex;

/** Sets the location of the trace-checkpoints array to store trace checkpoints into.
  * @param location A pointer to an array of at least (MUSCLE_TRACE_CHECKPOINTS) uint32s, or NULL.
  *                 If NULL (or if this function is never called), the default array will be used.
  */
void SetTraceValuesLocation(volatile uint32 * location);

/** Set this process's current trace value to (v).  This can be used as a primitive debugging tool, to determine
  * where this process was last seen executing -- useful for determining where the process is spinning at.
  * @note this function is a no-op if MUSCLE_TRACE_CHECKPOINTS is not defined to a value greater than zero.
  */
static inline void StoreTraceValue(uint32 v) 
{
   _muscleTraceValues[_muscleNextTraceValueIndex] = v;  /* store the current value */
   _muscleNextTraceValueIndex                     = (_muscleNextTraceValueIndex+1)%MUSCLE_TRACE_CHECKPOINTS; /* move the pointer */
   _muscleTraceValues[_muscleNextTraceValueIndex] = ((uint32)-1);  /* mark the next position with a special tag to show that it's next */
}

/** Returns a pointer to the first value in the trace-values array. */
static inline const volatile uint32 * GetTraceValues() {return _muscleTraceValues;}

/** A macro for automatically setting a trace checkpoint value based on current code location. 
  * The value will be the two characters of the function or file name, left-shifted by 16 bits, 
  * and then OR'd together with the current line number.  This should give the debugging person a 
  * fairly good clue as to where the checkpoint was located, while still being very cheap to implement.
  *
  * @note This function will be a no-op unless MUSCLE_TRACE_CHECKPOINTS is defined to be greater than zero.
  */
#if defined(__GNUC__)
#define TCHECKPOINT                                   \
{                                                     \
   const char * d = __FUNCTION__;                     \
   StoreTraceValue((d[0]<<24)|(d[1]<<16)|(__LINE__)); \
}
#else
#define TCHECKPOINT                                   \
{                                                     \
   const char * d = __FILE__;                         \
   StoreTraceValue((d[0]<<24)|(d[1]<<16)|(__LINE__)); \
}
#endif

#else
/* no-op implementations for when we aren't using the trace facility */
static inline void SetTraceValuesLocation(volatile uint32 * location) {(void) location;}  /* named param is necessary for C compatibility */
static inline void StoreTraceValue(uint32 v) {(void) v;}  /* named param is necessary for C compatibility */
#define TCHECKPOINT {/* empty */}
#endif

END_NAMESPACE(muscle);

#endif /* _MUSCLE_SUPPORT_H */
