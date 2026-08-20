namespace Foo {
    class Bar {
    public:
        void Baz();
    };
    void Bar::Baz() {
        auto lam = [](int x){};
        lam();
    }
}
