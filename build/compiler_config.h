// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Compilers configuration.

#ifndef WB_BUILD_COMPILER_CONFIG_H_
#define WB_BUILD_COMPILER_CONFIG_H_

#include "build/build_config.h"

// clang-format off
#if defined(__cplusplus)
#if __cplusplus > 201703L
// Has C++20 support.
#define WB_COMPILER_HAS_CXX20     1
// Has C++17 support.
#define WB_COMPILER_HAS_CXX17     1
// Has C++14 support.
#define WB_COMPILER_HAS_CXX14     1
#define WB_COMPILER_HAS_CXX11     1
#define WB_COMPILER_HAS_CXX98     1
#define WB_COMPILER_HAS_CXXPRE98  1
#elif __cplusplus > 201402L
// Has C++20 support.
#define WB_COMPILER_HAS_CXX20     0
// Has C++17 support.
#define WB_COMPILER_HAS_CXX17     1
// Has C++14 support.
#define WB_COMPILER_HAS_CXX14     1
#define WB_COMPILER_HAS_CXX11     1
#define WB_COMPILER_HAS_CXX98     1
#define WB_COMPILER_HAS_CXXPRE98  1
#elif __cplusplus > 201103L
// Has C++20 support.
#define WB_COMPILER_HAS_CXX20     0
// Has C++17 support.
#define WB_COMPILER_HAS_CXX17     0
// Has C++14 support.
#define WB_COMPILER_HAS_CXX14     1
#define WB_COMPILER_HAS_CXX11     1
#define WB_COMPILER_HAS_CXX98     1
#define WB_COMPILER_HAS_CXXPRE98  1
#elif __cplusplus > 199711L
// Has C++20 support.
#define WB_COMPILER_HAS_CXX20     0
// Has C++17 support.
#define WB_COMPILER_HAS_CXX17     0
// Has C++14 support.
#define WB_COMPILER_HAS_CXX14     0
#define WB_COMPILER_HAS_CXX11     1
#define WB_COMPILER_HAS_CXX98     1
#define WB_COMPILER_HAS_CXXPRE98  1
#elif __cplusplus > 1L
// Has C++20 support.
#define WB_COMPILER_HAS_CXX20     0
// Has C++17 support.
#define WB_COMPILER_HAS_CXX17     0
// Has C++14 support.
#define WB_COMPILER_HAS_CXX14     0
#define WB_COMPILER_HAS_CXX11     0
#define WB_COMPILER_HAS_CXX98     1
#define WB_COMPILER_HAS_CXXPRE98  1
#elif __cplusplus == 1L
// Has C++20 support.
#define WB_COMPILER_HAS_CXX20     0
// Has C++17 support.
#define WB_COMPILER_HAS_CXX17     0
// Has C++14 support.
#define WB_COMPILER_HAS_CXX14     0
#define WB_COMPILER_HAS_CXX11     0
#define WB_COMPILER_HAS_CXX98     0
#define WB_COMPILER_HAS_CXXPRE98  1
#else
// Has C++20 support.
#define WB_COMPILER_HAS_CXX20     0
// Has C++17 support.
#define WB_COMPILER_HAS_CXX17     0
// Has C++14 support.
#define WB_COMPILER_HAS_CXX14     0
#define WB_COMPILER_HAS_CXX11     0
#define WB_COMPILER_HAS_CXX98     0
#define WB_COMPILER_HAS_CXXPRE98  0
#endif
#else  // !__cplusplus
// Has C++20 support.
#define WB_COMPILER_HAS_CXX20     0
// Has C++17 support.
#define WB_COMPILER_HAS_CXX17     0
// Has C++14 support.
#define WB_COMPILER_HAS_CXX14     0
#define WB_COMPILER_HAS_CXX11     0
#define WB_COMPILER_HAS_CXX98     0
#define WB_COMPILER_HAS_CXXPRE98  0
#endif  // __cplusplus
// clang-format on

