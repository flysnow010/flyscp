#ifndef CONFIG_H
#define CONFIG_H

#include <QtCore/qglobal.h>

#if defined(RSA_LIBRARY)
#  define RSA_EXPORT Q_DECL_EXPORT
#else
#  define RSA_EXPORT Q_DECL_IMPORT
#endif

#endif // CONFIG_H
