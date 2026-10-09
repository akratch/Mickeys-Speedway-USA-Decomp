/*
 * Resident formatting and debug-text system -- ROM 0x43470-0x45760.
 *
 * PROVENANCE: The TU correspondence and names are adapted from Jet Force
 * Gemini's public src/diprint.c and its built diprint.c.o. Diddy Kong Racing's
 * public built unused_string.c.o/printf.c.o independently identify strcpy,
 * memset and sprintf byte-for-byte, and its public src/printf.c supplies the
 * debug_text_width name for the one routine absent from JFG. Mickey's bytes,
 * call graph and strings decide the mapping. Adapted bodies carry a
 * point-of-use PROVENANCE note.
 */

#include "PR/ultratypes.h"
#include "libc/stdarg.h"
#include "libc/string.h"
#include "n_audio/mbi.h"

typedef struct DebugFontCoords {
    u8 u;
    u8 v;
} DebugFontCoords;

s32 vsprintf(char *s, const char *format, va_list arg);
void *texLoadTexture(s16 resourceId);
void debug_text_background(Gfx **dList, u32 ulx, u32 uly, u32 lrx, u32 lry);
s32 debug_text_character(Gfx **dList, s32 asciiVal);
void debug_text_newline(void);

extern char D_800D4150[];
extern Gfx D_8007CF58[];
/* PROVENANCE: formatter constants adapted from JFG src/diprint.c. */
const char D_80082A80[] = "0123456789abcdefghijklmnopqrstuvwxyz";
const char D_80082AA8[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char D_80082AD0[] = "";
const char D_80082AD4[] = "(null)";
const char D_80082ADC[] =
    "(nil)\0\0\0*** diPrintf Error *** ---> Out of string space. (Print less text!)\n";
extern DebugFontCoords D_8007CE98[3][32];
extern char *D_8007CE94;
extern u16 D_800D4A5C;
extern u16 D_800D4A5E;
extern s32 D_8007CE90;
extern void *D_800D4A50;
extern void *D_800D4A54;
extern void *D_800D4A58;
u16 D_800D4A60;
u16 D_800D4A62;
s32 D_800D4A64;
s32 D_800D4A68;
s32 D_800D4A6C;
s32 D_800D4A70;
s32 D_800D4A74;
s32 D_800D4A78;
s32 D_800D4A7C;
u16 D_800D4A80;
u16 D_800D4A82;

void rcpInitDp(Gfx **dList);
void viGetCurrentSize(s32 *width, s32 *height);
void debug_text_bounds(void);
void debug_text_origin(void);
s32 debug_text_parse(Gfx **dList, char *buffer);

/* PROVENANCE: body adapted from DKR src/unused_string.c:strcpy. */
char *strcpy(char *src, const char *dest) {
    char *ret = src;

    while ((*src++ = *dest++) != '\0') {}
    return ret;
}
/* PROVENANCE: body adapted from DKR src/unused_string.c:memset. */
void *memset(void *s, int c, size_t n) {
    unsigned char *ret = s;

    while (n-- > 0) {
        *ret++ = c;
    }
    return s;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:_itoa. */
char *_itoa(unsigned long long n, char *buflim, unsigned int base, int upperCase) {
    const char *alphabet = upperCase ? D_80082AA8 : D_80082A80;
    register char *bp = buflim;

    while (n > 0) {
        *(--bp) = alphabet[n % base];
        n /= base;
    }

    return bp;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:sprintfSetSpacingCodes. */
void sprintfSetSpacingCodes(s32 setting) {
    D_8007CE90 = setting;
}
/* PROVENANCE: body adapted from DKR src/printf.c:sprintf. */
s32 sprintf(char *s, const char *format, ...) {
    va_list arg;
    s32 done;

    va_start(arg, format);
    done = vsprintf(s, format, arg);
    va_end(arg);

    return done;
}
#define outchar(x)  \
    do {            \
        done++;     \
        (*s++) = x; \
    } while (0)

#define PAD(x)            \
    while (width-- > 0) { \
        outchar(x);       \
    }

#define HAVE_LONGLONG 1

#define PTR void *

/* Cast the next arg, of type ARGTYPE, into CASTTYPE, and put it in VAR.  */
#define castarg(var, argtype, casttype) var = (casttype) va_arg(args, argtype)

/* Get the next arg, of type TYPE, and put it in VAR.  */
#define nextarg(var, type) castarg(var, type, type)

#define BUFSIZ 100

#define isdigit(c) ((c >= '0') && (c <= '9'))

/* PROVENANCE: body adapted from JFG src/diprint.c:vsprintf. */
/* Workbench verdict=relocation-layout-mismatch; all 1220 instruction words and allocator lanes are exact, first real mismatch +0x130.
 * Flag lattice and exponent-sign line-assignment levers were tried; the C body links with 28 different formatter-table/data relocation identities.
 * Remains: preserve the asm split for those local table/data bindings; no instruction-code hypothesis remains. */
s32 vsprintf(char *s, const char *fmt, va_list args) {
    /* Pointer into the format string.  */
    const char *f;

    /* The string describing the size of groups of digits.  */
    const char *grouping;

    /* Number of characters written.  */
    int done = 0;

    f = fmt;
    while (*f != '\0') {
        /* Type modifiers.  */
        char is_short, is_long, is_long_double;

#ifdef HAVE_LONGLONG
        /* We use the `L' modifier for `long long int'.  */
#define is_longlong is_long_double
#else
#define is_longlong 0
#endif

        /* Format spec modifiers.  */
        char space, showsign, left, alt;

        /* Padding character: ' ' or '0'.  */
        char pad;
        /* Width of a field.  */
        int width;

        /* Precision of a field.  */
        int prec;

        /* Decimal integer is negative.  */
        char is_neg;

        /* Current character of the format.  */
        char fc;

        /* Base of a number to be written.  */
        int base;

        /* Integral values to be written.  */
        u64 num;
        s64 signed_num;

        /* String to be written.  */
        const char *str;

        char work[BUFSIZ];

        s32 a1;
        s32 i;
        s32 unused2;
        s32 v1;
        s32 a0;
        s32 digit;

        if (*f != '%') {
            /*   This isn't a format spec, so write everything out until the
                 next one.  To properly handle multibyte characters, we cannot
                 just search for a '%'.  Since multibyte characters are hairy
                 (and dealt with above), if we hit any byte above 127 (only
                 those can start a multibyte character) we just punt back to
                 that code.  */
            while (*f != '%' && *f != '\0') {
                outchar(*f++);
            }
            continue;
        }

        ++f;

        /* Check for "%%".  Note that although the ANSI standard lists
            '%' as a conversion specifier, it says "The complete format
            specification shall be `%%'," so we can avoid all the width
            and precision processing.  */
        if (*f == '%') {
            ++f;
            outchar('%');
            continue;
        }

        /* Check for spec modifiers.  */
        space = showsign = left = alt = 0;
        pad = ' ';
        while (*f == ' ' || *f == '+' || *f == '-' || *f == '#' || *f == '0') {
            switch (*f++) {
                case ' ':
                    /* Output a space in place of a sign, when there is no sign.  */
                    space = 1;
                    break;
                case '+':
                    /* Always output + or - for numbers.  */
                    showsign = 1;
                    break;
                case '-':
                    /* Left-justify things.  */
                    left = 1;
                    break;
                case '#':
                    /* Use the "alternate form":
                    Hex has 0x or 0X, FP always has a decimal point.  */
                    alt = 1;
                    break;
                case '0':
                    /* Pad with 0s.  */
                    pad = '0';
                    break;
            }
        }
        // end of while loop

        if (left) {
            pad = ' ';
        }

        /* Get the field width.  */
        width = 0;
        if (*f == '*') {
            /* The field width is given in an argument.
               A negative field width indicates left justification.  */
            nextarg(width, int);
            if (width < 0) {
                width = -width;
                left = 1;
            }
            ++f;
        } else {
            while (isdigit(*f)) {
                width *= 10;
                width += *f++ - '0';
            }
        }

        /* Get the precision.  */
        /* -1 means none given; 0 means explicit 0.  */
        prec = -1;
        if (*f == '.') {
            ++f;
            if (*f == '*') {
                /* The precision is given in an argument.  */
                nextarg(prec, int);
                /* Avoid idiocy.  */
                if (prec < 0) {
                    prec = -1;
                }
                ++f;
            } else if (isdigit(*f)) {
                prec = 0;
                while (isdigit(*f)) {
                    prec *= 10;
                    prec += *f++ - '0';
                }
            }
        }

        /* Check for type modifiers.  */
        is_short = is_long = is_long_double = 0;
        while (*f == 'h' || *f == 'l' || *f == 'L' || *f == 'Z' || *f == 'q') {
            switch (*f++) {
                case 'h':
                    /* int's are short int's.  */
                    is_short = 1;
                    break;
                case 'l':
#ifdef HAVE_LONGLONG
                    if (is_long) {
                        /* A double `l' is equivalent to an `L'.  */
                        is_longlong = 1;
                    } else {
#endif
                        /* int's are long int's.  */
                        is_long = 1;
                    }
                    break;
                case 'L':
                    /* double's are long double's, and int's are long long int's.  */
                    is_long_double = 1;
                    break;
                case 'Z':
                    /* int's are size_t's.  */
#ifdef HAVE_LONGLONG
                    // assert (sizeof(size_t) <= sizeof(unsigned long long int));
                    // is_longlong = sizeof(size_t) > sizeof(unsigned long int);
#endif
                    is_long = TRUE; // sizeof(size_t) > sizeof(unsigned int);
                    break;
                case 'q':
                    is_longlong = 1;
                    break;
            }
        }

        /* Format specification.  */
        fc = *f++;
        switch (fc) {
            case 'i':
            case 'd':
                /* Decimal integer.  */
                base = 10;
                if (is_longlong) {
                    nextarg(signed_num, s64);
                } else if (is_long) {
                    nextarg(signed_num, long int);
                } else if (!is_short) {
                    castarg(signed_num, int, long int);
                } else {
                    castarg(signed_num, int, short int);
                }

                is_neg = signed_num < 0;
                num = is_neg ? (-signed_num) : signed_num;
                goto number;
            case 'u':
                /* Decimal unsigned integer.  */
                base = 10;
                goto unsigned_number;
            case 'o':
                /* Octal unsigned integer.  */
                base = 8;
                goto unsigned_number;
            case 'X':
                /* Hexadecimal unsigned integer.  */
                base = 16;
                goto unsigned_number;
            case 'x':
                /* Hex with lower-case digits.  */
                base = 16;

            unsigned_number:
                /* Unsigned number of base BASE.  */
                if (is_longlong) {
                    castarg(num, s64, u64);
                } else if (is_long) {
                    castarg(num, long int, unsigned long int);
                } else if (!is_short) {
                    castarg(num, int, unsigned int);
                } else {
                    castarg(num, int, unsigned short int);
                }
                /* ANSI only specifies the `+' and
                   ` ' flags for signed conversions.  */
                is_neg = showsign = space = 0;

            number:
                /* Number of base BASE.  */
                {
                    char *w;
                    char *workend = &work[sizeof(work) - 1];

                    if (D_8007CE90) {
                        outchar(0x84);
                    }
                    if (prec >= 0) {
                        pad = ' ';
                    }
                    /* Supply a default precision if none was given.  */
                    if (prec == -1) {
                        prec = 1;
                    }

                    /* Put the number in WORK.  */
                    w = _itoa(num, workend + 1, base, fc == 'X') - 1;
                    width -= workend - w;
                    prec -= workend - w;

                    if (alt && base == 8 && prec <= 0) {
                        *w-- = '0';
                        width--;
                    }

                    if (prec > 0) {
                        width -= prec;
                        while (prec-- > 0) {
                            *w-- = '0';
                        }
                    }

                    if (alt && base == 16) {
                        width -= 2;
                    }

                    if (is_neg || showsign || space) {
                        width--;
                    }

                    if (!left && pad == ' ') {
                        PAD(' ');
                    }

                    if (is_neg) {
                        outchar('-');
                    } else if (showsign) {
                        outchar('+');
                    } else if (space) {
                        outchar(' ');
                    }

                    if (alt && base == 16) {
                        outchar('0');
                        outchar(fc);
                    }

                    if (!left && pad == '0') {
                        PAD('0');
                    }

                    /* Write the number.  */
                    while (++w <= workend) {
                        outchar(*w);
                    }

                    if (left) {
                        PAD(' ');
                    }
                }
                break;

            case 'e':
            case 'E': {
                s32 showDash;
                s32 unused2;
                f32 f16;
                f32 f02;
                f32 fpValue;
                f32 f0;
                s32 exponent;
                s32 unused;

                showDash = FALSE;
                if (D_8007CE90) {
                    outchar(0x84);
                }

                if (prec < 0) {
                    prec = 6;
                }

                if (is_short) {
                    f32 *ptr;
                    castarg(ptr, f32 *, f32 *);
                    fpValue = *ptr;
                } else {
                    f32 *ptr;
                    nextarg(ptr, f32 *);
                    fpValue = *ptr;
                }

                if (*((s8 *) &fpValue) < 0) {
                    showDash = TRUE;
                    fpValue = -fpValue;
                }

                if (fpValue == 0.0f) {
                    exponent = 0;
                    f16 = 1.0f;
                } else if (fpValue < 1.0f) {
                    exponent = 0;
                    f16 = 1.0f;
                    while (fpValue < f16) {
                        f16 /= 10.0f;
                        exponent--;
                    }
                }

                if (fpValue >= 1.0f) {
                    exponent = 0;
                    f16 = 1.0f;
                    f0 = 10.0f;
                    while (f0 <= fpValue) {
                        f16 = f0;
                        f0 *= 10.0f;
                        exponent++;
                    }
                }

                f02 = f16 * 0.5f;

                for (digit = prec; digit > 0; digit--) {
                    f02 /= 10.0f;
                }

                fpValue += f02;
                if (fpValue >= f16 * 10.0f) {
                    f16 = f16 * 10.0f;
                    exponent++;
                }

                a1 = (showDash || showsign || space) + prec + (prec > 0 || alt) + (exponent >= 100) + 5;

                if (!left && pad == ' ') {
                    while (width-- > a1) {
                        outchar(pad);
                    }
                }

                if (showDash) {
                    outchar('-');
                } else if (showsign) {
                    outchar('+');
                } else if (space) {
                    outchar(' ');
                }

                if (!left && pad == '0') {
                    while (width-- > a1) {
                        outchar(pad);
                    }
                }

                digit = '0';
                while (fpValue >= f16) {
                    fpValue -= f16;
                    digit++;
                }
                outchar(digit);
                f16 /= 10.0f;

                if (prec > 0 || alt) {
                    outchar('.');
                }

                while (prec > 0) {
                    digit = '0';
                    while (fpValue >= f16) {
                        fpValue -= f16;
                        digit++;
                    }
                    outchar(digit);
                    f16 /= 10.0f;
                    prec--;
                }

                outchar(fc);

                if (exponent < 0) { exponent = -exponent; outchar('-'); } else { outchar('+'); }

                if (exponent >= 100) {
                    outchar('0' + (exponent / 100));
                }

                outchar('0' + ((exponent / 10) % 10));
                outchar('0' + (exponent % 10));

                if (left) {
                    while (width-- > a1) {
                        outchar(' ');
                    }
                }
                break;
            }
            case 'G':
            case 'g':
                break;
            case 'f': {
                f32 f12 = 1.0f;
                f32 f14;
                f32 f2;
                s32 length;
                s32 showDash;
                f32 *ptr;
                s32 i;
                f32 fpValue;

                showDash = FALSE;

                if (D_8007CE90) {
                    outchar(0x84);
                }
                if (prec < 0) {
                    prec = 6;
                }

                for (digit = 0; digit < prec; digit++) {
                    f12 /= 10.0f;
                }

                if (is_short) {
                    castarg(ptr, f32 *, f32 *);
                    fpValue = *ptr;
                } else {
                    nextarg(ptr, f32 *);
                    fpValue = *ptr;
                }

                if (fpValue < 0.0f) {
                    showDash = TRUE;
                    fpValue = -fpValue;
                }

                fpValue += f12 * 0.5f;

                digit = 1;
                f2 = 1.0f;
                f14 = 10.0f;
                while (fpValue >= f14) {
                    f2 = f14;
                    f14 *= 10.0f;
                    digit++;
                }

                length = (showDash || showsign || space) + (prec > 0 || alt) + digit + prec;
                if (!left && pad == ' ') {
                    while (width-- > length) {
                        outchar(pad);
                    }
                }

                if (showDash) {
                    outchar('-');
                } else if (showsign) {
                    outchar('+');
                } else if (space) {
                    outchar(' ');
                }

                if (!left && pad == '0') {
                    while (width-- > length) {
                        outchar(pad);
                    }
                }

                do {
                    digit = '0';
                    while (fpValue >= f2) {
                        fpValue -= f2;
                        digit++;
                    }
                    f2 /= 10.0f;
                    outchar(digit);
                } while (f2 >= 1.0f);

                if (prec > 0 || alt) {
                    outchar('.');
                }

                while (prec > 0) {
                    digit = '0';
                    while (fpValue >= f2) {
                        fpValue -= f2;
                        digit++;
                    }
                    outchar(digit);
                    f2 /= 10.0f;
                    prec--;
                }

                if (left) {
                    while (width-- > length) {
                        outchar(' ');
                    }
                }
                break;
            }
            case 'c':
                /* Character.  */
                nextarg(num, int);
                if (!left) {
                    while (--width > 0) {
                        outchar(pad);
                    }
                }
                outchar((unsigned char) num);
                if (left) {
                    while (--width > 0) {
                        outchar(' ');
                    }
                }
                break;

            case 's': {
                s32 len;

                nextarg(str, char *);
                if (str == NULL) {
                    /* Write "(null)" if there's space.  */
                    if (prec == -1 || prec >= 6) {
                        str = D_80082AD4;
                        len = 6;
                    } else {
                        str = D_80082AD0;
                        len = 0;
                    }
                } else {
                    len = strlen(str);
                }

                if (prec != -1 && prec < len) {
                    len = prec;
                }
                width -= len;

                if (!left) {
                    PAD(' ');
                }
                while (len-- > 0) {
                    outchar(*str++);
                }

                if (left) {
                    PAD(' ');
                }
            } break;

            case 'p':
                /* Generic pointer.  */
                {
                    PTR ptr;
                    nextarg(ptr, PTR);
                    if (ptr != NULL) {
                        /* If the pointer is not NULL, write it as a %#x spec.  */
                        base = 16;
                        fc = 'x';
                        alt = 1;
                        num = (unsigned long int) ptr;
                        is_neg = 0;
                        goto number;
                    } else {
                        /* Write "(nil)" for a nil pointer.  */
                        register const char *p;

                        width -= 5;
                        if (!left) {
                            PAD(' ');
                        }
                        grouping = D_80082ADC;
                        while (*grouping != '\0') {
                            outchar(*grouping++);
                        }
                        if (left) {
                            PAD(' ');
                        }
                    }
                }
                break;

            case 'n':
                /* Answer the count of characters written.  */
                if (is_longlong) {
                    s64 *p;
                    nextarg(p, s64 *);
                    *p = done;
                } else if (is_long) {
                    long int *p;
                    nextarg(p, long int *);
                    *p = done;
                } else if (!is_short) {
                    int *p;
                    nextarg(p, int *);
                    *p = done;
                } else {
                    short int *p;
                    nextarg(p, short int *);
                    *p = done;
                }
                break;
            default:
                /* Unrecognized format specifier.  */
                break;
        }
        if (D_8007CE90) {
            outchar(0x83);
        }
    }
    *s = '\0';
    return done;
}

/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintfInit. */
void diPrintfInit(void) {
    D_800D4A50 = texLoadTexture(0);
    D_800D4A54 = texLoadTexture(1);
    D_800D4A58 = texLoadTexture(2);
    D_8007CE94 = D_800D4150;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintf. */
s32 diPrintf(const char *format, ...) {
    va_list args;
    s32 written;

    va_start(args, format);
    if ((D_8007CE94 - D_800D4150) > 0x800) {
        return -1;
    }
    sprintfSetSpacingCodes(1);
    written = vsprintf(D_8007CE94, format, args);
    sprintfSetSpacingCodes(0);
    if (written > 0) {
        D_8007CE94 = &D_8007CE94[written] + 1;
    }
    return 0;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintfAll. */
void diPrintfAll(Gfx **dList) {
    s32 width;
    s32 height;
    char *buffer;

    rcpInitDp(dList);
    viGetCurrentSize(&width, &height);
    D_800D4A80 = width;
    D_800D4A82 = height;
    gDPSetScissor((*dList)++, 0, 0, 0, D_800D4A80, D_800D4A82);
    debug_text_bounds();
    gSPDisplayList((*dList)++, D_8007CF58);
    buffer = D_800D4150;
    debug_text_origin();
    D_800D4A7C = -1;
    D_800D4A64 = 0;
    D_800D4A60 = D_800D4A5C;
    D_800D4A62 = D_800D4A5E;
    while (buffer != D_8007CE94) {
        D_800D4A68 = 0;
        buffer += debug_text_parse(dList, buffer);
    }
    debug_text_background(dList, D_800D4A60, D_800D4A62, D_800D4A5C, D_800D4A5E + 10);
    buffer = D_800D4150;
    debug_text_origin();
    D_800D4A7C = -1;
    D_800D4A64 = 0;
    while (buffer != D_8007CE94) {
        D_800D4A68 = 1;
        buffer += debug_text_parse(dList, buffer);
    }
    D_8007CE94 = D_800D4150;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintfSetCol. */
void diPrintfSetCol(u8 red, u8 green, u8 blue, u8 alpha) {
    *D_8007CE94 = 0x81;
    D_8007CE94++;
    *D_8007CE94 = red;
    D_8007CE94++;
    *D_8007CE94 = green;
    D_8007CE94++;
    *D_8007CE94 = blue;
    D_8007CE94++;
    *D_8007CE94 = alpha;
    D_8007CE94++;
    *D_8007CE94 = 0;
    D_8007CE94++;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintfSetBG. */
void diPrintfSetBG(u8 red, u8 green, u8 blue, u8 alpha) {
    *D_8007CE94 = 0x85;
    D_8007CE94++;
    *D_8007CE94 = red;
    D_8007CE94++;
    *D_8007CE94 = green;
    D_8007CE94++;
    *D_8007CE94 = blue;
    D_8007CE94++;
    *D_8007CE94 = alpha;
    D_8007CE94++;
    *D_8007CE94 = 0;
    D_8007CE94++;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintfSetXY. */
void diPrintfSetXY(u16 x, u16 y) {
    u16 tempX;
    u16 tempY;

    *D_8007CE94 = 0x82;
    D_8007CE94++;
    *D_8007CE94 = x & 0xFF;
    D_8007CE94++;
    tempX = x >> 8;
    *D_8007CE94 = tempX;
    D_8007CE94++;
    *D_8007CE94 = y & 0xFF;
    D_8007CE94++;
    tempY = y >> 8;
    *D_8007CE94 = tempY;
    D_8007CE94++;
    *D_8007CE94 = 0;
    D_8007CE94++;
}
/* PROVENANCE: body adapted from DKR src/printf.c:debug_text_width. */
/* Matched 2026-09-12, lane p9-tight. Four edits, each measured load-bearing:
 *
 * 1. The `if (1)` region [L97]. The two-web form -- raw byte in `charIndex`,
 *    copied into `pad` at the loop top -- is an exact v0/v1 transposition of
 *    the target at 20 words. Opening one region anywhere in the function
 *    turns it the right way up (20 -> 5); the fact is the region's existence,
 *    not its placement.
 * 2. `newlineMark` plus the reversed newline test. cfe canonicalises
 *    `constant != variable` back to variable-first, so no spelling of a
 *    literal test moves the operand order. With the constant in an `s32`
 *    variable the source order survives and the target's `beq mark, current`
 *    is emitted.
 * 3. `s[256]` pays for `newlineMark`. The extra scalar takes a frame home, and
 *    the buffer length is unobservable, so [L112] solves it: 253-256 all
 *    restore the 0x138 frame and 257-260 do not.
 * 4. The redundant `charIndex = pad;` inside the newline arm. It is the whole
 *    of the last four words and it compiles to nothing. uopt copy-propagates
 *    `pad` back to `charIndex` at every use up to but NOT including the
 *    statement that redefines `charIndex`, which is why the four range tests
 *    read the raw byte where the target reads the copy. Redefining
 *    `charIndex` before them breaks the propagation; the self-copy is then
 *    eliminated, so the instruction count is unchanged. Do not delete it.
 */
s32 debug_text_width(const char *format, ...) {
    s32 stringLength;
    s32 fontTexture;
    s32 charIndex;
    s32 newlineMark;
    s32 pad;
    char s[256];
    u8 *ch;
    va_list args;

    va_start(args, format);
    sprintfSetSpacingCodes(1);
    vsprintf(s, format, args);
    sprintfSetSpacingCodes(0);
    newlineMark = '\n';
    charIndex = (u8)s[0];
    stringLength = 0;
    ch = (u8 *)&s[1];
    if (1) {
        if (charIndex != '\0') {
            do {
                pad = charIndex;
                if (newlineMark != charIndex) {
                    charIndex = pad;
                    if (charIndex == ' ') {
                        stringLength += 6;
                    } else if (pad >= 0x21 && pad < 0x80) {
                        fontTexture = 0;
                        if (pad < 0x40) {
                            charIndex = (pad - 0x21) & 0xFF;
                        } else {
                            fontTexture = 2;
                            if (pad < 0x60) {
                                fontTexture = 1;
                                charIndex = (pad - 0x40) & 0xFF;
                            } else {
                                charIndex = (pad - 0x60) & 0xFF;
                            }
                        }
                        stringLength = ((stringLength + D_8007CE98[fontTexture][charIndex].v) -
                                        D_8007CE98[fontTexture][charIndex].u) + 1;
                    }
                }
                charIndex = *ch;
                ch++;
            } while (charIndex != '\0');
        }
    }
    va_end(args);
    return stringLength;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:debug_text_parse. */
/* Workbench: relocation-symbol-mismatch; words-identical, four reloc sites, first +0x44.
 * Levers tried: paired-struct and bounded address-alias spellings; codegen regressed or retained separate D_800D4A62.
 * Remains: linkable D_800D4A60+2 and jtbl_80082CD8 ownership; wrapper retained. */
s32 debug_text_parse(Gfx **dList, char *buffer) {
    char *bufferCopy;
    s32 xOffset;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    u8 bufferValue;

    bufferCopy = buffer;
    bufferValue = *buffer;
    buffer++;
    while (bufferValue != 0) {
        xOffset = 0;
        switch (bufferValue) {
            case 0x83: // Leave fixed-width mode
                D_800D4A64 = FALSE;
                break;
            case 0x84: // Enter fixed-width mode
                D_800D4A64 = TRUE;
                break;
            case 0x81: // Set the text color from the next 4 bytes
                red = buffer[0];
                green = buffer[1];
                blue = buffer[2];
                alpha = buffer[3];
                buffer += 4;
                if (D_800D4A68) {
                    gDPSetEnvColor((*dList)++, red, green, blue, alpha);
                }
                break;
            case 0x85: // Set the background color from the next 4 bytes
                red = buffer[0];
                green = buffer[1];
                blue = buffer[2];
                alpha = buffer[3];
                buffer += 4;
                if (!D_800D4A68) {
                    gDPSetPrimColor((*dList)++, 0, 0, red, green, blue, alpha);
                }
                break;
            case 0x82: // Set debug text position from the next 4 bytes
                if (!D_800D4A68) {
                    debug_text_background(dList, D_800D4A60, D_800D4A62, D_800D4A5C, D_800D4A5E + 10);
                }
                D_800D4A5C = buffer[0];
                D_800D4A5C |= buffer[1] << 8;
                D_800D4A5E = buffer[2];
                D_800D4A5E |= buffer[3] << 8;
                D_800D4A60 = D_800D4A5C;
                D_800D4A62 = D_800D4A5E;
                buffer += 4;
                break;
            case ' ': // Space
                xOffset = 6;
                break;
            case '\n': // Line Feed
                if (!D_800D4A68) {
                    debug_text_background(dList, D_800D4A60, D_800D4A62, D_800D4A5C, D_800D4A5E + 10);
                }
                debug_text_newline();
                D_800D4A60 = D_800D4A5C;
                D_800D4A62 = D_800D4A5E;
                break;
            case '\t': // HT - Horizontal Tab
                if (!(D_800D4A5C % 32)) {
                    xOffset = 32;
                } else {
                    xOffset = 32 - (D_800D4A5C % 32);
                }
                break;
            default:
                xOffset = debug_text_character(dList, bufferValue); break;
        }

        if (D_800D4A64 && bufferValue >= 0x20 && bufferValue < 0x80) {
            xOffset = 7;
        }
        D_800D4A5C += xOffset;
        if ((D_800D4A80 - 16) < D_800D4A5C) {
            if (!D_800D4A68) {
                debug_text_background(dList, D_800D4A60, D_800D4A62, D_800D4A5C, D_800D4A5E + 10);
            }
            debug_text_newline();
            D_800D4A60 = D_800D4A5C;
            D_800D4A62 = D_800D4A5E;
        }
        bufferValue = *buffer;
        buffer++;
    }

    return buffer - bufferCopy;
}

/* PROVENANCE: body adapted from JFG src/diprint.c:debug_text_background. */
void debug_text_background(Gfx **dList, u32 ulx, u32 uly, u32 lrx, u32 lry) {
    if (!((ulx == lrx) | (uly == lry))) {
        if (ulx >= 2) {
            ulx -= 2;
        }
        lrx += 2;
        gDPSetCombineMode((*dList)++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPFillRectangle((*dList)++, ulx, uly, lrx, lry);
    }
}
#define G_CC_DEBUG_TEXT 0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0
/*
 * PROVENANCE: body adapted from JFG src/diprint.c:debug_text_character;
 * Mickey's assembly selects the texture pointer and width before the shared
 * load block, unlike the donor's three duplicated load blocks.
 */
s32 debug_text_character(Gfx **dList, s32 asciiVal) {
    s32 fontCharWidth;
    s32 fontCharU;
    union {
        s32 value;
        s64 align;
    } fontTextureWidth;
    s32 fontTexture;
    s32 textureIndex;

    textureIndex = D_800D4A7C;
    if (asciiVal < 0x40) {
        textureIndex = 0;
        fontTexture = (s32)D_800D4A50 + 0x20;
        fontTextureWidth.value = 0xC0;
        asciiVal -= 0x21;
    } else if (asciiVal < 0x60) {
        textureIndex = 1;
        fontTexture = (s32)D_800D4A54 + 0x20;
        fontTextureWidth.value = 0xF8;
        asciiVal -= 0x40;
    } else if (asciiVal < 0x80) {
        textureIndex = 2;
        fontTexture = (s32)D_800D4A58 + 0x20;
        fontTextureWidth.value = 0xC0;
        asciiVal -= 0x60;
    }
    fontCharU = D_8007CE98[textureIndex][asciiVal].u;
    fontCharWidth = D_8007CE98[textureIndex][asciiVal].v - fontCharU + 1;
    if (D_800D4A68 != 0) {
        if (textureIndex != D_800D4A7C) {
            gDPLoadTextureBlockS((*dList)++, fontTexture + 0x80000000,
                                 G_IM_FMT_IA, G_IM_SIZ_8b, fontTextureWidth.value,
                                 11, 0, 2, 2, 0, 0, 0, 0);
            D_800D4A7C = textureIndex;
        }
        gDPSetCombineMode((*dList)++, G_CC_DEBUG_TEXT, G_CC_DEBUG_TEXT);
        gSPTextureRectangle((*dList)++, D_800D4A5C << 2, D_800D4A5E << 2,
                            (D_800D4A5C + fontCharWidth) << 2,
                            (D_800D4A5E + 10) << 2, 0, fontCharU << 5, 0,
                            1024, 1024);
    }
    return fontCharWidth;
}
#undef G_CC_DEBUG_TEXT
/* PROVENANCE: body adapted from JFG src/diprint.c:debug_text_bounds. */
void debug_text_bounds(void) {
    if (D_800D4A80 <= 320) {
        D_800D4A6C = 16;
        D_800D4A70 = D_800D4A80 - 16;
    } else {
        D_800D4A6C = 32;
        D_800D4A70 = D_800D4A80 - 32;
    }
    if (D_800D4A82 <= 240) {
        D_800D4A74 = 16;
        D_800D4A78 = D_800D4A82 - 16;
    } else {
        D_800D4A74 = 32;
        D_800D4A78 = D_800D4A82 - 32;
    }
}
/* PROVENANCE: body adapted from JFG src/diprint.c:debug_text_origin. */
void debug_text_origin(void) {
    D_800D4A5C = D_800D4A6C;
    D_800D4A5E = D_800D4A74;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:debug_text_newline. */
void debug_text_newline(void) {
    D_800D4A5C = D_800D4A6C;
    D_800D4A5E += 11;
}