// We extended NDEBUG from asserts (as C++ standard says) to all debug code.
// Done for simplicity and as extensively used practise.
#ifdef NDEBUG
/*
 * @brief Compiling in NON-debug mode.
 */
#define WB_COMPILER_HAS_DEBUG 0
#else
/*
 * @brief Compiling in debug mode.
 */
#define WB_COMPILER_HAS_DEBUG 1
#endif

// Under Mac attributes detected as valid during sample compliation, but not
// in source tree :(
// TODO(dimhotepus): Check on new Clang all is ok (13.0.0+).
#if defined(WB_CPP_HAS_LIKELY_UNLIKELY_ATTRIBUTES) && !defined(WB_OS_MACOSX)
/*
 * @brief [[likely]]
 */
#define WB_ATTRIBUTE_LIKELY [[likely]]
/*
 * @brief [[unlikely]]
 */
#define WB_ATTRIBUTE_UNLIKELY [[unlikely]]
#else
/*
 * @brief Do nothing,
 */
#define WB_ATTRIBUTE_LIKELY
/*
 * @brief Do nothing,
 */
#define WB_ATTRIBUTE_UNLIKELY
#endif

// Common attributes.
#ifdef WB_COMPILER_MSVC

/*
 * @brief Exports functions, data, and objects from a DLL.
 *
 * "If a class is marked declspec(dllexport), any specializations of class
 * templates in the class hierarchy are implicitly marked as
 * declspec(dllexport).  This means that class templates are explicitly
 * instantiated and the class's members must be defined."
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/dllexport-dllimport
 */
#define WB_ATTRIBUTE_DLL_EXPORT __declspec(dllexport)

/*
 * @brief Imports functions, data, and objects from a DLL.
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/dllexport-dllimport
 */
#define WB_ATTRIBUTE_DLL_IMPORT __declspec(dllimport)

/*
 * @brief The WB_ATTRIBUTE_FORCEINLINE overrides the cost-benefit analysis and
 * relies on the judgment of the programmer instead.
 *
 * Please, follow suggestions from profiler before putting
 * WB_ATTRIBUTE_FORCEINLINE!
 *
 * Indiscriminate use of WB_ATTRIBUTE_FORCEINLINE can result in larger code or,
 * in some cases, even performance losses (because of the increased paging of a
 * larger executable, for example).
 */
#define WB_ATTRIBUTE_FORCEINLINE __forceinline

/*
 * @brief When applied to a function declaration or definition that returns a
 * pointer type, restrict tells the compiler that the function returns an object
 * that is not aliased, that is, referenced by any other pointers.  This allows
 * the compiler to perform additional optimizations.
 *
 * The compiler propagates __declspec(restrict).  For example, the CRT malloc
 * function has a __declspec(restrict) decoration, and therefore, the compiler
 * assumes that pointers initialized to memory locations by malloc are also not
 * aliased by previously existing pointers.

 * The compiler does not check that the returned pointer is not actually
 * aliased.  It is the developer's responsibility to ensure the program does not
 * alias a pointer marked with the restrict __declspec modifier.
 *
 * WB_ATTRIBUTE_MALLOC_LIKE pointer_return_type function();
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/restrict
 */
#define WB_ATTRIBUTE_MALLOC_LIKE(deallocator, ptr_index) __declspec(restrict)

/*
 * @brief Compiler generates code without prolog and epilog code.  You can use
 * this feature to write your own prolog/epilog code sequences using inline
 * assembler code:
 *
 * WB_ATTRIBUTE_NAKED int func( formal_parameters ) {}
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/naked-cpp
 */
#define WB_ATTRIBUTE_NAKED __declspec(naked)

