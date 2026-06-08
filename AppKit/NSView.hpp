/*
 *
 * Copyright 2020-2021 Apple Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// AppKit/NSView.hpp
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "AppKitPrivate.hpp"
#include <Foundation/NSObject.hpp>
#include <CoreGraphics/CGGeometry.h>
#include <QuartzCore/CADisplayLink.hpp>

namespace NS
{
	class View : public Referencing< View >
	{
		public:
			View*							init( CGRect frame );
			SharedPtr<CA::DisplayLink> 		displayLink(CA::Callback handleDisplayLinkCB);
	};
}


_NS_INLINE NS::View* NS::View::init( CGRect frame )
{
	return Object::sendMessage< View* >( _APPKIT_PRIVATE_CLS( NSView ), _APPKIT_PRIVATE_SEL( initWithFrame_ ), frame );
}

_NS_INLINE NS::SharedPtr<CA::DisplayLink> NS::View::displayLink( CA::Callback handleDisplayLinkCB )
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
		typedef void (*HandleDisplayLinkFunction)( Object *, SEL, CA::DisplayLink *);

		HandleDisplayLinkFunction handleDisplayLink = []( Object * self, SEL, CA::DisplayLink * displayLink)
		{
			Class helperCls = objc_getClass("CppDisplayLinkTarget");
			auto cbPtr = *reinterpret_cast<CA::Callback**>(
								(uint8_t*)self + ivar_getOffset(
									class_getInstanceVariable(helperCls, "_cbPtr")));
			(*cbPtr)(displayLink);
		};

		SEL selHandle = _APPKIT_PRIVATE_SEL(handleDisplayLink_);
		bool result = class_addMethod(helperCls, selHandle,(IMP)handleDisplayLink, "v@:@");
		assert(result);

		// -dealloc
		SEL selDealloc = _APPKIT_PRIVATE_SEL(dealloc);
		Method superDealloc = class_getInstanceMethod((Class)objc_getClass("NSObject"), selDealloc);
		IMP impDealloc = (IMP)+[](id self, SEL _cmd){
			Class helperCls = objc_getClass("CppDisplayLinkTarget");
			auto iv = class_getInstanceVariable(helperCls, "_cbPtr");
			auto cbPtr = *reinterpret_cast<CA::Callback**>(
								(uint8_t*)self + ivar_getOffset(iv));
			delete cbPtr;
			SEL selDealloc = _APPKIT_PRIVATE_SEL(dealloc);
			Method superDealloc = class_getInstanceMethod((Class)objc_getClass("NSObject"), selDealloc);
			((void(*)(id,SEL))method_getImplementation(superDealloc))(self, _cmd);
		};
		result = class_addMethod(helperCls, selDealloc, impDealloc, method_getTypeEncoding(superDealloc));
		assert(result);
		objc_registerClassPair(helperCls);
	}

	// 2) alloc/init target and stash our std::function*
	id target = ((id(*)(Class,SEL))objc_msgSend)(helperCls, sel_registerName("alloc"));
	CA::Callback* heapCb = new CA::Callback(std::move(handleDisplayLinkCB));
	object_setInstanceVariable(target, "_cbPtr", heapCb);
	target = ((id(*)(id,SEL))objc_msgSend)(target, sel_registerName("init"));

	// DisplayLink will retain the target
	return RetainPtr(Object::sendMessage<CA::DisplayLink*>(this, _APPKIT_PRIVATE_SEL(displayLinkWithTarget_selector_), target, _APPKIT_PRIVATE_SEL(handleDisplayLink_)));
}
