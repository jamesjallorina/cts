#include "../include/cts/cts.h"
#include <memory>

class Foo
{
public:
	void test();

private:
	int *p1 GUARDED_BY(mu);
	int *p2 PT_GUARDED_BY(mu);
	std::unique_ptr<int> p3 PT_GUARDED_BY(mu);
	cts::mutex mu;
};

void Foo::test()
{
	p1 = 0;

	*p2 = 42;
	p2 = new int;

	*p3 = 42;
	p3.reset(new int);
}


int main()
{
	Foo f;
	f.test();

	return 0;
}