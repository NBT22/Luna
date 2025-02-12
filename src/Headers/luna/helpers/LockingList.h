// Created by NBT22 on 2/12/2024.
//

#pragma once

#include <iterator>
#include <mutex>
#include <vector>

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

#include <luna/implementations/LockingList.tpp>

// private:
// /// The number of slots that are actually in use
// size_t length;
// /// The data that the list is storing
// T *elements;
// /// Fix a synchronization segfault
// std::mutex mutex;
//
// public:
// /// Create a new list
// LockingList();
// /// Create a new list and fill with data
// /// @param count The length to initalize the list
// /// @param data The data to fill the list with
// LockingList(size_t count, const T data[]);
// explicit LockingList(size_t count, ...);
// void *operator new(size_t);
// void *operator new(size_t listLength, T data[]);
// void operator delete(void *list);
//
// /// Add an element to the end of a list
// /// @param element The element to add
//
//
//
// void remove(T element);
// void removeIndex(size_t index);
// void insert(T element, size_t index);
// void insert(T elements[], size_t index);
// void insert(LockingList element, size_t index);
// size_t find(T element);
//
//
// LockingList operator+(const T rightSide[]);
// LockingList operator+(const LockingList &rightSide);

//
// /**
//  * Append an item to the list
//  * @param list List to append to
//  * @param data Data to append
//  */
// void ListAdd(List *list, void *data);
//
// /**
//  * Append a group of items to a list
//  * @param list The list to append the values to
//  * @param count The number of items to append to the list
//  * @param ... The items to append to the list
//  */
// void ListAddBatched(List *list, size_t count, ...);
//
// /**
//  * Remove an item from the list by index
//  * @param list List to remove from
//  * @param index Index to remove
//  */
// void ListRemoveAt(List *list, size_t index);
//
// /**
//  * Insert an item after a node
//  * @param list List to insert into
//  * @param index Index to insert after
//  * @param data Data to insert
//  */
// void ListInsertAfter(List *list, size_t index, void *data);
//
// /**
//  * Find an item in the list
//  * @param list List to search
//  * @param data Data to search for
//  * @return Index of the item in the list, -1 if not found
//  */
// size_t ListFind(const List &list, const void *data);
//
// /**
//  * Lock the mutex on a list
//  * @param list The list to lock
//  */
// void ListLock(const List &list);
//
// /**
//  * Unlock the mutex on a list
//  * @param list The list to unlock
//  */
// void ListUnlock(List list);
//
// /**
//  * Clear all items from the list
//  * @param list List to clear
//  * @warning This does not free the data in the list
//  */
// void ListClear(List *list);
//
// /**
//  * Free the list structure
//  * @param list List to free
//  * @param freeListPointer A boolean indicating if the pointer passed to the first argument should be freed.
//  * @warning This does not free the data in the list, but does free the data pointer
//  */
// void ListFree(List *list, bool freeListPointer);
//
// /**
//  * Free the data stored in the list
//  * @param list List to free
//  */
// void ListFreeOnlyContents(List list);
//
// /**
//  * Free the list structure and the data in the list
//  * @param list List to free
//  * @param freeListPointer A boolean indicating if the pointer passed to the first argument should be freed.
//  * @warning If the data is a struct, any pointers in the struct will not be freed, just the struct itself
//  */
// void ListAndContentsFree(List *list, bool freeListPointer);
//
// /**
//  * Get an item from the list by index
//  * @param list The list to get from
//  * @param index The index to get
//  */
// #define ListGet(list, index) (list).data[(index)]
//
// /**
//  * Reallocates memory for an array of arrayLength elements of size bytes each.
//  * @param ptr Pointer to the memory block to be reallocated.
//  * @param arrayLength Number of elements.
//  * @param elementSize Size of each element.
//  * @return Pointer to the reallocated memory block.
//  */
// void *GameReallocArray(void *ptr, size_t arrayLength, size_t elementSize);
