#include <type_traits>
#include <cstddef>

namespace typelist {

// Основной шаблон
template<typename... Types>
struct TypeList {};

// ---------- Получение размера ----------
template<typename> struct Size;
template<typename... Types>
struct Size<TypeList<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

template<typename TList>
inline constexpr std::size_t size_v = Size<TList>::value;

// ---------- Получение элемента по индексу ----------
template<std::size_t I, typename TList> struct At;

// Базовый случай: список не пуст, I == 0 -> первый тип
template<typename Head, typename... Tail>
struct At<0, TypeList<Head, Tail...>> {
    using type = Head;
};

// Рекурсия: I > 0
template<std::size_t I, typename Head, typename... Tail>
struct At<I, TypeList<Head, Tail...>> : At<I-1, TypeList<Tail...>> {};

// Ошибка компиляции при выходе за границы (специализация для пустого списка)
template<std::size_t I>
struct At<I, TypeList<>>; // намеренно не определена

template<std::size_t I, typename TList>
using at_t = typename At<I, TList>::type;

// ---------- Проверка наличия типа ----------
template<typename T, typename TList> struct Contains;

template<typename T>
struct Contains<T, TypeList<>> : std::false_type {};

template<typename T, typename Head, typename... Tail>
struct Contains<T, TypeList<Head, Tail...>>
    : std::conditional_t<std::is_same_v<T, Head>,
                         std::true_type,
                         Contains<T, TypeList<Tail...>>> {};

template<typename T, typename TList>
inline constexpr bool contains_v = Contains<T, TList>::value;

// ---------- Получение индекса типа ----------
template<typename T, typename TList> struct IndexOf;

template<typename T>
struct IndexOf<T, TypeList<>>; // отсутствует -> ошибка компиляции

template<typename T, typename Head, typename... Tail>
struct IndexOf<T, TypeList<Head, Tail...>> {
    static constexpr std::size_t value = []() {
        if constexpr (std::is_same_v<T, Head>)
            return 0;
        else
            return 1 + IndexOf<T, TypeList<Tail...>>::value;
    }();
};

template<typename T, typename TList>
inline constexpr std::size_t index_of_v = IndexOf<T, TList>::value;

// ---------- Добавление в конец ----------
template<typename T, typename TList> struct PushBack;

template<typename T, typename... Types>
struct PushBack<T, TypeList<Types...>> {
    using type = TypeList<Types..., T>;
};

template<typename T, typename TList>
using push_back_t = typename PushBack<T, TList>::type;

// ---------- Добавление в начало ----------
template<typename T, typename TList> struct PushFront;

template<typename T, typename... Types>
struct PushFront<T, TypeList<Types...>> {
    using type = TypeList<T, Types...>;
};

template<typename T, typename TList>
using push_front_t = typename PushFront<T, TList>::type;

} // namespace typelist

// ---------- Тесты ----------
using namespace typelist;

// 1. Создаём списки
using Empty = TypeList<>;
using Single = TypeList<int>;
using ABC = TypeList<char, bool, double>;

// 2. Размер
static_assert(size_v<Empty> == 0);
static_assert(size_v<Single> == 1);
static_assert(size_v<ABC> == 3);

// 3. Доступ по индексу
static_assert(std::is_same_v<at_t<0, ABC>, char>);
static_assert(std::is_same_v<at_t<1, ABC>, bool>);
static_assert(std::is_same_v<at_t<2, ABC>, double>);
// Следующая строка выдаст ошибку компиляции (как и требуется):
// static_assert(std::is_same_v<at_t<3, ABC>, void>); // не скомпилируется

// 4. Проверка наличия
static_assert(contains_v<int, Single>);
static_assert(!contains_v<float, Single>);
static_assert(contains_v<char, ABC>);
static_assert(contains_v<bool, ABC>);
static_assert(contains_v<double, ABC>);
static_assert(!contains_v<int, ABC>);

// 5. Индекс типа
static_assert(index_of_v<char, ABC> == 0);
static_assert(index_of_v<bool, ABC> == 1);
static_assert(index_of_v<double, ABC> == 2);
// static_assert(index_of_v<int, ABC> == ?); // ошибка компиляции, как и нужно

// 6. Добавление в конец
using ABCC = push_back_t<float, ABC>; // char, bool, double, float
static_assert(size_v<ABCC> == 4);
static_assert(std::is_same_v<at_t<3, ABCC>, float>);

// 7. Добавление в начало
using IntABC = push_front_t<int, ABC>; // int, char, bool, double
static_assert(size_v<IntABC> == 4);
static_assert(std::is_same_v<at_t<0, IntABC>, int>);
static_assert(std::is_same_v<at_t<1, IntABC>, char>);
static_assert(std::is_same_v<at_t<2, IntABC>, bool>);
static_assert(std::is_same_v<at_t<3, IntABC>, double>);

// Дополнительно: проверка цепочек операций
using Stacked = push_front_t<long, push_back_t<short, Single>>; // long, int, short
static_assert(size_v<Stacked> == 3);
static_assert(std::is_same_v<at_t<0, Stacked>, long>);
static_assert(std::is_same_v<at_t<1, Stacked>, int>);
static_assert(std::is_same_v<at_t<2, Stacked>, short>);

int main() {
    // Всё проверено на этапе компиляции
    return 0;
}
