#pragma once

#ifdef _MSC_VER

#ifdef BLACKJACK_EXPORTS
#define BLACKJACK_API __declspec(dllexport)
#else
#define BLACKJACK_API __declspec(dllimport)
#endif

#else
#define BLACKJACK_API
#endif

#define BLACKJACK_EXTERN_C extern "C"
