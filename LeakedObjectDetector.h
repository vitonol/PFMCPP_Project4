#pragma once

#include "Atomic.h"
#include <cassert>

namespace juce
{
    template <class OwnerClass>
    class LeakedObjectDetector
    {
    public:
        //==============================================================================
        LeakedObjectDetector() noexcept                                 { ++(getCounter().numObjects); }
        LeakedObjectDetector (const LeakedObjectDetector&) noexcept     { ++(getCounter().numObjects); }
        
        ~LeakedObjectDetector()
        {
            if (--(getCounter().numObjects) < 0)
            {
                std::cout << "*** Dangling pointer deletion! Class: " << getLeakedObjectOwnerClass() << std::endl;
                assert(false);
            }
        }
        
    private:
        //==============================================================================
        class LeakCounter
        {
        public:
            LeakCounter() = default;
            
            ~LeakCounter()
            {
                if (numObjects.value > 0)
                {
                    std::cout << "*** Leaked objects detected: " << numObjects.value << " instance(s) of class " << getLeakedObjectOwnerClass() << std::endl;
                    assert(false);
                }
            }
            
            Atomic<int> numObjects;
        };
        
        static const char* getLeakedObjectOwnerClass()
        {
            return OwnerClass::getLeakedObjectOwnerClass();
        }
        
        static LeakCounter& getCounter() noexcept
        {
            static LeakCounter counter;
            return counter;
        }
    };
}

#define JUCE_JOIN_MACRO_HELPER(a, b) a ## b
#define JUCE_JOIN_MACRO(item1, item2)  JUCE_JOIN_MACRO_HELPER (item1, item2)
#define JUCE_LEAK_DETECTOR(OwnerClass) \
friend class juce::LeakedObjectDetector<OwnerClass>; \
static const char* getLeakedObjectOwnerClass() noexcept { return #OwnerClass; } \
juce::LeakedObjectDetector<OwnerClass> JUCE_JOIN_MACRO (leakDetector, __LINE__);

#define JUCE_DECLARE_NON_COPYABLE(OwnerClass) \
            OwnerClass (const OwnerClass&) = delete;\
            OwnerClass& operator= (const OwnerClass&) = delete;

#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OwnerClass) \
            JUCE_DECLARE_NON_COPYABLE(OwnerClass) \
            JUCE_LEAK_DETECTOR(OwnerClass)

