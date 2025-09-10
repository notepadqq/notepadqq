#ifndef QTPROMISE_QPROMISEGLOBAL_H
#define QTPROMISE_QPROMISEGLOBAL_H

// QtCore
#include <QtGlobal>

// STL
#include <functional>
#include <array>

namespace QtPromisePrivate
{
// https://rmf.io/cxx11/even-more-traits#unqualified_types
template <typename T>
using Unqualified = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

/*!
 * \struct HasCallOperator
 * http://stackoverflow.com/a/5117641
 */
template <typename T>
struct HasCallOperator
{
    template <typename U>
    static char check(decltype(&U::operator(), char(0)));

    template <typename U>
    static char (&check(...))[2];

    static const bool value = (sizeof(check<T>(0)) == 1);
};

/*!
 * \struct ArgsOf
 * http://stackoverflow.com/a/7943765
 * http://stackoverflow.com/a/27885283
 */
template <typename... Args>
struct ArgsTraits
{
    using types = std::tuple<Args...>;
    using first = typename std::tuple_element<0, types>::type;
    static const size_t count = std::tuple_size<types>::value;
};

template <>
struct ArgsTraits<>
{
    using types = std::tuple<>;
    using first = void;
    static const size_t count = 0;
};

template <typename T, typename Enable = void>
struct ArgsOf : public ArgsTraits<>
{ };

template <typename T>
struct ArgsOf<T, typename std::enable_if<HasCallOperator<T>::value>::type>
    : public ArgsOf<decltype(&T::operator())>
{ };

template <typename TReturn, typename... Args>
struct ArgsOf<TReturn(Args...)> : public ArgsTraits<Args...>
{ };

template <typename TReturn, typename... Args>
struct ArgsOf<TReturn(*)(Args...)> : public ArgsTraits<Args...>
{ };

template <typename T, typename TReturn, typename... Args>
struct ArgsOf<TReturn(T::*)(Args...)> : public ArgsTraits<Args...>
{ };

template <typename T, typename TReturn, typename... Args>
struct ArgsOf<TReturn(T::*)(Args...) const> : public ArgsTraits<Args...>
{ };

template <typename T, typename TReturn, typename... Args>
struct ArgsOf<TReturn(T::*)(Args...) volatile> : public ArgsTraits<Args...>
{ };

template <typename T, typename TReturn, typename... Args>
struct ArgsOf<TReturn(T::*)(Args...) const volatile> : public ArgsTraits<Args...>
{ };

template <typename T>
struct ArgsOf<std::function<T>> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<T&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<const T&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<volatile T&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<const volatile T&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<T&&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<const T&&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<volatile T&&> : public ArgsOf<T>
{ };

template <typename T>
struct ArgsOf<const volatile T&&> : public ArgsOf<T>
{ };

} // namespace QtPromisePrivate

#endif // QTPROMISE_QPROMISEGLOBAL_H
