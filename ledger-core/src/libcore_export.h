
#ifndef LIBCORE_EXPORT_H
#define LIBCORE_EXPORT_H

#ifdef LEDGER_CORE_STATIC_DEFINE
#  define LIBCORE_EXPORT
#  define LEDGER_CORE_NO_EXPORT
#else
#  ifndef LIBCORE_EXPORT
#    ifdef ledger_core_EXPORTS
        /* We are building this library */
#      define LIBCORE_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define LIBCORE_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef LEDGER_CORE_NO_EXPORT
#    define LEDGER_CORE_NO_EXPORT 
#  endif
#endif

#ifndef LEDGER_CORE_DEPRECATED
#  define LEDGER_CORE_DEPRECATED __declspec(deprecated)
#endif

#ifndef LEDGER_CORE_DEPRECATED_EXPORT
#  define LEDGER_CORE_DEPRECATED_EXPORT LIBCORE_EXPORT LEDGER_CORE_DEPRECATED
#endif

#ifndef LEDGER_CORE_DEPRECATED_NO_EXPORT
#  define LEDGER_CORE_DEPRECATED_NO_EXPORT LEDGER_CORE_NO_EXPORT LEDGER_CORE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LEDGER_CORE_NO_DEPRECATED
#    define LEDGER_CORE_NO_DEPRECATED
#  endif
#endif

#endif /* LIBCORE_EXPORT_H */