/*
 * @brief Never inline a particular member function (function in a class).
 *
 * "It may be worthwhile to not inline a function if it is small and not
 * critical to the performance of your code.  That is, if the function is small
 * and not likely to be called often, such as a function that handles an error
 * condition.  Keep in mind that if a function is marked noinline, the calling
 * function will be smaller and thus, itself a candidate for compiler inlining."
 *
 * WB_ATTRIBUTE_NOINLINE int mbrfunc() { return 0; }  // will not inline
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/noinline
 */
#define WB_ATTRIBUTE_NOINLINE __declspec(noinline)

/**
 * Does nothing for now.
 */
#define WB_ATTRIBUTE_PURE

#elif defined(WB_COMPILER_GCC) || defined(WB_COMPILER_CLANG)

/*
 * @brief Exports functions, data, and objects from a shared library.
 */
#define WB_ATTRIBUTE_DLL_EXPORT __attribute__((visibility("default")))

/*
 * @brief Imports functions, data, and objects from a shared library.
 */
#define WB_ATTRIBUTE_DLL_IMPORT

/*
 * @brief Generally, functions are not inlined unless optimization is specified.
 * For functions declared inline, this attribute inlines the function
 * independent of any restrictions that otherwise apply to inlining.  Failure to
 * inline such a function is diagnosed as an error.  Note that if such a
 * function is called indirectly the compiler may or may not inline it depending
 * on optimization level and a failure to inline an indirect call may or may not
 * be diagnosed.
 */
#define WB_ATTRIBUTE_FORCEINLINE __attribute__((always_inline))

/*
 * @brief Attribute malloc indicates that a function is malloc-like, i.e., that
 * the pointer P returned by the function cannot alias any other pointer valid
 * when the function returns, and moreover no pointers to valid objects occur in
 * any storage addressed by P.  In addition, the GCC predicts that a function
 * with the attribute returns non-null in most cases.
 *
 * Associates deallocator as a suitable deallocation function for pointers
 * returned from the malloc-like function.  ptr-index denotes the positional
 * argument to which when the pointer is passed in calls to deallocator has the
 * effect of deallocating it.
 *
 * The analyzer assumes that deallocators can gracefully handle the nullptr
 * pointer.  If this is not the case, the deallocator can be marked with
 * __attribute__((nonnull)) so that -fanalyzer can emit a
 * -Wanalyzer-possible-null-argument diagnostic for code paths in which the
 * deallocator is called with nullptr.
 */
#define WB_ATTRIBUTE_MALLOC_LIKE(deallocator, ptr_index) \
  __attribute__((malloc, malloc(deallocator, ptr_index)))

/*
 * @brief This attribute allows the compiler to construct the requisite function
 * declaration, while allowing the body of the function to be assembly code. The
 * specified function will not have prologue/epilogue sequences generated by the
 * compiler.
 *
 * Only basic asm statements can safely be included in naked functions (see
 * Basic Asm).  While using extended asm or a mixture of basic asm and C code
 * may appear to work, they cannot be depended upon to work reliably and are not
 * supported.
 */
#define WB_ATTRIBUTE_NAKED __attribute__((naked))

/*
 * @brief This function attribute prevents a function from being considered for
 * inlining.  If the function does not have side effects, there are
 * optimizations other than inlining that cause function calls to be optimized
 * away, although the function call is live.  To keep such calls from being
 * optimized away, put
 *
 * asm ("");
 *
 * in the called function, to serve as a special side effect.
 */
#define WB_ATTRIBUTE_NOINLINE __attribute__((noinline))

/**
 * @brief The pure attribute prohibits a function from modifying the state of
 * the program that is observable by means other than inspecting the function’s
 * return value.  However, functions declared with the pure attribute can safely
 * read any non-volatile objects, and modify the value of objects in a way that
 * does not affect their return value or the observable state of the program.
 */
#define WB_ATTRIBUTE_PURE __attribute__((pure))

