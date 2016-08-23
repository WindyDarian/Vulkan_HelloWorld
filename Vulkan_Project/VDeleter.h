#pragma once

#include <vulkan/vulkan.h>
#include <functional>

// This RAII class is used to hold Vulkan handles.
// It will call deletef upon destruction or & operator
template <typename T>
class VDeleter
{
public:
	VDeleter()
		: object(VK_NULL_HANDLE)
		, deleter( [](T obj) {} )
	{}

	VDeleter(std::function<void(T, VkAllocationCallbacks*)> deletef)
		: object(VK_NULL_HANDLE)
		, deleter( [=](T obj) { deletef(obj, nullptr); } )
	{}

	VDeleter(const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef)
		: object(VK_NULL_HANDLE)
		, deleter( [&instance, deletef](T obj) { deletef(instance, obj, nullptr); } )
	{}

	VDeleter(const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef)
		: object(VK_NULL_HANDLE)
		, deleter( [&device, deletef](T obj) { deletef(device, obj, nullptr); } )
	{}

	~VDeleter()
	{
		cleanup();
	}

	T* operator &()
	{
		cleanup();
		return &object;
	}

	operator T() const
	{
		return object;
	}

	VDeleter(VDeleter<T>&& other)
		:object(VK_NULL_HANDLE) //to be swapped to "other"
	{
		swap(*this, other);
	}
	VDeleter<T>& operator=(VDeleter<T>&& other)
	{
		swap(*this, other);
		return *this;
	}
	friend void swap(VDeleter<T>& first, VDeleter<T>& second)
	{
		using std::swap;
		swap(first.object, second.object);
		swap(first.deleter, second.deleter);
	}

private:
	T object;
	std::function<void(T)> deleter;

	void cleanup()
	{
		if (object != VK_NULL_HANDLE)
		{
			deleter(object);
		}
		object = VK_NULL_HANDLE;
	}

	void operator=(const VDeleter<T>&) = delete;
	VDeleter(const VDeleter<T>&) = delete;
};