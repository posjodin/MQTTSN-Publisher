/*
 * Macros for writing sensor records 
 * A record is a unit of text that must be kept together 
 * in a report, ie to represent a senml element
 */

#define WARN printf
//#define WARN()

/*
 * Mark start of a record. 
 * Save current state (string pointer and length).
 */
#define STARTRECORD(STR, LEN) {                                         \
   __label__ _full, _notfull;                                           \
   char *_str = (STR);                                                  \
   size_t _len = (LEN), _n;                                             \

/*
 * Attempt to write print formatted text to a record
 */
#define PUTFMT(...) {                                                   \
      _n = snprintf(_str, _len, __VA_ARGS__);                           \
      if (_n < _len) {                                                  \
         _str += _n;                                                    \
         _len -= _n;                                                    \
      }                                                                 \
      else {                                                            \
         goto _full;                                                    \
      }                                                                 \
   }

/*
 * End of record -- commit if succeeded, else abort and revert to
 * saved state.
 */
#define ENDRECORD(STR, LEN, ORIGLEN)                                    \
   goto _notfull;                                                       \
  _full:                                                                \
    WARN("%s: %d: no space left\n", __FILE__, __LINE__);                \
    *(STR) = '\0';                                                      \
    return ((ORIGLEN)-(LEN));                                           \
  _notfull:                                                             \
    (STR) = _str;                                                       \
    (LEN) = _len;                                                       \
  }

/*
 * Iterators for keeping state between function calls. Intended for 
 * when reports need to be distributed over several messages.  
 */
#define ITERVAR(I) void *(I)

#define ITERSTART(I) {        \
      if ((I) !=  NULL)       \
         goto *(I);           \
     }   

#define ITERSTEP(I) {                     \
     __label__ _iterlabel;                \
     (I) = && _iterlabel;                 \
     _iterlabel: {}                       \
  }
     
#define ITERSTOP(I) (I) = NULL 

#define ITERMORE(I) ((I) != NULL)

#define ITERCALL(F, N) {                        \
   int _n;                                      \
   _n = (F);                                    \
   if (_n == 0)                                 \
      return (N);                               \
   else                                         \
      (N) += _n;                                \
   }