#else  // defined(WB_COMPILER_GCC) || defined(WB_COMPILER_CLANG)

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_DLL_EXPORT

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_DLL_IMPORT

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_FORCEINLINE

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_MALLOC_LIKE(deallocator, ptr_index)

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_NOINLINE

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_PURE

#endif  // !defined(WB_COMPILER_MSVC) && !defined(WB_COMPILER_GCC) &&
        // !defined(WB_COMPILER_CLANG)

// Sanitizers.
#ifdef WB_COMPILER_MSVC

/*
 * @brief Tells the compiler to disable the address sanitizer on functions,
 * local variables, or global variables.  This specifier is used in conjunction
 * with AddressSanitizer.
 *
 * WB_MSVC_NOSANITIZE_ADDRESS disables compiler behavior, not runtime
 * behavior.
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/no-sanitize-address
 */
#define WB_ATTRIBUTE_NOSANITIZE_ADDRESS __declspec(no_sanitize_address)

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_NOSANITIZE_COVERAGE

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_NOSANITIZE_THREAD

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_NOSANITIZE_UNDEFINED

#elif defined(WB_COMPILER_GCC) || defined(WB_COMPILER_CLANG)

/*
 * @brief The no_sanitize_address attribute on functions is used to inform the
 * compiler that it should not instrument memory accesses in the function when
 * compiling with the -fsanitize=address option.
 */
#define WB_ATTRIBUTE_NOSANITIZE_ADDRESS __attribute__((no_sanitize_address))

/*
 * @brief The no_sanitize_coverage attribute on functions is used to inform the
 * compiler that it should not do coverage-guided fuzzing code instrumentation
 * (-fsanitize-coverage).
 */
#define WB_ATTRIBUTE_NOSANITIZE_COVERAGE __attribute__((no_sanitize_coverage))

/*
 * @brief The no_sanitize_thread attribute on functions is used to inform the
 * compiler that it should not instrument memory accesses in the function when
 * compiling with the -fsanitize=thread option.
 */
#define WB_ATTRIBUTE_NOSANITIZE_THREAD __attribute__((no_sanitize_thread))

/*
 * @brief The no_sanitize_undefined attribute on functions is used to inform the
 * compiler that it should not check for undefined behavior in the function when
 * compiling with the -fsanitize=undefined option.
 */
#define WB_ATTRIBUTE_NOSANITIZE_UNDEFINED __attribute__((no_sanitize_undefined))

#else  // !defined(WB_COMPILER_GCC) && !defined(WB_COMPILER_CLANG)

/*
 * @brief Do nothing.
 */
#define WB_MSVC_NOSANITIZE_ADDRESS

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_NOSANITIZE_COVERAGE

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_NOSANITIZE_THREAD

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_NOSANITIZE_UNDEFINED

#endif  // !defined(WB_COMPILER_MSVC) && !defined(WB_COMPILER_GCC) &&
        // !defined(WB_COMPILER_CLANG)

// Source annotations.
#ifdef WB_COMPILER_MSVC

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_NONNULL(...)

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_RETURNS_NONNULL

#elif defined(WB_COMPILER_GCC) || defined(WB_COMPILER_CLANG)

/*
 * @brief The no_sanitize_address attribute on functions is used to inform the
 * compiler that it should not instrument memory accesses in the function when
 * compiling with the -fsanitize=address option.
 */
#define WB_ATTRIBUTE_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

/*
 * @brief The returns_nonnull attribute specifies that the function return value
 * should be a non-null pointer.  Lets the compiler optimize callers based on
 * the knowledge that the return value will never be null.
 */
#define WB_ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))

#else  // !defined(WB_COMPILER_GCC) && !defined(WB_COMPILER_CLANG)

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_NONNULL(...)

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_RETURNS_NONNULL

#endif  // !defined(WB_COMPILER_MSVC) && !defined(WB_COMPILER_GCC) &&
        // !defined(WB_COMPILER_CLANG)

// MSVC specific.
#ifdef WB_COMPILER_MSVC

