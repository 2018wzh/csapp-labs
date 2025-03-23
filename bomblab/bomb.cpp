#include <iostream>
#include <string>
using namespace std;

string s1 = "I am the mayor. I can do anything I want.";
bool phase_1()
{
    string s;
    cin >> s;
    if (s == s1)
        return 1;
    return 0;
}

bool phase_2()
{
    int a[6];
    for (int i = 0; i < 6; i++)
        cin >> a[i];
    if (a[0] < 0)
        return 1;
    for (int i = 1; i < 6; i++)
        if (a[i] - a[i - 1] != i)
            return 1;
    return 0;
}

bool phase_3()
{
    int a, b;
    cin >> a >> b;
    switch (a)
    {
    case 0:
        if (b != 531)
            return 1;
        break;
    case 1:
        if (b != 289)
            return 1;
        break;
    case 2:
        if (b != 645)
            return 1;
        break;
    case 3:
        if (b != 629)
            return 1;
        break;
    case 4:
        if (b != 52)
            return 1;
        break;
    case 5:
        if (b != 555)
            return 1;
        break;
    case 6:
        if (b != 201)
            return 1;
        break;
    case 7:
        if (b != 513)
            return 1;
        break;
    default:
        return 1;
    }
    return 0;
}

int func4(int a, int b, int c)
{
    int mid = (b + c) / 2;
    if (mid > a)
        mid += func4(a, b, mid - 1);
    else if (mid < a)
        mid += func4(a, mid + 1, c);
    return mid;
}

bool phase_4()
{
    int a, b;
    cin >> a >> b;
    if (a > 14)
        return 1;
    if (func4(a, 0, 14) != 31)
        return 1;
    if (b != 31)
        return 1;
    return 0;
}

int array_0[16] = {10, 2, 14, 7, 8, 12, 15, 11, 0, 4, 1, 13, 3, 9, 6, 5};
bool phase_5()
{
    int a, b;
    cin >> a >> b;
    a %= 15;
    int t = a;
    if (t == 15)
        return 1;
    int cnt = 0;
    int sum = 0;
    do
    {
        cnt++;
        a = array_0[a];
        sum += a;
    } while (a != 15);
    t = 15;
    if (cnt != 15 || sum != b)
        return 1;
    return 0;
}

struct chainnode
{
    short id;
    unsigned long v;
    chainnode *next;
} table[6]{{1, 0x34a, &table[1]}, {2, 0x15e, &table[2]}, {3, 0x26f, &table[3]}, {4, 0x2fb, &table[4]}, {5, 0x26b, &table[5]}, {6, 0x36b, nullptr}};
bool phase_6()
{
    unsigned long a[6];
    unsigned long ct[6];
    auto *ptr = &a[0];
    for (int i = 0; i < 6; i++)
        cin >> a[i];
    int t = 0;
    for (int i = 1; i < 6; i++)
    {
        if (*ptr > 6)
            return 1;
        for (int j = i + 1; j < 6; j++)
            if (*ptr == a[j])
                return 1;
        ++ptr;
    }
    for (int i = 0; i < 6; i++)
    {
        int x = a[i];
        auto *p = &table[0];
        if (x > 1)
            if (x > 1)
                for (int j = 1; j < x; j++)
                    p = p->next;
        ct[i] = p->v;
    }
    for (int i = 0; i < 5; i++)
        if (ct[i] < ct[i + 1])
            return 1;
    return 0;
}

struct treenode
{
    unsigned long v;
    treenode *lc, *rc;
} tree[15]{
    {0x24, &tree[1], &tree[2]},   // n1
    {0x8, &tree[3], &tree[4]},    // n21
    {0x32, &tree[5], &tree[6]},   // n22
    {0x6, &tree[7], &tree[8]},    // n31
    {0x16, &tree[9], &tree[10]},  // n32
    {0x2D, &tree[11], &tree[12]}, // n33
    {0x6B, &tree[13], &tree[14]}, // n34
    {0x1, nullptr, nullptr},      // n41
    {0x7, nullptr, nullptr},      // n42
    {0x14, nullptr, nullptr},     // n43
    {0x23, nullptr, nullptr},     // n44
    {0x28, nullptr, nullptr},     // n45
    {0x2F, nullptr, nullptr},     // n46
    {0x63, nullptr, nullptr},     // n47
    {0x30E9, nullptr, nullptr}};  // n48
unsigned long fun7(treenode *a, unsigned long b)
{
    if (a == nullptr)
        return 0xFFFFFFFF;
    if (a->v > b)
        return 2 * fun7(a->lc, b);
    if (a->v < b)
        return 2 * fun7(a->rc, b) + 1;
    return 0;
}
bool secret_phase()
{
    string s;
    cin >> s;
    unsigned int v = stoi(s);
    if (v > 1001)
        return 1;
    if (fun7(&tree[0], v) != 1)
        return 1;
    return 0;
}

void explode_bomb()
{
    cout << "BOOM!!!\n";
    exit(1);
}
int main()
{
    if (phase_1())
        explode_bomb();
    cout << "Phase 1 passed\n";
    if (phase_2())
        explode_bomb();
    cout << "Phase 2 passed\n";
    if (phase_3())
        explode_bomb();
    cout << "Phase 3 passed\n";
    if (phase_4())
        explode_bomb();
    cout << "Phase 4 passed\n";
    if (phase_5())
        explode_bomb();
    cout << "Phase 5 passed\n";
    if (phase_6())
        explode_bomb();
    cout << "Phase 6 passed\n";
    if (secret_phase())
        explode_bomb();
    cout << "Secret phase passed\n";
}