#include <iostream>
#include <type_traits>
#include <string>

namespace type_list_impl {

// ==============================
// Main TypeList template
// ==============================
template<typename... Types>
struct TypeList {};

// ==============================
// 1. Get element by index
// ==============================
template<std::size_t Index, typename List>
struct At;

template<typename Head, typename... Tail>
struct At<0, TypeList<Head, Tail...>> {
    using type = Head;
};

template<std::size_t Index, typename Head, typename... Tail>
struct At<Index, TypeList<Head, Tail...>> {
    static_assert(Index < sizeof...(Tail) + 1, "TypeList::At: index out of range");
    using type = typename At<Index - 1, TypeList<Tail...>>::type;
};

template<std::size_t Index, typename List>
using At_t = typename At<Index, List>::type;

// ==============================
// 2. Get size of list
// ==============================
template<typename List>
struct Size;

template<typename... Types>
struct Size<TypeList<Types...>> {
    static constexpr std::size_t value = sizeof...(Types);
};

template<typename List>
inline constexpr std::size_t Size_v = Size<List>::value;

// ==============================
// 3. Check if type exists in list
// ==============================
template<typename T, typename List>
struct Contains;

template<typename T>
struct Contains<T, TypeList<>> {
    static constexpr bool value = false;
};

template<typename T, typename Head, typename... Tail>
struct Contains<T, TypeList<Head, Tail...>> {
    static constexpr bool value = std::is_same_v<T, Head> || Contains<T, TypeList<Tail...>>::value;
};

template<typename T, typename List>
inline constexpr bool Contains_v = Contains<T, List>::value;

// ==============================
// 4. Get index of type in list
// ==============================
template<typename T, typename List>
struct IndexOfHelper;

template<typename T, typename Head, typename... Tail>
struct IndexOfHelper<T, TypeList<Head, Tail...>> {
    static constexpr std::size_t value =
        std::is_same_v<T, Head> ? 0 : 1 + IndexOfHelper<T, TypeList<Tail...>>::value;
};

template<typename T>
struct IndexOfHelper<T, TypeList<>> {
    static constexpr std::size_t value = 0;
};

template<typename T, typename List>
struct IndexOf {
    static_assert(Contains_v<T, List>, "TypeList::IndexOf: type not found in list");
    static constexpr std::size_t value = IndexOfHelper<T, List>::value;
};

template<typename T, typename List>
inline constexpr std::size_t IndexOf_v = IndexOf<T, List>::value;

// ==============================
// 5. Add type to the end of list
// ==============================
template<typename NewType, typename List>
struct Append;

template<typename NewType, typename... Types>
struct Append<NewType, TypeList<Types...>> {
    using type = TypeList<Types..., NewType>;
};

template<typename NewType, typename List>
using Append_t = typename Append<NewType, List>::type;

// ==============================
// 6. Add type to the beginning of list
// ==============================
template<typename NewType, typename List>
struct Prepend;

template<typename NewType, typename... Types>
struct Prepend<NewType, TypeList<Types...>> {
    using type = TypeList<NewType, Types...>;
};

template<typename NewType, typename List>
using Prepend_t = typename Prepend<NewType, List>::type;

// ==============================
// Helper function to print type names
// ==============================
template<typename T>
std::string type_name() {
    if constexpr (std::is_same_v<T, int>) return "int";
    else if constexpr (std::is_same_v<T, double>) return "double";
    else if constexpr (std::is_same_v<T, char>) return "char";
    else if constexpr (std::is_same_v<T, std::string>) return "string";
    else if constexpr (std::is_same_v<T, float>) return "float";
    else if constexpr (std::is_same_v<T, void>) return "void";
    else if constexpr (std::is_same_v<T, bool>) return "bool";
    else if constexpr (std::is_same_v<T, long>) return "long";
    else return "unknown";
}

template<typename... Types>
void print_list(TypeList<Types...>) {
    std::cout << "TypeList<";
    size_t count = 0;
    ((std::cout << type_name<Types>() << (++count < sizeof...(Types) ? ", " : "")), ...);
    std::cout << "> (size = " << sizeof...(Types) << ")\n";
}

} // namespace type_list_impl

using namespace type_list_impl;