/*
 * @brief Begins MSVC warning override scope.
 */
#define WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE() __pragma(warning(push))

/*
 * @brief Disables MSVC warning |warning_level|.
 */
#define WB_MSVC_DISABLE_WARNING(warning_level) \
  __pragma(warning(disable : warning_level))

/*
 * @brief Ends MSVC warning override scope.
 */
#define WB_MSVC_END_WARNING_OVERRIDE_SCOPE() __pragma(warning(pop))

/*
 * @brief Disable MSVC warning |warning_level| for code |code|.
 */
#define WB_MSVC_SCOPED_DISABLE_WARNING(warning_level, code) \
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()                    \
    WB_MSVC_DISABLE_WARNING(warning_level)                  \
    code                                                    \
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

/*
 * @brief Can be applied to custom memory-allocation functions to make the
 * allocations visible via Event Tracing for Windows (ETW):
 *
 * WB_MSVC_HEAP_ALLOCATOR void* myMalloc(size_t size)
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/allocator
 */
#define WB_MSVC_HEAP_ALLOCATOR __declspec(allocator)

/*
 * @brief Function call doesn't modify or reference visible global state and
 * only modifies the memory pointed to directly by pointer parameters
 * (first-level indirections).
 *
 * "The noalias annotation only applies within the body of the annotated
 * function.  Marking a function as __declspec(noalias) doesn't affect the
 * aliasing of pointers returned by the function.  For another annotation that
 * can impact aliasing, see __declspec(restrict)."
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/noalias
 */
#define WB_MSVC_NOALIAS __declspec(noalias)

/*
 * @brief Should only be applied to pure interface classes, that is, classes
 * that will never be instantiated on their own.  "The WB_MSVC_NOVTABLE
 * stops the compiler from generating code to initialize the vfptr in the
 * constructor(s) and destructor of the class.  In many cases, this removes the
 * only references to the vtable that are associated with the class and, thus,
 * the linker will remove it."
 *
 * "Using this can result in a significant reduction in code size.  If you
 * attempt to instantiate a class marked with novtable and then access a class
 * member, you will receive an access violation (AV)."
 *
 * struct WB_MSVC_NOVTABLE X {
 *   virtual void mf() = 0;
 * };
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/novtable
 */
#define WB_MSVC_NOVTABLE __declspec(novtable)

/*
 * @brief Indicates that a symbol isn't aliased in the current scope.  The
 * __restrict keyword differs from the __declspec(restrict) modifier in the
 * following ways:
 * The __restrict keyword is valid only on variables, and __declspec
 * (restrict) is only valid on function declarations and definitions.
 *
 * __restrict is similar to restrict for C starting in C99, but __restrict can
 * be used in both C++ and C programs.
 *
 * When __restrict is used, the compiler won't propagate the no-alias property
 * of a variable.  That is, if you assign a __restrict variable to a
 * non-__restrict variable, the compiler will still allow the non-__restrict
 * variable to be aliased.
 *
 * This is different from the behavior of the C99 C language restrict keyword.
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/extension-restrict
 */
#define WB_MSVC_RESTRICT_VAR __restrict

/*
 * @brief Tells the compiler that the declared global data item (variable or
 * object) is a pick-any COMDAT (a packaged function).
 *
 * "At link time, if multiple definitions of a COMDAT are seen, the linker picks
 * one and discards the rest.  If the linker option /OPT:REF (Optimizations) is
 * selected, then COMDAT elimination will occur to remove all the unreferenced
 * data items in the linker output."
 *
 * WB_MSVC_SELECTANY declarator
 *
 * See https://docs.microsoft.com/en-us/cpp/cpp/selectany
 */
#define WB_MSVC_SELECTANY __declspec(selectany)

#else  // WB_COMPILER_MSVC

/*
 * @brief Do nothing.
 */
#define WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()

/*
 * @brief Do nothing.
 */
