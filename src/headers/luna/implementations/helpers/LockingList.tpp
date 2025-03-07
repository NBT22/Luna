//
// Created by NBT22 on 2/12/2024.
//

#pragma once

namespace luna::helpers
{
template<template<typename listType> class ListType, typename T> LockingList<ListType, T>::LockingList()
{
	list = ListType<T>();
}
template<template<typename listType> class ListType, typename T>
LockingList<ListType, T>::LockingList(const T elements[], size_t length)
{
	list = ListType<T>();
	add(elements, length);
}
template<template<typename listType> class ListType, typename T>
LockingList<ListType, T>::LockingList(std::initializer_list<T> &&elements)
{
	list = ListType<T>();
	list.insert(list.end(), elements.begin(), elements.end());
}


template<template<typename listType> class ListType, typename T> void LockingList<ListType, T>::add(T &element)
{
	list.push_back(element);
}
template<template<typename listType> class ListType, typename T>
void LockingList<ListType, T>::add(T elements[], const size_t listLength)
{
	list.insert(list.end(), elements, elements[listLength - 1]);
}
template<template<typename listType> class ListType, typename T>
void LockingList<ListType, T>::add(std::initializer_list<T> &&elements)
{
	list.insert(list.end(), elements.begin(), elements.end());
}
template<template<typename listType> class ListType, typename T>
void LockingList<ListType, T>::add(LockingList &elements)
{
	list.insert(list.end(), elements.list.begin(), elements.list.end());
}
template<template<typename listType> class ListType, typename T>
LockingList<ListType, T> LockingList<ListType, T>::operator+(const LockingList &rightSide)
{
	add(rightSide);
	return this;
}
template<template<typename listType> class ListType, typename T>
LockingList<ListType, T> LockingList<ListType, T>::operator+(const std::initializer_list<T> &&rightSide)
{
	add(rightSide);
	return this;
}
} // namespace luna::helpers
