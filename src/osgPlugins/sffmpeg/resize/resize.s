#import <TargetConditionals.h>

#if TARGET_IPHONE_SIMULATOR
#else
#ifdef __LP64__
#include "resize_armv8.s"
#else
#include "resize_armv7.s"
#endif
#endif
