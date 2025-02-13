//
// Created by NBT22 on 2/11/25.
//

#include <iostream>
#include <luna/helpers/LockingList.hpp>
#include <luna/luna.h>
#include <vector>

using namespace luna;

int main()
{
	helpers::LockingList<std::vector, int> list({1, 2, 3});
	for (const std::scoped_lock lock(list.mutex); const int i: list)
	{
		std::cout << i << ' ';
	}

	return 0;
}
