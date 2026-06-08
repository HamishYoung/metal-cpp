//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// QuartzCore/CAMetalDisplayLink.hpp
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

#include "../Metal/MTLDrawable.hpp"
#include "../Metal/MTLTexture.hpp"

#include "CADefines.hpp"
#include "CAMetalLayer.hpp"
#include "CAPrivate.hpp"

#include <QuartzCore/CAFrameRateRange.h>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace CA
{
class MetalDisplayLink;

class MetalDisplayLinkUpdate : public NS::Referencing<MetalDisplayLinkUpdate>
{
public:
	CFTimeInterval 		targetPresentationTimestamp() const;
	CFTimeInterval 		targetTimestamp() const;
	CA::MetalDrawable* 	drawable() const;
};

class MetalDisplayLinkDelegate
{
	public:
		virtual						~MetalDisplayLinkDelegate() { }
		virtual void				needsUpdate( class MetalDisplayLink* link, 
												 class MetalDisplayLinkUpdate* update) { }
};

class MetalDisplayLink : public NS::Referencing<MetalDisplayLink>
{
public:
	static MetalDisplayLink* alloc();

    class MetalDisplayLink* init(class MetalLayer* ) const;

	CAFrameRateRange* preferredFrameRateRange() const;
	void setPreferredFrameRateRange(CAFrameRateRange* preferredFrameRateRange);

	float preferredFrameLatency() const;
	void setPreferredFrameLatency(float preferredFrameLatency);

	CA::MetalDisplayLinkDelegate* delegate() const;
	void setDelegate(CA::MetalDisplayLinkDelegate* delegate);

	void addToRunLoop(NS::RunLoop* runLoop, NS::RunLoopMode mode);
	void removeFromRunLoop(NS::RunLoop* runLoop, NS::RunLoopMode mode);
	void invalidate();

	bool paused() const;
	void setPaused(bool paused);
};
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CA::MetalDisplayLink* CA::MetalDisplayLink::alloc()
{
	return NS::Object::alloc<CA::MetalDisplayLink>(_CA_PRIVATE_CLS(CAMetalDisplayLink));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CA::MetalDisplayLink* CA::MetalDisplayLink::init(class CA::MetalLayer* layer) const
{
	return Object::sendMessage<CA::MetalDisplayLink*>(this, _CA_PRIVATE_SEL(initWithMetalLayer_), layer);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CAFrameRateRange* CA::MetalDisplayLink::preferredFrameRateRange() const
{
	return Object::sendMessage<CAFrameRateRange*>(this, _CA_PRIVATE_SEL(preferredFrameRateRange));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::setPreferredFrameRateRange(CAFrameRateRange* preferredFrameRateRange)
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(setPreferredFrameRateRange_), preferredFrameRateRange);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE float CA::MetalDisplayLink::preferredFrameLatency() const
{
	return Object::sendMessage<float>(this, _CA_PRIVATE_SEL(preferredFrameLatency));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::setPreferredFrameLatency(float preferredFrameLatency)
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(setPreferredFrameLatency_), preferredFrameLatency);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CA::MetalDisplayLinkDelegate* CA::MetalDisplayLink::delegate() const
{
	NS::Value* pDelegatePtr = ( NS::Value* )(objc_getAssociatedObject((id)this, _CA_DELEGATE_ASSOCIATION_KEY));
	if(!pDelegatePtr)
	{
		return nullptr;	
	}

    return reinterpret_cast<CA::MetalDisplayLinkDelegate* >( pDelegatePtr->pointerValue() );
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::setDelegate(CA::MetalDisplayLinkDelegate* delegate)
{
	if(delegate)
	{
		// Associate the value with the application instance
		NS::Value* pWrapper = NS::Value::value( delegate );
		objc_setAssociatedObject( this, _CA_DELEGATE_ASSOCIATION_KEY, pWrapper, OBJC_ASSOCIATION_COPY );

		// Dispatch functions
		typedef void (*DispatchFunction)( CA::MetalDisplayLink*, SEL, Object *, Object *);

		DispatchFunction needsUpdate = []( CA::MetalDisplayLink* pSelf, SEL, Object * metalDisplayLink, Object * update){
			auto pDel = pSelf->delegate();
			pDel->needsUpdate( (CA::MetalDisplayLink *)metalDisplayLink,  
							(CA::MetalDisplayLinkUpdate*)update );
		};

		Class cls = (Class)_CA_PRIVATE_CLS( CAMetalDisplayLink );
		SEL sel = (SEL)_CA_PRIVATE_SEL( metalDisplayLink_needsUpdate_ );
		bool result = false;
		if (class_getInstanceMethod(cls, sel) == nullptr)
		{
			// If the method doesn't exist, add it
			result = class_addMethod(cls, sel, (IMP)needsUpdate, "v@:@@");
		}
		else 
		{
			result = class_replaceMethod(cls, sel, (IMP)needsUpdate, "v@:@@");
		}
		assert(result);

		Object::sendMessage< void >( this, _CA_PRIVATE_SEL( setDelegate_ ), this );
	}
	else
	{
		Object::sendMessage< void >( this, _CA_PRIVATE_SEL( setDelegate_ ), nullptr );
		objc_setAssociatedObject( this, _CA_DELEGATE_ASSOCIATION_KEY, nullptr, OBJC_ASSOCIATION_ASSIGN );
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::addToRunLoop(NS::RunLoop* runLoop, NS::RunLoopMode mode)
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(addToRunLoop_forMode_), runLoop, mode);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::removeFromRunLoop(NS::RunLoop* runLoop, NS::RunLoopMode mode)
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(removeFromRunLoop_forMode_), runLoop, mode);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::invalidate()
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(invalidate));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE bool CA::MetalDisplayLink::paused() const
{
	return Object::sendMessage<bool>(this, _CA_PRIVATE_SEL(isPaused));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::setPaused(bool paused)
{
	Object::sendMessage<void>(this, _CA_PRIVATE_SEL(setPaused_), paused);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CFTimeInterval CA::MetalDisplayLinkUpdate::targetPresentationTimestamp() const
{
	return Object::sendMessage<CFTimeInterval>(this, _CA_PRIVATE_SEL(targetPresentationTimestamp));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CFTimeInterval CA::MetalDisplayLinkUpdate::targetTimestamp() const
{
	return Object::sendMessage<CFTimeInterval>(this, _CA_PRIVATE_SEL(targetTimestamp));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CA::MetalDrawable* 	CA::MetalDisplayLinkUpdate::drawable() const
{
	return Object::sendMessage<MetalDrawable*>(this, _CA_PRIVATE_SEL(drawable));
}