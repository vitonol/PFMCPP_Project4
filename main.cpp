/*
Project 4: Part 9 / 9
 video: Chapter 5 Part 8

Create a branch named Part9

 Rule of 3-5-0 and S.R.P.
 
 DO NOT EDIT YOUR PREVIOUS main(). 
 
 1) add the Leak Detector files from Project5 
 
 2) add these macros after the JUCE_LEAK_DETECTOR macro :
 */

// #define JUCE_DECLARE_NON_COPYABLE(className) \
//             className (const className&) = delete;\
//             className& operator= (const className&) = delete;

// #define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(className) \
//             JUCE_DECLARE_NON_COPYABLE(className) \
//             JUCE_LEAK_DETECTOR(className)

/*
 3) add JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Temporary) to the end of the  Temporary<> struct
 
 4) add JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Numeric) to the end of the Numeric<> struct
 
 if you compile it, you'll see lots of errors about deleted functions.
 
 5) Implement the Rule of 5 on Numeric<> and Temporary<> to fix this.
 
 You should end up with the same program output as Part 8's task if you did it right.
 */

/*
 If you did everything correctly, this is the output you should get:
 
I'm a Temporary<f> object, #0
I'm a Temporary<i> object, #0
I'm a Temporary<d> object, #0
f: -1.89
d: -3.024
i: -9
Point { x: -1.89, y: -9 }
d: 3.024
I'm a Temporary<d> object, #1
I'm a Temporary<d> object, #2
d: 1.49519e+08
Point { x: -2.82591e+08, y: -1.34567e+09 }
I'm a Temporary<f> object, #1
I'm a Temporary<i> object, #1
I'm a Temporary<i> object, #2
I'm a Temporary<i> object, #3
intNum: 5
I'm a Temporary<f> object, #2
f squared: 3.5721
I'm a Temporary<f> object, #3
f cubed: 45.5796
I'm a Temporary<d> object, #3
d squared: 2.2356e+16
I'm a Temporary<d> object, #4
d cubed: 1.11733e+49
I'm a Temporary<i> object, #4
i squared: 81
I'm a Temporary<i> object, #5
i cubed: 531441

Use a service like https://www.diffchecker.com/diff to compare your output. 
*/

 
#include <iostream>
#include <cmath>
#include <memory>
#include <functional>
#include "LeakedObjectDetector.h"
#include <typeinfo>

/*
RULE OF 5:
destructor
copy constructor
copy assignment operator
move constructor
move assignment operator

*/

//+++++++++++++ TEMPORARY CLASS++++++++++++++++++++++
template<typename NumericType>
struct Temporary
{
    Temporary(NumericType t) : v(t)
    {
        std::cout << "I'm a Temporary<" << typeid(v).name() << "> object, #"
                  << counter++ << std::endl;
    }

    // ~Temporary() = default; { } // destructor 
    ~Temporary() { delete v; } 

    Temporary(const Temporary& other)
    {
        reallocateFromOther( other ); // copy constructor
    }  
    // // Temporary(const Temporary& _t) : v(_t.t) { } // user defined
    // // Temporary(const Temporary&) = default; 

    Temporary& operator= (Temporary other) // Copy assignment operator
    {
        reallocateFromOther( other );
        return *this; // allows chaining
    } 
    // // Temporary& Temporary::operator= (const Temporary&) = default;

    // Temporary(Temporary&&) { } // move constructor
    // // Temporary(Temporary&&) = default;

    // Temporary& operator=( Temporary&&) { } // move assignment operator
    // // Temporary& Temporary:: operator=( Temporary&&) = default;

    operator NumericType() const 
    {
        return v;   /* read-only function */ 
    }
    operator NumericType& ()
    {
        return v;   /* read/write function */
    }


private:
    static int counter;
    NumericType v;

    void reallocateFromOther(const Temporary& other)
    {
        delete v; // prevents from double deletion
        v = new Temporary& t;
        memcpy( v, other.v, t );
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Temporary)
};

//++++++++++++ POINT CLASS ++++++++++++++++
template<typename NumericType>
int Temporary<NumericType>::counter { 0 };

struct Point
{
    Point(float a, float b) : x(a), y(b) { }

    template<typename NumericType>
    Point(NumericType& a, NumericType& b) : 
    Point(static_cast<float>(a), static_cast<float>(b)) { }  

    template<typename NumericType>
    Point& multiply(const NumericType& m)
    {
        x *= static_cast<float>(m);
        y *= static_cast<float>(m);
        return *this;
    }
    
    void toString()
    {
        std::cout << "Point " <<"{ x: " << x << ", y: " << y << " }" <<std::endl;
    }
private:
    float x{0}, y{0};
};

//+++++++++++++ NUMERIC CLASS++++++++++++++++++++++
template <typename NumericType>
struct Numeric
{
    using Type = Temporary<NumericType>;
    
