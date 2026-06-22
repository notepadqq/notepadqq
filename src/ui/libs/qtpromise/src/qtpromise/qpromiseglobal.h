/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#ifndef QTPROMISE_QPROMISEGLOBAL_H
#define QTPROMISE_QPROMISEGLOBAL_H

#include <QtGlobal>

#include <array>
#include <functional>

namespace QtPromisePrivate {

// https://rmf.io/cxx11/even-more-traits#unqualified_types
template<typename T>
using Unqualified = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

/*!
 * \struct HasCallOperator
 * http://stackoverflow.com/a/5117641
 */
template<typename T>
struct HasCallOperator
{
    template<typename U>
    static char check(decltype(&U::operator(), char(0)));

    template<typename U>
    static char (&check(...))[2];

    static const bool value = (sizeof(check<T>(0)) == 1);
};

/*!
 * \struct ArgsOf
 * http://stackoverflow.com/a/7943765
 * http://stackoverflow.com/a/27885283
 */
template<typename... Args>
struct ArgsTraits
{
    using types = std::tuple<Args...>;
    using first = typename std::tuple_element<0, types>::type;
    static const size_t count = std::tuple_size<types>::value;
};

template<>
struct ArgsTraits<>
{
    using types = std::tuple<>;
    using first = void;
    static const size_t count = 0;
};

// Fallback implementation, including types (T) which are not functions but
// also lambda with `auto` arguments, which are not covered but still valid
// callbacks (see the QPromiseBase<T> template constructor).
template<typename T, typename Enabled = void>
struct ArgsOf : public ArgsTraits<>
{ };

// Partial specialization for null function.
template<>
struct ArgsOf<std::nullptr_t> : public ArgsTraits<>
{ };

// Partial specialization for type with a non-overloaded operator().
// This applies to lambda, std::function but not to std::bind result.
template<typename T>
struct ArgsOf<T, typename std::enable_if<HasCallOperator<T>::value>::type>
    : public ArgsOf<decltype(&T::operator())>
{ };

// Partial specialization to remove reference and rvalue (e.g. lambda, std::function, etc.).
template<typename T>
struct ArgsOf<T&> : public ArgsOf<T>
{ };

template<typename T>
struct ArgsOf<T&&> : public ArgsOf<T>
{ };

// Partial specialization for function type.
template<typename R, typename... Args>
struct ArgsOf<R(Args...)> : public ArgsTraits<Args...>
{ };

// Partial specialization for function pointer.
template<typename R, typename... Args>
struct ArgsOf<R (*)(Args...)> : public ArgsTraits<Args...>
{ };

// Partial specialization for pointer-to-member-function (i.e. operator()'s).
template<typename R, typename T, typename... Args>
struct ArgsOf<R (T::*)(Args...)> : public ArgsTraits<Args...>
{ };

template<typename R, typename T, typename... Args>
struct ArgsOf<R (T::*)(Args...) const> : public ArgsTraits<Args...>
{ };

template<typename R, typename T, typename... Args>
struct ArgsOf<R (T::*)(Args...) volatile> : public ArgsTraits<Args...>
{ };

template<typename R, typename T, typename... Args>
struct ArgsOf<R (T::*)(Args...) const volatile> : public ArgsTraits<Args...>
{ };

} // namespace QtPromisePrivate

#endif // QTPROMISE_QPROMISEGLOBAL_H
