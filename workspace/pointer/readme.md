The six rules violate the import dry (don't repeat yourself) principle for software development. 
In the end, we have only four rules what makes our life as a software developer a lot easier.   
Here are the rules.

R.32: Take a unique_ptr<widget> parameter to express that a function assumes ownership of a widget  
R.33: Take a unique_ptr<widget>& parameter to express that a function reseats the widget  
R.34: Take a shared_ptr<widget> parameter to express that a function is part owner  
R.35: Take a shared_ptr<widget>& parameter to express that a function might reseat the shared pointer  
R.36: Take a const shared_ptr<widget>& parameter to express that it might retain a reference count to the object ???  
R.37: Do not pass a pointer or reference obtained from an aliased smart pointer  
 Let's start with the first two rules for std::unique_ptr.  

R.32: Take a unique_ptr<widget> parameter to express that a function assumes ownership of a widget  
If a function should take ownership of a Widget, you should take the std::unique_ptr<Widget> by copy. The consequence is that the caller has to move the std::unique_ptr<Widget> to make the code run.  

```c++
#include <memory>
#include <utility>

struct Widget{
    Widget(int){}
};

void sink(std::unique_ptr<Widget> uniqPtr){
    // do something with uniqPtr
}

int main(){
    auto uniqPtr = std::make_unique<Widget>(1998);
    
    sink(std::move(uniqPtr));      // (1)
    sink(uniqPtr);                 // (2) ERROR
}
```

The call (1) is fine but call (2) breaks because you can not copy a std::unique_ptr.  
If your function only wants to use the Widget, it should take its parameter by the pointer or by reference.  
The difference between a pointer and a reference is that a pointer can be a null pointer.  

```c++
void useWidget(Widget* wid); 
void useWidget(Widget& wid);
R.33: Take a unique_ptr<widget>& parameter to express that a function reseats the widget
Sometimes a function want's to reseat a Widget. In this use case, you should pass the std::unique_ptr<Widget> by a non-const reference.

#include <memory>
#include <utility>

struct Widget{
    Widget(int){}
};

void reseat(std::unique_ptr<Widget>& uniqPtr){
    uniqPtr.reset(new Widget(2003));   // (0)
    // do something with uniqPtr
}

int main(){
    auto uniqPtr = std::make_unique<Widget>(1998);
    
    reseat(std::move(uniqPtr));       // (1) ERROR
    reseat(uniqPtr);                  // (2) 
}
 
```

Now, the call (1) fails because you can not bind an rvalue to a non-const lvalue reference.  
This will not hold for the copy in (2). A lvalue can be bound to an lvalue reference.  
By the way. The call (0) will not only construct a new Widget(2003), it will also destruct the old Widget(1998).

The next three rules to std::shared_ptr are literally repetitions; therefore, I will make one out of them.

R.34: Take a shared_ptr<widget> parameter to express that a function is part owner, R.35: Take a shared_ptr<widget>& parameter to express that a function might reseat the shared pointer, 
and R.36: Take a const shared_ptr<widget>& parameter to express that it might retain a reference count to the object ???  
Here are the three function signatures, we have to deal with.  

```c++
void share(std::shared_ptr<Widget> shaWid);  
void reseat(std::shard_ptr<Widget>& shadWid);  
void mayShare(const std::shared_ptr<Widget>& shaWid);
```

Let's look at each function signature in isolation. What does this mean from the function perspective?

void share(std::shared_ptr<Widget> shaWid): I'm for the lifetime of the function body a shared owner of the Widget. At the beginning of the function body, I will increase the reference counter; at the end of the function, I will decrease the reference counter; therefore, the Widget will stay alive, as long as I use it.
void reseat(std::shared_ptr<Widget>& shaWid): I'm not a shared owner of the Widget, because I will not change the reference counter. I have not guaranteed that the Widget will stay alive during the execution of my function, but I can reseat the resource. A non-const lvalue reference is more like: I borrow the resource and can reseat it. 
void mayShare(const std::shared_ptr<Widget>& shaWid): I only borrow the resource. Either can I extend the lifetime of the resource nor can I reseat the resource? To be honest, you should use a pointer (Widget*) or a reference (Widget&) as a parameter instead, because there is no added value in using a std::shared_ptr.
R.37: Do not pass a pointer or reference obtained from an aliased smart pointer
Let me present you with a short code snippet to make the rule clear.

```c++
void oldFunc(Widget* wid){
  // do something with wid
}

void shared(std::shared_ptr<Widget>& shaPtr){       // (2)
    
   oldFunc(*shaPtr);                                // (3)
   
   // do something with shaPtr
     
 }

auto globShared = std::make_shared<Widget>(2011);   // (1)


...

shared(globShared);                                 
```

globShared (1) is a globally shared pointer. The function shared takes its argument per reference (2).  
Therefore, the reference counter of shaPtr will not be increased and the function share will not extend the lifetime of Widget(2011). 
The issue begins with (3). oldFunc accepts a pointer to the Widget; therefore, oldFunc has no guarantee that the Widget will stay alive during its execution. 
oldFunc only borrows the Widget.

The cure is quite simple. You have to ensure that the reference count of globShared will be increased before the call to the function oldFunc.  
This means you have to make a copy of the std::shared_ptr:

Pass the std::shared_ptr by copy to the function shared:
 ```c++
 void shared(std::shared_ptr<Widget> shaPtr){
 
   oldFunc(*shaPtr);
   
   // do something with shaPtr
     
 } 
Make a copy of the shaPtr in the function shared:
 void shared(std::shared_ptr<Widget>& shaPtr){
   
   auto keepAlive = shaPtr;   
   oldFunc(*shaPtr);
   
   // do something with keepAlive or shaPtr
     
 } 
```
The same reasoning also applies to std::unique_ptr but I have no simple cure in mind because you can not copy a std::unique_ptr. 
I suggest you should clone your std::unique_ptr and, therefore, make a new std::unique_ptr.

What's next?
This was the last of my four posts about resource management in the C++ core guidelines.  
The C++ core guidelines have more than 50 rules for expressions and statements. 
I will have a closer look at my next post.

 

 

 

Thanks a lot to my Patreon Supporters: Matt Braun, Roman Postanciuc, Tobias Zindl, Marko, G Prvulovic, Reinhold Dröge, Abernitzke, Frank Grimm, Sakib, Broeserl, António Pina, Sergey Agafyin, Андрей Бурмистров, Jake, GS, Lawton Shoemake, Animus24, Jozo Leko, John Breland, Louis St-Amour, Venkat Nandam, Jose Francisco, Douglas Tinkham, Kuchlong Kuchlong, Robert Blanch, Truels Wissneth, Kris Kafka, Mario Luoni, Neil Wang, Friedrich Huber, lennonli, Pramod Tikare Muralidhara, Peter Ware, Daniel Hufschläger, Alessandro Pezzato, Evangelos Denaxas, Bob Perry, Satish Vangipuram, Andi Ireland, Richard Ohnemus, Michael Dunsky, Leo Goodstadt, John Wiederhirn, Yacob Cohen-Arazi, Florian Tischler, Robin Furness, Michael Young, Holger Detering, Bernd Mühlhaus, Matthieu Bolt, Stephen Kelley, Kyle Dean, Tusar Palauri, Dmitry Farberov, Ralf Holly, Juan Dent, George Liao, Daniel Ceperley, and Jon T Hess.

 

Thanks in particular to Jon Hess, Lakshman, Christian Wittenhorst, Sherhy Pyton, Dendi Suhubdy, Sudhakar Belagurusamy, Richard Sargeant, Rusty Fleming, Ralf Abramowitsch, John Nebel, and Mipko.

# reference 

[unique](https://www.modernescpp.com/index.php/c-core-guidelines-passing-smart-pointer)