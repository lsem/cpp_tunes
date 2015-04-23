#include <iostream>
#include <unordered_map>

using namespace std;


// Instead of Cpp's RTTI we use own, portable and ugly solution
enum class DependencyInjectionEntities
{
    Foo,
    Bar
};

template <class TInterface>
struct InterfaceTypeIDResolver
{
    static constexpr DependencyInjectionEntities ResolveTypeID();
};

#define REGISTER_DEPENDENCY_ENTITY(TInterface, TInterfaceID) \
    template <> struct InterfaceTypeIDResolver<TInterface> { \
        static constexpr DependencyInjectionEntities ResolveTypeID() { return TInterfaceID; } \
    }

// Container class
class DependencyInjectionContainer
{
public:
    template <class TInterface>
    void RegisterInstance(TInterface *instance)
    {
        const auto id = InterfaceTypeIDResolver<TInterface>::ResolveTypeID();

        if (m_container.find(id) == m_container.end())
        {
            m_container.insert(std::make_pair(id, EntryHolder(instance, [instance]() {
                delete static_cast<TInterface *>(instance);
            })));
        }
        else
        {
            assert(false);
        }
    }

    template <class TInterface>
    TInterface &QueryInstance()
    {
        const auto id = InterfaceTypeIDResolver<TInterface>::ResolveTypeID();
        auto findIt = m_container.find(id);

        if (findIt != m_container.end())
            return *static_cast<TInterface*>(findIt->second.entryInstance);
        else
            assert(false);
    }

    void Free()
    {
        for (auto &kv : m_container)
            kv.second.entryDestructor();

        m_container.clear();
    }

private:
    struct EntryHolder
    {
        typedef std::function<void()> destructor_fn_t;

        EntryHolder(void *instance, destructor_fn_t destructor):
                entryInstance(instance),
                entryDestructor(destructor)
        {}

        void *entryInstance;
        destructor_fn_t entryDestructor;
    };

    unordered_map<DependencyInjectionEntities, EntryHolder> m_container;
};

// Demonstation types
class IFoo
{
public:
    virtual ~IFoo() {}

    virtual void do_smth() = 0;
};

class IBar
{
public:
    virtual ~IBar() {}

    virtual void do_smth() = 0;
};

class Foo : public IFoo
{
public:
    ~Foo() { std::cout << "~Foo(); " << std::endl; }

    virtual void do_smth() override {
        std::cout << "Foo::do_smth(); " << std::endl;
    }
};
REGISTER_DEPENDENCY_ENTITY(IFoo, DependencyInjectionEntities::Foo);


class Bar : public IBar
{
public:
    ~Bar() { std::cout << "~Bar(); " << std::endl; }

    virtual void do_smth() override {
        std::cout << "Bar::do_smth(); " << std::endl;
    }
};
REGISTER_DEPENDENCY_ENTITY(IBar, DependencyInjectionEntities::Bar);



int main()
{
    DependencyInjectionContainer container;

    container.RegisterInstance<IFoo>(new Foo());
    container.RegisterInstance<IBar>(new Bar());

    auto &fooInstance = container.QueryInstance<IFoo>();
    fooInstance.do_smth();

    auto &barInstance = container.QueryInstance<IBar>();
    barInstance.do_smth();

    container.Free();

    return 0;
}


