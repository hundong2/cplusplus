#include <memory>
#include <utility>

struct Widget
{
	Widget(int){}
};
	
void sink(std::unique_ptr<Widget> uniqPtr)
{
	// do something with unique_ptr
}


int main()
{
	auto unique_ptr = std::make_unique<Widget>(1998);
	
	sink(std::move(unique_ptr));
	sink(unique_ptr);
}