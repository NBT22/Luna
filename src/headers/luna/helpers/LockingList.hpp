// Created by NBT22 on 2/12/2024.
//

#pragma once

#include <mutex>

namespace luna::helpers
{
template<template<typename listType> typename ListType, typename T> class LockingList
{
	private:
		ListType<T> list;

	public:
		/// Create a new list
		LockingList();
		/// Create a new list and fill with data
		/// @param elements The data to fill the list with
		/// @param length The length to initialize the list
		LockingList(const T elements[], size_t length); // TODO: Should elements be *& instead of *
		LockingList(std::initializer_list<T> &&elements);

		/// Mutex used for threaded sync
		std::mutex mutex;

		constexpr typename ListType<T>::iterator begin()
		{
			return list.begin();
		}
		constexpr typename ListType<T>::const_iterator cbegin()
		{
			return list.cbegin();
		}
		constexpr typename ListType<T>::iterator end()
		{
			return list.end();
		}
		constexpr typename ListType<T>::const_iterator cend()
		{
			return list.cend();
		}
		constexpr typename ListType<T>::reverse_iterator rbegin()
		{
			return list.rbegin();
		}
		constexpr typename ListType<T>::const_reverse_iterator crbegin()
		{
			return list.crbegin();
		}
		constexpr typename ListType<T>::reverse_iterator rend()
		{
			return list.rend();
		}
		constexpr typename ListType<T>::const_reverse_iterator crend()
		{
			return list.crend();
		}

		void add(T &element);
		void add(T elements[], size_t listLength); // TODO: Should elements be *& instead of *
		void add(std::initializer_list<T> &&elements);
		void add(LockingList &elements);
		LockingList operator+(const LockingList &rightSide);
		LockingList operator+(const std::initializer_list<T> &&rightSide);
};

} // namespace luna::helpers

#include <luna/implementations/helpers/LockingList.tpp>
