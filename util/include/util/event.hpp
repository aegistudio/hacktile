#pragma once
// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file event.hpp
 * @brief event system common template
 * @author aegistudio
 *
 * This file provides template definition for event
 * handling and dispatching. Any other modules can define
 * their own events and handler interfaces, and dispatch
 * their defined events through the event manager.
 */
#include <algorithm>

namespace hacktile {
namespace util {

// forward eventRegistry definition for being used later.
template<typename handlerType>
class eventRegistry;

/**
 * eventSubscription is a doubly linked list that is kept
 * in track while being subscribed.
 *
 * XXX: This class is not multithread safe, and since
 * hacktile is not designed for working under multithreaded
 * environment, it is safe for now (actually it is hard to
 * write this class in multithreaded manner).
 */
template<typename handlerType>
class eventSubscription {
	eventSubscription *previous, *next;
	handlerType* handler;
	friend class eventRegistry<handlerType>;
public:
	/// default empty constructor for defining in classes.
	eventSubscription() noexcept:
		previous(nullptr), next(nullptr), handler(nullptr) {}

	/// eventSubscription move constructor for swapping the
	/// eventsubscription relationship.
	eventSubscription(eventSubscription&& r) noexcept:
		previous(r.previous), next(r.next), handler(r.handler) {
		if(previous != nullptr) previous->next = this;
		if(next != nullptr) next->previous = this;
		r.previous = nullptr;
		r.next = nullptr;
	}

	/// default destructor for removing the instance from
	/// its previous and next node.
	~eventSubscription() noexcept {
		if(previous != nullptr) previous->next = next;
		if(next != nullptr) next->previous = previous;
	}
private:
	/// dissolveChain is called to remove all items in the
	/// event subscription chain.
	void dissolveChain() noexcept {
		if(previous != nullptr) previous->next = nullptr;
		while(next != nullptr) {
			next->previous = nullptr;
			auto nextNext = next->next;
			next->next = nullptr;
			next = nextNext;
		}	
	}

	/// private insertion constructor for adding the item
	/// right after the last item. The previous node is
	/// asserted to be non-null.
	eventSubscription(
		eventSubscription* previous,
		handlerType* handler) noexcept:
		previous(previous), next(previous->next), handler(handler) {
		// assert(previous != mullptr);
		previous->next = this;
		if(next) next->previous = this;
	}
};

template <typename handlerType>
class eventRegistry {
	eventSubscription<handlerType> sentinel;
public:
	// default constructor for the registry.
	eventRegistry() noexcept: sentinel() {
		sentinel.previous = &sentinel;
		sentinel.next = &sentinel;
	}

	// destructor for dissolving the current registry chain.
	~eventRegistry() noexcept {
		sentinel.dissolveChain();
	}
protected:
	/**
	 * @brief dispatch event to all handlers.
	 *
	 * By calling this function, all subscribers must
	 * be assumed to be equivalent though they are called
	 * in the order that they registered.
	 */
	template<typename eventType> void dispatch(
		void (handlerType::*f)(const eventType&),
		const eventType& event) const {

		for(auto sub = sentinel.next;
			sub != &sentinel; sub = sub->next)
			(sub->handler->*f)(event);
	}

	/**
	 * @brief prefilter incoming event by handlers.
	 *
	 * By calling this function, all subscribers will be
	 * queried for whether this event should happen, and
	 * the event must be canceled if one of them rejects.
	 */
	template<typename eventType> bool prefilter(
		bool (handlerType::*f)(const eventType&),
		const eventType& event) const {

		for(auto sub = sentinel.next;
			sub != &sentinel; sub = sub->next)
			if(!(sub->handler->*f)(event)) return false;
		return true;
	}
public:
	/**
	 * subscribe attempt to subscribe the given handler
	 * and create an subscription in the class.
	 *
	 * The xvalue of the subscription will be returned,
	 * and the caller must pass it to proper manager to
	 * correctly manage the item.
	 */
	eventSubscription<handlerType>
	subscribe(handlerType* handler) noexcept {
		return std::move(eventSubscription<handlerType>(
			sentinel.previous, handler));
	}
};

} // namespace hacktile::util
} // namespace hacktile