    Numeric(NumericType v) : un( std::make_unique<Type>(v)) { }
 
    ~Numeric()
    {
        un = nullptr;
    }

    operator NumericType() const
    {
        return *un;
    }
    operator NumericType&()
    {
        return *un;
    }

    template <typename OtherTT>
    Numeric& operator = (const OtherTT& t)
    {
        *un = static_cast<NumericType>(t);
        return *this;
    }

    template <typename OtherTT>
    Numeric& operator += (const OtherTT& t)
    {
        *un += static_cast<NumericType>(t);
        return *this;
    }

    template <typename OtherTT>
    Numeric& operator -= (const OtherTT& t)
    {
        *un -= static_cast<NumericType>(t);
        return *this;
    }

    template <typename OtherTT>
    Numeric& operator *= (const OtherTT& t)
    {
        *un *= static_cast<NumericType>(t);
        return *this;
    }
    
    template <typename OtherTT>
    Numeric& operator /= (const OtherTT& t)
    {
        if constexpr (std::is_same<NumericType, int>::value)
        {
            if constexpr (std::is_same<decltype(t), const int>::value)
            {
                if ( t == 0)
                {
                    std::cout << "can't divide integers by zero!\n";
                    return *this;
                }
            }
            else if (t < std::numeric_limits<OtherTT>::epsilon() )
            {
                std::cout << "can't divide integers by zero!\n";
                return *this;
            }
        }
        else if (t < std::numeric_limits<OtherTT>::epsilon() )
        {
            std::cout << "warning: trying to divide by zero!\n";
        }
        *un /= static_cast<NumericType>(t);
        return *this;
    }

    
    Numeric& apply( std::function<Numeric&(std::unique_ptr<Type>&)> callable)   
    {
        // std::cout << "std::function<>" << std::endl;
        if( callable )
        {
            return callable(un);  
        }
        return *this;
    }
        
        
    template <typename OtherTT>
    Numeric& pow(const OtherTT& x) 
    {
        if(un != nullptr)
        {
            *un = static_cast<Type>(std::pow(*un, static_cast<const NumericType>(x)) );

        }
        return *this; 
    }

    template <typename Callable>
    Numeric& apply (Callable callableFunc)
    {
        callableFunc(un);
        return *this; 
    }


private:
    std::unique_ptr<Type> un { new Type() };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Numeric)
};

//++++++++++++++ CUBE FUNCTION +++++++++++++++++
template <typename NumericType>
void cube (std::unique_ptr<NumericType>& un)
{
    auto& r = *un;
    r = r * r * r;
}
//_________________________________________________


int main() // NEW MAIN
{
    Numeric<float> f(0.1f);
    Numeric<int> i(3);
    Numeric<double> d(4.2);

    f += 2.f;
    f -= i;
    f *= d;
    f /= 2.f;
    std::cout << "f: " << f << std::endl;
    
    d += 2.f;
    d -= i;
    d *= f;
    d /= 2.f;
    std::cout << "d: " << d << std::endl;
    
    i += 2.f; i -= f; i *= d; i /= 2.f;
    std::cout << "i: "<< i << std::endl;
    
    Point p(f, i);
    p.toString();
    
    d *= -1;
    std::cout << "d: " << d << std::endl;
    
    p.multiply(d.pow(f).pow(i));
    std::cout << "d: " << d << std::endl;
    
    p.toString();
    
    Numeric<float> floatNum(4.3f);
    Numeric<int> intNum(2);
    Numeric<int> intNum2(6);
    intNum = 2 + (intNum2 - 4) + static_cast<double>(floatNum) / 2.3;
    std::cout << "intNum: " << intNum << std::endl;
    
    {
        using Type = decltype(f)::Type;
        f.apply([&f](std::unique_ptr<Type>&value) -> decltype(f)&
                {
                    auto& v = *value;
                    v = v * v;
                    return f;
                });
        std::cout << "f squared: " << f << std::endl;
        
        f.apply( cube<Type> );
        std::cout << "f cubed: " << f << std::endl;
    }
    
    {
        using Type = decltype(d)::Type;
        d.apply([&d](std::unique_ptr<Type>&value) -> decltype(d)&
                {
                    auto& v = *value;
                    v = v * v;
                    return d;
                });
        std::cout << "d squared: " << d << std::endl;
        
        d.apply( cube<Type> );
        std::cout << "d cubed: " << d << std::endl;
    }
    
    {
        using Type = decltype(i)::Type;
        i.apply([&i](std::unique_ptr<Type>&value) -> decltype(i)&
                {
                    auto& v = *value;
                    v = v * v;
                    return i;
                });
        std::cout << "i squared: " << i << std::endl;
        
        i.apply( cube<Type> );
        std::cout << "i cubed: " << i << std::endl;
    }
}