#define WB_MSVC_DISABLE_WARNING(warning_level)

/*
 * @brief Do nothing.
 */
#define WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

/*
 * @brief Do nothing.
 */
#define WB_MSVC_SCOPED_DISABLE_WARNING(warning_level, code) code

/*
 * @brief Do nothing.
 */
#define WB_MSVC_HEAP_ALLOCATOR

/*
 * @brief Do nothing.
 */
#define WB_MSVC_NOALIAS

/*
 * @brief Do nothing.
 */
#define WB_MSVC_NOVTABLE

/*
 * @brief Do nothing.
 */
#define WB_MSVC_RESTRICT_VAR

/*
 * @brief Do nothing.
 */
#define WB_MSVC_SELECTANY

#endif  // !WB_COMPILER_MSVC

// GCC / Clang specific.
#if defined(WB_COMPILER_GCC) || defined(WB_COMPILER_CLANG)

/*
 * @brief The cold attribute on functions is used to inform the compiler that
 * the function is unlikely to be executed.  The function is optimized for size
 * rather than speed.  The paths leading to calls of cold functions within code
 * are marked as unlikely by the branch prediction mechanism.  It is thus useful
 * to mark functions used to handle unlikely conditions, such as perror, as cold
 * to improve optimization of hot functions that do call marked functions in
 * rare occasions.
 */
#define WB_ATTRIBUTE_COLD __attribute__((cold))

/*
 * @brief Calls to functions whose return value is not affected by changes to
 * the observable state of the program and that have no observable effects on
 * such state other than to return a value may lend themselves to optimizations
 * such as common subexpression elimination.  Declaring such functions with the
 * const attribute allows GCC to avoid emitting some calls in repeated
 * invocations of the function with the same argument.
 *
 * The const attribute prohibits a function from reading objects that affect its
 * return value between successive invocations.  However, functions declared
 * with the attribute can safely read objects that do not change their return
 * value, such as non-volatile constants.
 *
 * Note that a function that has pointer arguments and examines the data pointed
 * to must not be declared const if the pointed-to data might change between
 * successive invocations of the function.  In general, since a function cannot
 * distinguish data that might change from data that cannot, const functions
 * should never take pointer or, in C++, reference arguments.
 */
#define WB_ATTRIBUTE_CONST __attribute__((const))

/*
 * @brief The hot attribute on a function is used to inform the compiler that
 * the function is a hot spot of the compiled program.  The function is
 * optimized more aggressively and on many targets it is placed into a special
 * subsection of the text section so all hot functions appear close together,
 * improving locality.
 */
#define WB_ATTRIBUTE_HOT __attribute__((hot))

/*
 * @brief The weak attribute causes a declaration of an external symbol to be
 * emitted as a weak symbol rather than a global.  This is primarily useful in
 * defining library functions that can be overridden in user code, though it can
 * also be used with non-function declarations.
 *
 * The overriding symbol must have the same type as the weak symbol.  In
 * addition, if it designates a variable it must also have the same size and
 * alignment as the weak symbol.  Weak symbols are supported for ELF targets,
 * and also for a.out targets when using the GNU assembler and linker.
 */
#define WB_ATTRIBUTE_WEAK __attribute__((weak))

/*
 * @brief Begins GCC / Clang warning override scope.
 */
#define WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE() _Pragma("GCC diagnostic push")

/*
 * @brief Disables GCC / Clang old-style-cast warning.
 */
#define WB_GCC_DISABLE_OLD_STYLE_CAST_WARNING() \
  _Pragma("GCC diagnostic ignored \"-Wold-style-cast\"")

/*
 * @brief Disables GCC / Clang padded warning.
 */
#define WB_GCC_DISABLE_PADDED_WARNING() \
  _Pragma("GCC diagnostic ignored \"-Wpadded\"")

/*
 * @brief Disables GCC / Clang missing [[noreturn]] warning.
 */
