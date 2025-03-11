#include <iostream>
using namespace std;

union StateMachine
{
    char character;
    int number;
    char *str;
    StateMachine(char c)
    {
        character = c;
    }
    StateMachine(int n)
    {
        number = n;
    }
    StateMachine(char *s)
    {
        str = s;
    }
};
enum State
{
    character,
    number,
    str
};

int main()
{
    State state = character;
    StateMachine machine('J');
    if (state == character)
        cout << machine.character << endl;
    else if (state == number)
        cout << machine.number << endl;
    else if (state == str)
        cout << machine.str << endl;
    return 0;
}
