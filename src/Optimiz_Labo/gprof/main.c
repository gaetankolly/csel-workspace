void func1(void)
{
    for(int i=0; i<0xfffffff; i++); // wait...
    return;
}

void func2(void)
{
    for(int i=0;i<0xfffffff;i++); // wait...
    func1();
    return;
}

int main(void)
{
    for(int i=0;i<0xfffffff;i++);
    func1();
    func2();

    return 0;
}