#define WB_GCC_DISABLE_MISSING_NORETURN_WARNING() \
  _Pragma("GCC diagnostic ignored \"-Wmissing-noreturn\"")

/*
 * @brief Disables GCC / Clang switch-default warning.
 */
#define WB_GCC_DISABLE_SWITCH_DEFAULT_WARNING() \
  _Pragma("GCC diagnostic ignored \"-Wswitch-default\"")

/*
 * @brief Disables GCC / Clang switch-enum warning.
 */
#define WB_GCC_DISABLE_SWITCH_ENUM_WARNING() \
  _Pragma("GCC diagnostic ignored \"-Wswitch-enum\"")

/*
 * @brief Disables GCC / Clang undef warning.
 */
#define WB_GCC_DISABLE_UNDEF_WARNING() \
  _Pragma("GCC diagnostic ignored \"-Wundef\"")

#if defined(WB_COMPILER_GCC)

/**
 * @brief Nothing.  GCC-11 somehow doesn't like unused on object fields.
 */
#define WB_ATTRIBUTE_UNUSED_FIELD

/**
 * @brief Do nothing.
 */
#define WB_CLANG_EXPLICIT explicit

/*
 * @brief Disables GCC suggest-attribute=malloc warning.
 */
#define WB_GCC_DISABLE_SUGGEST_MALLOC_ATTRIBUTE_WARNING() \
  _Pragma("GCC diagnostic ignored \"-Wsuggest-attribute=malloc\"")

#else  // WB_COMPILER_CLANG

/**
 * @brief Unused attribute.  Prevents Wunused-private-field.
 */
#define WB_ATTRIBUTE_UNUSED_FIELD [[maybe_unused]]

/**
 * @brief Explicit keyword for places where Clang accepts it.
 */
#define WB_CLANG_EXPLICIT explicit

/*
 * @brief Do nothing.
 */
#define WB_GCC_DISABLE_SUGGEST_MALLOC_ATTRIBUTE_WARNING()

#endif  // WB_COMPILER_GCC || WB_COMPILER_CLANG

/*
 * @brief Ends GCC / Clang warning override scope.
 */
#define WB_GCC_END_WARNING_OVERRIDE_SCOPE() _Pragma("GCC diagnostic pop")

#else  // defined(WB_COMPILER_GCC) || defined(WB_COMPILER_CLANG)

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_COLD

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_CONST

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_HOT

/*
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_WEAK

/**
 * @brief Do nothing.
 */
#define WB_ATTRIBUTE_UNUSED_FIELD [[maybe_unused]]

/**
 * @brief Do nothing.
 */
#define WB_CLANG_EXPLICIT

/*
 * @brief Do nothing.
 */
#define WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()

/*
 * @brief Do nothing.
 */
#define WB_GCC_DISABLE_OLD_STYLE_CAST_WARNING()

/*
 * @brief Do nothing.
 */
#define WB_GCC_DISABLE_PADDED_WARNING()

/*
 * @brief Do nothing.
 */
#define WB_GCC_DISABLE_MISSING_NORETURN_WARNING()

/*
 * @brief Do nothing.
 */
#define WB_GCC_DISABLE_SWITCH_DEFAULT_WARNING()

/*
 * @brief Do nothing.
 */
#define WB_GCC_DISABLE_SWITCH_ENUM_WARNING()

/*
 * @brief Do nothing.
 */
#define WB_GCC_DISABLE_UNDEF_WARNING()

/*
 * @brief Do nothing.
 */
#define WB_GCC_DISABLE_SUGGEST_MALLOC_ATTRIBUTE_WARNING()

/*
 * @brief Do nothing.
 */
#define WB_GCC_END_WARNING_OVERRIDE_SCOPE()

#endif  // !defined(WB_COMPILER_GCC) && !defined(WB_COMPILER_CLANG)

#endif  // !WB_BUILD_COMPILER_CONFIG_H_
