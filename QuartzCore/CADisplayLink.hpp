//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// QuartzCore/CADisplayLink.hpp
//
// Copyright 2020-2023 Apple Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "CADefines.hpp"
#include "CAPrivate.hpp"
#include <Foundation/NSPrivate.hpp>

#include <Foundation/Foundation.hpp>
#include <QuartzCore/CAFrameRateRange.h>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace CA
{

class DisplayLink;

using Callback = std::function<void(const DisplayLink*)>;

class DisplayLink : public NS::Referencing<DisplayLink>
{
public:
	static NS::SharedPtr<DisplayLink> displayLinkWithTarget(Callback handleDisplayLinkCB);

	CFTimeInterval duration() const;

	CAFrameRateRange preferredFrameRateRange() const;
	void setPreferredFrameRateRange(CAFrameRateRange preferredFrameRateRange);

	CFTimeInterval timestamp() const;
	CFTimeInterval targetTimestamp() const;
	
	void addToRunLoop(NS::RunLoop* runLoop, NS::RunLoopMode mode);
	void removeFromRunLoop(NS::RunLoop* runLoop, NS::RunLoopMode mode);
	void invalidate();

	bool paused() const;
	void setPaused(bool paused);
};
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

// new: take a C++ callback, wrap in an ObjC target Object, and pass to CADisplayLink
_CA_INLINE NS::SharedPtr<CA::DisplayLink> CA::DisplayLink::displayLinkWithTarget(Callback handleDisplayLinkCB)
{
	// 1) build a small ObjC target to hold our std::function
	Class helperCls = objc_getClass("CppDisplayLinkTarget");
	if (!helperCls) 
	{
		helperCls = objc_allocateClassPair((Class)objc_getClass("NSObject"),
											"CppDisplayLinkTarget", 0);
		// add ivar to store pointer to Callback
		class_addIvar(helperCls, "_cbPtr", sizeof(void*), alignof(void*), "^v");
		// -handleDisplayLink:
		// Dispatch functions
		typedef void (*HandleDisplayLinkFunction)( Object *, SEL, DisplayLink *);

		HandleDisplayLinkFunction handleDisplayLink = []( Object * self, SEL, DisplayLink * displayLink)
		{
			Class helperCls = objc_getClass("CppDisplayLinkTarget");
			auto cbPtr = *reinterpret_cast<Callback**>(
								(uint8_t*)self + ivar_getOffset(
									class_getInstanceVariable(helperCls, "_cbPtr")));
			(*cbPtr)(displayLink);
		};

		SEL selHandle = _CA_PRIVATE_SEL(handleDisplayLink_);
		bool result = class_addMethod(helperCls, selHandle,(IMP)handleDisplayLink, "v@:@");
		assert(result);

		// -dealloc
		SEL selDealloc = _CA_PRIVATE_SEL(dealloc);
		Method superDealloc = class_getInstanceMethod((Class)objc_getClass("NSObject"), selDealloc);
		IMP impDealloc = (IMP)+[](id self, SEL _cmd){
			Class helperCls = objc_getClass("CppDisplayLinkTarget");
			auto iv = class_getInstanceVariable(helperCls, "_cbPtr");
			auto cbPtr = *reinterpret_cast<Callback**>(
								(uint8_t*)self + ivar_getOffset(iv));
			delete cbPtr;
			SEL selDealloc = _CA_PRIVATE_SEL(dealloc);
			Method superDealloc = class_getInstanceMethod((Class)objc_getClass("NSObject"), selDealloc);
			((void(*)(id,SEL))method_getImplementation(superDealloc))(self, _cmd);
		};
		result = class_addMethod(helperCls, selDealloc, impDealloc, method_getTypeEncoding(superDealloc));
		assert(result);
		objc_registerClassPair(helperCls);
	}

	// 2) alloc/init target and stash our std::function*
	id target = ((id(*)(Class,SEL))objc_msgSend)(helperCls, sel_registerName("alloc"));
	Callback* heapCb = new Callback(std::move(handleDisplayLinkCB));
	object_setInstanceVariable(target, "_cbPtr", heapCb);
	target = ((id(*)(id,SEL))objc_msgSend)(target, sel_registerName("init"));

	// DisplayLink will retain the target
	return RetainPtr(Object::sendMessage<CA::DisplayLink*>(_CA_PRIVATE_CLS(CADisplayLink), _CA_PRIVATE_SEL(displayLinkWithTarget_selector_), target, _CA_PRIVATE_SEL(handleDisplayLink_)));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CFTimeInterval CA::DisplayLink::duration() const
{
	return Object::sendMessage<CFTimeInterval>(this, _CA_PRIVATE_SEL(duration));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CAFrameRateRange CA::DisplayLink::preferredFrameRateRange() const
{
	return Object::sendMessage<CAFrameRateRange>(this, _CA_PRIVATE_SEL(preferredFrameRateRange));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::DisplayLink::setPreferredFrameRateRange(CAFrameRateRange preferredFrameRateRange)
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(setPreferredFrameRateRange_), preferredFrameRateRange);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CFTimeInterval CA::DisplayLink::timestamp() const
{
	return Object::sendMessage<CFTimeInterval>(this, _CA_PRIVATE_SEL(timestamp));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CFTimeInterval CA::DisplayLink::targetTimestamp() const
{
	return Object::sendMessage<CFTimeInterval>(this, _CA_PRIVATE_SEL(targetTimestamp));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::DisplayLink::addToRunLoop(NS::RunLoop* runLoop, NS::RunLoopMode mode)
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(addToRunLoop_forMode_), runLoop, mode);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::DisplayLink::removeFromRunLoop(NS::RunLoop* runLoop, NS::RunLoopMode mode)
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(removeFromRunLoop_forMode_), runLoop, mode);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::DisplayLink::invalidate()
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(invalidate));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE bool CA::DisplayLink::paused() const
{
	return Object::sendMessage<bool>(this, _CA_PRIVATE_SEL(isPaused));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::DisplayLink::setPaused(bool paused)
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(setPaused_), paused);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------