// ==============================
// Main function with demonstrations
// ==============================
int main() {
    std::cout << "========== TypeList Demonstration ==========\n\n";

    // Original list
    using MyList = TypeList<int, double, char, std::string>;
    std::cout << "Original list: ";
    print_list(MyList{});

    // 1. Get size
    std::cout << "\n--- 1. List Size ---\n";
    std::cout << "Size: " << Size_v<MyList> << "\n";
    static_assert(Size_v<MyList> == 4);

    // 2. Get element by index
    std::cout << "\n--- 2. Get Element by Index ---\n";
    using Element0 = At_t<0, MyList>;
    using Element1 = At_t<1, MyList>;
    using Element2 = At_t<2, MyList>;
    using Element3 = At_t<3, MyList>;

    std::cout << "Element at index 0: " << type_name<Element0>() << "\n";
    std::cout << "Element at index 1: " << type_name<Element1>() << "\n";
    std::cout << "Element at index 2: " << type_name<Element2>() << "\n";
    std::cout << "Element at index 3: " << type_name<Element3>() << "\n";

    static_assert(std::is_same_v<Element0, int>);
    static_assert(std::is_same_v<Element1, double>);
    static_assert(std::is_same_v<Element2, char>);
    static_assert(std::is_same_v<Element3, std::string>);

    // 3. Check if type exists
    std::cout << "\n--- 3. Check if Type Exists ---\n";
    std::cout << "Contains int? " << std::boolalpha << Contains_v<int, MyList> << "\n";
    std::cout << "Contains float? " << Contains_v<float, MyList> << "\n";
    std::cout << "Contains string? " << Contains_v<std::string, MyList> << "\n";

    static_assert(Contains_v<int, MyList> == true);
    static_assert(Contains_v<float, MyList> == false);
    static_assert(Contains_v<std::string, MyList> == true);

    // 4. Get index of type
    std::cout << "\n--- 4. Get Index of Type ---\n";
    std::cout << "Index of int: " << IndexOf_v<int, MyList> << "\n";
    std::cout << "Index of char: " << IndexOf_v<char, MyList> << "\n";
    std::cout << "Index of string: " << IndexOf_v<std::string, MyList> << "\n";

    static_assert(IndexOf_v<int, MyList> == 0);
    static_assert(IndexOf_v<char, MyList> == 2);
    static_assert(IndexOf_v<std::string, MyList> == 3);

    // 5. Add type to the end
    std::cout << "\n--- 5. Add Type to the End ---\n";
    using ListWithFloat = Append_t<float, MyList>;
    std::cout << "After adding float to the end: ";
    print_list(ListWithFloat{});
    static_assert(Size_v<ListWithFloat> == 5);
    static_assert(std::is_same_v<At_t<4, ListWithFloat>, float>);

    // 6. Add type to the beginning
    std::cout << "\n--- 6. Add Type to the Beginning ---\n";
    using ListWithBool = Prepend_t<bool, MyList>;
    std::cout << "After adding bool to the beginning: ";
    print_list(ListWithBool{});
    static_assert(Size_v<ListWithBool> == 5);
    static_assert(std::is_same_v<At_t<0, ListWithBool>, bool>);
    static_assert(std::is_same_v<At_t<1, ListWithBool>, int>);

    // 7. Combined operations
    std::cout << "\n--- 7. Combined Operations ---\n";
    using Combined = Prepend_t<float, Append_t<double, TypeList<int, char>>>;
    std::cout << "Combined list: ";
    print_list(Combined{});
    static_assert(std::is_same_v<Combined, TypeList<float, int, char, double>>);

    // 8. Working with empty list
    std::cout << "\n--- 8. Working with Empty List ---\n";
    using Empty = TypeList<>;
    std::cout << "Empty list: ";
    print_list(Empty{});
    std::cout << "Size of empty list: " << Size_v<Empty> << "\n";

    using NonEmpty = Prepend_t<int, Append_t<double, Empty>>;
    std::cout << "After adding types: ";
    print_list(NonEmpty{});

    // 9. Error handling (commented out)
    std::cout << "\n--- 9. Error Handling ---\n";
    std::cout << "The following operations would cause compilation errors:\n";
    std::cout << "  - At_t<10, MyList> // index out of range\n";
    std::cout << "  - IndexOf_v<float, MyList> // type not found\n";

    std::cout << "\n========== All Tests Passed Successfully! ==========\n";

    return 0;
}
